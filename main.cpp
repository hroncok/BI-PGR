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
#include "resources/ShaderProgram.h"
#include "AnimNode.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

#define TITLE "BI-PGR"
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PITCHLIM 1.55f // pi/2 - small

const int NUM_SPOT_LIGHTS = 1;

// file name used during the scene graph creation
#define TERRAIN_FILE_NAME "./data/terrain"
#define BOTTLE_FILE_NAME "./data/bottle/bottle.obj"
#define PATH_FILE_NAME "./data/path/path.obj"

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

glm::vec4 reflector = glm::vec4(0.0f);

struct Light {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

struct DirectionalLight: public Light {
	glm::vec4 direction;
};

struct SpotLight: public Light {
	glm::vec4 position;
	glm::vec4 spotDirection;
	float spotCosCutoff;
	float spotExponent;
};

struct State {
	glm::mat4 projection;
	glm::vec3 cameraPosition;
	float cameraYaw;
	float cameraPitch;
	glm::vec3 cameraDirection;
	SpotLight refLights[2];
} state;



class LightingShader: public MeshShaderProgram {
public:
	struct LightLocations {
		GLint ambient;
		GLint diffuse;
		GLint specular;
		GLint position;
		GLint spotDirection;
		GLint spotCosCutoff;
		GLint spotExponent;
	};
	
	LightingShader(GLint prId): MeshShaderProgram(prId) {}
	
	void initLocations() {
		MeshShaderProgram::initLocations();

		for(int l = 0; l < NUM_SPOT_LIGHTS; ++l) {
			char buf[255];
			snprintf(buf, 255, "lights[%i].ambient", l);
			m_lights[l].ambient = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].diffuse", l);
			m_lights[l].diffuse = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].specular", l);
			m_lights[l].specular = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].position", l);
			m_lights[l].position = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].spotDirection", l);
			m_lights[l].spotDirection = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].spotCosCutoff", l);
			m_lights[l].spotCosCutoff = glGetUniformLocation(m_programId, buf);
			snprintf(buf, 255, "lights[%i].spotExponent", l);
			m_lights[l].spotExponent = glGetUniformLocation(m_programId, buf);
		}
	}

	LightLocations m_lights[NUM_SPOT_LIGHTS];
};

struct Resources {
	LightingShader * shaderProgram;
} resources;

void FuncTimerCallback(int) {
	// this is from screenGraph
	double timed = 0.001 * (double)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds
	// ELAPSED_TIME is number of milliseconds since glutInit called 

	if(rootNode_p)
		rootNode_p->update(timed);

	glutTimerFunc(33, FuncTimerCallback, 0);
	glutPostRedisplay();
}

void reloadShader() {
	if(resources.shaderProgram)
		delete resources.shaderProgram;

	GLuint shaderList[] = {
		pgr::createShaderFromFile(GL_VERTEX_SHADER,   "resources/MeshNode.vert"),
		pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "resources/MeshNode.frag"),
		0
	};
	resources.shaderProgram = new LightingShader(pgr::createProgram(shaderList));
	resources.shaderProgram->initLocations();
	CHECK_GL_ERROR();
}

void reflectorSwitch() {
	std::cout << "" << std::endl;
	if (reflector == glm::vec4(0.0f)) reflector = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
	else  reflector = glm::vec4(0.0f);
	glutPostRedisplay();
}

void functionDraw() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection =  glm::perspective(60.0f, g_aspect_ratio, 1.0f, 10000.0f);

	glm::mat4 view(1.0f);
	view = glm::lookAt(state.cameraPosition,state.cameraDirection+state.cameraPosition,glm::vec3(0,1,0));

	for(int l = 0; l < NUM_SPOT_LIGHTS; ++l) {
		glUniform3fv(resources.shaderProgram->m_lights[l].ambient, 1, glm::value_ptr(state.refLights[l].ambient));
		glUniform3fv(resources.shaderProgram->m_lights[l].diffuse, 1, glm::value_ptr(state.refLights[l].diffuse));
		glUniform3fv(resources.shaderProgram->m_lights[l].specular, 1, glm::value_ptr(state.refLights[l].specular));
		glUniform3fv(resources.shaderProgram->m_lights[l].position, 1, glm::value_ptr(state.refLights[l].position));
		glUniform3fv(resources.shaderProgram->m_lights[l].spotDirection, 1, glm::value_ptr(state.refLights[l].spotDirection));
		glUniform1f(resources.shaderProgram->m_lights[l].spotCosCutoff, state.refLights[l].spotCosCutoff);
		glUniform1f(resources.shaderProgram->m_lights[l].spotExponent, state.refLights[l].spotExponent);
	}

	// Position of the reflector
	state.refLights[0].position = view * glm::vec4(1.0f, 20.0f, 1.0f, 1.0f);
	state.refLights[0].spotDirection = view * reflector;

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

void createPath() {
	TransformNode* path_transform = new TransformNode("pathTranf", rootNode_p);
	path_transform->translate(glm::vec3(0.0, -17.3, 0.0));
	//path_transform->rotate(-90,glm::vec3(1,0,0));
	path_transform->scale(glm::vec3(35,20,35));
	
	MeshGeometry* meshGeom_p = MeshManager::Instance()->get(PATH_FILE_NAME);
	MeshNode * path_mesh_p = new MeshNode("path", path_transform);
	path_mesh_p->setGeometry(meshGeom_p);
}

void createBottle() {
	// Index the names so more bottles are possible
	AnimNode* bottle_anim = new AnimNode("bottleAnim",rootNode_p);
	
	TransformNode* bottle_transform = new TransformNode("bottleTranf", bottle_anim);
	bottle_transform->translate(glm::vec3(0.0, -12.5, 0.0));
	//bottle_transform->rotate(-90,glm::vec3(1,0,0));
	bottle_transform->scale(glm::vec3(4));

	MeshGeometry* meshGeom_p = MeshManager::Instance()->get(BOTTLE_FILE_NAME);
	MeshNode * bottle_mesh_p = new MeshNode("bottle", bottle_transform);
	bottle_mesh_p->setGeometry(meshGeom_p);
}

void calculateState(void) {
	state.cameraDirection = glm::vec3(cos(state.cameraYaw),tan(state.cameraPitch),sin(state.cameraYaw));
	//state.cameraDirection = glm::vec3(cos(state.cameraYaw)*cos(state.cameraPitch),sin(state.cameraPitch),sin(state.cameraYaw)*cos(state.cameraPitch));
	// both works and acts the same, the second should be more right, but I like the forst one more
	// both acts wierd when paged up/down over the top/bottom, so it is limited by PITCHLIM constant (little less than pi/2)
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
	case 77:
		reflectorSwitch();
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
	glutAddMenuEntry("Static 1         [B]", 1);
	glutAddMenuEntry("Static 2         [N]", 2);
	glutAddMenuEntry("Free camera      [F]", 3);
	
	glutCreateMenu(myMenu);
	glutAddSubMenu("Camera", submenuID);

	glutAddMenuEntry("Reflector on/off [R]", 77);
	glutAddMenuEntry("Debug info       [D]", 88);
	glutAddMenuEntry("Exit           [Esc]", 99);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void initializeScene() {
	// create scene root node
	rootNode_p = new SceneNode("root");
	createTerrain();
	//createPath();
	createBottle();
	// dump our scene graph tree for debug
	rootNode_p->dump();
}

void display() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	functionDraw();
	glutSwapBuffers();
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
	case'r':
	case'R':
		reflectorSwitch();
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
	reloadShader();
	state.refLights[0].ambient = glm::vec4(0.0f);
	state.refLights[0].diffuse = glm::vec4(1.0f);
	state.refLights[0].specular = glm::vec4(1.0f);
	state.refLights[0].spotCosCutoff = 0.7f;
	state.refLights[0].spotExponent = 3.0f;
	switchCam(1);
	
	//glDisable(GL_CULL_FACE); // draw both back and front faces
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE); // draw front faces only
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow(TITLE);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(mySpecialKeyboard);
	//glutMouseFunc(myMouse);
	//glutMotionFunc(myMotion);
	glutTimerFunc(33, FuncTimerCallback, 0);
	createMenu();
	if(!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
		pgr::dieWithError("PGR init failed, now f*ck around and get another computer, cool huh?");
	init();
	glutMainLoop();
	return 0;
}
