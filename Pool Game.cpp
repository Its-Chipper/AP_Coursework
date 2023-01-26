// Pool Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdafx.h"
#include "threaded_client.h"
#include <glut.h>
#include <math.h>


//cue variables
float gCueAngle = 0.0;
float gCuePower = 0.25;
bool gCueControl[4] = { false,false,false,false };
float gCueAngleSpeed = 1.0f; //radians per second
float gCuePowerSpeed = 0.15f;
float gCuePowerMax = 0.75;
float gCuePowerMin = 0.1;
float gCueBallFactor = 8.0;
bool gDoCue = true;

//camera variables
vec3 gCamPos(0.0, 10.7, 2.1);
vec3 gCamLookAt(0.0, 0.0, 0.0);
bool gCamRotate = true;
float gCamRotSpeed = 0.2;
float gCamMoveSpeed = 0.5;
bool gCamL = false;
bool gCamR = false;
bool gCamU = false;
bool gCamD = false;
bool gCamZin = false;
bool gCamZout = false;

//rendering options
#define DRAW_SOLID	(1)

Client client;

void DoCamera(int ms)// cam mover
{
	static const vec3 up(0.0, 1.0, 0.0);

	if (gCamRotate)
	{
		if (gCamL)//rotate cam left
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL * ((gCamRotSpeed * ms) / 1000.0));
			gCamLookAt = gCamPos + camDir + inc;
		}
		if (gCamR)//right
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = up.Cross(camDir);
			vec3 inc = (localR * ((gCamRotSpeed * ms) / 1000.0));
			gCamLookAt = gCamPos + camDir - inc;
		}
		if (gCamU)// up
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp * ((gCamMoveSpeed * ms) / 1000.0));
			gCamLookAt = gCamPos + camDir + inc;
		}
		if (gCamD)// down
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp * ((gCamMoveSpeed * ms) / 1000.0));
			gCamLookAt = gCamPos + camDir - inc;
		}
	}
	else
	{
		if (gCamL)//move cam left
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL * ((gCamMoveSpeed * ms) / 1000.0));
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if (gCamR)//move cam right
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 inc = (localR * ((gCamMoveSpeed * ms) / 1000.0));
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if (gCamU)//move cam up
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp * ((gCamMoveSpeed * ms) / 1000.0));
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if (gCamD)//move cam down
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localDown = camDir.Cross(localR);
			vec3 inc = (localDown * ((gCamMoveSpeed * ms) / 1000.0));
			gCamPos += inc;
			gCamLookAt += inc;
		}
	}

	if (gCamZin)//zoom in
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir * ((gCamMoveSpeed * ms) / 1000.0));
		gCamPos += inc;
		gCamLookAt += inc;
	}
	if (gCamZout)//zoom out
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir * ((gCamMoveSpeed * ms) / 1000.0));
		gCamPos -= inc;
		gCamLookAt -= inc;
	}
}

void CamSetLoc(vec3 _position, vec3 _lookat) {//move cam to a set location
	gCamPos = _position;
	gCamLookAt = _lookat;
}

void DrawCircle(float cx, float cy, float r, int num_segments)//draws the rings
{
	glBegin(GL_LINE_LOOP);
	for (int seg = 0; seg < num_segments; seg++)//draws each segment of the ring
	{
		float theta = 2.0f * 3.1415926f * float(seg) / float(num_segments);//get the current angle

		float x = r * cosf(theta);//calculate the x component
		float y = r * sinf(theta);//calculate the y component

		glVertex3f(x + cx, 0.0, y + cy);//output vertex
	}
	glEnd();
}


int RenderTable(size_t tab) {//render table function
	for (int i = 0; i < gm.tables[tab].stoneCount; i++)
	{
		glDisable(GL_LIGHTING);
		glColor3f(gm.tables[tab].stones[i].stoneTeam.colour(0), gm.tables[tab].stones[i].stoneTeam.colour(1), gm.tables[tab].stones[i].stoneTeam.colour(2));// colour stone to team colour
		//drawing of the stones
		glPushMatrix();
		glTranslatef(gm.tables[tab].stones[i].position(0), (BALL_RADIUS / 2.0), gm.tables[tab].stones[i].position(1));
		glScalef(1.0, 0.3, 1.0);//scaled so it looks more flat ontop
#if DRAW_SOLID
		glutSolidSphere(gm.tables[tab].stones[i].radius, 32, 32);
#else
		glutWireSphere(gm.tables[tab].balls[i].radius, 12, 12);
#endif
		glPopMatrix();
		glColor3f(0.0, 0.0, 1.0);
	}
	glColor3f(1.0, 1.0, 1.0);

	//draw the table
	for (int i = 0; i < NUM_CUSHIONS; i++)//cushion loop
	{
		//draw each vertx of the cushion
		glBegin(GL_LINE_LOOP);
		glVertex3f(gm.tables[tab].cushions[i].vertices[0](0), 0.0, gm.tables[tab].cushions[i].vertices[0](1));
		glVertex3f(gm.tables[tab].cushions[i].vertices[0](0), 0.1, gm.tables[tab].cushions[i].vertices[0](1));
		glVertex3f(gm.tables[tab].cushions[i].vertices[1](0), 0.1, gm.tables[tab].cushions[i].vertices[1](1));
		glVertex3f(gm.tables[tab].cushions[i].vertices[1](0), 0.0, gm.tables[tab].cushions[i].vertices[1](1));
		glEnd();
	}

	//feature loop to draw the rings and lines
	for (int i = 0; i < NUM_FEATURES; i++)
	{
		//trys to cast as a line if it does uses the line drawer else uses the ring drawer function
		if (line* x = dynamic_cast<line*>(gm.tables[tab].features[i])) {
			glBegin(GL_LINE_LOOP);
			glVertex3f(x->vertices[0](0), 0.0, x->vertices[0](1));
			glVertex3f(x->vertices[1](0), 0.0, x->vertices[1](1));
			glEnd();
		}
		else if (ring* x = dynamic_cast<ring*>(gm.tables[tab].features[i])) {
			DrawCircle(x->center(0), x->center(1), x->rad, 30);
		}
	}
	return(0);
}

void RenderScene(void) {//main render loop function
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set camera
	glLoadIdentity();
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);

	//draw the ball
	glColor3f(1.0, 1.0, 1.0);

	for (size_t tab = 0; tab < gm.tables.size(); tab++) {
		RenderTable(tab);
	}

	//draw the cue
	if (localPlayer->doCue)
	{
		glBegin(GL_LINES);
		float cuex = sin(gCueAngle) * gCuePower;
		float cuez = cos(gCueAngle) * gCuePower;
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(gm.tables[0].stones[gm.tables[0].stoneCount - 1].position(0), (BALL_RADIUS / 2.0f), gm.tables[0].stones[gm.tables[0].stoneCount - 1].position(1));
		glVertex3f((gm.tables[0].stones[gm.tables[0].stoneCount - 1].position(0) + cuex), (BALL_RADIUS / 2.0f), (gm.tables[0].stones[gm.tables[0].stoneCount - 1].position(1) + cuez));
		glColor3f(1.0, 1.0, 1.0);
		glEnd();
	}

	//glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

//Input function
void SpecKeyboardFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		gCueControl[0] = true;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		gCueControl[1] = true;
		break;
	}
	case GLUT_KEY_UP:
	{
		gCueControl[2] = true;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		gCueControl[3] = true;
		break;
	}
	}
}

// removal of input function
void SpecKeyboardUpFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		gCueControl[0] = false;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		gCueControl[1] = false;
		break;
	}
	case GLUT_KEY_UP:
	{
		gCueControl[2] = false;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		gCueControl[3] = false;
		break;
	}
	}
}

//gets key presses
void KeyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case(13):
	{
		if (localPlayer->doCue)
		{
			vec2 imp((-sin(gCueAngle) * gCuePower * gCueBallFactor),
				(-cos(gCueAngle) * gCuePower * gCueBallFactor));
			gm.tables[0].stones[gm.tables[0].stoneCount - 1].ApplyImpulse(imp);
			localPlayer->doCue = false;
			client.sendImpulse(imp, 0);
		}
		break;
	}
	case(27):
	{
		for (int i = 0; i < gm.tables[0].stoneCount; i++)
		{
			gm.tables[0].stones[i].Reset();
		}
		break;
	}
	case(32):
	{
		gCamRotate = false;
		break;
	}
	case('z'):
	{
		gCamL = true;
		break;
	}
	case('c'):
	{
		gCamR = true;
		break;
	}
	case('s'):
	{
		gCamU = true;
		break;
	}
	case('x'):
	{
		gCamD = true;
		break;
	}
	case('f'):
	{
		gCamZin = true;
		break;
	}
	case('v'):
	{
		gCamZout = true;
		break;
	}
	}

}

//gets removal of key presses
void KeyboardUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case(32):
	{
		gCamRotate = true;
		break;
	}
	case('z'):
	{
		gCamL = false;
		break;
	}
	case('c'):
	{
		gCamR = false;
		break;
	}
	case('s'):
	{
		gCamU = false;
		break;
	}
	case('x'):
	{
		gCamD = false;
		break;
	}
	case('f'):
	{
		gCamZin = false;
		break;
	}
	case('v'):
	{
		gCamZout = false;
		break;
	}
	}
}

//chaing of window size
void ChangeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0) h = 1;
	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, ratio, 0.2, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0,0.7,2.1, 0.0,0.0,0.0, 0.0f,1.0f,0.0f);
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
}

//deafault lights
void InitLights(void)
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_ambient[] = { 2.0, 2.0, 2.0, 1.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
}

//main update loop
void UpdateScene(int ms)
{
	//for each table check if any stones are moving
	for (size_t tab = 0; tab < gm.tables.size(); tab++)
	{
		if (gm.tables[tab].AnyStoneMoving() == false) {
			if (gm.tables[tab].doCue == false) {
				//if master client (gm.setUP)
				if (gm.setUp == true) {
					//add stones if none moving
					gm.tables[tab].CheckStones();
					gm.tables[tab].AddStone();
					client.sendAddStone(tab);
				}
			}
			//sets tables cue to true
			gm.tables[tab].doCue = true;
			CamSetLoc(vec3(0.0, 10, 2.1), vec3(0.0, 0.0, -3.0));//move cam (high to see all tables)
		}
		else {
			gm.tables[tab].doCue = false;
			CamSetLoc(vec3(0.0, 5, -15 * SHEET_SCALE), vec3(0.0, 0.0, -7));
		}

		//update table call
		gm.tables[tab].Update(ms);
	}

	//cue controls
	if (localPlayer->doCue)
	{
		if (gCueControl[0]) gCueAngle -= ((gCueAngleSpeed * ms) / 1000);
		if (gCueControl[1]) gCueAngle += ((gCueAngleSpeed * ms) / 1000);
		if (gCueAngle < 0.0) gCueAngle += TWO_PI;
		if (gCueAngle > TWO_PI) gCueAngle -= TWO_PI;

		if (gCueControl[2]) gCuePower += ((gCuePowerSpeed * ms) / 1000);
		if (gCueControl[3]) gCuePower -= ((gCuePowerSpeed * ms) / 1000);
		if (gCuePower > gCuePowerMax) gCuePower = gCuePowerMax;
		if (gCuePower < gCuePowerMin) gCuePower = gCuePowerMin;
	}

	DoCamera(ms);

	//sets a timer to recall the updateScene function
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutPostRedisplay();
}

//main function
int _tmain(int argc, _TCHAR* argv[])
{
	//client inputs
	std::string _playerName, _port, _hostName;
	std::cout << "Player Name:";
	std::cin >> _playerName;

	localPlayer->name = _playerName;

	std::cout << std::endl << "HostName/IP:";
	std::cin >> _hostName;
	std::cout << std::endl << "Port:";
	std::cin >> _port;
	std::cout << "Conecting to " << _hostName <<" port " << _port << std::endl;
	//opening the connection
	client.start("localhost", _port.c_str());
	client.interact();

	client.sendthis("Config");//asks server for config

	//starting the openGL window and starting the updatescene loop
	glutInit(&argc, ((char**)argv));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1000, 700);
	//glutFullScreen();
	glutCreateWindow("Curling Game");
#if DRAW_SOLID
	InitLights();
#endif
	glutDisplayFunc(RenderScene);
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(RenderScene);

	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(KeyboardFunc);
	glutKeyboardUpFunc(KeyboardUpFunc);
	glutSpecialFunc(SpecKeyboardFunc);
	glutSpecialUpFunc(SpecKeyboardUpFunc);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}
