#pragma once
#include"vecmath.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class player {
	static int IDcnt;
private:
	int ID;
public:
	bool doCue;
	std::string name;
	int tableID;

	player();
	std::string PackagePlayer(void);
	void UnpackPlayer(std::string);
};

class team {
public:
	static std::vector<player*> activePlayers; //vector of players to prevent duplication of players
	std::string name;
	vec3 colour;
	std::vector<player*> players;
	int currentScore = 0;

	team();
	void AddPlayer(player*);
	void RemovePlayer(player*);
	void DeletePlayer(player*);
	std::string PackageTeam(void);
	void UnpackTeam(std::string);
};

std::vector<std::string> splitstr(std::string str, std::string deli = " ");
