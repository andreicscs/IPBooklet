#ifndef IPBSERVERDATA_H
#define IPBSERVERDATA_H

// This library is an API for the server to manage all data.

#include "IPBtypes.h"

#define MAX_STREAMS 255


/**
* @brief gets all registered user's ids.
* @param outUsers: the matrix where all users ids wil be stored.
* @param outCount: user count.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataGetUsers(char outUsers[][ID_BYTES + 1], int* outCount);

/**
* @brief Registra un utente.
* @param id: the id of the user to register.
* @param pass: the password of the user.
* @param udpPort: udp port of the client.
* @post user "id" and its data added to the database.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataRegisterUser(const char* id, const int udpPort, const int pass, char* ip);

/**
* @brief Registra un utente.
* @param id: the id of the user to register.
* @param pass: the password of the user.
* @return IPBstatus: OK if auth is correct, ERROR_SERVER_REJECTED if wrong, ERROR_UNKNOWN if user doesn't exist.
*/
IPBstatus IPBdataCheckAuth(const char* id, const int pass);

/**
* @brief adds user targetId to sourceId's friends
* @param sourceId: the id of the user that sent the friend request.
* @param targetId: the id of the user that received the friend request.
* @post added targetId to the sourceId's list of friends.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataAddFriend(const char* sourceId, const char* targetId);

/**
* @brief verify if 2 users are friends.
* @param sourceId: the id of the first user.
* @param targetId: the id of the second user.
* @post added targetId to the sourceId's list of friends.
* @return IPBstatus: status code.
*/
bool IPBdataAreFriends(const char* sourceId, const char* targetId);

/**
* @brief adds a stream to id's stream queue.
* @param id: the id of the user to add the stream to.
* @param stream: the stream to be added to id's stream queue.
* @post added stream to id's queue of streams.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataAddStream(const char* id, const IPBpacket* stream);

/**
* @brief returns the number of streams of a given user id.
* @param id: the id of the user.
* @return int: the number of streams.
*/
int IPBdataGetStreamCount(const char* id);

/**
* @brief returns the number of streams of a given user id.
* @param id: the id of the user to pop the stream from.
* @param outPacket: the pointer of the struct where the stream will be stored.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataPopStream(const char* id, IPBpacket* outPacket);

/**
* @brief returns the udp port of the user.
* @param id: the id of the user.
* @param outUdpPort: the pointer of the int where the udp port will be stored.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataGetUserNotifyAddr(const char* id, int* outUdpPort, char* outIp);

/**
* @brief returns the udp port of the user.
* @param senderId: the id of the user that sent the flood.
* @param outTargetList: the matrix where all users ids wil be stored. // +1 for null termination
* @param outCount: user count.
* @return IPBstatus: status code.
*/
IPBstatus IPBdataGetFloodTargets(const char* senderId, char outTargetList[][ID_BYTES + 1], int* outCount);


#endif