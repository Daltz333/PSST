typedef struct {
    unsigned int user_id;
    unsigned int public_key;
} PublicKeyItem;

int getPublicKey(unsigned int user_id, PublicKeyItem* keys);
int addPublicKey(unsigned int user_id, unsigned int public_key, PublicKeyItem* keys);
