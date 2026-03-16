#include "IPBclientAPI.h"
#include "IPBnetwork.h"
#include "IPBparser.h"
#include "IPBtypes.h"

#include <stddef.h>
#include <sys/time.h>

// macro to "automate" send-receive logic.
#define IPB_NET_EXCHANGE(serialize_call, out_packet_ptr) \
    do { \
        char buffer[MAX_PACKET_SIZE]; \
        int len; \
        IPBstatus res; \
        \
        /* serialize */\
        res = (serialize_call); \
        if (res != IPB_OK) return res; \
        \
        /* send */ \
        if (!IPBsendRaw(sessionPTR->socketTcpFD, buffer, len)) return IPB_ERROR_SEND_FAILED; \
        \
        /* receive*/ \
        len = IPBreceiveStream(buffer, MAX_PACKET_SIZE, sessionPTR->socketTcpFD); \
        if (len <= 0) return IPB_ERROR_RECV_FAILED; \
        \
        /* deserialize */ \
        res = IPBdeserialize((out_packet_ptr), buffer, len); \
        if (res != IPB_OK) return IPB_ERROR_MALFORMED_RESPONSE; \
        \
    } while(0) // false, used to manipulate scope.

// macro to "automate" client server transaction
#define IPB_CLIENT_TRANSACTION(serialize_call, success_type, fail_type) \
	do { \
		IPBpacket response; \
		IPB_NET_EXCHANGE(serialize_call, &response); \
		\
		/* verify */ \
		if (response.type == (success_type)) return IPB_OK; \
        \
		if (response.type == (fail_type)) return IPB_ERROR_SERVER_REJECTED; \
        \
		if (response.type == MSG_RSP_GOODBYE) return IPB_ERROR_SERVER_REJECTED; \
        \
		return IPB_ERROR_MALFORMED_RESPONSE; \
	} while(0)

IPBstatus IPBinitClientNetwork(IPBclientSession* outSession, const char* serverIp, const int serverPort, const int localUdpPort){
	outSession->socketTcpFD = IPBconnectTCP(serverIp, serverPort);
	if (outSession->socketTcpFD == -1) return IPB_ERROR_CONN_FAILED;
	
	struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
	// set 3 second timeout.
	setsockopt(outSession->socketTcpFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
	outSession->socketUdpFD = IPBbindUDP(localUdpPort);
	if (outSession->socketUdpFD == -1) {
		IPBclose(outSession->socketTcpFD);
		return IPB_ERROR_SOCKET_SETUP;
	}
	
	return IPB_OK;
}

IPBstatus IPBreconnectClientNetwork(IPBclientSession* outSession, const char* serverIp, const int serverPort){
	outSession->socketTcpFD = IPBconnectTCP(serverIp, serverPort);
	if (outSession->socketTcpFD == -1) return IPB_ERROR_CONN_FAILED;
	struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
	// set 3 second timeout.
	setsockopt(outSession->socketTcpFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
	return IPB_OK;
}

void IPBcloseClientNetwork(IPBclientSession* sessionPTR){
	IPBclose(sessionPTR->socketTcpFD);
	IPBclose(sessionPTR->socketUdpFD);
	return;
}

IPBstatus IPBlistenNotifications(IPBpacket* outResponse, const IPBclientSession* sessionPTR){
	int len;
	char buffer[MAX_PACKET_SIZE];
	IPBstatus res;
	
	len = IPBreceiveDatagram(buffer, MAX_PACKET_SIZE, sessionPTR->socketUdpFD);
	if (len <= 0) return IPB_ERROR_RECV_FAILED;
	
	res = IPBdeserialize(outResponse, buffer, len);
	if (res != IPB_OK) return res;
	
	switch(outResponse->type){
		case MSG_UDP_NOTIFICATION:
			return IPB_OK;
		default:
			return IPB_ERROR_MALFORMED_RESPONSE;
	}
	return IPB_OK;
}

IPBstatus IPBlistenMessages(IPBpacket* outResponse, const IPBclientSession* sessionPTR){
	int len;
	char buffer[MAX_PACKET_SIZE];
	IPBstatus res;
	
	len = IPBreceiveStream(buffer, MAX_PACKET_SIZE, sessionPTR->socketTcpFD);
	if (len <= 0) return IPB_ERROR_RECV_FAILED;
	
	res = IPBdeserialize(outResponse, buffer, len);
	if (res != IPB_OK) return res;
	
	return IPB_OK;
}


IPBstatus IPBregister(const IPBclientSession* sessionPTR, const char* id, int portUDP, int pass){

	IPB_CLIENT_TRANSACTION(
		IPBserializeRegister(buffer, MAX_PACKET_SIZE, &len, id, portUDP, pass),
		MSG_RSP_WELCOME,			// success
		MSG_RSP_GOODBYE				// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBconnect(const IPBclientSession* sessionPTR, const char* id, const int pass){

	IPB_CLIENT_TRANSACTION(
		IPBserializeConnect(buffer, MAX_PACKET_SIZE, &len, id, pass),
		MSG_RSP_HELLO,				// success
		MSG_RSP_GOODBYE				// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBsendFriendRequest(const IPBclientSession* sessionPTR, const char* targetId){

	IPB_CLIENT_TRANSACTION(
		IPBserializeFriendRequest(buffer, MAX_PACKET_SIZE, &len, targetId),
		MSG_RSP_FRIEND_SENT,		// success
		MSG_RSP_FRIEND_UNKNOWN		// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBsendMessage(const IPBclientSession* sessionPTR, const char* targetId, const char* mess){
	
	IPB_CLIENT_TRANSACTION(
		IPBserializeMessage(buffer, MAX_PACKET_SIZE, &len, targetId, mess),
		MSG_RSP_MSG_SENT,			// success
		MSG_RSP_MSG_FAIL			// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBflood(const IPBclientSession* sessionPTR, const char* mess){
	
	IPB_CLIENT_TRANSACTION(
		IPBserializeFlood(buffer, MAX_PACKET_SIZE, &len, mess),
		MSG_RSP_FLOOD_SENT,			// success
		MSG_RSP_GOODBYE				// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBgetClientList(IPBpacket* outResponse, const IPBclientSession* sessionPTR){
	
	IPB_NET_EXCHANGE(
        IPBserializeGetClientList(buffer, MAX_PACKET_SIZE, &len), 
        outResponse
    );

	/* verify */
	switch(outResponse->type){
		case MSG_RSP_LIST_HEAD:
		case MSG_RSP_LIST_ITEM:
			return IPB_OK;
		case MSG_RSP_GOODBYE:
			return IPB_ERROR_SERVER_REJECTED;
		default:
			return IPB_ERROR_MALFORMED_RESPONSE;
	}
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBgetStream(IPBpacket* outResponse, const IPBclientSession* sessionPTR){
	
	IPB_NET_EXCHANGE(
        IPBserializeGetStreams(buffer, MAX_PACKET_SIZE, &len), 
        outResponse
    );

	/* verify */
	switch(outResponse->type){
		case MSG_STR_NO_CONTENT:
		case MSG_STR_FRIEND_ACC:
		case MSG_STR_FRIEND_REJ:
		case MSG_STR_FRIEND_REQ:
		case MSG_STR_MESSAGE:
		case MSG_STR_FLOOD:
			return IPB_OK;
		case MSG_RSP_GOODBYE:
			return IPB_ERROR_SERVER_REJECTED;
		default:
			return IPB_ERROR_MALFORMED_RESPONSE;
	}
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBacceptFriendRequest(const IPBclientSession* sessionPTR){
	
	IPB_CLIENT_TRANSACTION(
		IPBserializeAcceptFriendRequest(buffer, MAX_PACKET_SIZE, &len),
		MSG_RSP_ACK,				// success
		MSG_RSP_GOODBYE				// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBrejectFriendRequest(const IPBclientSession* sessionPTR){
	
	IPB_CLIENT_TRANSACTION(
		IPBserializeRejectFriendRequest(buffer, MAX_PACKET_SIZE, &len),
		MSG_RSP_ACK,				// success
		MSG_RSP_GOODBYE				// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}

IPBstatus IPBdisconnect(const IPBclientSession* sessionPTR){
	
	IPB_CLIENT_TRANSACTION(
		IPBserializeDisconnect(buffer, MAX_PACKET_SIZE, &len),
		MSG_RSP_GOODBYE,			// success
		MSG_TYPE_UNKNOWN			// fail
	);
	
	return IPB_ERROR_UNKNOWN;
}