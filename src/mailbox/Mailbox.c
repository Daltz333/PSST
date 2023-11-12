#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/ServerConstants.h"
#include "Mailbox.h"

#define PUBLIC_KEY_MAX 255 /* We can handle up to 255 clients */

/* Represents the index of the next unused client in our client array */
int client_iter = 0;

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort = MAILBOX_SERVER_PORT;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    PublicKeyItem keys[PUBLIC_KEY_MAX];
    PsstMailboxMessage* recMessage;
    
    recMessage = (PsstMailboxMessage*)malloc(sizeof(PsstMailboxMessage));

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        
        printf("Waiting for message from client...\n");

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, recMessage, sizeof(*recMessage), 0,
                                    (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        printf("Sender ID is: %d\n", recMessage->user_id);
        printf("Requested message is: %d\n", recMessage->message_type);
        if (recMessage->message_type == register_key) 
        {
            printf("Client registering public key: %i\n", recMessage->public_key);

            ConfirmLoginMessage* ack;
            ack = (ConfirmLoginMessage*)malloc(sizeof(ConfirmLoginMessage));
            
            int ret = addPublicKey(recMessage->user_id, recMessage->public_key, keys);
            if (ret == 0)
            {
                printf("Successfully registered public key.\n");
                printf("CHECK: %i\n", getPublicKey(recMessage->user_id, keys));
                ack->err = 0;
            } else {
                printf("Failed to register public key with statuscode: %i\n", ret);
                ack->err = ret;
            }
            
            ack->message_type = ack_register_key;
            
            printf("Sending ack_register_key...\n");
            /* Send ACK back to the client */
            if (sendto(sock, ack, sizeof(*ack), 0,
                    (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(*ack))
            {
                DieWithError("sendto() sent a different number of bytes than expected");
    
                free(ack);
            }

            free(ack);
        } else if (recMessage->message_type == login)
        {
            int pub_key = getPublicKey(recMessage->user_id, keys);
        }
    }
}

/* Retrieves a given user's public ID*/
/* Returns -1 if user is not found */
int getPublicKey(unsigned int user_id, PublicKeyItem* keys)
{
    PublicKeyItem key;
    for (int i = 0; i < sizeof(*keys); ++i)
    {
        key = keys[i];
        if (key.user_id == user_id) {
            return key.public_key;
        }
    }

    return -1;
}

/* Adds a given public key to keys */
/* Returns 0 if success, -1 if max clients */
/* -2 if key already exists */
int addPublicKey(unsigned int user_id, unsigned int public_key, PublicKeyItem* keys) 
{
    /* Only add if the user doesn't already exist */
    if (getPublicKey(user_id, keys) == -1) 
    {
        // Size check
        if (sizeof(*keys) == PUBLIC_KEY_MAX) 
        {
            return -1;
        } else {
            keys[client_iter].user_id = user_id;
            keys[client_iter].public_key = public_key;
            
            // Increment client iterator now that we have filled in a key
            client_iter++;

            return 0;
        }
    } else {
        return -2;
    }
}