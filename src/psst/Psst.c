#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdio.h>      /* for console input */
#include <time.h>       /* for system time */
#include <stdlib.h>     /* for string manip */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/MessageUtil.h"
#include "Psst.h"

#define ECHOMAX 255     /* Longest string to echo */

int main(int argc, char *argv[])
{
    int sock;                               /* Socket descriptor */
    struct sockaddr_in echoServAddr;        /* Echo server address */
    unsigned short echoServPort = 8000;            /* Echo server port */
    char *servIP;                           /* IP address of server */
    PsstMailboxMessage *mailboxMessage;     /* Message to send to mailbox server */

    unsigned int receiver_id;
    unsigned int user_id;
    unsigned int public_key;
    unsigned int private_key;

    mailboxMessage = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP>\n", argv[0]);
        exit(1);
    }

    printf("num args %d\n", argc);
    
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */

    char pub_key[100];
    printf("\nPlease enter a public key (e) for n = 221 (e.g. 23): ");
    if (fgets(pub_key, sizeof(pub_key), stdin) == NULL) 
    {
        DieWithError("Invalid public key entered.");
    }

    sscanf(pub_key, "%u", &public_key);

    /* Generate private key given public key (e) */
    printf("Generating private key...\n");

    // all clients share the same factor, so not very secure
    // but this is fine for this project
    private_key = getPrivKey(public_key, 13, 17);

    printf("Generated private key: %i\n", private_key);

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    char senderId[100];
    printf("\nPlease enter your ID: ");
    if (fgets(senderId, sizeof(senderId), stdin) == NULL) 
    {
        DieWithError("Invalid sender ID entered.");
    }

    sscanf(senderId, "%u", &user_id);
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

    printf("Registering public key...\n");
    int ret = registerAuth(user_id, public_key, sock, echoServAddr);
    if (ret == 0)
    {
        printf("Successfully registered public key.\n");
    } else 
    {
        DieWithError("Failed to register public key");
    }

    printf("Logging in...\n");
    login_user(user_id, private_key, sock, echoServAddr);

    for (;;) 
    {
        char receiverId[100];
        printf("\nWho would you like to send to (ID)? ");
        if (fgets(receiverId, sizeof(receiverId), stdin) == NULL) 
        {
            DieWithError("Invalid receiver ID entered.");
        }

        sscanf(receiverId, "%u", &receiver_id);

        char message[100];
        printf("\nPlease enter a message to send securely: ");
        if (fgets(message, sizeof(message), stdin) == NULL) 
        {
            DieWithError("Invalid message entered.");
        }
        
        printf("Encrypting message...\n");
        int buffer[sizeof(message)];
        ret = encryptMessage(buffer, message, sizeof(buffer) / sizeof(buffer[0]));

        if (ret == 0) {
            printf("Successfully encrypted message\n");
            printf("Encrypted message: %s\n", message);
        } else {
            printf("Failed to encrypt message due to buffer mismatch!\n");
        }
    }

    free(mailboxMessage);
    close(sock);
    exit(0);
}

/* Sends and waits for a successful login to the mailbox server */
/* Returns an int that represents an error code */
int login_user(unsigned int user_id, unsigned int private_key, int sock, struct sockaddr_in echoServAddr)
{
    PsstMailboxMessage* registerKeyMessage;
    registerKeyMessage = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));

    registerKeyMessage->message_type = login;
    registerKeyMessage->user_id = user_id;
    registerKeyMessage->public_key = 0;
    
    int timestamp = getTimestamp();
    registerKeyMessage->timestamp = timestamp;
    registerKeyMessage->digital_dig = encrypt(timestamp, private_key);
    
    printf("Sending login request to mailbox...\n");
    
    /* Send the request to the server */
    int sentBytes = sendto(sock, registerKeyMessage, sizeof(*registerKeyMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
    if (sentBytes != sizeof(*registerKeyMessage))
    {
        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(*registerKeyMessage), sentBytes);
        return -1;
    }

    free(registerKeyMessage);

    ConfirmLoginMessage* ack;
    ack = (ConfirmLoginMessage*)malloc(sizeof(ConfirmLoginMessage));

    unsigned int recLen = sizeof(echoServAddr);

    printf("Waiting for mailbox server to confirm successful login...\n");
    /* Wait for ACK */
    if(recvfrom(sock, ack, sizeof(*ack), 0,
         (struct sockaddr*)&echoServAddr, &recLen) < 0){
        DieWithError("Error while receiving server's msg");
    }
    
    printf("Received statuscode from server: %i\n", ack->message_type);
    
    int ret = -1;
    if (ack->message_type == ack_register_key) {
        ret = ack->err;
    } else {
        ret = ack->message_type;
    }

    free(ack);
    return ret;
}

/**
 * Registers a user on the public key (mailbox) server
 * Returns an int where 0 represents success
*/
int registerAuth(unsigned int user_id, unsigned int public_key, int sock, struct sockaddr_in echoServAddr)
{
    PsstMailboxMessage* registerKeyMessage;
    registerKeyMessage = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));

    registerKeyMessage->message_type = register_key;
    registerKeyMessage->user_id = user_id;
    registerKeyMessage->public_key = public_key;

    /* Send the string to the server */
    int sentBytes = sendto(sock, registerKeyMessage, sizeof(*registerKeyMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
    if (sentBytes != sizeof(*registerKeyMessage))
    {
        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(*registerKeyMessage), sentBytes);
        return -1;
    }

    free(registerKeyMessage);

    ConfirmLoginMessage* ack;
    ack = (ConfirmLoginMessage*)malloc(sizeof(ConfirmLoginMessage));

    unsigned int recLen = sizeof(echoServAddr);
    /* Wait for ACK */
    if(recvfrom(sock, ack, sizeof(*ack), 0,
         (struct sockaddr*)&echoServAddr, &recLen) < 0){
        DieWithError("Error while receiving server's msg");
    }
    
    printf("Received statuscode from server: %i\n", ack->message_type);
    
    int ret = -1;
    if (ack->message_type == ack_register_key) {
        ret = ack->err;
    } else {
        ret = ack->message_type;
    }

    free(ack);
    return ret;
}