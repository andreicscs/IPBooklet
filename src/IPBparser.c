#include "IPBparser.h"
#include "IPBprotocol.h"
#include "IPBtypes.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static IPBstatus IPBvalidateId(const char* id){
	if (!id) return IPB_ERROR_INVALID_ARGUMENT;
	
	size_t len = 0;
	bool terminated = false;
	
	for (int i = 0; i <= ID_BYTES; i++) { // check up to ID_BYTES+1
		if (id[i] == '\0') {
			terminated = true;
			len = i;
			break;
		}
	}
	if (!terminated) return IPB_ERROR_ARGUMENT_TOO_BIG;
	if (len != ID_BYTES) return IPB_ERROR_ARGUMENT_LENGTH_MISMATCH;
	if (len == 0) return IPB_ERROR_INVALID_ARGUMENT;
	
	
	if (strchr(id, ' ') != NULL) return IPB_ERROR_ARGUMENT_BAD_CHARS;
	if (strstr(id, MSG_TERMINATOR) != NULL) return IPB_ERROR_ARGUMENT_BAD_CHARS;
	for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)id[i])) {
            return IPB_ERROR_ARGUMENT_BAD_CHARS;
        }
    }
	
	return IPB_OK;
}

static IPBstatus IPBvalidatePort(const unsigned int port){
	if (port == 0) return IPB_ERROR_INVALID_ARGUMENT;
	if (port > MAX_PORT) return IPB_ERROR_ARGUMENT_TOO_BIG;
	
	return IPB_OK;
}

static IPBstatus IPBvalidatePass(const unsigned short pass){
	#if MAX_PASSWORD_VALUE < 0xFFFF // just in case MAX_PASSWORD_VALUE is smaller then max unsigned short value
		if (pass > MAX_PASSWORD_VALUE) return IPB_ERROR_ARGUMENT_TOO_BIG; 
	#else
		(void)pass; // forcefully remove unused warning. it is impossible for the value to be greater then MAX_PASSWORD_VALUE.
	#endif
	
	return IPB_OK;
}

static IPBstatus IPBvalidateMsg(const char* msg){
	if (!msg) return IPB_ERROR_INVALID_ARGUMENT;
	
	size_t len = 0;
	bool terminated = false;
	
	for (int i = 0; i <= MAX_MESSAGE_LENGTH; i++) { // check up to MAX_MESSAGE_LENGTH+1
		if (msg[i] == '\0') {
			terminated = true;
			len = i;
			break;
		}
	}
	
	if (!terminated) return IPB_ERROR_ARGUMENT_TOO_BIG;
	if (len > MAX_MESSAGE_LENGTH) return IPB_ERROR_ARGUMENT_TOO_BIG;
	
	if (strstr(msg, MSG_TERMINATOR) != NULL) return IPB_ERROR_ARGUMENT_BAD_CHARS;
	
	return IPB_OK;
}

static IPBpacketType stringToType(const char* cmdStr){
	// client -> server
	if (strcmp(cmdStr, CMD_REGISTER) == 0)		return MSG_CMD_REGISTER;		// "REGIS"
	if (strcmp(cmdStr, CMD_CONNECT) == 0)		return MSG_CMD_CONNECT;			// "CONNE"
	if (strcmp(cmdStr, CMD_FRIEND_REQ) == 0)	return MSG_CMD_FRIEND_REQ;		// "FRIE?"
	if (strcmp(cmdStr, CMD_MESSAGE) == 0)		return MSG_CMD_MESSAGE;			// "MESS?"
	if (strcmp(cmdStr, CMD_FLOOD) == 0)			return MSG_CMD_FLOOD;			// "FLOO?"
	if (strcmp(cmdStr, CMD_LIST) == 0)			return MSG_CMD_LIST;			// "LIST?"
	if (strcmp(cmdStr, CMD_CONSULT) == 0)		return MSG_CMD_CONSULT;			// "CONSU"
	if (strcmp(cmdStr, ANS_OK_FRIEND_REQ) == 0) return MSG_ANS_OK_FRIEND;		// "OKIRF"
	if (strcmp(cmdStr, ANS_NOK_FRIEND_REQ) == 0)return MSG_ANS_NOK_FRIEND;		// "NOKRF"
	if (strcmp(cmdStr, CMD_IQUIT) == 0)			return MSG_CMD_DISCONNECT;		// "IQUIT"

	// server -> client
	if (strcmp(cmdStr, RSP_GOODBYE) == 0)		return MSG_RSP_GOODBYE;			// "GOBYE"
	if (strcmp(cmdStr, RSP_WELCOME) == 0)		return MSG_RSP_WELCOME;			// "WELCO"
	if (strcmp(cmdStr, RSP_HELLO) == 0)			return MSG_RSP_HELLO;			// "HELLO"
	if (strcmp(cmdStr, RSP_FRIE_OK) == 0)		return MSG_RSP_FRIEND_SENT;		// "FRIE>"
	if (strcmp(cmdStr, RSP_FRIE_FAIL) == 0)		return MSG_RSP_FRIEND_UNKNOWN;	// "FRIE<"
	if (strcmp(cmdStr, RSP_MESS_OK) == 0)		return MSG_RSP_MSG_SENT;		// "MESS>"
	if (strcmp(cmdStr, RSP_MESS_FAIL) == 0)		return MSG_RSP_MSG_FAIL;		// "MESS<"
	if (strcmp(cmdStr, RSP_FLOO_OK) == 0)		return MSG_RSP_FLOOD_SENT;		// "FLOO>"
	if (strcmp(cmdStr, RSP_LIST_HEAD) == 0)		return MSG_RSP_LIST_HEAD;		// "RLIST"
	if (strcmp(cmdStr, RSP_LIST_ITEMS) == 0)	return MSG_RSP_LIST_ITEM;		// "LINUM"
	if (strcmp(cmdStr, RSP_ACK) == 0)			return MSG_RSP_ACK;				// "ACKRF"

	// stream content
	if (strcmp(cmdStr, STR_MESSAGE) == 0)		return MSG_STR_MESSAGE;			// "SSEM>"
	if (strcmp(cmdStr, STR_FLOOD) == 0)			return MSG_STR_FLOOD;			// "OOLF>"
	if (strcmp(cmdStr, STR_FRIEND_REQ) == 0)	return MSG_STR_FRIEND_REQ;		// "EIRF>"
	if (strcmp(cmdStr, STR_FRIEND_ACCEPT) == 0) return MSG_STR_FRIEND_ACC;		// "FRIEN"
	if (strcmp(cmdStr, STR_FRIEND_REJECT) == 0) return MSG_STR_FRIEND_REJ;		// "NOFRI"
	if (strcmp(cmdStr, STR_NO_CONTENT) == 0)	return MSG_STR_NO_CONTENT;		// "NOCON"

	return MSG_TYPE_UNKNOWN;
}

// --- api implementation ---

IPBstatus IPBdeserialize(IPBpacket* outPacket, const char* buffer, const size_t bufferSize){
	if (outPacket == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (buffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	
	if (memcmp( buffer+bufferSize-MSG_TERMINATOR_BYTES, MSG_TERMINATOR, MSG_TERMINATOR_BYTES)!=0){ // if last MSG_TERMINATOR_BYTES of buffer are not MSG_TERMINATOR:
		// is it udp notifications ? 
		if (bufferSize == 3) {
			outPacket->type = MSG_UDP_NOTIFICATION;
			
			char code[2];
			memcpy(&code, buffer, 1);
			code[1] = '\0';
			outPacket->streamCode = atoi(code);
			
			char count[3];
			count[0] = buffer[2]; // high nibble first
			count[1] = buffer[1]; // low nibble second
			count[2] = '\0';
			
			outPacket->numItems = (int)strtol(count, NULL, 16); // from hex to long cast to int.
			
			return IPB_OK; 
		}
		
		return IPB_ERROR_INVALID_PACKET;
	}
	
	
	char typeStr[CMD_BYTES+1]; // + \0
	memcpy(typeStr, buffer, CMD_BYTES);
	typeStr[CMD_BYTES] = '\0';
	
	outPacket->type = stringToType(typeStr);
	if (outPacket->type == MSG_TYPE_UNKNOWN) return IPB_ERROR_INVALID_PACKET_TYPE;

	size_t binOffset = 0;
	size_t msgLen = 0;
	char portStr[PORT_DIGITS + 1];
	char numStr[NUMITEM_BYTES + 1];
	IPBstatus res;
	switch (outPacket->type){
		case MSG_CMD_REGISTER:
			// Format: [CMD_REGISTER id port passMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1 + ID_BYTES + 1 + PORT_DIGITS + 1;
			if (bufferSize < binOffset + PASSWORD_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;

			memcpy(outPacket->id, buffer + CMD_BYTES + 1, ID_BYTES);
			outPacket->id[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->id);
			if (res != IPB_OK) return res;

			memcpy(portStr, buffer + CMD_BYTES + 1 + ID_BYTES + 1, PORT_DIGITS);
			portStr[PORT_DIGITS] = '\0';
			outPacket->port = atoi(portStr);
			res = IPBvalidatePort(outPacket->port);
			if (res != IPB_OK) return res;
			
			
			// ex for 1234 (0x04D2): lowByte=0xD2 (11010010), highByte=0x04 (00000100)
			// highByte << 8 = 0x0400 (00000100 00000000)
			// highByte |  lowByte = 0x04D2 (00000100 11010010) results 2 bytes little endian.
			unsigned char lowByte  = (unsigned char)buffer[binOffset];
			unsigned char highByte = (unsigned char)buffer[binOffset + 1];
			outPacket->pass = (unsigned short)(lowByte | (highByte << 8)); // shift highByte 1 byte to the left, or with lowByte.
			
			//memcpy(&outPacket->pass, buffer + binOffset, PASSWORD_BYTES); // already little endian (sent from little endian machine) TODO, consider explicit conversion to little endian...
			res = IPBvalidatePass(outPacket->pass);
			if (res != IPB_OK) return res;
		break;
		
		case MSG_CMD_CONNECT:
			// Format: [CMD_CONNECT id passMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1 + ID_BYTES + 1;
			if (bufferSize < binOffset + PASSWORD_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;

			memcpy(outPacket->id, buffer + CMD_BYTES + 1, ID_BYTES);
			outPacket->id[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->id);
			if (res != IPB_OK) return res;
			
			memcpy(&outPacket->pass, buffer + binOffset, PASSWORD_BYTES);
			res = IPBvalidatePass(outPacket->pass);
			if (res != IPB_OK) return res;
		break;

		case MSG_CMD_FRIEND_REQ:
			// Format: [CMD_FRIEND_REQ target_idMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1;
			if (bufferSize < binOffset + ID_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(outPacket->targetId, buffer + binOffset, ID_BYTES);
			outPacket->targetId[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->targetId);
			if (res != IPB_OK) return res;
		break;

		case MSG_CMD_MESSAGE:
			// Format: [CMD_MESSAGE target_id messageMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1 + ID_BYTES + 1;
			if (bufferSize < binOffset + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(outPacket->targetId, buffer + CMD_BYTES + 1, ID_BYTES);
			outPacket->targetId[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->targetId);
			if (res != IPB_OK) return res;
			
			msgLen = bufferSize - binOffset - MSG_TERMINATOR_BYTES;
			if (msgLen > MAX_MESSAGE_LENGTH) return IPB_ERROR_ARGUMENT_TOO_BIG;
			
			memcpy(outPacket->message, buffer + binOffset, msgLen);
			outPacket->message[msgLen] = '\0';
			
			res = IPBvalidateMsg(outPacket->message);
			if (res != IPB_OK) return res;
		break;

		case MSG_CMD_FLOOD:
			// Format: [CMD_FLOOD messageMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1;
			if (bufferSize < binOffset + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;

			msgLen = bufferSize - binOffset - MSG_TERMINATOR_BYTES;
			if (msgLen > MAX_MESSAGE_LENGTH) return IPB_ERROR_ARGUMENT_TOO_BIG;
			
			memcpy(outPacket->message, buffer + binOffset, msgLen);
			outPacket->message[msgLen] = '\0';
			
			res = IPBvalidateMsg(outPacket->message);
			if (res != IPB_OK) return res;
		break;

		case MSG_RSP_LIST_HEAD:
			// Format: [RSP_LIST_HEAD num-itemMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1;
			if (bufferSize < binOffset + NUMITEM_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(numStr, buffer + binOffset, NUMITEM_BYTES);
			numStr[NUMITEM_BYTES] = '\0';
			outPacket->numItems = atoi(numStr);
		break;

		case MSG_RSP_LIST_ITEM:
			// Format: [RSP_LIST_ITEMS idMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1;
			if (bufferSize < binOffset + ID_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(outPacket->id, buffer + binOffset, ID_BYTES);
			outPacket->id[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->id);
			if (res != IPB_OK) return res;
		break;

		case MSG_STR_MESSAGE:
		case MSG_STR_FLOOD:
			// Format: [STR_FLOOD id messageMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1 + ID_BYTES + 1;
			if (bufferSize < binOffset + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(outPacket->id, buffer + CMD_BYTES + 1, ID_BYTES);
			outPacket->id[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->id);
			if (res != IPB_OK) return res;
			
			msgLen = bufferSize - binOffset - MSG_TERMINATOR_BYTES;
			if (msgLen > MAX_MESSAGE_LENGTH) return IPB_ERROR_ARGUMENT_TOO_BIG;
			
			memcpy(outPacket->message, buffer + binOffset, msgLen);
			outPacket->message[msgLen] = '\0';
			
			res = IPBvalidateMsg(outPacket->message);
			if (res != IPB_OK) return res;
		break;

		case MSG_STR_FRIEND_REQ:
		case MSG_STR_FRIEND_ACC:
		case MSG_STR_FRIEND_REJ:
			// Format: [STR_FRIEND_REJECT idMSG_TERMINATOR]
			binOffset = CMD_BYTES + 1;
			if (bufferSize < binOffset + ID_BYTES + MSG_TERMINATOR_BYTES) return IPB_ERROR_BUFFER_TOO_SMALL;
			
			memcpy(outPacket->id, buffer + binOffset, ID_BYTES);
			outPacket->id[ID_BYTES] = '\0';
			res = IPBvalidateId(outPacket->id);
			if (res != IPB_OK) return res;
		break;

		default:
			// packets without payload other then packet type.
		break;
	}

	return IPB_OK;
}

// --- serialize client messages ---


// Format: [CMD_REGISTER id port passMSG_TERMINATOR]
IPBstatus IPBserializeRegister(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const unsigned int portUDP, const unsigned short pass){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	res = IPBvalidatePort(portUDP); if(res != IPB_OK) return res;
	res = IPBvalidatePass(pass); if(res != IPB_OK) return res;
	
	// fill fields with empty spaces or 0s, * tells snprintf, that the numebr of fillers will be the next argument.
	int offset = snprintf(outBuffer, bufferSize, "%s %s %0*u ", CMD_REGISTER, id, PORT_DIGITS, portUDP);
	if ((size_t)(offset + PASSWORD_BYTES + MSG_TERMINATOR_BYTES) > bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	
	outBuffer[offset] = pass & 0xFF; // 1111 1111 mask, keep lowest byte.
	outBuffer[offset+1] = (pass >> 8) & 0xFF; // shift to the right and repeat.
	offset += PASSWORD_BYTES;
	
	memcpy(outBuffer + offset, MSG_TERMINATOR, MSG_TERMINATOR_BYTES);
	offset += MSG_TERMINATOR_BYTES;
	
	*outLen = offset;
	
	return IPB_OK;
}

// Format: [CMD_CONNECT id passMSG_TERMINATOR]
IPBstatus IPBserializeConnect(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const unsigned short pass){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	res = IPBvalidatePass(pass); if(res != IPB_OK) return res;
	
	// fill fields with empty spaces or 0s, * tells snprintf, that the numebr of fillers will be the next argument.
	int offset = snprintf(outBuffer, bufferSize, "%s %s ", CMD_CONNECT, id);
	if ((size_t)(offset + PASSWORD_BYTES + MSG_TERMINATOR_BYTES) > bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	// copy the memory of the little endian encoded password to outBuffer (avoid it being converted to string if snprintf were to be used).
	memcpy(outBuffer + offset, &pass, PASSWORD_BYTES);
	offset += PASSWORD_BYTES;
	
	memcpy(outBuffer + offset, MSG_TERMINATOR, MSG_TERMINATOR_BYTES);
	offset += MSG_TERMINATOR_BYTES;
	
	*outLen = offset;
	
	return IPB_OK;
}

// Format: [CMD_FRIEND_REQ targetIdMSG_TERMINATOR]
IPBstatus IPBserializeFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen, const char* targetId){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(targetId); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", CMD_FRIEND_REQ, targetId, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written; // returns length excluding \0.
	
	return IPB_OK;
}

// Format: [CMD_MESSAGE targetId messMSG_TERMINATOR]
IPBstatus IPBserializeMessage(char* outBuffer, const size_t bufferSize, int* outLen, const char* targetId, const char* message){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(targetId); if(res != IPB_OK) return res;
	res = IPBvalidateMsg(message); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s %s%s", CMD_MESSAGE, targetId, message, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [CMD_FLOOD messMSG_TERMINATOR]
IPBstatus IPBserializeFlood(char* outBuffer, const size_t bufferSize, int* outLen, const char* message){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateMsg(message); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", CMD_FLOOD, message, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [CMD_LISTMSG_TERMINATOR]
IPBstatus IPBserializeGetClientList(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", CMD_LIST, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [CMD_CONSULTMSG_TERMINATOR]
IPBstatus IPBserializeGetStreams(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", CMD_CONSULT, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [ANS_OK_FRIEND_REQMSG_TERMINATOR]
IPBstatus IPBserializeAcceptFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", ANS_OK_FRIEND_REQ, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [ANS_NOK_FRIEND_REQMSG_TERMINATOR]
IPBstatus IPBserializeRejectFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", ANS_NOK_FRIEND_REQ, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [CMD_IQUITMSG_TERMINATOR]
IPBstatus IPBserializeDisconnect(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", CMD_IQUIT, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}


// --- serialize server responses ---

// Format: [RSP_WELCOMEMSG_TERMINATOR]
IPBstatus IPBserializeConfirmRegister(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_WELCOME, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_GOODBYEMSG_TERMINATOR]
IPBstatus IPBserializeCloseSession(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_GOODBYE, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_HELLOMSG_TERMINATOR]
IPBstatus IPBserializeConfirmLogin(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_HELLO, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_FRIE_OKMSG_TERMINATOR]
IPBstatus IPBserializeFriendReqSent(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_FRIE_OK, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_FRIE_FAILMSG_TERMINATOR]
IPBstatus IPBserializeFriendReqUnknown(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_FRIE_FAIL, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_ACKMSG_TERMINATOR]
IPBstatus IPBserializeAckFriendRsp(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_ACK, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_MESS_OKMSG_TERMINATOR]
IPBstatus IPBserializeMessageSent(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_MESS_OK, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_MESS_FAILMSG_TERMINATOR]
IPBstatus IPBserializeMessageNotSent(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_MESS_FAIL, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_FLOO_OKMSG_TERMINATOR]
IPBstatus IPBserializeFloodSent(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", RSP_FLOO_OK, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_LIST_HEAD num-itemMSG_TERMINATOR]
IPBstatus IPBserializeClientListHead(char* outBuffer, const size_t bufferSize, int* outLen, const int count){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s %0*d%s", RSP_LIST_HEAD, NUMITEM_BYTES, count, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [RSP_LIST_ITEMS idMSG_TERMINATOR]
IPBstatus IPBserializeClientListItem(char* outBuffer, const size_t bufferSize, int* outLen, const char* id){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", RSP_LIST_ITEMS, id, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_MESSAGE id messMSG_TERMINATOR]
IPBstatus IPBserializeMessageStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const char* message){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	res = IPBvalidateMsg(message); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s %s%s", STR_MESSAGE, id, message, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_FLOOD id messMSG_TERMINATOR]
IPBstatus IPBserializeFloodStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const char* message){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	res = IPBvalidateMsg(message); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s %s%s", STR_FLOOD, id, message, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_FRIEND_REQ idMSG_TERMINATOR]
IPBstatus IPBserializeFriendReqStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", STR_FRIEND_REQ, id, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_FRIEND_ACCEPT idMSG_TERMINATOR]
IPBstatus IPBserializeFriendAccStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", STR_FRIEND_ACCEPT, id, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_FRIEND_REJECT idMSG_TERMINATOR]
IPBstatus IPBserializeFriendRejStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	IPBstatus res;
	
	res = IPBvalidateId(id); if(res != IPB_OK) return res;
	
	int written = snprintf(outBuffer, bufferSize, "%s %s%s", STR_FRIEND_REJECT, id, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [STR_NO_CONTENTMSG_TERMINATOR]
IPBstatus IPBserializeNoStreams(char* outBuffer, const size_t bufferSize, int* outLen){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int written = snprintf(outBuffer, bufferSize, "%s%s", STR_NO_CONTENT, MSG_TERMINATOR);
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}

// Format: [codeXX] where XX is the stream count in little-endian.
IPBstatus IPBserializeNotification(char* outBuffer, const size_t bufferSize, int* outLen, const StreamCode code, const int count){
	if (outBuffer == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	if (outLen == NULL) return IPB_ERROR_INVALID_ARGUMENT;
	
	int safeCount = (count > 255) ? 255 : count;
	
	int lowNibble = safeCount & 0x0F; // 0000 1111 mask, keep last 4
    int highNibble = (safeCount >> 4) & 0x0F; // shift to the right and repeat.
	
	int written = snprintf(outBuffer, bufferSize, "%c%X%X", code + '0', lowNibble, highNibble); // + '0' to convert to ascii, low nibble first for little endian.
	if ((size_t)written >= bufferSize) return IPB_ERROR_BUFFER_TOO_SMALL;
	
	*outLen = written;
	
	return IPB_OK;
}






