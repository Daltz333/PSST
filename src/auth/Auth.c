#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/ServerConstants.h"
#include "../shared/MessageUtil.h"

#define ECHOMAX 255     /* Longest string to echo */

int main(int argc, char *argv[])
{
    int sock;                               /* Socket descriptor */
    struct sockaddr_in echoServAddr;        /* Echo server address */
    unsigned short echoServPort = AUTH_SERVER_PORT;            /* Echo server port */
    char *servIP;                           /* IP address of server */
    AuthMessage *authMessage;     /* Message to send to mailbox server */

    unsigned int private_key;

    authMessage = (AuthMessage*)malloc(sizeof(AuthMessage));

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP>\n", argv[0]);
        exit(1);
    }

    printf("num args %d\n", argc);
    
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */

    char privKeyChar[100];
    printf("\nPlease enter your private key: ");
    if (fgets(privKeyChar, sizeof(privKeyChar), stdin) == NULL) 
    {
        DieWithError("Invalid private key entered.");
    }
    sscanf(privKeyChar, "%u", &private_key);

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */
    
    printf("Registering client...\n");
    int timestamp = getTimestamp();

    authMessage->user_id = 0; // user id does not matter, we are authenticating with priv key
    authMessage->digital_sig = encrypt(timestamp, private_key);
    authMessage->timestamp = timestamp;
    authMessage->message_type = register_syh;

    printf("Sending client attached to syh server...\n");
    /* Send the message to the server */
    int sentBytes = sendto(sock, authMessage, sizeof(*authMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
    if (sentBytes != sizeof(*authMessage))
    {
        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(*authMessage), sentBytes);
        return -1;
    }
    
    ConfirmLoginMessage* ack;
    ack = (ConfirmLoginMessage*)malloc(sizeof(ConfirmLoginMessage));

    unsigned int recLen = sizeof(echoServAddr);
    /* Wait for ACK */
    if(recvfrom(sock, ack, sizeof(*ack), 0,
         (struct sockaddr*)&echoServAddr, &recLen) < 0){
        DieWithError("Error while receiving server's msg");
    }

    printf("Received confirmation from syh server.\n");
    if (ack->err == 0)
    {
        PushNotif* pushMsg;
        pushMsg = (PushNotif*)malloc(sizeof(PushNotif));

        /* Wait for any push notifications */
        for(;;)
        {
            /* Wait for push notifications */
            if(recvfrom(sock, pushMsg, sizeof(*pushMsg), 0,
                (struct sockaddr*)&echoServAddr, &recLen) < 0){
                DieWithError("Error while receiving server's msg");
            }
            printf("Received new login request\n");

            char cnfPush[100];
            for(;;)
            {
                printf("\nPlease confirm login (y/n): ");
                if (fgets(cnfPush, sizeof(cnfPush), stdin) == NULL) 
                {
                    DieWithError("Invalid private key entered.");
                }

                if (cnfPush[0] == 'y')
                {
                    authMessage->message_type = ack_push_syh;

                    int sentBytes = sendto(sock, authMessage, sizeof(*authMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
                    if (sentBytes != sizeof(*authMessage))
                    {
                        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(*authMessage), sentBytes);
                        return -1;
                    }

                    printf("Sent login success to syh server.\n");
                    break;
                } else if (cnfPush[0] == 'n')
                {
                    authMessage->message_type = neg_ack_push_syh;

                    int sentBytes = sendto(sock, authMessage, sizeof(*authMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
                    if (sentBytes != sizeof(*authMessage))
                    {
                        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(*authMessage), sentBytes);
                        return -1;
                    }
                    
                    printf("Sent login forbidden to syh server.\n");
                    break;
                }
            }
        }

        free(pushMsg);
    } else
    {
        DieWithError("Client registration failed with a non-zero error code from SYH server.");
    }

    free(ack);
    free(authMessage);
    close(sock);
    exit(0);
}