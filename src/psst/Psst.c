#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdio.h>      /* for console input */
#include <time.h>       /* for system time */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/MessageUtil.h"

#define ECHOMAX 255     /* Longest string to echo */

int main(int argc, char *argv[])
{
    int sock;                               /* Socket descriptor */
    struct sockaddr_in echoServAddr;        /* Echo server address */
    struct sockaddr_in fromAddr;            /* Source address of echo */
    unsigned short echoServPort = 8000;            /* Echo server port */
    unsigned int fromSize;                  /* In-out of address size for recvfrom() */
    char *servIP;                           /* IP address of server */
    struct PsstMailboxMessage *mailboxMessage;     /* Message to send to mailbox server */
    char echoBuffer[ECHOMAX+1];             /* Buffer for receiving echoed string */
    int echoStringLen;                      /* Length of string to echo */
    int respStringLen;                      /* Length of received response */
    
    mailboxMessage = malloc(sizeof(mailboxMessage));

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP>\n", argv[0]);
        exit(1);
    }

    printf("num args %d\n", argc);
    
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */

    char prv_key[100];
    printf("\nPlease enter a private key: ");
    if (fgets(prv_key, sizeof(prv_key), stdin) == NULL) 
    {
        DieWithError("Invalid private key entered.");
    }

    char pub_key[100];
    printf("\nPlease enter a public key: ");
    if (fgets(pub_key, sizeof(pub_key), stdin) == NULL) 
    {
        DieWithError("Invalid public key entered.");
    }

    char senderId[100];
    printf("\nPlease enter your ID: ");
    if (fgets(senderId, sizeof(senderId), stdin) == NULL) 
    {
        DieWithError("Invalid sender ID entered.");
    }

    /* TODO, authenticate user ID */
    printf("Authenticating...\n");

    char receiverId[100];
    printf("\nWho would you like to send to (ID)? ");
    if (fgets(receiverId, sizeof(receiverId), stdin) == NULL) 
    {
        DieWithError("Invalid receiver ID entered.");
    }

    char message[100];
    printf("\nPlease enter a message to send securely: ");
    if (fgets(message, sizeof(message), stdin) == NULL) 
    {
        DieWithError("Invalid message entered.");
    }
    
    printf("Initializing message buffer...\n");
    int buffer[sizeof(message)];

    int ret = encryptMessage(buffer, message, sizeof(buffer) / sizeof(buffer[0]));

    if (ret == 0) {
        printf("Successfully encrypted message\n");
        printf("Encrypted message: %s\n", message);
    } else {
        printf("Failed to encrypt message due to buffer mismatch!\n");
    }

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */
    
    /* Send the string to the server */
    int sentBytes = sendto(sock, mailboxMessage, sizeof(mailboxMessage), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
    if (sentBytes != sizeof(mailboxMessage))
    {
        fprintf(stderr, "Expected %d but sent %d\n", (int)sizeof(mailboxMessage), sentBytes);
        DieWithError("sendto() sent a different number of bytes than expected\n");
    }
    
    /*
    fromSize = sizeof(fromAddr);
    if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
         (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
        DieWithError("recvfrom() failed");
    */

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
    
    /*
    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);
    */
    
    close(sock);
    exit(0);
}