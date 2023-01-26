#include "threaded_client.h"

void Client::start(const char* ipaddress, const char* port) {
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ipaddress, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	printf("Connected!\n");
}

void Client::sendthis(const char* buffer) {
	iResult = send(ConnectSocket, buffer, strlen(buffer), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		terminate();
		return;
	}
}

const char* Client::getthis() {
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) return recvbuf;
}

void Client::sendmsg() {
	while (true) {
		printf("Client: ");
		fgets(recvbuf, 255, stdin);
		iResult = send(ConnectSocket, recvbuf, sizeof(recvbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			terminate();
			return;
		}
	}
}

void Client::getmsg() {
	while (true) {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, NULL);
		if (iResult > 0)
			handlemsg(recvbuf);
	}
}

void Client::interact() {
	std::thread t(&Client::getmsg, this);
	//sendmsg();
	t.detach();
}

void Client::terminate() {
	closesocket(ConnectSocket);
	WSACleanup();
}

void Client::handlemsg(std::string _msg) {

	if (std::string(_msg).substr(0, 6) == "Config") {
		if (gm.setUp == true) {
			sendthis(gm.PackageGM().c_str());
		}
		else {
			printf("Create Config");
			gm.AutoGenerateTeams(2);
			gm.GenerateTables(5);
			gm.tables[0].teams[gm.teams[0]].push_back(0);
			gm.tables[0].teams[gm.teams[1]].push_back(0);
			gm.AutoAddPlayer(localPlayer);
			gm.tables[0].SetupOrder();
			gm.tables[0].AddStone();
			gm.setUp = true;
		}
	}
	else if (_msg[0] == 'C') {
		gm.UnpackGM(_msg);
		team _localTeam = gm.AutoAddPlayer(localPlayer);
		if (gm.setUp == false) {
			gm.setUp == true;
			sendAddNewPlayer(localPlayer, _localTeam.name);
		}
	}
	else if (_msg[0] == 'I') {
		_msg = splitstr(_msg, "$")[0];
		std::vector<std::string> splitMsg = splitstr(_msg, ":");
		vec2 imp = vec2(std::stof(splitMsg[1]), std::stof(splitMsg[2]));
		int tabIndex = std::stoi(splitMsg[3]);
		gm.tables[tabIndex].stones[gm.tables[tabIndex].stoneCount - 1].ApplyImpulse(imp);
	}
	else if (_msg[0] == 'A') {
		_msg = splitstr(_msg, "$")[0];
		std::vector<std::string> splitMsg = splitstr(_msg, ":");
		gm.tables[std::stoi(splitMsg[1])].CheckStones();
		gm.tables[std::stoi(splitMsg[1])].AddStone();
	}
	else if (_msg[0] == 'P') {
		_msg = splitstr(_msg, "$")[0];
		std::vector<std::string> splitMsg = splitstr(_msg, ":");
		player* newPlayer = new player();
		newPlayer->UnpackPlayer(splitMsg[1]);
		for (int i = 0; i < gm.teams.size(); i++) {
			if (gm.teams[i].name == splitMsg[2]) {
				gm.teams[i].AddPlayer(newPlayer);
			}
		}
	}
}

void Client::sendImpulse(vec2 _imp, int _tableInd) {
	std::stringstream ss;
	ss << "I:" << _imp(0) << ":" << _imp(1) << ":" << _tableInd << "$";
	sendthis(ss.str().c_str());
}

void Client::sendAddStone(int _tab) {
	std::stringstream ss;
	ss << "A:" << _tab << "$";
	sendthis(ss.str().c_str());
}

void Client::sendAddNewPlayer(player* _player, std::string _teamName) {
	std::stringstream ss;
	ss << "P:" << _player->PackagePlayer() << ":" << _teamName << "$";
	sendthis(ss.str().c_str());
}



/*int main(int argc, const char* argv[]) {
	if (argc > 3) {
		printf("Usage: %s hotname port\n", argv[0]);

	}
	else {
		Client client;
		std::string _port;
		std::cout << "Port:";
		std::cin >> _port;
		std::cout << "Conecting to localhost port " << _port << std::endl;
		client.start("localhost", _port.c_str());
		client.interact();
	}

	return 0;
}*/