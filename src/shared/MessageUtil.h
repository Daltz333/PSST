#include <stddef.h>

int encryptMessage(int* buffer, char* msg, size_t bufferSize);
int decryptMessage(char* decryptedMsg, int* encryptedMsg);
int getTimestamp();
long long int encrypt(int msg, int prvKey);
long long int decrypt(int msg, int pubKey);
int getPrivKey(int e, int p, int q);
