/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"
#include <vector>

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
	/*-----------------------------------------------------------
	  plane normals
	  -----------------------------------------------------------*/
	  /*
	  extern vec2	gPlaneNormal_Left;
	  extern vec2	gPlaneNormal_Top;
	  extern vec2	gPlaneNormal_Right;
	  extern vec2	gPlaneNormal_Bottom;
	  */


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
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;

	stone() : position(0.0), velocity(0.0), radius(BALL_RADIUS),
		mass(BALL_MASS) {
		index = ballIndexCnt++; Reset();
	}

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
	virtual int FeatureType() = 0; //returns an int in reference to the type of feature 0 line 1 ring
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
	float innerRad, outerRad;

	int FeatureType() { return 1; };
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	float yScale = (7 / 5) * TABLE_SCALE;
	int stoneCount = 0;
	std::vector<stone> stones;
	cushion cushions[NUM_CUSHIONS];
	feature* features[NUM_FEATURES];
	particleSet parts;
	float hogLine, hackLine;

	void SetupEdges(void);
	void SetupFeatures(void);
	void Update(int ms);
	bool AnyBallsMoving(void) const;
	void AddBall(void);
	void CheckStones(void);
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern table gTable;
