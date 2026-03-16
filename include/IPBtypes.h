#ifndef IPBTYPES_H
#define IPBTYPES_H

// This header is used by all libraries of the project to standardize error naming, types and contains all definitions that the user should be aware of.

// api limits
#define MAX_MESSAGE_LENGTH		200
#define MAX_PASSWORD_VALUE		65535
#define PASSWORD_BYTES			2
#define MAX_PORT				9999
#define PORT_DIGITS				4
#define ID_BYTES				8
#define MSG_TERMINATOR_BYTES	3
#define MAX_PACKET_SIZE			512
#define NUMITEM_BYTES			3
#define STR_TOREAD_BYTES		2
#define STR_CODE_BYTES			1
#define CMD_BYTES				5
#define MAX_CLIENTS				100


// error codes
typedef enum {
	IPB_OK,
	
	// generic system errors.
	IPB_ERROR_INVALID_ARGUMENT,
	IPB_ERROR_INVALID_PACKET,
	IPB_ERROR_INVALID_PACKET_TYPE,
	IPB_ERROR_ARGUMENT_TOO_BIG,
	IPB_ERROR_ARGUMENT_LENGTH_MISMATCH,
	IPB_ERROR_ARGUMENT_BAD_CHARS,
	IPB_ERROR_BUFFER_TOO_SMALL,
	IPB_ERROR_MEMORY_ALLOCATION,
	IPB_ERROR_IO_FILE, 
	IPB_ERROR_USER_NOT_FOUND,
	IPB_ERROR_USER_ALREADY_REGISTERED,
	IPB_ERROR_MAX_REACHED,
	IPB_ERROR_PASS_MISMATCH,
	IPB_ERROR_ALREADY_FRIENDS,
	IPB_ERROR_NO_STREAMS,
	
	
	// parser errors.
	IPB_ERROR_MALFORMED_RESPONSE,
	IPB_ERROR_MALFORMED_MSG,
	
	// network errors.
	IPB_ERROR_CONN_FAILED,
	IPB_ERROR_SEND_FAILED,
	IPB_ERROR_RECV_FAILED,
	IPB_ERROR_SOCKET_SETUP,
	
	// server response errors.
	IPB_ERROR_SERVER_REJECTED,
	
	IPB_ERROR_UNKNOWN,
} IPBstatus;

// package types
typedef enum {
	// client -> server
	MSG_CMD_REGISTER,
	MSG_CMD_CONNECT,
	MSG_CMD_FRIEND_REQ,
	MSG_CMD_MESSAGE,
	MSG_CMD_FLOOD,
	MSG_CMD_LIST,
	MSG_CMD_CONSULT,
	MSG_CMD_DISCONNECT,
	MSG_ANS_NOK_FRIEND,
	MSG_ANS_OK_FRIEND,
	
	// server -> client
	MSG_RSP_WELCOME,
	MSG_RSP_GOODBYE,
	MSG_RSP_HELLO,
	MSG_RSP_FRIEND_SENT,
	MSG_RSP_FRIEND_UNKNOWN,
	MSG_RSP_ACK,
	MSG_ACK_FRIEND_RSP,
	MSG_RSP_MSG_SENT,
	MSG_RSP_MSG_FAIL,
	MSG_RSP_FLOOD_SENT,
	MSG_RSP_LIST_HEAD,
	MSG_RSP_LIST_ITEM,
	
	// stream content
	MSG_STR_MESSAGE,
	MSG_STR_FLOOD,
	MSG_STR_FRIEND_REQ, 
	MSG_STR_FRIEND_ACC,
	MSG_STR_FRIEND_REJ,
	MSG_STR_NO_CONTENT,

	// UDP notification
	MSG_UDP_NOTIFICATION,
	
	MSG_TYPE_UNKNOWN,
} IPBpacketType;

typedef enum {
	CODE_FRIEND_REQ_IN = 0,
	CODE_FRIEND_ACCEPTED = 1,
	CODE_FRIEND_REJECTED = 2,
	CODE_MESSAGE_IN = 3,
	CODE_FLOOD_IN = 4,
} StreamCode;

typedef struct {
	IPBpacketType type;
	
	// not all fields may be used based on packet type.
	char id[ID_BYTES + 1];				// +1 for \0
	char targetId[ID_BYTES + 1];
	char message[MAX_MESSAGE_LENGTH + 1];
	int port;
	unsigned short pass;			// little-endian
	int numItems;					// used for RLIST and UDP notifications (XX)
	StreamCode streamCode;			// notification code
} IPBpacket;


/**
 * @brief Centralized error reporting. Translates status codes to text.
 */
const char* IPBstatusToString(IPBstatus status);

#endif