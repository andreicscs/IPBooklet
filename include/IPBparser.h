#ifndef IPBPARSER_H
#define IPBPARSER_H

// This library is used to serialize and deserialize data into the protocol standard.

#include "IPBtypes.h"

#include <stddef.h>

/**
* @brief parses a raw buffer, validates syntax and fills in IPBpacket.
* @param outPacket: pointer of the struct to be filled in.
* @param buffer: the received buffer.
* @param bufferSize: the size of the buffer.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if syntax not valid, IPB_ERROR_INVALID_PACKET_TYPE if command not recognized.
*/
IPBstatus IPBdeserialize(IPBpacket* outPacket, const char* buffer, const size_t bufferSize);


// --- serialize client messages ---

/**
* @brief formats the CMD_REGISTER command string into the provided outBuffer.
* Format: [CMD_REGISTER id port passMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @param portUDP: the UDP port the client will listen for notifications on.
* @param pass: the password of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES, port exceeds MAX_PORT, pass exceeds MAX_PASSWORD_VALUE, or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeRegister(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const unsigned int portUDP, const unsigned short pass);

/**
* @brief formats the CMD_CONNECT command string into the provided outBuffer.
* Format: [CMD_CONNECT id passMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @param pass: the password of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES, pass exceeds MAX_PASSWORD_VALUE, or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeConnect(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const unsigned short pass);

/**
* @brief formats the CMD_FRIEND_REQ command string into the provided outBuffer.
* Format: [CMD_FRIEND_REQ id MSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param targetId: the alphanumeric identification of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen, const char* targetId);

/**
* @brief formats the CMD_MESSAGE command string into the provided outBuffer.
* Format: [CMD_MESSAGE id messMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param targetId: the alphanumeric identification of the recipient.
* @param message: the message content.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES, message exceeds MAX_MESSAGE_LENGTH, or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeMessage(char* outBuffer, const size_t bufferSize, int* outLen, const char* targetId, const char* message);

/**
* @brief formats the CMD_FLOOD command string into the provided outBuffer.
* Format: [CMD_FLOOD messMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param message: the flood message content.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if message exceeds MAX_MESSAGE_LENGTH or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFlood(char* outBuffer, const size_t bufferSize, int* outLen, const char* message);

/**
* @brief formats the CMD_LIST command string into the provided outBuffer.
* Format: [CMD_LISTMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeGetClientList(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the CMD_CONSULT command string into the provided outBuffer.
* Format: [CMD_CONSULTMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeGetStreams(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the ANS_OK_FRIEND_REQ command string into the provided outBuffer.
* Format: [ANS_OK_FRIEND_REQMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeAcceptFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the ANS_NOK_FRIEND_REQ command string into the provided outBuffer.
* Format: [ANS_NOK_FRIEND_REQMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeRejectFriendRequest(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the CMD_IQUIT command string into the provided outBuffer.
* Format: [CMD_IQUITMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeDisconnect(char* outBuffer, const size_t bufferSize, int* outLen);


// --- serialize server responses ---

/**
* @brief formats the RSP_WELCOME command string into the provided outBuffer.
* Format: [RSP_WELCOMEMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeConfirmRegister(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_GOODBYE command string into the provided outBuffer.
* Format: [RSP_GOODBYEMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeCloseSession(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_HELLO command string into the provided outBuffer.
* Format: [RSP_HELLOMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeConfirmLogin(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_FRIE_OK command string into the provided outBuffer.
* Format: [RSP_FRIE_OKMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendReqSent(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_FRIE_FAIL command string into the provided outBuffer.
* Format: [RSP_FRIE_FAILMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendReqUnknown(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_ACK command string into the provided outBuffer.
* Format: [RSP_ACKMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeAckFriendRsp(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_MESS_OK command string into the provided outBuffer.
* Format: [RSP_MESS_OKMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeMessageSent(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_MESS_FAIL command string into the provided outBuffer.
* Format: [RSP_MESS_FAILMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeMessageNotSent(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_FLOO_OK command string into the provided outBuffer.
* Format: [RSP_FLOO_OKMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFloodSent(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats the RSP_LIST_HEAD command string into the provided outBuffer.
* Format: [RSP_LIST_HEAD num-itemMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param count: the number of clients in the list.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeClientListHead(char* outBuffer, const size_t bufferSize, int* outLen, const int count);

/**
* @brief formats the RSP_LIST_ITEMS command string into the provided outBuffer.
* Format: [RSP_LIST_ITEMS idMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeClientListItem(char* outBuffer, const size_t bufferSize, int* outLen, const char* id);

/**
* @brief formats the STR_MESSAGE command string into the provided outBuffer.
* Format: [STR_MESSAGE id messMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @param message: the message content.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES, message exceeds MAX_MESSAGE_LENGTH, or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeMessageStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const char* message);

/**
* @brief formats the STR_FLOOD command string into the provided outBuffer.
* Format: [STR_FLOOD id messMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @param message: the flood message content.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES, message exceeds MAX_MESSAGE_LENGTH, or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFloodStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id, const char* message);

/**
* @brief formats the STR_FRIEND_REQ command string into the provided outBuffer.
* Format: [STR_FRIEND_REQ idMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendReqStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id);

/**
* @brief formats the STR_FRIEND_ACCEPT command string into the provided outBuffer.
* Format: [STR_FRIEND_ACCEPT idMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendAccStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id);

/**
* @brief formats the STR_FRIEND_REJECT command string into the provided outBuffer.
* Format: [STR_FRIEND_REJECT idMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param id: the alphanumeric identification of the client.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if id exceeds ID_BYTES or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeFriendRejStream(char* outBuffer, const size_t bufferSize, int* outLen, const char* id);

/**
* @brief formats the STR_NO_CONTENT command string into the provided outBuffer.
* Format: [STR_NO_CONTENTMSG_TERMINATOR]
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_BUFFER_TOO_SMALL if outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeNoStreams(char* outBuffer, const size_t bufferSize, int* outLen);

/**
* @brief formats a UDP notification into the provided outBuffer.
* Format: [YXX] where Y is the stream code and XX is the stream count in little-endian.
* @param outBuffer: the char array where the formatted string will be stored.
* @param bufferSize: the maximum size of the outBuffer.
* @param outLen: the pointer of the int where the length of the serialized protocol message will be stored.
* @param code: the stream code of the notification.
* @param count: the number of unread streams.
* @return IPBstatus: IPB_OK if successful, IPB_ERROR_INVALID_ARGUMENT if outBuffer or outLen is NULL, IPB_ERROR_ARGUMENT_TOO_BIG if count exceeds 0xFFFF or outBuffer is too small.
* @post outBuffer contains the null-terminated protocol message ready to send.
*/
IPBstatus IPBserializeNotification(char* outBuffer, const size_t bufferSize, int* outLen, const StreamCode code, const int count);

#endif

