#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include "IPBnetwork.h"
#include "IPBparser.h"
#include "IPBserverData.h"
#include "IPBtypes.h"

#define LOG_V(...) \
	do { \
		if(verbose) { printf("[VERBOSE] " __VA_ARGS__); printf("\n"); } \
	} while(0)

#define SEND_OR_LOG(sock, pkt) \
	do { \
		IPBstatus _res = sendPacket(sock, pkt); \
		if (_res != IPB_OK) { \
			fprintf(stderr, "[ERROR] Send failed: %s (Code %d)\n", IPBstatusToString(_res), _res); \
		} \
	} while(0)


int globalUdpSocket = -1;
bool verbose = false;


IPBstatus sendPacket(int socketFD, const IPBpacket* packet) {
    char buffer[MAX_PACKET_SIZE];
    int len = 0;
    IPBstatus res = IPB_ERROR_UNKNOWN;

    switch (packet->type) {
        case MSG_RSP_WELCOME:       res = IPBserializeConfirmRegister(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_GOODBYE:       res = IPBserializeCloseSession(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_HELLO:         res = IPBserializeConfirmLogin(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_FRIEND_SENT:   res = IPBserializeFriendReqSent(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_FRIEND_UNKNOWN:res = IPBserializeFriendReqUnknown(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_ACK:           res = IPBserializeAckFriendRsp(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_MSG_SENT:      res = IPBserializeMessageSent(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_MSG_FAIL:      res = IPBserializeMessageNotSent(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_FLOOD_SENT:    res = IPBserializeFloodSent(buffer, MAX_PACKET_SIZE, &len); break;
        case MSG_RSP_LIST_HEAD:     res = IPBserializeClientListHead(buffer, MAX_PACKET_SIZE, &len, packet->numItems); break;
        case MSG_RSP_LIST_ITEM:     res = IPBserializeClientListItem(buffer, MAX_PACKET_SIZE, &len, packet->id); break;
        
		case MSG_STR_MESSAGE:       res = IPBserializeMessageStream(buffer, MAX_PACKET_SIZE, &len, packet->id, packet->message); break;
        case MSG_STR_FLOOD:         res = IPBserializeFloodStream(buffer, MAX_PACKET_SIZE, &len, packet->id, packet->message); break;
        case MSG_STR_FRIEND_REQ:    res = IPBserializeFriendReqStream(buffer, MAX_PACKET_SIZE, &len, packet->id); break;
        case MSG_STR_FRIEND_ACC:    res = IPBserializeFriendAccStream(buffer, MAX_PACKET_SIZE, &len, packet->id); break;
        case MSG_STR_FRIEND_REJ:    res = IPBserializeFriendRejStream(buffer, MAX_PACKET_SIZE, &len, packet->id); break;
        case MSG_STR_NO_CONTENT:    res = IPBserializeNoStreams(buffer, MAX_PACKET_SIZE, &len); break;

        default: return IPB_ERROR_INVALID_PACKET_TYPE;
    }

    if (res != IPB_OK) return res;
    
    if (!IPBsendRaw(socketFD, buffer, len)) return IPB_ERROR_SEND_FAILED;
    
    return IPB_OK;
}

void notifyUser(const char* targetId, StreamCode code) {
	int targetPort;
	char targetIp[INET_ADDRSTRLEN];
	
    IPBstatus res = IPBdataGetUserNotifyAddr(targetId, &targetPort, targetIp);
	if ( res != IPB_OK){
		LOG_V("[DATA] Couldn't fetch notification address for %s: %s (Code %d)", targetIp, IPBstatusToString(res), res);
		return;
	}

    int count = IPBdataGetStreamCount(targetId);
    if (count < 0){
		count = 0;
		LOG_V("[DATA] Couldn't find user %s", targetId);
	}

    struct sockaddr_in targetAddr;
    memset(&targetAddr, 0, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(targetPort);
    inet_pton(AF_INET, targetIp, &targetAddr.sin_addr);

    char buffer[MAX_PACKET_SIZE];
	memset(buffer, 0, MAX_PACKET_SIZE);
    int len = 0;


    res = IPBserializeNotification(buffer, MAX_PACKET_SIZE, &len, code, count);
    if(res != IPB_OK) {
		LOG_V("[PARSER] Couldn't serialize notification for %s: %s (Code %d)", targetIp, IPBstatusToString(res), res);
    }
	if (len != 3) {
		LOG_V("[WARN] Serializer returned len %d for notification, forcing 3", len);
		len = 3;
	}
	
	IPBsendUDP(globalUdpSocket, buffer, len, &targetAddr);
	LOG_V("[NET] Notified %s:%d (Enum %d -> Sent)", targetId, targetPort, code);
}


bool handleFriendReq(int clientSock, const char* currentUserId, const IPBpacket* request) {
	LOG_V("[CMD] Friend Req: %s -> %s", currentUserId, request->targetId);
	IPBpacket response;
	
	if(IPBdataAreFriends(currentUserId, request->targetId)){
		LOG_V("[CMD] Friend Req: %s and %s are already friends, rejected.", currentUserId, request->targetId);
		response.type = MSG_RSP_FRIEND_UNKNOWN;
		SEND_OR_LOG(clientSock, &response);
		
		return true;
	}
	
	IPBpacket streamPacket; // for internal stream creation
	streamPacket.type = MSG_STR_FRIEND_REQ;
	strncpy(streamPacket.id, currentUserId, ID_BYTES + 1);
	
	IPBstatus res = IPBdataAddStream(request->targetId, &streamPacket);
	
	if (res == IPB_OK) notifyUser(request->targetId, CODE_FRIEND_REQ_IN);
	
	response.type = (res == IPB_OK) ? MSG_RSP_FRIEND_SENT : MSG_RSP_FRIEND_UNKNOWN;
	SEND_OR_LOG(clientSock, &response);
	
	return true;
}

bool handleMessage(int clientSock, const char* currentUserId, const IPBpacket* request) {
	LOG_V("[CMD] Message: %s -> %s", currentUserId, request->targetId);
    IPBpacket response;
	
	if(!IPBdataAreFriends(currentUserId, request->targetId)){
		LOG_V("[CMD] Message: %s and %s are not friends, rejected.", currentUserId, request->targetId);
		response.type = MSG_RSP_MSG_FAIL;
		SEND_OR_LOG(clientSock, &response);
		
		return true;
	}
	
	IPBpacket streamPacket; // for internal stream creation
	streamPacket.type = MSG_STR_MESSAGE;
	strncpy(streamPacket.id, currentUserId, ID_BYTES + 1);
	strncpy(streamPacket.message, request->message, MAX_MESSAGE_LENGTH + 1);
	
	IPBstatus res = IPBdataAddStream(request->targetId, &streamPacket);
	
	if (res == IPB_OK) notifyUser(request->targetId, CODE_MESSAGE_IN);
	
	response.type = (res == IPB_OK) ? MSG_RSP_MSG_SENT : MSG_RSP_MSG_FAIL;
	SEND_OR_LOG(clientSock, &response);
	
	return true;
}

bool handleFlood(int clientSock, const char* currentUserId, const IPBpacket* request) {
	LOG_V("[CMD] Flood from: %s", currentUserId);
	IPBpacket response;
	char targets[MAX_CLIENTS][ID_BYTES + 1];
	int count = 0;
	
	IPBdataGetFloodTargets(currentUserId, targets, &count);
	
	IPBpacket streamPacket; // for internal stream creation
	streamPacket.type = MSG_STR_FLOOD;
	strncpy(streamPacket.id, currentUserId, ID_BYTES + 1);
	strncpy(streamPacket.message, request->message, MAX_MESSAGE_LENGTH + 1);
	
	for(int i=0; i<count; i++) {
		IPBdataAddStream(targets[i], &streamPacket);
		notifyUser(targets[i], CODE_FLOOD_IN);
	}
	
	response.type = MSG_RSP_FLOOD_SENT;
	SEND_OR_LOG(clientSock, &response);
	
	return true;
}

bool handleList(int clientSock, const char* currentUserId) {
	LOG_V("[CMD] Fetch list: %s", currentUserId);
	IPBpacket response;
	char users[MAX_CLIENTS][ID_BYTES + 1];
	int userCount = 0;
	
	IPBdataGetUsers(users, &userCount);
	
	response.type = MSG_RSP_LIST_HEAD;
	response.numItems = userCount;
	SEND_OR_LOG(clientSock, &response);
	
	for(int i=0; i<userCount; i++){
		response.type = MSG_RSP_LIST_ITEM;
		strncpy(response.id, users[i], ID_BYTES + 1);
		SEND_OR_LOG(clientSock, &response);
	}
	
	return true;
}

bool handleConsult(int clientSock, const char* currentUserId) {
	LOG_V("[CMD] Fetch stream: %s", currentUserId);
	IPBpacket response;
	IPBpacket streamPacket; // for internal stream creation
	
	IPBstatus res = IPBdataPopStream(currentUserId, &streamPacket);
	if (res != IPB_OK) {
		response.type = MSG_STR_NO_CONTENT;
		SEND_OR_LOG(clientSock, &response);
		
		return true;
	}
	
	// send stream
	SEND_OR_LOG(clientSock, &streamPacket);
	// if stream was a friend requestm, handle response:
	if (streamPacket.type == MSG_STR_FRIEND_REQ) {
		LOG_V("[SERVER] Waiting for Friend request response from %s regarding %s...", currentUserId, streamPacket.id);
		
		char ansBuffer[MAX_PACKET_SIZE];
		int ansLen = IPBreceiveStream(ansBuffer, MAX_PACKET_SIZE, clientSock);
		if (ansLen <= 0) return false;

		IPBpacket answer;
		IPBstatus ansRes = IPBdeserialize(&answer, ansBuffer, ansLen);
		if ( ansRes != IPB_OK){
			LOG_V("[SERVER] Friend request response failed for %s: %s (Code %d)", currentUserId, IPBstatusToString(ansRes), ansRes);
			response.type = MSG_RSP_GOODBYE;
			SEND_OR_LOG(clientSock, &response);
			
			return true;
		}
		
		IPBpacket update;
		switch (answer.type) {
			case MSG_ANS_OK_FRIEND:
				LOG_V("[CMD] %s Accepted friendship with %s", currentUserId, streamPacket.id);
				
				IPBdataAddFriend(currentUserId, streamPacket.id);
				
				update.type = MSG_STR_FRIEND_ACC;
				strncpy(update.id, currentUserId, ID_BYTES + 1);
				
				IPBdataAddStream(streamPacket.id, &update);
				
				notifyUser(streamPacket.id, CODE_FRIEND_ACCEPTED);
				
				response.type = MSG_RSP_ACK;
			break;

			case MSG_ANS_NOK_FRIEND:
				LOG_V("[CMD] %s Rejected friendship with %s", currentUserId, streamPacket.id);
				
				notifyUser(streamPacket.id, CODE_FRIEND_REJECTED);
				
				update.type = MSG_STR_FRIEND_REJ;
				strncpy(update.id, currentUserId, ID_BYTES + 1);
				IPBdataAddStream(streamPacket.id, &update);
			
				response.type = MSG_RSP_ACK;
			break;
			
			default:
				LOG_V("[CMD] Unknown/Unexpected command during Friend request response: %s", currentUserId);
				response.type = MSG_RSP_GOODBYE;
			break;
		}
		SEND_OR_LOG(clientSock, &response);
	}
	return true;
}

bool clientCmdHandler(const int clientSock, const char* currentUserId){
	char rawBuffer[MAX_PACKET_SIZE];
	int len = IPBreceiveStream(rawBuffer, MAX_PACKET_SIZE, clientSock);
	if (len <= 0) return false;
	
	IPBpacket request;
	IPBpacket response; // used to simplify sendPacket arguments input
	IPBstatus res = IPBdeserialize(&request, rawBuffer, len);
	if (res != IPB_OK) {
		LOG_V("[SERVER] Command failed for %s: %s (Code %d)", currentUserId, IPBstatusToString(res), res);
		response.type = MSG_RSP_GOODBYE;
		SEND_OR_LOG(clientSock, &response);
		
		return true;
	}
	
	switch (request.type) {
		case MSG_CMD_FRIEND_REQ: 	return handleFriendReq(clientSock, currentUserId, &request);
		case MSG_CMD_MESSAGE: 		return handleMessage(clientSock, currentUserId, &request);
		case MSG_CMD_FLOOD: 		return handleFlood(clientSock, currentUserId, &request);
		case MSG_CMD_LIST:			return handleList(clientSock, currentUserId);
		case MSG_CMD_CONSULT: 		return handleConsult(clientSock, currentUserId);
		case MSG_CMD_DISCONNECT: 	break;
		
		default: 
			LOG_V("[CMD] Unknown/Unexpected command: %s", currentUserId); 
			response.type = MSG_RSP_GOODBYE;
			SEND_OR_LOG(clientSock, &response);
			
			return true;
		break;
	}
	
    response.type = MSG_RSP_GOODBYE;
	SEND_OR_LOG(clientSock, &response);
    return false;
}

bool clientAuthHandler(const int clientSock, char* outUserId){
	char rawBuffer[MAX_PACKET_SIZE];
	int len = IPBreceiveStream(rawBuffer, MAX_PACKET_SIZE, clientSock);
	if (len <= 0) return false; 
	
	IPBpacket request;
	IPBpacket response; // used to simplify sendPacket arguments input
	IPBstatus res = IPBdeserialize(&request, rawBuffer, len);
	if (res != IPB_OK) {
		LOG_V("[SERVER] Auth failed for %d: %s (Code %d)", clientSock, IPBstatusToString(res), res);
		response.type = MSG_RSP_GOODBYE;
		SEND_OR_LOG(clientSock, &response);
		
		return false;
	}
	
	switch (request.type) {
		case MSG_CMD_REGISTER:
			LOG_V("[CMD] Register: %s", request.id);
			
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);
			char clientIp[INET_ADDRSTRLEN];
			getpeername(clientSock, (struct sockaddr*)&addr, &len);
			inet_ntop(AF_INET, &addr.sin_addr, clientIp, INET_ADDRSTRLEN);
			
			res = IPBdataRegisterUser(request.id, request.port, request.pass, clientIp);
			if (res != IPB_OK){
				LOG_V("[DATA] Failed registering %s: %s (Code %d)", request.id, IPBstatusToString(res), res);
				break;
			}
			
			response.type = MSG_RSP_WELCOME;
			SEND_OR_LOG(clientSock, &response);
			
			strncpy(outUserId, request.id, ID_BYTES + 1); // +1 for '\0'
			
			return true;
		break;

		case MSG_CMD_CONNECT:
			LOG_V("[CMD] Login: %s", request.id);
			
			res = IPBdataCheckAuth(request.id, request.pass);
			if (res != IPB_OK){
				LOG_V("[DATA] Failed login %s: %s (Code %d)", request.id, IPBstatusToString(res), res);
				break;	
			}
			
			response.type = MSG_RSP_HELLO;
			SEND_OR_LOG(clientSock, &response);
			
			strncpy(outUserId, request.id, ID_BYTES + 1);
			
			return true;
		break;
		
		default:
			LOG_V("[CMD] Unknown/Unexpected Auth command from sock %d", clientSock);
		break;
	}
	
	response.type = MSG_RSP_GOODBYE;
    SEND_OR_LOG(clientSock, &response);
	
	return false;
}

void* clientHandler(void* arg) {
    int clientSock = *(int*)arg;
	
	char currentUserId[ID_BYTES + 1];
	
	LOG_V("[SERVER] Client connected on socket %d", clientSock);
	
	if(clientAuthHandler(clientSock, currentUserId)){
		LOG_V("[SERVER] %s authenticated successfully on socket %d.", currentUserId, clientSock);
		while (clientCmdHandler(clientSock, currentUserId));
	} else {
        LOG_V("[SERVER] Auth failed for socket %d.\n", clientSock);
    }
	
    close(clientSock);
	LOG_V("[SERVER] User %s disconnected from socket %d.", currentUserId, clientSock);
    return NULL;
}


int main(int argc, char *argv[]) {
	
	// GET ARGUMENTS
	
	int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
			case 'v':
				verbose = true;
				break;
			default:
				fprintf(stderr, "Usage: %s [-v] <Port>\n", argv[0]);
				return 1;
        }
    }
	
	int remainingArgs = argc - optind;
	if (remainingArgs!=1) {
		fprintf(stderr, "Usage: %s [-v] <Port>\n", argv[0]);
		return 1;
	}	
	
	// INITIALIZE SERVER
	
    int port = atoi(argv[optind]);
    int masterSocket = IPBbindAndListenTCP(port);
    globalUdpSocket = IPBbindUDP(port);

    if (masterSocket == -1 || globalUdpSocket == -1) {
        fprintf(stderr, "[ERROR] Could not bind sockets on port %d\n", port);
        return 1;
    }

    printf("--- Server Ready on Port %d ---\n", port);
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);


	// MAIN LOOP
	
    while (1) {
        int clientSock = accept(masterSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock < 0) continue;
		
		int *newSock = malloc(sizeof(int)); // fix for race condition, passing clientSock directly risks it being overwritten before the caller can store it.
        *newSock = clientSock;
		
        pthread_t tid;
        if (pthread_create(&tid, NULL, clientHandler, (void*)newSock) != 0) {
            close(clientSock);
			free(newSock);
        } else {
            pthread_detach(tid);
        }
    }
	
    close(masterSocket);
    close(globalUdpSocket);
    return 0;
}