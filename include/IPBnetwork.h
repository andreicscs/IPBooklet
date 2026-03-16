#ifndef IPBNETWORK_H
#define IPBNETWORK_H

// This library manages low level socket operations and raw data transmission.

#include <stdbool.h>
#include <stddef.h>
#include <netinet/in.h>

/**
* @brief creates a TCP socket and connects it to the specified server address.
*
* @param ip: the IP address string.
* @param port: the TCP port to connect to.
* @return int: the file descriptor of the connected socket, or -1 if an error occurred.
* @post a socket is created and connected.
*/
int IPBconnectTCP(const char* ip, int port);

/**
* @brief creates a UDP socket and binds it to the specified local port.
*
* @param port: the local port to bind.
* @return int: the file descriptor of the bound socket, or -1 if an error occurred.
* @post a socket is created and bound to the port.
*/
int IPBbindUDP(int port);

/**
* @brief creates a TCP socket, binds it to the specified port and sets it to listen.
*
* @param port: the local port to bind.
* @return int: the file descriptor of the listening socket, or -1 if an error occurred.
* @post a socket is created, bound, and listening for connections, the user has to accept() incoming connection requests.
*/
int IPBbindAndListenTCP(int port);

/**
* @brief closes the specified socket file descriptor.
*
* @param socketFD: the file descriptor to close.
* @post the socket is closed if it was valid.
*/
void IPBclose(int socketFD);

/**
* @brief sends raw bytes over a socket.
*
* @param socketFD: the file descriptor of the socket.
* @param buffer: the buffer containing the data to send.
* @param len: the number of bytes to send.
* @return bool: true if all bytes were sent successfully, false if a socket error occurred.
*/
bool IPBsendRaw(int socketFD, const char* buffer, size_t len);

/**
* @brief sends a raw UDP packet to socketFD.
*
* @param socketFD: the file descriptor of the UDP socket.
* @param buffer: the buffer containing the data to send.
* @param len: the number of bytes to send.
* @param sockaddr_in: the destination IP address and udp port.
* @return bool: true if the packet was sent, false if an error occurred.
*/
bool IPBsendUDP(const int socketFD, const char* buffer, const size_t len, const struct sockaddr_in* clientaddr);

/**
* @brief reads from a TCP stream byte by byte until the protocol terminator is found.
*
* @param outBuffer: the buffer where read data will be stored.
* @param maxLen: the maximum size of the buffer.
* @param socketFD: the file descriptor of the socket.
* @return int: the total number of bytes read, 0 if connection closed, -1 if error or buffer full.
* @post buffer contains the message ending with the terminator.
*/
int IPBreceiveStream(char* outBuffer, const size_t bufferSize, const int socketFD);

/**
* @brief reads a single UDP datagram from the socket.
*
* @param outBuffer: the buffer where read data will be stored.
* @param maxLen: the maximum size of the buffer.
* @param socketFD: the file descriptor of the socket.
* @return int: the number of bytes read, or -1 if an error occurred.
*/
int IPBreceiveDatagram(char* outBuffer, const size_t bufferSize, const int socketFD);

#endif