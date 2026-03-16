#include "IPBprotocol.h"
#include "IPBtypes.h"

#include <pthread.h>
#include <stdbool.h>
#include <string.h>

# define MAX_STREAMS 255

typedef struct {
	char userId[ID_BYTES + 1]; // + '\0'
	int pass;
	int udpPort;
	char ip[15]; // max ip string length
	
	// Adjacency matrix, if friends[i] is true, i is a friend.
	bool isFriend[MAX_CLIENTS];

	IPBpacket streamsQueue[MAX_STREAMS];
	unsigned int queueHead;
	unsigned int queueTail;
	unsigned int queueCount;
	

} UserData;


static UserData users[MAX_CLIENTS];
static unsigned int userCount = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


static int findUser(const char* id){
	unsigned int i = 0;
	for(i = 0; i<userCount; i++){
		if(strcmp(users[i].userId, id) == 0) return i;
	}
	
	return -1;
}

IPBstatus IPBdataRegisterUser(const char* id, const int udpPort, const int pass, char* ip){
	pthread_mutex_lock(&mutex);
	
	if (userCount >= MAX_CLIENTS){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_MAX_REACHED;
	}
	
	if(findUser(id) != -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_ALREADY_REGISTERED;
	}
	
	strcpy(users[userCount].userId, id);
	users[userCount].pass = pass;
	users[userCount].udpPort = udpPort;
	strcpy(users[userCount].ip, ip);
	users[userCount].queueHead = 0;
	users[userCount].queueTail = 0;
	users[userCount].queueCount = 0;
	
	userCount++;
	
	pthread_mutex_unlock(&mutex);
	
	
	return IPB_OK;
}


IPBstatus IPBdataCheckAuth(const char* id, const int pass){
	pthread_mutex_lock(&mutex);
	
	int userIndex = findUser(id);
	if (userIndex == -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}
	if(users[userIndex].pass == pass){
		pthread_mutex_unlock(&mutex);
		return IPB_OK;
	}
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_ERROR_PASS_MISMATCH;
}

IPBstatus IPBdataAddFriend(const char* sourceId, const char* targetId){
	pthread_mutex_lock(&mutex);
	
	int srcUserIndex = findUser(sourceId);
	int dstUserIndex = findUser(targetId);
	if(dstUserIndex == -1 || srcUserIndex == -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}
	
	if(users[srcUserIndex].isFriend[dstUserIndex]){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_ALREADY_FRIENDS;
	} else{
		users[srcUserIndex].isFriend[dstUserIndex] = true; // symmetric adjacency matrix
		users[dstUserIndex].isFriend[srcUserIndex] = true;
	}
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_OK;
}

bool IPBdataAreFriends(const char* firstId, const char* secondId){
		pthread_mutex_lock(&mutex);
		
		if(strcmp(firstId, secondId) == 0){
			pthread_mutex_unlock(&mutex);
			return true;
		}
		int firstUserIndex = findUser(firstId);
		int secondUserIndex = findUser(secondId);
		if(firstUserIndex == -1 || secondUserIndex == -1){ 
			pthread_mutex_unlock(&mutex);	
			return false;
		}
		bool res = users[firstUserIndex].isFriend[secondUserIndex];
		
		pthread_mutex_unlock(&mutex);
		
		return res;
}

IPBstatus IPBdataAddStream(const char* id, const IPBpacket* stream){
	pthread_mutex_lock(&mutex);

	int userIndex = findUser(id);
	if(userIndex == -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}

	users[userIndex].streamsQueue[users[userIndex].queueHead] = *stream;
	users[userIndex].queueHead = (users[userIndex].queueHead + 1) % MAX_STREAMS;


	if (users[userIndex].queueCount < MAX_STREAMS) {
		users[userIndex].queueCount++;
	} else{
		users[userIndex].queueTail = (users[userIndex].queueTail + 1) % MAX_STREAMS; // overwrite tail.
	}

	pthread_mutex_unlock(&mutex);

	return IPB_OK;
}

int IPBdataGetStreamCount(const char* id){
	pthread_mutex_lock(&mutex);
	
	int userIndex = findUser(id);
	if(userIndex == -1){ 
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	int res = users[userIndex].queueCount;
	
	pthread_mutex_unlock(&mutex);
	
	return res;
}

IPBstatus IPBdataPopStream(const char* id, IPBpacket* outPacket){
	pthread_mutex_lock(&mutex);
	
	int userIndex = findUser(id);
	if(userIndex == -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}
	
	if(users[userIndex].queueCount<=0){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_NO_STREAMS;
	}
	
	*outPacket = users[userIndex].streamsQueue[users[userIndex].queueTail];
	users[userIndex].queueTail = (users[userIndex].queueTail + 1) % MAX_STREAMS;
	users[userIndex].queueCount--;
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_OK;
}

IPBstatus IPBdataGetUserNotifyAddr(const char* id, int* outUdpPort, char* outIp){
	pthread_mutex_lock(&mutex);
	
	int userIndex = findUser(id);
	if(userIndex == -1){ 
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}
	
	*outUdpPort = users[userIndex].udpPort;
	strcpy(outIp, users[userIndex].ip);
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_OK;
}

void floodTargetsHelper(char outTargetList[][ID_BYTES + 1], int* outCount, const unsigned int currentIndex, bool visited[]){
	visited[currentIndex] = true;
	
	unsigned int i = 0;
	for(i=0; i<userCount; ++i){
		if (users[currentIndex].isFriend[i] && !visited[i]){
			strcpy(outTargetList[*outCount], users[i].userId);
			(*outCount)++;
			floodTargetsHelper(outTargetList, outCount, i, visited);
		}
	}

	return;
}

IPBstatus IPBdataGetFloodTargets(const char* senderId, char outTargetList[][ID_BYTES + 1], int* outCount){
	pthread_mutex_lock(&mutex);
	
	int startIndex = findUser(senderId);
	if (startIndex == -1){
		pthread_mutex_unlock(&mutex);
		return IPB_ERROR_USER_NOT_FOUND;
	}
	
	bool visited[MAX_CLIENTS];
	memset(visited, 0, sizeof(visited));
	visited[startIndex] = true; // ignore sender
	*outCount = 0;
	
	floodTargetsHelper(outTargetList, outCount, startIndex, visited);
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_OK;
}

IPBstatus IPBdataGetUsers(char outUsers[][ID_BYTES + 1], int* outCount){
	pthread_mutex_lock(&mutex);
	
	unsigned int i = 0;
	for(i=0; i<userCount; ++i){
		strcpy(outUsers[*outCount], users[i].userId);
		(*outCount)++;
	}
	
	pthread_mutex_unlock(&mutex);
	
	return IPB_OK;
}