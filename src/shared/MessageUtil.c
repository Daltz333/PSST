#include <string.h>
#include "MessageUtil.h"

/* Encrypts a given message and places it in the provided buffer */
/* Returns -1 if the size of msg and buffer do not match */
int encryptMessage(int* buffer, char* msg, size_t bufferSize) {
    if (bufferSize < strlen(msg) + 1) { // Include space for the null terminator
        return -1;
    }

    for (size_t i = 0; i < strlen(msg) + 1; ++i) {
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