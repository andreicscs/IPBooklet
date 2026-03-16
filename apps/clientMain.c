#include "IPBclientAPI.h"
#include "IPBtypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>


#define LOG_V(...) do { \
		if(verbose) { printf("[VERBOSE] " __VA_ARGS__); printf("\n"); } \
	} while(0)

#define INPUT_BUFFER_SIZE 1024

bool verbose = false;

void* notificationListener(void* arg) {
    IPBclientSession* session = (IPBclientSession*)arg;
    IPBpacket packet;
    IPBstatus res;

    while (1) {
        res = IPBlistenNotifications(&packet, session); 

        if ( res != IPB_OK) {
			switch(res){
				case IPB_ERROR_RECV_FAILED:
					fprintf(stderr, "[ERROR] Receive notification failed.\n");
				break;
				case IPB_ERROR_MALFORMED_RESPONSE:
					fprintf(stderr, "[ERROR] Received a malformed notification.\n");
				break;
				default:
					fprintf(stderr, "[ERROR] Unexpected error.\n");
				break;
			}
			
			return NULL;
		}
		switch(packet.streamCode){
			case CODE_FRIEND_REQ_IN:
				printf("\n[NOTIFICATION] New friend request!\n");
			break;
			case CODE_FRIEND_ACCEPTED:
				printf("\n[NOTIFICATION] Friend request got accepted.\n");
			break;
			case CODE_FRIEND_REJECTED:
				printf("\n[NOTIFICATION] Friend request got rejected.\n");
			break;
			case CODE_MESSAGE_IN:
				printf("\n[NOTIFICATION] New message!\n");
			break;
			case CODE_FLOOD_IN:
				printf("\n[NOTIFICATION] New flood!\n");
			break;
			default:
				fprintf(stderr, "[ERROR] Unexpected notification type.\n");
			break;
		}
    }
    return NULL;
}


void printAuthHelp() {
    printf("\n--- IPB Authentication ---\n");
    printf(" [0] Show this menu\n");
	printf(" [1] Quit\n");
	
    printf(" [2] Register\n");
    printf(" [3] Login\n");
    printf("-----------------------\n");
}

void printCmdHelp() {
    printf("\n--- IPB Client Menu ---\n");
    printf(" [0] Show this menu\n");
	printf(" [1] Quit\n");
	
    printf(" [2] Send Friend Request\n");
    printf(" [3] Send Message\n");
    printf(" [4] Send Flood (All friends)\n");
    printf(" [5] List Users\n");
    printf(" [6] Consult Notifications (Streams)\n");
    printf("-----------------------\n");
}

void printFriendResponseHelp() {
    printf("\n--- IPB Friend Response Menu ---\n");
    printf(" [0] Show this menu\n");
	printf(" [1] Quit\n");
	
    printf(" [2] Accept\n");
    printf(" [3] Reject\n");
    printf("-----------------------\n");
}

void getConsoleInput(char* buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        char *newline = strchr(buffer, '\n');
		if (newline) {
            *newline = '\0';
        } else {
            // clear stdin from stray \n in case size wasn't big enough to store \n and it was left inside the buffer
			int c;
			while ((c = getchar()) != '\n' && c != EOF);
		}
    }
}

void cmdAcceptFriend(IPBclientSession* session){
	IPBstatus res = IPBacceptFriendRequest(session);
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Accept Friend failed: %s (Code %d)\n", IPBstatusToString(res), res);
	}
}

void cmdRejectFriend(IPBclientSession* session){
	IPBstatus res = IPBrejectFriendRequest(session);
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Reject Friend failed: %s (Code %d)\n", IPBstatusToString(res), res);
	}
}


void cmdConsult(IPBclientSession* session) {
	IPBpacket responsePacket;
	
	LOG_V("Fetching streams...");
	bool fetching = true;
	IPBstatus res;

	while(fetching) {
		res = IPBgetStream(&responsePacket, session);
		
		if (res != IPB_OK) {
			fprintf(stderr, "[ERROR] Fetch stream failed: %s (Code %d)\n", IPBstatusToString(res), res);
			fetching = false; 
			break;
		}

		switch(responsePacket.type){
			case MSG_STR_MESSAGE:
				printf("[MSG] From %s: %s\n", responsePacket.id, responsePacket.message);
			break;
				
			case MSG_STR_FLOOD:
				printf("[FLOOD] From %s: %s\n", responsePacket.id, responsePacket.message);
			break;

			case MSG_STR_FRIEND_REQ:
				printf("\n[!!!] FRIEND REQUEST from %s\n", responsePacket.id);
				printf("(!) Automatic fetching stopped, Accept [2] or Reject [3], [0] for help.\n");
				printf("> ");
				
				char inputBuffer[INPUT_BUFFER_SIZE];
				int cmd;
				
				
				
				bool didRespond = false;
				do{
					getConsoleInput(inputBuffer, sizeof(inputBuffer));
					if(strlen(inputBuffer) == 0) continue;
					cmd = atoi(inputBuffer);
					
					switch(cmd){
						case 0: printFriendResponseHelp(); break;
						case 1: IPBdisconnect(session); IPBcloseClientNetwork(session); break;
						case 2: cmdAcceptFriend(session); didRespond = true; break;
						case 3: cmdRejectFriend(session); didRespond = true; break;
						default: printf("Unknown command, type '0' for help.\n"); break;
					}
				}while(!didRespond);
			break;
			
			case MSG_STR_FRIEND_ACC:
				printf("[FRIEND] %s Accepted your friend request!\n", responsePacket.id);
			break;
			case MSG_STR_FRIEND_REJ:
				printf("[FRIEND] %s Rejected your friend request.\n", responsePacket.id);
			break;
			
			case MSG_STR_NO_CONTENT:
				printf("--- No more notifications ---\n");
				fetching = false;
			break;
				
			default:
				printf("UNKNOWN TYPE (Code %d)\n", responsePacket.streamCode);
				fetching = false;
			break;
		}
	}
}

bool cmdRegister(IPBclientSession* session, int udpPort, char* serverIp, int serverPort) {
    char id[INPUT_BUFFER_SIZE];
    char passStr[INPUT_BUFFER_SIZE];
    
    printf("Enter ID to register: ");
    getConsoleInput(id, sizeof(id));
    
    printf("Enter numeric Password: ");
    getConsoleInput(passStr, sizeof(passStr));
    
    LOG_V("Registering %s on UDP port %d...", id, udpPort);
    IPBstatus res = IPBregister(session, id, udpPort, atoi(passStr));
	if( res == IPB_ERROR_SERVER_REJECTED){
		LOG_V("Server closed the connection, trying to reconnect once.\n");
		printf("[INFO] Reconnecting to server...");
		res = IPBreconnectClientNetwork(session, serverIp, serverPort);
        if(res != IPB_OK){
			fprintf(stderr, "[ERROR] Reconnect failed: %s (Code %d)\n", IPBstatusToString(res), res);
		}
		return false;
	} else if(res != IPB_OK) {
		fprintf(stderr, "[ERROR] Registration failed: %s (Code %d)\n", IPBstatusToString(res), res);
		return false;
	}
	
	return true;
}

bool cmdLogin(IPBclientSession* session, char* serverIp, int serverPort) {
    char id[INPUT_BUFFER_SIZE];
    char passStr[INPUT_BUFFER_SIZE];

    printf("Enter ID: ");
    getConsoleInput(id, sizeof(id));
    
    printf("Enter numeric Password: ");
    getConsoleInput(passStr, sizeof(passStr));
    
    LOG_V("Logging in as %s...", id);
	IPBstatus res = IPBconnect(session, id, atoi(passStr));
	if( res == IPB_ERROR_SERVER_REJECTED){
		LOG_V("Server closed the connection, trying to reconnect once.");
		printf("[INFO] Reconnecting to server...\n");
		res = IPBreconnectClientNetwork(session, serverIp, serverPort);
        if(res != IPB_OK){
			fprintf(stderr, "[ERROR] Reconnect failed: %s (Code %d)\n", IPBstatusToString(res), res);
		}
		return false;
	} else if(res != IPB_OK) {
		fprintf(stderr, "[ERROR] Login failed: %s (Code %d)\n", IPBstatusToString(res), res);
		return false;
	}
	
	return true;
}

void cmdFriendRequest(IPBclientSession* session) {
    char target[INPUT_BUFFER_SIZE];
    printf("Enter Target ID: ");
    getConsoleInput(target, sizeof(target));
    
	IPBstatus res = IPBsendFriendRequest(session, target);
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Friend request failed: %s (Code %d)\n", IPBstatusToString(res), res);
	}
}

void cmdSendMessage(IPBclientSession* session) {
    char target[INPUT_BUFFER_SIZE];
    char msg[INPUT_BUFFER_SIZE];

    printf("Enter Target ID: ");
    getConsoleInput(target, sizeof(target));
    printf("Enter Message: ");
    getConsoleInput(msg, sizeof(msg));
    
	IPBstatus res = IPBsendMessage(session, target, msg);
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Send message failed: %s (Code %d)\n", IPBstatusToString(res), res);
	}
}

void cmdFlood(IPBclientSession* session) {
    char msg[INPUT_BUFFER_SIZE];
    printf("Enter Flood Message: ");
    getConsoleInput(msg, sizeof(msg));
    
	IPBstatus res = IPBflood(session, msg);
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Flood failed: %s (Code %d)\n", IPBstatusToString(res), res);
	}
}

void cmdListUsers(IPBclientSession* session) {
    IPBpacket responsePacket;
    LOG_V("Fetching user list...");
	IPBstatus res = IPBgetClientList(&responsePacket, session);
	
	if(res != IPB_OK){
		fprintf(stderr, "[ERROR] Fetch user list head failed: %s (Code %d)\n", IPBstatusToString(res), res);
		return;
	}


	int numItems = responsePacket.numItems;
	LOG_V("[CMD] Received client list header, preparing to receive %d list items...", numItems);
	
	int i = 0;
	for(i=0; i<numItems; ++i){
		LOG_V("[CMD] Fetching next client list item command.");
		res = IPBlistenMessages(&responsePacket, session);
		if (res == IPB_OK){
			printf("%s\n", responsePacket.id);
		} else {
			fprintf(stderr, "[ERROR] Fetch user list item failed: %s (Code %d)\n", IPBstatusToString(res), res);
		}
	}
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
				fprintf(stderr, "Usage: %s [-v] <ServerIP> <ServerPort> <LocalUDPPort>\n", argv[0]);
				return 1;
        }
    }
	
	int remainingArgs = argc - optind;
	if (remainingArgs!=3) { // 3 arguments, server ip, server port, local udp port.
		fprintf(stderr, "Usage: %s [-v] <ServerIP> <ServerPort> <LocalUDPPort>\n", argv[0]);
		return 1;
	}	

	// INITIALIZE CLIENT

	char* serverIp = argv[optind];
	int serverPort = atoi(argv[optind + 1]);
	int udpPort = atoi(argv[optind + 2]);
	
	IPBclientSession session;
	LOG_V("[INIT] Connecting to %s:%d...", serverIp, serverPort);
    
	IPBstatus res = IPBinitClientNetwork(&session, serverIp, serverPort, udpPort);
	if ( res != IPB_OK) {
		fprintf(stderr, "[ERROR] Initialization failed: %s (Code %d)\n", IPBstatusToString(res), res);
		return 1;
	}
	
    LOG_V("[INIT] Connection established.");
	
	pthread_t tid;
    if (pthread_create(&tid, NULL, notificationListener, (void*)&session) != 0) {
        perror("[ERROR] Thread initialization failed.\n");
		IPBcloseClientNetwork(&session);
        return 1;
    }
	LOG_V("[INIT] Listening for notifications on port %d", udpPort);
	
	
	
	
	// AUTH LOOP
	char inputBuffer[INPUT_BUFFER_SIZE];
	int cmd;
	bool authenticated = false;
	printf("\nPlease Authenticate. type [0] for commands.\n");
	do{
		printf("> ");
		getConsoleInput(inputBuffer, sizeof(inputBuffer));
		if(strlen(inputBuffer) == 0) continue;
		cmd = atoi(inputBuffer);
		
		switch(cmd){
			case 0: printAuthHelp(); break;
			case 1: IPBdisconnect(&session); IPBcloseClientNetwork(&session); return 0;
			case 2: authenticated = cmdRegister(&session, udpPort, serverIp, serverPort); break;
			case 3: authenticated = cmdLogin(&session, serverIp, serverPort); break;
			default: printf("Unknown command, type [0] for help.\n"); break;
		}
	}while(!authenticated);
	
	// CMD LOOP
	bool running = true;
	
	printf("\nClient ready! type [0] for commands.\n");
	do{
		printf("> ");
		getConsoleInput(inputBuffer, sizeof(inputBuffer));
		if(strlen(inputBuffer) == 0) continue;
        cmd = atoi(inputBuffer);
		
		switch(cmd){
			case 0: printCmdHelp(); break;
			case 1: IPBdisconnect(&session); running = false; IPBcloseClientNetwork(&session); return 0;
            case 2: cmdFriendRequest(&session); break;
            case 3: cmdSendMessage(&session); break;
            case 4: cmdFlood(&session); break;
            case 5: cmdListUsers(&session); break;
            case 6: cmdConsult(&session); break;
			default: printf("Unknown command, type [0] for help.\n"); break;
		}
		
	}while(running);
	
	IPBcloseClientNetwork(&session);
	
    return 0;
}