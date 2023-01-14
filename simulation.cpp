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

table gTable;

static const float gRackPositionX[] = { 0.0f,0.0f,(BALL_RADIUS * 2.0f),(-BALL_RADIUS * 2.0f),(BALL_RADIUS * 4.0f) };
static const float gRackPositionZ[] = { 0.5f,0.0f,(-BALL_RADIUS * 3.0f),(-BALL_RADIUS * 3.0f) };

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;


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

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int stone::ballIndexCnt = 0;

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
		gTable.parts.AddParticle(pos);
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
		gTable.parts.AddParticle(pos);
	}
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
void table::SetupEdges(void)
{
	cushions[0].vertices[0](0) = -TABLE_X;
	cushions[0].vertices[0](1) = -10;
	cushions[0].vertices[1](0) = -TABLE_X;
	cushions[0].vertices[1](1) = 1;

	cushions[1].vertices[0](0) = -TABLE_X;
	cushions[1].vertices[0](1) = 1;
	cushions[1].vertices[1](0) = TABLE_X;
	cushions[1].vertices[1](1) = 1;

	cushions[2].vertices[0](0) = TABLE_X;
	cushions[2].vertices[0](1) = 1;
	cushions[2].vertices[1](0) = TABLE_X;
	cushions[2].vertices[1](1) = -10;

	cushions[3].vertices[0](0) = TABLE_X;
	cushions[3].vertices[0](1) = -10;
	cushions[3].vertices[1](0) = -TABLE_X;
	cushions[3].vertices[1](1) = -10;

	for (int i = 0; i < NUM_CUSHIONS; i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

void table::SetupFeatures(void) {
	features[0] = new line(vec2(-yScale, -12 * TABLE_SCALE), vec2(yScale, -12 * TABLE_SCALE)); // hog line definition

	features[1] = new line(vec2(-yScale, -17 * TABLE_SCALE), vec2(yScale, -17 * TABLE_SCALE)); // hack line definition
}

void table::Update(int ms)
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
	parts.update(ms);

	//make some new particles
	//vec3 pos(0.0,BALL_RADIUS,0.0);
	//parts.AddParticle(pos);
}

bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < stoneCount; i++)
	{
		if (stones[i].velocity(0) != 0.0) return true;
		if (stones[i].velocity(1) != 0.0) return true;
	}
	return false;
}

void table::CheckStones(void){
	for (int i = 0; i < stoneCount; i++) {

	}
}

void table::AddBall(void) {
	stones.push_back(stone());
	stoneCount += 1;
}

line::line(vec2 vertex1, vec2 vertex2){
	vertices[0] = vertex1;
	vertices[1] = vertex2;
}



