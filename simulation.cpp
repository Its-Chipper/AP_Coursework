/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>


  /*-----------------------------------------------------------
	macros
	-----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

	/*-----------------------------------------------------------
	  globals
	  -----------------------------------------------------------*/
	  /*
	  vec2	gPlaneNormal_Left(1.0,0.0);
	  vec2	gPlaneNormal_Top(0.0,1.0);
	  vec2	gPlaneNormal_Right(-1.0,0.0);
	  vec2	gPlaneNormal_Bottom(0.0,-1.0);
	  */

//table gTable;

static const float gRackPositionX[] = { 0.0f,0.0f,(BALL_RADIUS * 2.0f),(-BALL_RADIUS * 2.0f),(BALL_RADIUS * 4.0f) };
static const float gRackPositionZ[] = { 0.5f,0.0f,(-BALL_RADIUS * 3.0f),(-BALL_RADIUS * 3.0f) };

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;

gameManager gm = gameManager();
player* localPlayer = new player();

/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1] - vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

std::string cushion::PackageCushion() {
	std::stringstream ss;
	ss << vertices[0](0) << " " << vertices[0](1) << " " << vertices[1](0) << " " << vertices[1](1) << " ";
	return ss.str();
}

void cushion::UnpackCushion(std::string _str) {
	std::vector<std::string> cushionVector = splitstr(_str, " ");

	vertices[0] = vec2(std::stof(cushionVector[0]), std::stof(cushionVector[1]));
	vertices[1] = vec2(std::stof(cushionVector[2]), std::stof(cushionVector[3]));
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int stone::ballIndexCnt = 0;

stone::stone(team _team) : position(0.0), velocity(0.0), radius(BALL_RADIUS), mass(BALL_MASS) 
{
	index = ballIndexCnt++;
	stoneTeam = _team;
	Reset();
}

void stone::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	position(1) = 0.5;
	position(0) = 0.0;
}

void stone::ApplyImpulse(vec2 imp)
{
	velocity = imp;
}

void stone::ApplyFrictionForce(int ms)
{
	if (velocity.Magnitude() <= 0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms) / 1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if (velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void stone::DoBallCollision(stone& b)
{
	if (HasHitBall(b)) HitBall(b);
}

void stone::DoPlaneCollision(const cushion& b)
{
	if (HasHitPlane(b)) HitPlane(b);
}

void stone::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;
}

bool stone::HasHitPlane(const cushion& c) const
{
	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if (sep > radius) return false;
	return true;
}

bool stone::HasHitBall(const stone& b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float)relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if (relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if (dist > (radius + b.radius)) return false;
	return true;
}

void stone::HitPlane(const cushion& c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0 + gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;

	//make some particles
	int n = (rand() % 4) + 3;
	vec3 pos(position(0), radius / 2.0, position(1));
	vec3 oset(c.normal(0), 0.0, c.normal(1));
	pos += (oset * radius);
	for (int i = 0; i < n; i++)
	{
		//gTable.parts.AddParticle(pos);
	}

	/*
		//assume elastic collision
		//find plane normal
		vec2 planeNorm = gPlaneNormal_Left;
		//split velocity into 2 components:
		//find velocity component perpendicular to plane
		vec2 perp = planeNorm*(velocity.Dot(planeNorm));
		//find velocity component parallel to plane
		vec2 parallel = velocity - perp;
		//reverse perpendicular component
		//parallel component is unchanged
		velocity = parallel + (-perp)*gCoeffRestitution;
	*/
}

void stone::HitBall(stone& b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity - (relDir * perpV);
	vec2 parallelV2 = b.velocity - (relDir * perpV2);

	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV * (mass - b.mass)) / sumMass) + (float)((perpV2 * (2.0 * b.mass)) / sumMass);
	float perpVNew2 = (float)((perpV2 * (b.mass - mass)) / sumMass) + (float)((perpV * (2.0 * mass)) / sumMass);

	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir * perpVNew);
	b.velocity = parallelV2 + (relDir * perpVNew2);


	//make some particles
	int n = (rand() % 5) + 5;
	vec3 pos(position(0), radius / 2.0, position(1));
	vec3 oset(relDir(0), 0.0, relDir(1));
	pos += (oset * radius);
	for (int i = 0; i < n; i++)
	{
		//gTable.parts.AddParticle(pos);
	}
}

std::string stone::PackageStone() {
	std::stringstream ss;
	ss << ballIndexCnt << " " << stoneTeam.name << " " << position(0) << " " << position(1) << " " << velocity(0) << " " << velocity(1) << " " << radius << " " << mass << " " << index;
	return ss.str();
}

void stone::UnpackStone(std::string _str) {
	std::vector<std::string> stoneVector = splitstr(_str, " ");
	ballIndexCnt = std::stoi(stoneVector[0]);
	for (int i = 0; i < gm.teams.size(); i++) {
		if (gm.teams[i].name == stoneVector[1]) {
			stoneTeam = gm.teams[i];
		}
	}
	position = vec2(stof(stoneVector[2]), stof(stoneVector[3]));
	velocity = vec2(stof(stoneVector[4]), stof(stoneVector[5]));
	radius = stof(stoneVector[6]);
	mass = stof(stoneVector[7]);
	index = stoi(stoneVector[8]);
}

/*-----------------------------------------------------------
  particle class members
  -----------------------------------------------------------*/
void particle::update(int ms)
{
	position += (velocity * ms) / 1000.0;
	velocity(1) -= (4.0 * ms) / 1000.0; //(9.8*ms)/1000.0;
}

/*-----------------------------------------------------------
  particle set class members
  -----------------------------------------------------------*/
void particleSet::AddParticle(const vec3& pos)
{
	if (num >= MAX_PARTICLES) return;
	particles[num] = new particle;
	particles[num]->position = pos;

	particles[num]->velocity(0) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(2) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(1) = 2.0 * ((rand() % 100) / 100.0);

	num++;
}

void particleSet::update(int ms)
{
	int i = 0;
	while (i < num)
	{
		particles[i]->update(ms);
		if ((particles[i]->position(1) < 0.0) && (particles[i]->velocity(1) < 0.0))
		{
			delete particles[i];
			particles[i] = particles[num - 1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
std::map<team, std::vector<int>> sheet::activePlayers = {};

sheet::sheet(int sheetNum) {
	sheetPosition = pow(-1, float(sheetNum)) * ceil(float(sheetNum) / 2);
	SetupEdges();
	SetupFeatures();
	teamIt[gm.teams[0]] = 0;
	teamIt[gm.teams[1]] = 0;
}

void sheet::SetupEdges(void)
{
	cushions[0].vertices[0](0) = sheetPosition * yScale * 3 - yScale;
	cushions[0].vertices[0](1) = -18 * SHEET_SCALE;
	cushions[0].vertices[1](0) = sheetPosition * yScale * 3 - yScale;
	cushions[0].vertices[1](1) = 2 * SHEET_SCALE;

	cushions[1].vertices[0](0) = sheetPosition * yScale * 3 - yScale;
	cushions[1].vertices[0](1) = 2 * SHEET_SCALE;
	cushions[1].vertices[1](0) = sheetPosition * yScale * 3 + yScale;
	cushions[1].vertices[1](1) = 2 * SHEET_SCALE;

	cushions[2].vertices[0](0) = sheetPosition * yScale * 3 + yScale;
	cushions[2].vertices[0](1) = 2 * SHEET_SCALE;
	cushions[2].vertices[1](0) = sheetPosition * yScale * 3 + yScale;
	cushions[2].vertices[1](1) = -18 * SHEET_SCALE;

	cushions[3].vertices[0](0) = sheetPosition * yScale * 3 + yScale;
	cushions[3].vertices[0](1) = -18 * SHEET_SCALE;
	cushions[3].vertices[1](0) = sheetPosition * yScale * 3 - yScale;
	cushions[3].vertices[1](1) = -18 * SHEET_SCALE;

	for (int i = 0; i < NUM_CUSHIONS; i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

void sheet::SetupFeatures(void) {
	features[0] = new line(vec2(sheetPosition * yScale * 3 - yScale, -12 * SHEET_SCALE), vec2(sheetPosition * yScale * 3 + yScale, -12 * SHEET_SCALE)); // hog line definition
	hogLine = -12 * SHEET_SCALE;
	features[1] = new line(vec2(sheetPosition * yScale * 3 - yScale, -17 * SHEET_SCALE), vec2(sheetPosition * yScale * 3 + yScale, -17 * SHEET_SCALE)); // hack line definition
	hackLine = -17 * SHEET_SCALE;

	scoreCenter = vec2(sheetPosition * yScale * 3, -15 * SHEET_SCALE);
	features[2] = new ring(vec2(sheetPosition * yScale * 3, -15 * SHEET_SCALE), SHEET_SCALE / 10);
	features[3] = new ring(vec2(sheetPosition * yScale * 3, -15 * SHEET_SCALE), SHEET_SCALE / 3);
	features[4] = new ring(vec2(sheetPosition * yScale * 3, -15 * SHEET_SCALE), (2 * SHEET_SCALE) / 3);
	features[5] = new ring(vec2(sheetPosition * yScale * 3, -15 * SHEET_SCALE), SHEET_SCALE);
}

void sheet::Update(int ms)
{
	//check for collisions for each ball
	for (int i = 0; i < stoneCount; i++)
	{
		for (int j = 0; j < NUM_CUSHIONS; j++)
		{
			stones[i].DoPlaneCollision(cushions[j]);
		}

		for (int j = (i + 1); j < stoneCount; j++)
		{
			stones[i].DoBallCollision(stones[j]);
		}
	}

	//update all balls
	for (int i = 0; i < stoneCount; i++) stones[i].Update(ms);

	//update particles
	//parts.update(ms);

	//make some new particles
	//vec3 pos(0.0,BALL_RADIUS,0.0);
	//parts.AddParticle(pos);
}

bool sheet::AnyStoneMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < stoneCount; i++)
	{
		if (stones[i].velocity(0) != 0.0) return true;
		if (stones[i].velocity(1) != 0.0) return true;
	}
	return false;
}

void sheet::CheckStones(void){
	for (int i = stoneCount - 1; i >= 0; i--) {
		if (stones[i].position(1) > hogLine) {
			stones.erase(stones.begin() + i);
			stoneCount--;
		}
		else if (stones[i].position(1) < hackLine) {
			stones.erase(stones.begin() + i);
			stoneCount--;
		}
	}
}

template<typename T>
void pop_front(std::vector<T>& vec)
{
	assert(!vec.empty());
	vec.erase(vec.begin());
}

void sheet::AddStone(void) {
	if (stoneOrder.size() > 0) {
		stones.push_back(stone(stoneOrder[0]));

		//selecting next player
		teamIt[stoneOrder[0]]++;
		if (teamIt[stoneOrder[0]] >= teams[stoneOrder[0]].size()) {
			teamIt[stoneOrder[0]] = 0;
		}
		int i = 0;
		for (i; i < gm.teams.size(); i++) {
			if (stoneOrder[0].name == gm.teams[i].name)break;
		}

		gm.teams[i].players[teams[stoneOrder[0]][teamIt[stoneOrder[0]]]]->doCue = true;

		pop_front(stoneOrder);

		stones[stones.size() - 1].position(0) = sheetPosition * yScale * 3;//resets position of stone
		stoneCount = stones.size();//increments the counter	
	}
	else {
		GetScores();
		SetupOrder();
		AddStone();
	}
}

void sheet::SetPlayer(team _team){
	teamIt[_team]++;
	if (teamIt[_team] >= teams[_team].size()) {
		teamIt[_team] = 0;
	}
	_team.players[teams[_team][teamIt[_team]]]->doCue = true;
}

std::map<team, int> sheet::GetScores(void) {
	std::map<team,std::vector<float>> scoreDict;
	std::map<team, int> returnValue;
	float closest = 100;
	team closestTeam = teams.begin()->first;
	for (int i = 0; i < stoneCount; i++) {
		float score = sqrt(pow((scoreCenter(0) - stones[i].position(0)), 2) + pow((scoreCenter(1) - stones[i].position(1)), 2));
		if (score < SHEET_SCALE) {
			scoreDict[stones[i].stoneTeam].push_back(score);
			if (score < closest) {
				closest = score;
				closestTeam = stones[i].stoneTeam;
			}
		}
	}
	std::cout << "Team " << closestTeam.name << " won that round with " << scoreDict[closestTeam].size() << " points" << std::endl;// output of winner
	for (int i = 0; i < gm.teams.size(); i++) {
		if (gm.teams[i].name == closestTeam.name)gm.teams[i].currentScore += scoreDict[closestTeam].size();//adds score to current score
	}
	return returnValue;
}

void sheet::SetupOrder(void) {
	if (teams.size() >= 2) {
		stoneOrder.clear();
		for (int i = 0; i < 4; i++) {
			auto iterator = teams.begin();
			stoneOrder.push_back(iterator->first);
			stoneOrder.push_back(iterator->first);
			iterator++;
			stoneOrder.push_back(iterator->first);
			stoneOrder.push_back(iterator->first);
		}
	}
	else {
		std::cout << "Insufficient teams to make an order" << std::endl;
	} 
}

void sheet::AddPlayer(team _team, int _player) {
	teams[_team].push_back(_player);
	activePlayers[_team].push_back(_player);
}

void sheet::RemovePlayer(team _team, int _player) {
	for (int i = activePlayers.size() - 1; i >= 0; i--) {
		if (teams[_team][i] == _player) {
			teams[_team].erase(teams[_team].begin() + i);
		}
	}

	for (int i = activePlayers.size() - 1; i >= 0; i--) {
		if (activePlayers[_team][i] == _player) {
			activePlayers[_team].erase(activePlayers[_team].begin() + i);
		}
	}
}

std::string sheet::PackageSheet() {
	std::stringstream ss;
	ss << _sheetScale << "," << sheetPosition << "," << yScale << "," << stoneCount << ",";
	for (int i = 0; i < stones.size(); i++) {
		ss << stones[i].PackageStone() << ",";
	}
	ss << NUM_CUSHIONS << ",";
	for (int i = 0; i < NUM_CUSHIONS; i++) {
		ss << cushions[i].PackageCushion() << ",";
	}
	ss << NUM_FEATURES << ",";
	for (int i = 0; i < NUM_FEATURES; i++) {
		ss << features[i]->PackageFeature() << ",";
	}
	ss << hogLine << "," << hackLine << "," << scoreCenter(0) << "," << scoreCenter(1) << "," << teams.size();
	for (std::map<team, std::vector<int>>::iterator it = teams.begin(); it != teams.end(); it++) {
		ss << "," << it->first.name << ";" << it->second.size() << ";";
		for (int i = 0; i < it->second.size(); i++) {
			ss << it->second[i] << ";";
		}
	}
	ss << "," << teamIt.size() << ",";
	for (std::map<team, int>::iterator it = teamIt.begin(); it != teamIt.end(); it++) {
		ss << it->first.name << "," << it->second << ",";
	}
	ss << stoneOrder.size() << ",";
	for (int i = 0; i < stoneOrder.size(); i++) {
		ss << stoneOrder[i].name << ",";
	}
	ss << doCue;
	return ss.str();
}

void sheet::UnpackSheet(std::string _str) {
	std::vector<std::string> tableVector = splitstr(_str, ",");

	int i = 0;
	int bot = 0;
	int limit = 0;
	_sheetScale = std::stof(tableVector[0]);
	sheetPosition = std::stof(tableVector[1]);
	yScale = std::stof(tableVector[2]);

	limit = 4 + std::stoi(tableVector[3]);
	for (i = 4; i < limit; i++) {
		stone newStone = stone();
		newStone.UnpackStone(tableVector[i]);
		stones.push_back(newStone);
	}

	stoneCount = std::stoi(tableVector[3]);

	bot = 4 + stoneCount + 1;
	limit += std::stoi(tableVector[4 + stoneCount]) + 1;
	for (i += 1; i < limit; i++) {
		cushion newCushion = cushion();
		newCushion.UnpackCushion(tableVector[i]);
		newCushion.MakeNormal();
		newCushion.MakeCentre();
		cushions[i - bot] = newCushion;
	}

	feature* newFeature = new line(vec2(), vec2());
	newFeature->UnpackFeature(tableVector[i + 1]);
	features[0] = newFeature;
	newFeature = new line(vec2(), vec2());
	newFeature->UnpackFeature(tableVector[i + 2]);
	features[1] = newFeature;

	newFeature = new ring(vec2(), 0.0);
	newFeature->UnpackFeature(tableVector[i + 3]);
	features[2] = newFeature;
	newFeature = new ring(vec2(), 0.0);
	newFeature->UnpackFeature(tableVector[i + 4]);
	features[3] = newFeature;
	newFeature = new ring(vec2(), 0.0);
	newFeature->UnpackFeature(tableVector[i + 5]);
	features[4] = newFeature; 
	newFeature = new ring(vec2(), 0.0);
	newFeature->UnpackFeature(tableVector[i + 6]);
	features[5] = newFeature;

	i += 7;

	hogLine = std::stof(tableVector[i]);
	hackLine = std::stof(tableVector[i + 1]);
	scoreCenter = vec2(std::stof(tableVector[i + 2]), std::stof(tableVector[i + 3]));

	i += 4;
	limit += 12 + std::stoi(tableVector[i]);
	for (i += 1; i < limit; i++) {
		std::vector<std::string> _teamArrStr = splitstr(tableVector[i], ";");
		for (int j = 0; j < gm.teams.size(); j++) {
			if (gm.teams[j].name == _teamArrStr[0]) {
				for (int x = 2; x < 2 + std::stoi(_teamArrStr[1]); x++) {
					teams[gm.teams[j]].push_back(std::stoi(_teamArrStr[x]));
				}
			}
		}
	}

	limit = i + std::stoi(tableVector[i]) * 2 + 1;
	for (i += 1; i < limit; i+=2) {
		for (int j = 0; j < gm.teams.size(); j++) {
			if (gm.teams[j].name == tableVector[i]) {
				teamIt[gm.teams[j]] = std::stoi(tableVector[i + 1]);
			}
		}
	}

	limit = i + std::stoi(tableVector[i]) + 1;
	stoneOrder.clear();
	for (i += 1; i < limit; i += 1) {
		for (int x = 0; x < gm.teams.size(); x++) {
			if (gm.teams[x].name == tableVector[i]) {
				stoneOrder.push_back(gm.teams[x]);
			}
		}

	}	

	//doCue = (tableVector[i] == "1");
}

line::line(vec2 _vertex1, vec2 _vertex2){
	vertices[0] = _vertex1;
	vertices[1] = _vertex2;
}

std::string line::PackageFeature() {
	std::stringstream ss;
	ss << vertices[0](0) << " " << vertices[0](1) << " " << vertices[1](0) << " " << vertices[1](1) << " ";
	return ss.str();
}

void line::UnpackFeature(std::string _str) {
	std::vector<std::string> lineVector = splitstr(_str, " ");
	vertices[0] = vec2(std::stof(lineVector[0]), std::stof(lineVector[1]));
	vertices[1] = vec2(std::stof(lineVector[2]), std::stof(lineVector[3]));
}

ring::ring(vec2 _center, float _rad) {
	center = _center;
	rad = _rad;
}

std::string ring::PackageFeature() {
	std::stringstream ss;
	ss << center(0) << " " << center(1) << " " << rad;
	return ss.str();
}

void ring::UnpackFeature(std::string _str) {
	std::vector<std::string> ringVector = splitstr(_str, " ");
	center = vec2(std::stof(ringVector[0]), std::stof(ringVector[1]));
	rad = std::stof(ringVector[2]);
}


gameManager::gameManager() {
	AutoGenerateTeams(2);
}

team gameManager::AutoAddPlayer(player* _player) {
	team returnTeam;
	if (std::find(teams[0].activePlayers.begin(), teams[0].activePlayers.end(), _player) == teams[0].activePlayers.end()) //checks if player is already in the active players list
	{
		int teamSize = teams[0].players.size();
		for (int i = 0; i < teams.size(); i++) {
			if (teamSize > teams[i].players.size()) {
				teams[i].AddPlayer(_player);
				returnTeam = teams[i];
				int _numOnTable = gm.tables[0].teams[teams[i]].size();
				for (int x = 0; x < gm.tables.size(); x++) {
					if (_numOnTable > gm.tables[x].teams[teams[i]].size()) {
						gm.tables[x].AddPlayer(teams[i], teams[i].players.size() - 1);
					}
					else if (x == gm.tables.size() - 1) {
						gm.tables[0].AddPlayer(teams[i], teams[i].players.size() - 1);
					}
				}
			}
			else if (i == teams.size() - 1) {
				teams[0].AddPlayer(_player);
				returnTeam = teams[0];
				int _numOnTable = gm.tables[0].teams[teams[0]].size();
				for (int x = 0; x < gm.tables.size(); x++) {
					if (_numOnTable > gm.tables[x].teams[teams[0]].size()) {
						gm.tables[x].AddPlayer(teams[0], teams[0].players.size() - 1);
					}
					else if (x == gm.tables.size() - 1) {
						gm.tables[0].AddPlayer(teams[0], teams[0].players.size() - 1);
					}
				}
			}
		}
	}
	return returnTeam;
}

void gameManager::AutoGenerateTeams(int _numTeams) {
	teams.clear();
	for (int i = 0; i < _numTeams; i++) {
		teams.push_back(team());
	}
}

void gameManager::GenerateTeam(std::string _name, vec3 _colour) {
	team newTeam = team();
	newTeam.name = _name;
	newTeam.colour = _colour;
	teams.push_back(newTeam);
}

void gameManager::GenerateTables(int _num) {
	for (int i = 0; i < _num; i++) {
		tables.push_back(sheet(i));
	}
}

std::string gameManager::PackageGM() {
	std::stringstream ss;
	ss << "C:" << teams.size() << ":";
	for (int i = 0; i < teams.size(); i++) {
		ss << teams[i].PackageTeam() << ":";
	}
	ss << tables.size() << ":";
	for (int i = 0; i < tables.size(); i++) {
		ss << tables[i].PackageSheet() << ":";
	}
	ss << "$";
	return ss.str();
}

void gameManager::UnpackGM(std::string _charString) {
	_charString  = splitstr(_charString, "&")[0];//removes any extra info sent
	std::vector<std::string> strVector = splitstr(_charString, ":");//breaks apart the char string into the indiviual segments of infomation
	int i = 2;
	int limit = std::stoi(strVector[1]) + 2;
	teams.clear();
	for (i; i < limit; i++) {
		team newTeam = team();
		newTeam.UnpackTeam(strVector[i]);
		teams.push_back(newTeam);
	}
	limit = i + std::stoi(strVector[i]) + 1;
	for (i += 1; i < limit; i++) {
		sheet newTable = sheet(0);
		newTable.UnpackSheet(strVector[i]);
		tables.push_back(newTable);
	}
}