#include "IPBnetwork.h"
#include "IPBprotocol.h"
#include "IPBtypes.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int IPBconnectTCP(const char* ip, int port){
	int socketFD = socket(PF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) return -1;

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // ipv4
	serv_addr.sin_port = htons(port); // big endian formmated port

	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) { // binary formatted ip
		close(socketFD);
		return -1;
	}

	if (connect(socketFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		close(socketFD);
		return -1;
	}
	return socketFD;
}

int IPBbindUDP(int port){
	int socketFD = socket(PF_INET, SOCK_DGRAM, 0);
	if (socketFD < 0) return -1;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // ipv4
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // big endian formmated INADDR_ANY
	addr.sin_port = htons(port); // big endian formmated port

	if (bind(socketFD, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(socketFD);
		return -1;
	}
	return socketFD;
}

int IPBbindAndListenTCP(int port){
	int socketFD = socket(PF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) return -1;

	int opt = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { // SOL_SOCKET modify socket option, SO_REUSEADDR reuse same address after restart.
		close(socketFD);
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // ipv4
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // big endian formmated INADDR_ANY
	addr.sin_port = htons(port); // big endian formmated port

	if (bind(socketFD, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(socketFD);
		return -1;
	}

	if (listen(socketFD, 0) < 0) { // start listening for incoming tcp connections
		close(socketFD);
		return -1;
	}
	return socketFD;
}

void IPBclose(int socketFD){
	if (socketFD >= 0) {
		close(socketFD);
	}
}


bool IPBsendRaw(const int socketFD, const char* buffer, const size_t bufferSize){
	size_t total_sent = 0;
	
	while (total_sent < bufferSize) {
		ssize_t n = write(socketFD, buffer + total_sent, bufferSize - total_sent);
		if (n <= 0) return false;
		total_sent += n;
	}
	return true;
}

bool IPBsendUDP(const int socketFD, const char* buffer, const size_t len, const struct sockaddr_in* clientaddr){
	ssize_t n = sendto(socketFD, buffer, len, 0, (struct sockaddr*)clientaddr, sizeof(*clientaddr));
	return (n == (ssize_t)len);
}

int IPBreceiveStream(char* outBuffer, const size_t bufferSize, const int socketFD){
	size_t totalRead = 0;
	char c;

	while (totalRead < bufferSize) {
		ssize_t n = read(socketFD, &c, 1);
		
		if (n == 0) return 0;
		if (n < 0){
			if (errno == EINTR) continue; // signal interruption, keep reading
			return -1;
		}

		outBuffer[totalRead++] = c;
		
		// check if we reached MSG_TERMINATOR
		if (totalRead >= MSG_TERMINATOR_BYTES) {
			if (memcmp(outBuffer + totalRead - MSG_TERMINATOR_BYTES, MSG_TERMINATOR, MSG_TERMINATOR_BYTES) == 0) {
				return totalRead;
			}
		}
	}
	return -1;
}

int IPBreceiveDatagram(char* outBuffer, const size_t bufferSize, const int socketFD){
	ssize_t n = recvfrom(socketFD, outBuffer, bufferSize, 0, NULL, NULL); // doesn't metter who sent it.
	
	if (n >= 0 && n < (ssize_t)bufferSize) {
		return n;
	}
	return -1;
}