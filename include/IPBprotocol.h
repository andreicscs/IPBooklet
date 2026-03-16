#ifndef IPBPROTOCOL_H
#define IPBPROTOCOL_H

// This header contains all protocol definitions that should remain hidden from the user.

// general limits.
#define MESS_TYPE_BYTES	5


// terminator
#define MSG_TERMINATOR			"+++"

// client commands
#define CMD_REGISTER	"REGIS"
#define CMD_CONNECT 	"CONNE"
#define CMD_FRIEND_REQ	"FRIE?"
#define CMD_MESSAGE		"MESS?"
#define CMD_FLOOD 		"FLOO?"
#define CMD_LIST 		"LIST?"
#define CMD_CONSULT		"CONSU"
#define CMD_IQUIT		"IQUIT"

// server responses
#define RSP_WELCOME		"WELCO"
#define RSP_GOODBYE		"GOBYE"
#define RSP_HELLO		"HELLO"
#define RSP_FRIE_OK		"FRIE>"
#define RSP_FRIE_FAIL	"FRIE<"
#define RSP_ACK			"ACKRF"
#define RSP_MESS_OK		"MESS>"
#define RSP_MESS_FAIL	"MESS<"
#define RSP_FLOO_OK		"FLOO>"
#define RSP_LIST_HEAD	"RLIST"
#define RSP_LIST_ITEMS	"LINUM"

// streams
#define STR_MESSAGE			"SSEM>"
#define STR_FLOOD			"OOLF>"
#define STR_FRIEND_REQ		"EIRF>"
#define STR_FRIEND_ACCEPT	"FRIEN"
#define STR_FRIEND_REJECT	"NOFRI"
#define STR_NO_CONTENT		"NOCON"


// client stream responses
#define ANS_OK_FRIEND_REQ	"OKIRF"
#define ANS_NOK_FRIEND_REQ	"NOKRF"

#endif