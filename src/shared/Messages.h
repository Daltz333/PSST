/**
 * Represents a message between Mailbox/Auth MGMT and Auth (SYH) server
 * 
*/
typedef struct {
    enum {register_syh, ack_reg, syh, ack_push_syh, request_auth, neg_ack_push_syh} message_type;

    unsigned int user_id;
    unsigned long timestamp;
    unsigned long digital_sig;
} AuthMessage;

/**
 * Represents a message from Auth Server to MGMT
*/
typedef struct {
    enum {confirm_syh, push_syh} message_type;

    unsigned int user_id;
} PushNotif;

/**
 * Represents a message from Mailbox to Client
*/
typedef struct {
    enum {ack_login, ack_message, ack_retrieve} message_type;

    unsigned int user_id;
    unsigned int count;
} MailboxMessage;

/**
 * Represents a message from Mailbox/PKS to Client
*/
typedef struct {
    enum {ack_register_key, response_public_key} message_type;

    unsigned int user_id;
    unsigned int public_key;
    unsigned int err; /* Error response */
} ConfirmLoginMessage;

typedef struct {
    enum {register_key, request_key, response_auth, login, send_msg, retrieve_msg} message_type;

    unsigned int user_id; /* ID of the sender */
    unsigned int public_key; 
    unsigned int recipient_id; /* ID of the recipient */
    unsigned int timestamp; /* Plaintext timestamp */
    unsigned long digital_dig; /* Timestamp encrypted with the sender's private key */
    unsigned long psst_msg[32]; /* Encrypted message, encrypted with the receiver's public key */
    unsigned int err; /* Error response */
} PsstMailboxMessage;