#include <string.h>
#include <time.h>
#include <math.h>
#include "MessageUtil.h"
#include "MathUtil.h"

/**
 * In a real application, the mailbox server would not know the factors of 221
 * Which is what makes it secure.
*/

const int n = 221;

/* Encrypts a given message and places it in the provided buffer */
/* Returns -1 if the size of msg and buffer do not match */
int encryptMessage(int* buffer, char* msg, size_t bufferSize) {
    if (bufferSize < strlen(msg) + 1)
    { // Include space for the null terminator
        return -1;
    }

    for (size_t i = 0; i < strlen(msg) + 1; ++i)
    {
        int chr = (int)msg[i]; // Cast each char to int representation

        /* Don't encrypt newlines */
        if (chr == 10) 
        {
            chr = 0;
        }

        buffer[i] = chr;
        //printf("Encrypted %c, became %d\n", msg[i], chr); FOR DEBUGGING
    }

    return 0;
}

/** 
 * Returns timestamp in years since epoch
 * We are using years, because our modfun fails on extremely large values
*/
int getTimestamp()
{
    return (int)time(NULL) / (86400*365);
}

/**
 * Encrypts using private key (d)
*/
long long int encrypt(int msg, int prvKey)
{
    return modfun(msg, prvKey, n);
}

/**
 * Decrypts using public key (e)
*/
long long int decrypt(int msg, int pubKey)
{
    return modfun(msg, pubKey, n);
}

/**
 * Generates a private key given e
 * Assumes e is denominator for n
*/
int getPrivKey(int e, int p, int q)
{
    return modInverse(e, ((p-1)*(q-1)));
}
