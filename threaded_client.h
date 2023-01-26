#define WIN32_LEAN_AND_MEAN

#include "simulation.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 4096

class Client {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	std::string sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

public:
	void start(const char*, const char*);
	void sendthis(const char* buffer);
	const char* getthis();
	void sendmsg();
	void getmsg();
	void interact();
	void terminate();
	void handlemsg(std::string);
	void sendImpulse(vec2, int);
	void sendAddStone(int);
	void sendAddNewPlayer(player*, std::string);
};