//----------------------------------------------------------------------------------------
/**
 * \file    main.cpp
 * \author  Tomas Barak, Vlastimil Havran, Jaroslav Sloup, Miroslav Hroncok
 */
//----------------------------------------------------------------------------------------

#include <string>
#include <vector>
#include <cstdio>
#include <string.h>
#include <iostream>

#include "pgr.h"   // includes all PGR libraries, like shader, glm, assimp ...

// scene graph includes
#include "resources/SceneNode.h" // superclass of all scene graph nodes
#include "resources/TransformNode.h" // model transformation
#include "resources/MeshNode.h" // model loaded from the file
#include "resources/Resources.h"
#include "resources/MeshGeometry.h"
#include "resources/AxesNode.h" // coordinate axes

#define TITLE "BI-PGR"
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PITCHLIM 1.55f // pi/2 - small

// file name used during the scene graph creation
#define TERRAIN_FILE_NAME "./data/terrain"
#define BOTTLE_FILE_NAME "./data/bottle/bottle.obj"

// scene graph root node
SceneNode * rootNode_p = NULL; // scene root

// global variables
float g_aspect_ratio = 1.0f;
int g_win_w, g_win_h; // windowSize
bool freeCam = false; // is the free camera motion available

/// animation time step for glutTimer
const int TIMER_STEP = 20;   // next event in [ms]

/// use this constant when incrementing the camera pith and yaw
const float CAMERA_ROTATION_DELTA = M_PI / 100.0f;
/// this constant is used to modify the spinAngle variable
const float ROTATION_DELTA = -M_PI / 500;
/// use this to increment the camera position
const float MOVE_DELTA = 0.2f;

/// radius of the model circle (distance from the scene center)
const float R = 3.0f;
/// the axis of the model rotation
const glm::vec3 spinAxis = glm::vec3(0.0, 1.0, 0.0);

struct State {
	glm::mat4 projection;
	glm::vec3 cameraPosition;
	float cameraYaw;
	float cameraPitch;
	glm::vec3 cameraDirection;
} state;

void FuncTimerCallback(int) {
	// this is from screenGraph
	double timed = 0.001 * (double)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds
	// ELAPSED_TIME is number of milliseconds since glutInit called 

	if(rootNode_p)
		rootNode_p->update(timed);

	glutTimerFunc(33, FuncTimerCallback, 0);
	glutPostRedisplay();
}

void functionDraw() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection =  glm::perspective(60.0f, g_aspect_ratio, 1.0f, 10000.0f);

	glm::mat4 view(1.0f);
	view = glm::lookAt(state.cameraPosition,state.cameraDirection+state.cameraPosition,glm::vec3(0,1,0));

	if(rootNode_p)
		rootNode_p->draw(view, projection);
}

void createTerrain() {
	TransformNode* terrain_transform = new TransformNode("terrainTranf", rootNode_p);
	
	terrain_transform->translate(glm::vec3(0.0, -17, 0.0));
	terrain_transform->scale(glm::vec3(80.0, 0.01, 80.0));
	
	if(!MeshManager::Instance()->exists(TERRAIN_FILE_NAME))
		MeshManager::Instance()->insert(TERRAIN_FILE_NAME, MeshGeometry::LoadRawHeightMap(TERRAIN_FILE_NAME));
	MeshGeometry * mesh_p = MeshManager::Instance()->get(TERRAIN_FILE_NAME);
	
	MeshNode* terrain_mesh_p = new MeshNode(TERRAIN_FILE_NAME, terrain_transform);
	terrain_mesh_p->setGeometry(mesh_p);
}

void createBottle() {
	TransformNode* bottle_transform = new TransformNode("bottleTranf", rootNode_p);
	bottle_transform->translate(glm::vec3(12.0, -13, -5.0));
	//bottle_transform->rotate(-90,glm::vec3(1,0,0));
	bottle_transform->scale(glm::vec3(4));
	
	MeshGeometry* meshGeom_p = MeshManager::Instance()->get(BOTTLE_FILE_NAME);
	MeshNode * bottle_mesh_p = new MeshNode("bottle", bottle_transform);
	bottle_mesh_p->setGeometry(meshGeom_p);
}

void calculateState(void) {
	state.cameraDirection = glm::vec3(cos(state.cameraYaw),tan(state.cameraPitch),sin(state.cameraYaw));
}

void flushState(void) {
	std::cout << "state.cameraPosition = glm::vec3(" << state.cameraPosition.x << "f, " << state.cameraPosition.y << "f, " << state.cameraPosition.x << "f);"  << std::endl;
	std::cout << "state.cameraPitch = " << state.cameraPitch << "f;"  << std::endl;
	std::cout << "state.cameraYaw = " << state.cameraYaw << "f;"  << std::endl;
}

void switchCam(int cam) {
	switch (cam) {
	case 1:
		freeCam = false;
		state.cameraPosition = glm::vec3(-4.0f, 12.0f, -4.0f);
		state.cameraYaw = -6.0f;
		state.cameraPitch = -1.0f;
		calculateState();
		glutPostRedisplay();
		break;
	case 2:
		freeCam = false;
		state.cameraPosition = glm::vec3(83.0f, 21.0f, 83.0f);
		state.cameraYaw = -8.7f;
		state.cameraPitch = -0.2f;
		calculateState();
		glutPostRedisplay();
		break;
	case 3:
		freeCam = true;
		break;
	}
}

//event processing of the menu commands
void myMenu(int item) {
	switch(item) {
	case 1:
	case 2:
	case 3:
		switchCam(item);
		break;
	case 88:
		flushState();
		break;
	case 99:
		glutLeaveMainLoop();
		break;
	}
}

// menu preparation
void createMenu(void) {
	int submenuID = glutCreateMenu(myMenu);
	glutAddMenuEntry("Static 1        [B]", 1);
	glutAddMenuEntry("Static 2        [N]", 2);
	glutAddMenuEntry("Free camera     [F]", 3);
	
	glutCreateMenu(myMenu);
	glutAddSubMenu("Camera", submenuID);
	glutAddMenuEntry("Debug info      [D]", 88);
	glutAddMenuEntry("Exit          [Esc]", 99);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void initializeScene() {
	// create scene root node
	rootNode_p = new SceneNode("root");
	createTerrain();
	createBottle();
	// dump our scene graph tree for debug
	rootNode_p->dump();
}

void display() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	functionDraw();
	glutSwapBuffers();
	CHECK_GL_ERROR();
}

void reshape (int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	g_aspect_ratio = (float)w/(float)h;
	g_win_w = w;
	g_win_h = h;
}

void myKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;
	case'b':
	case'B':
		switchCam(1);
		break;
	case'n':
	case'N':
		switchCam(2);
		break;
	case'f':
	case'F':
		switchCam(3);
		break;
	case'd':
	case'D':
		flushState();
		break;
	}
}

void mySpecialKeyboard(int specKey, int x, int y) {
	switch (specKey) {
	case GLUT_KEY_LEFT:
		if (freeCam) state.cameraYaw -= CAMERA_ROTATION_DELTA;
		break;
	case GLUT_KEY_RIGHT:
		if (freeCam) state.cameraYaw += CAMERA_ROTATION_DELTA;
		break;
	case GLUT_KEY_PAGE_DOWN:
		if (freeCam) {
			state.cameraPitch -= CAMERA_ROTATION_DELTA;
			if (state.cameraPitch < -PITCHLIM) state.cameraPitch = -PITCHLIM;
		}
		break;
	case GLUT_KEY_PAGE_UP:
		if (freeCam) {
			state.cameraPitch += CAMERA_ROTATION_DELTA;
			if (state.cameraPitch > PITCHLIM) state.cameraPitch = PITCHLIM;
		}
		break;
	case GLUT_KEY_UP:
		if (freeCam) state.cameraPosition += MOVE_DELTA*state.cameraDirection;
		break;
	case GLUT_KEY_DOWN:
		if (freeCam) state.cameraPosition -= MOVE_DELTA*state.cameraDirection;
		break;
	}
	calculateState();
	glutPostRedisplay();
}

void init() {
	initializeScene();
	switchCam(1);
	
	//glDisable(GL_CULL_FACE); // draw both back and front faces
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE); // draw front faces only
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

int main(int argc, char** argv) {
	// initialize windonwing system
	glutInit(&argc, argv);
	
	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	// initial window size
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow(TITLE);
	
	// register callback for drawing a window contents
	glutDisplayFunc(display);
	// register callback for change of window
	glutReshapeFunc(reshape);
	// register callback for keyboard
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(mySpecialKeyboard);
	//glutMouseFunc(myMouse);
	//glutMotionFunc(myMotion);
	glutTimerFunc(33, FuncTimerCallback, 0);
	
	createMenu();
	if(!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
		pgr::dieWithError("PGR init failed, required OpenGL not supported?");
	
	init();
	
	glutMainLoop();
	return 0;
}
