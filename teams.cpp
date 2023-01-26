#include "teams.h"

std::string gen_random(const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}

int player::IDcnt = 0;

player::player() {
	ID = IDcnt;
	IDcnt++;
	doCue = false;
	name = gen_random(5);
	tableID = -1;
}

std::string player::PackagePlayer() {
	std::stringstream ss;
	ss << IDcnt << " " << ID << " " << doCue << " " << name << " " << tableID;
	return ss.str();
}

void player::UnpackPlayer(std::string _str) {
	std::vector<std::string> playerVector = splitstr(_str, " ");
	IDcnt = std::stoi(playerVector[0]);
	ID = std::stoi(playerVector[1]);
	doCue = (playerVector[2] == "1");
	name = playerVector[3];
	tableID = std::stoi(playerVector[4]);
}

std::vector<player*> team::activePlayers = {};

team::team() {
	name = gen_random(5);
	players = {};
	activePlayers = {};
	for (int i = 0; i < 3; i++) {
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour(i) = r;
	}
}

void team::AddPlayer(player* _player) {
	players.push_back(_player);
	activePlayers.push_back(_player);
}

void team::RemovePlayer(player* _player) {
	for (int i = activePlayers.size() - 1; i >= 0; i--) {
		if (players[i] == _player) {
			players.erase(players.begin() + i);
		}
	}

	for (int i = activePlayers.size() - 1; i >= 0; i--) {
		if (activePlayers[i] == _player) {
			activePlayers.erase(activePlayers.begin() + i);
		}
	}
}

void team::DeletePlayer(player* _player) {
	for (int i = players.size() - 1; i >= 0; i--) {
		if (players[i] == _player) {
			players.erase(players.begin() + i);
		}
	}

	for (int i = activePlayers.size() - 1; i >= 0; i--) {
		if (activePlayers[i] == _player) {
			activePlayers.erase(activePlayers.begin() + i);
		}
	}

	delete _player;
}

std::string team::PackageTeam() {
	std::stringstream ss;
	ss << name << "," << colour(0) << "," << colour(1) << "," << colour(2) << "," << players.size() << ",";
	for (int i = 0; i < players.size(); i++) {
		ss << players[i]->PackagePlayer() << ",";
	}
	ss << currentScore;
	return ss.str();
}

void team::UnpackTeam(std::string _str) {

	//clear all players from memory before reseting values
	for (int i = 0; i < players.size(); i++) {

		delete players[i];
	}
	players.clear();


	int i = 0;
	std::vector<std::string> teamVector = splitstr(_str, ",");
	name = teamVector[0];
	colour = vec3(std::stof(teamVector[1]), std::stof(teamVector[2]), std::stof(teamVector[3]));
	for (i = 5; i < 5 + std::stoi(teamVector[4]); i++) {
		player newPlayer = player();
		newPlayer.UnpackPlayer(teamVector[i]);
		players.push_back(&newPlayer);
	}
	currentScore = std::stoi(teamVector[i]);
}



std::vector<std::string> splitstr(std::string str, std::string deli)
{
	std::vector<std::string> returnValue;
	int start = 0;
	int end = str.find(deli);
	while (end != -1) {
		returnValue.push_back(str.substr(start, end - start));
		start = end + deli.size();
		end = str.find(deli, start);
	}
	returnValue.push_back(str.substr(start, str.size()));
	return returnValue;
}