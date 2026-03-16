#ifndef IPBCLIENTAPI_H
#define IPBCLIENTAPI_H

// This library is an API for the client to manage all low level comunications with the server.

#include "IPBtypes.h"

#include <stddef.h>

typedef struct{
	int socketTcpFD; 	// file descriptor of the local TCP socket, used for communating.
	int socketUdpFD;	// file descriptor of the local udp socket, used for client notifications.
} IPBclientSession;

/**
* @brief initializes the client session, creates the TCP socket and connects it to the server,
* creates the UDP socket and binds it to the specified local port.
*
* @param outSession: pointer to the session struct to be initialized.
* @param serverIp: the IP address of the server.
* @param serverPort: the TCP port of the server.
* @param localUdpPort: the local port to bind to receive notifications.
* @return IPBstatus: IPB_OK, IPB_ERROR_CONN_FAILED, IPB_ERROR_SOCKET_SETUP.
* @post sessionPTR->socketTcpFD is connected and sessionPTR->socketUdpFD is bound.
*/
IPBstatus IPBinitClientNetwork(IPBclientSession* outSession, const char* serverIp, const int serverPort, const int localUdpPort);

/**
* @brief connects to the server via tcp useful in case of a disconnection,
*
* @param outSession: pointer to the session struct to be initialized.
* @param serverIp: the IP address of the server.
* @param serverPort: the TCP port of the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_CONN_FAILED.
* @post sessionPTR->socketTcpFD is connected.
*/
IPBstatus IPBreconnectClientNetwork(IPBclientSession* outSession, const char* serverIp, const int serverPort);

/**
* @brief closes session.
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @post the sessionPTR is closed and has to be initialized again if it needs to be used.
*/
void IPBcloseClientNetwork(IPBclientSession* sessionPTR);

/**
* @brief this function waits for an incoming notification and stores it in response.
* 
* @param outResponse: the IPBpacket struct where the response will be stored in.
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post waits for an incoming notification and stores it in response.
*/
IPBstatus IPBlistenNotifications(IPBpacket* outResponse, const IPBclientSession* sessionPTR);

/**
* @brief this function waits for an incoming message and stores it in response.
* 
* @param outResponse: the IPBpacket struct where the response will be stored in.
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: status code.
* @post waits for an incoming message and stores it in response.
*/
IPBstatus IPBlistenMessages(IPBpacket* outResponse, const IPBclientSession* sessionPTR);

/**
* @brief this function takes in user data, and sends a register request to the server.
*
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @param id: the alphanumeric identification of the client.
* @param portUDP: the UDP port the client will listen for notifications on.
* @param pass: the password the client will use to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends register message with user input sent to the server.
*/
IPBstatus IPBregister(const IPBclientSession* sessionPTR, const char* id, int portUDP, int pass);

/**
* @brief this function takes in a IPBclientSession, user id and it's password and sends a connection request to the server for the client to log in.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @param id: the alphanumeric identification of the client.
* @param pass: the password of the client.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends connect message with user details to the server.
*/
IPBstatus IPBconnect(const IPBclientSession* sessionPTR, const char* id, const int pass);

/**
* @brief this function takes in IPBclientSession, user id, and sends a friend request to the provided user id.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @param targetId: the alphanumeric identification of the client to send the request to.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends friend request message with user details to the server.
*/
IPBstatus IPBsendFriendRequest(const IPBclientSession* sessionPTR, const char* targetId);

/**
* @brief this function takes in IPBclientSession, user id, and sends a message to the provided user id.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @param targetId: the alphanumeric identification of the client to send the request to.
* @param mess: the message string to be delivered.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends user details and message to the server to get delivered.
*/
IPBstatus IPBsendMessage(const IPBclientSession* sessionPTR, const char* targetId, const char* mess);

/**
* @brief this function takes in IPBclientSession, and sends a message to all friends, and friends of friends recursively.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @param mess: the message string to be delivered.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to get delivered to all friends, and friends of friends recursively.
*/
IPBstatus IPBflood(const IPBclientSession* sessionPTR, const char* mess);

/**
* @brief this function takes in IPBclientSession, and requests the client list to the server.
*
* @param outResponse: the IPBpacket struct where the response will be stored in.
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to request list of all it's clients.
*/
IPBstatus IPBgetClientList(IPBpacket* outResponse, const IPBclientSession* sessionPTR);

/**
* @brief this function takes in IPBclientSession, and requests the stream list to the server.
*
* @param outResponse: the IPBpacket struct where the response will be stored in.
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to request list of all it's clients.
*/
IPBstatus IPBgetStream(IPBpacket* outResponse, const IPBclientSession* sessionPTR);

/**
* @brief this function takes in IPBclientSession, and sends a friend request confirmation to the server.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to accept friend request.
*/
IPBstatus IPBacceptFriendRequest(const IPBclientSession* sessionPTR);

/**
* @brief this function takes in IPBclientSession, and sends a friend request rejection to the server.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to reject friend request.
*/
IPBstatus IPBrejectFriendRequest(const IPBclientSession* sessionPTR);

/**
* @brief this function takes in IPBclientSession, and tells to the server the client wants to close the connection.
* 
* @param sessionPTR: the IPBclientSession* containing all information necessary to connect back to the server.
* @return IPBstatus: IPB_OK, IPB_ERROR_SERVER_REJECTED, IPB_ERROR_SEND_FAILED, IPB_ERROR_RECV_FAILED, IPB_ERROR_MALFORMED_RESPONSE.
* @post sends message to the server to close the session.
*/
IPBstatus IPBdisconnect(const IPBclientSession* sessionPTR);

#endif