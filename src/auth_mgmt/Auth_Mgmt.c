#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/ServerConstants.h"
#include "../shared/MessageUtil.h"

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address, this gets overwritten for every client */
    struct sockaddr_in authClntAddr; /* Auth Client Address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort = AUTH_SERVER_PORT;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    char *servIP;
    AuthMessage* authMessage;

    struct sockaddr_in mailboxServAddr;
    unsigned short mailboxServPort = MAILBOX_SERVER_PORT;

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Mailbox Server IP>\n", argv[0]);
        exit(1);
    }
    
    printf("num args %d\n", argc);
    
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */

    authMessage = (AuthMessage*)malloc(sizeof(AuthMessage));

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */
    
    /* Construct the mailbox address structure */
    memset(&mailboxServAddr, 0, sizeof(mailboxServAddr));    /* Zero out structure */
    mailboxServAddr.sin_family = AF_INET;                 /* Internet addr family */
    mailboxServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    mailboxServAddr.sin_port   = htons(mailboxServPort);     /* Server port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    int hasClientConnected = 0;

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        
        printf("Waiting for message from client...\n");

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, authMessage, sizeof(*authMessage), 0,
                                    (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        printf("Sender ID is: %d\n", authMessage->user_id);
        printf("Requested message is: %d\n", authMessage->message_type);
        /* */
        if (authMessage->message_type == register_syh) 
        {
            printf("New client has been attached.\n");
            
            /* Auth client attached, store */
            authClntAddr = echoClntAddr;
            hasClientConnected = 1;

            printf("Sending positive ACK\n");
            ConfirmLoginMessage* ackRegister;
            ackRegister = (ConfirmLoginMessage*)malloc(sizeof(ConfirmLoginMessage));

            ackRegister->err = 0;
            ackRegister->message_type = ack_register_key;

            printf("Sending login ack to auth client...\n");
            /* Send ack response */
            if (sendto(sock, ackRegister, sizeof(*ackRegister), 0,
                (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(*ackRegister))
            {
                printf("Failed to send request_key to mailbox.");
            }
            printf("Successfully sent login ack to auth client...\n");

            free(ackRegister);
        /* Send push notification to client */
        } else if (authMessage->message_type == request_auth)
        {
            PushNotif* pushMsg;
            pushMsg = (PushNotif*)malloc(sizeof(PushNotif));
            pushMsg->user_id = authMessage->user_id;
            
            if (hasClientConnected == 1)
            {
                printf("Sending push notification to client...\n");

                /* send push_syh to client, telling them mailbox has requested authentication */
                if (sendto(sock, pushMsg, sizeof(*pushMsg), 0,
                        (struct sockaddr *) &authClntAddr, sizeof(authClntAddr)) != sizeof(*pushMsg))
                {
                    printf("Failed to send push notification to client.\n");
                }
            } else 
            {
                printf("No client detected, sending negative ACK\n");

                /* Construct message telling mailbox server there is no client connected */
                PsstMailboxMessage* mailboxMsg;
                mailboxMsg = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));
                mailboxMsg->message_type = response_auth;
                mailboxMsg->err = -1;

                if (sendto(sock, mailboxMsg, sizeof(*mailboxMsg), 0,
                    (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(*mailboxMsg))
                {
                    printf("Failed to send BAD ACK to mailbox.");
                }

                free(mailboxMsg);
            }

            free(pushMsg);
        } else if (authMessage->message_type == ack_push_syh)
        {
            printf("Successfully received ACK from syh client, sending auth success to mailbox...\n");
            
            /* Construct message telling mailbox server there is no client connected */
            PsstMailboxMessage* mailboxMsg;
            mailboxMsg = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));
            mailboxMsg->message_type = response_auth;
            mailboxMsg->err = 0;

            if (sendto(sock, mailboxMsg, sizeof(*mailboxMsg), 0,
                (struct sockaddr *) &mailboxServAddr, sizeof(mailboxServAddr)) != sizeof(*mailboxMsg))
            {
                printf("Failed to send OK ACK to mailbox.");
            }

            free(mailboxMsg);
        }
    }

}