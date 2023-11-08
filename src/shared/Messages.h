/**
 * Represents a message between Mailbox and Auth (SYH) server
 * 
*/
typedef struct {
    enum {register_syh, ack_reg, syh, ack_push_syh, request_auth} message_type;

    unsigned int user_id;
    unsigned long timestamp;
    unsigned long digital_sig;
} AuthMessage;