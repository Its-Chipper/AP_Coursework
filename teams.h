#pragma once
#include"vecmath.h"
#include <vector>
#include <string>

class player {
	static int IDcnt;
private:
	int ID;
public:
	bool doCue;
	std::string name;
	char ip;

	player();
};

class team {
	static std::vector<player*> activePlayers; //vector of players to prevent duplication of players
public:
	std::string name;
	vec3 colour;
	std::vector<player*> players;

	team();
	void AddPlayer(player*);
	void RemovePlayer(player*);
	void DeletePlayer(player*);
};
