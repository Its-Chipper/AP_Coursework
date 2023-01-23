/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"
#include"teams.h"
#include <vector>
#include <map>
#include <assert.h>

  /*-----------------------------------------------------------
	Macros
	-----------------------------------------------------------*/
#define TABLE_SCALE		(0.5f)
#define TABLE_X			(0.6f) 
#define TABLE_Z			(1.2f)
#define TABLE_Y			(0.1f)
#define BALL_RADIUS		(0.05f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)
#define NUM_BALLS		(7)		
#define NUM_CUSHIONS	(4)
#define NUM_FEATURES	(6)
#define MAX_PARTICLES	(200)
#define NUM_TABLES		(5)

	/*-----------------------------------------------------------
	  plane normals
	  -----------------------------------------------------------*/
	  /*
	  extern vec2	gPlaneNormal_Left;
	  extern vec2	gPlaneNormal_Top;
	  extern vec2	gPlaneNormal_Right;
	  extern vec2	gPlaneNormal_Bottom;
	  */

namespace std
{
	template<> struct less<team>
	{
		bool operator() (const team& lhs, const team& rhs) const
		{
			return lhs.name < rhs.name;
		}
	};
}


/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class stone
{
	static int ballIndexCnt;
public:
	team	stoneTeam;
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;

	stone() : position(0.0), velocity(0.0), radius(BALL_RADIUS),
		mass(BALL_MASS) {
		index = ballIndexCnt++; Reset();
	}

	stone(team);
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion& c);
	void DoBallCollision(stone& b);
	void Update(int ms);

	bool HasHitPlane(const cushion& c) const;
	bool HasHitBall(const stone& b) const;

	void HitPlane(const cushion& c);
	void HitBall(stone& b);
};

class particle
{
public:
	vec3 position;
	vec3 velocity;

	particle() { position = 0; velocity = 0; }
	void update(int ms);
};

class particleSet
{
public:
	particle* particles[MAX_PARTICLES];
	int num;

	particleSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++) particles[i] = 0;
		num = 0;
	}

	~particleSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (particles[i]) delete particles[i];
		}
	}

	void AddParticle(const vec3& pos);
	void update(int ms);
};

class feature
{
public:
	virtual int FeatureType() = 0; //to use the dynaic cast a polymorphic function is required and as i can't draw outside of the poolgame file this will take its place
};

class line : public feature
{
public:
	vec2 vertices[2];

	line(vec2, vec2);

	int FeatureType() { return 0; };
};

class ring : public feature
{
public:
	vec2 center;
	float rad;

	ring(vec2, float);

	int FeatureType() { return 1; };
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
	static std::map<team, std::vector<int>> activePlayers; // map of active players to prevent duplication

private:
	float _tableScale = 0.5f;
	int sheetPosition;

public:

	float yScale = (7.0f / 5.0f) * TABLE_SCALE;
	int stoneCount = 0;
	std::vector<stone> stones;
	cushion cushions[NUM_CUSHIONS];
	feature* features[NUM_FEATURES];
	particleSet parts;
	float hogLine, hackLine;
	vec2 scoreCenter;
	std::map<team, std::vector<int>> teams;
	std::map<team, int> teamIt;
	std::vector<team> stoneOrder;
	bool doCue;

	table(int);
	void SetupEdges(void);
	void SetupFeatures(void);
	void Update(int ms);
	bool AnyStoneMoving(void) const;
	void AddStone(void);
	void CheckStones(void);
	int GetScores(void);
	void SetupOrder(void);
	void AddPlayer(team, int);
	void RemovePlayer(team, int);
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
  //extern table gTable;
