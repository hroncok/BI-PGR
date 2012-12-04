//----------------------------------------------------------------------------------------
/**
 * \file    main.cpp
 * \author  Tomas Barak, Vlastimil Havran, Jaroslav Sloup, Miroslav Hroncok
 * 
 * Courswork for BI-PGR on FIT CTU.
 * This file is derived from sceneGraph seminar, so I've left original authors here too.
 */
//----------------------------------------------------------------------------------------

#include <string>
#include <vector>
#include <cstdio>
#include <string.h>
#include <iostream>
#include <sstream>

#include "pgr.h"   // includes all PGR libraries, like shader, glm, assimp ...

// scene graph includes
#include "resources/SceneNode.h" // superclass of all scene graph nodes
#include "resources/TransformNode.h" // model transformation
#include "resources/MeshNode.h" // model loaded from the file
#include "resources/Resources.h"
#include "resources/MeshGeometry.h"
#include "resources/AxesNode.h" // coordinate axes
#include "resources/ShaderProgram.h"
// my own includes
#include "AnimNode.h"
#include "Configuration.h"

#if _MSC_VER
/// Define this for snprintf function
#define snprintf _snprintf
#endif

/// Window title
#define TITLE "BI-PGR"

/// Initial window width
#define WIN_WIDTH 800

/// Initial window height
#define WIN_HEIGHT 600

/// Limit for free camera Pitch angle, radians
#define PITCHLIM 1.55f // pi/2 - small

/// How many spot light is used
const int NUM_SPOT_LIGHTS = 1;

/// File name used during the scene graph creation
#define TERRAIN_FILE_NAME "./data/terrain"

/// File name used during the scene graph creation
#define BOTTLE_FILE_NAME "./data/bottle/bottle.obj"

/// File name used during the scene graph creation
#define STREAM_FILE_NAME "./data/stream/stream.obj"

/// File name used for stream texture
#define STREAM_TEXTURE_FILE_NAME "./data/stream/stream.png"

/// Scene graph root node
SceneNode * rootNode_p = NULL; // scene root

/// Aspect ratio
float g_aspect_ratio = 1.0f;

/// Actual window size
int g_win_w, g_win_h;

/// Determinates whether is the free camera motion available
bool freeCam = false;

/// Determinates whether is theanimation of bottles turned on
bool AnimNode::animation = true;

/// Loads and handles the config form the file
Configuration AnimNode::config;

/// Animation time step for glutTimer
const int TIMER_STEP = 20;   // next event in [ms]

/// Use this constant when incrementing the camera pith and yaw
const float CAMERA_ROTATION_DELTA = M_PI / 100.0f;

/// This constant is used to modify the spinAngle variable
const float ROTATION_DELTA = -M_PI / 500;

/// Use this to increment the camera position
const float MOVE_DELTA = 0.2f;

/// Reflector position
glm::vec4 reflector = glm::vec4(0.0f);

/// Handles light information
struct Light {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

/// Adds directional light attributes
struct DirectionalLight: public Light {
	glm::vec4 direction;
};

/// Adds spot light attributes
struct SpotLight: public Light {
	glm::vec4 position;
	glm::vec4 spotDirection;
	float spotCosCutoff;
	float spotExponent;
};

/// Some of the global variables for camera and reflector
struct State {
	glm::mat4 projection;
	glm::vec3 cameraPosition;
	float cameraYaw;
	float cameraPitch;
	glm::vec3 cameraDirection;
	SpotLight refLights[2];
	double time;
	glm::mat4 view;
} state;

/// How mony vertices are there in quad, value is obvious
const int numQuadVertices = 4;

/// Mapping the texture
const float streamVertexData[numQuadVertices * 5] = {
	//x  y  z  u  v
	-1, -1, 0, 0, 0,
	 1, -1, 0, 1, 0,
	-1,  1, 0, 0, 1,
	 1,  1, 0, 1, 1,
};

/// Global data about the texture
struct Stream {
	GLuint vbo;
	GLuint vao;
	GLuint texture;
	MeshShaderProgram * shader;
	glm::mat4 matrix;
} stream;

/// From lihgting seminar, used to send information to the shader
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

/// From lihgting seminar, used to send information to the shader
struct Resources {
	LightingShader * shaderProgram;
} resources;

/// For handling time events
void FuncTimerCallback(int) {
	// this is from screenGraph
	state.time = 0.001 * (double)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds
	// ELAPSED_TIME is number of milliseconds since glutInit called 

	if(rootNode_p)
		rootNode_p->update(state.time);

	glutTimerFunc(33, FuncTimerCallback, 0);
	glutPostRedisplay();
}

/// Reloads the shader
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

/// Turns the reflector on or off (to oposite value)
void reflectorSwitch() {
	if (reflector == glm::vec4(0.0f)) reflector = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
	else  reflector = glm::vec4(0.0f);
	glutPostRedisplay();
}

/// Turns the animation on or off (to oposite value)
void animationSwitch() {
	AnimNode::animation = !AnimNode::animation;
}

/// Basic stuff that draw things, defines the view and such
void functionDraw() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection =  glm::perspective(60.0f, g_aspect_ratio, 1.0f, 10000.0f);

	state.view = glm::mat4(1.0f);
	state.view = glm::lookAt(state.cameraPosition,state.cameraDirection+state.cameraPosition,glm::vec3(0,1,0));

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
	state.refLights[0].position = state.view * glm::vec4(1.0f, 20.0f, 1.0f, 1.0f);
	state.refLights[0].spotDirection = state.view * reflector;

	if(rootNode_p)
		rootNode_p->draw(state.view, projection);
}


/// Should draw textured quad
void drawTexturedQuad(const glm::mat4 & model, const glm::mat4 & view, const MeshShaderProgram * shader, GLuint vao, GLuint texture) {
	glUseProgram(shader->m_programId);

	glm::mat4 PVMmatrix = state.projection  * view * model;
	glUniformMatrix4fv(shader->m_PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMmatrix));        // model-view-projection
	glUniformMatrix4fv(shader->m_Vmatrix, 1, GL_FALSE, glm::value_ptr(view));               // view
	glUniform1f(shader->m_time, state.time);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuadVertices);

	CHECK_GL_ERROR();
}

/// Reloads the shader, used for texturing
MeshShaderProgram * reloadShader(MeshShaderProgram * shader, const char * vertSrt, const char * fragSrc) {
	if(shader)
	delete shader;

	GLuint shaderList[] = {
		pgr::createShaderFromFile(GL_VERTEX_SHADER,   vertSrt),
		pgr::createShaderFromFile(GL_FRAGMENT_SHADER, fragSrc),
		0
	};
	shader = new MeshShaderProgram(pgr::createProgram(shaderList));
	shader->initLocations();
	CHECK_GL_ERROR();
	return shader;
}

/// Sets the shader attributes for texturing
void setupShaderAttribsTexture(GLuint vao, GLuint vbo, MeshShaderProgram * shader) {
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if(shader->m_pos > -1) {
		glEnableVertexAttribArray(shader->m_pos);
		glVertexAttribPointer(shader->m_pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), 0);
	}

	if(shader->m_texCoord > -1) 	{
		glEnableVertexAttribArray(shader->m_texCoord);
		glVertexAttribPointer(shader->m_texCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void *)(3 * sizeof(GL_FLOAT)));
	}

	glBindVertexArray( 0 );
	CHECK_GL_ERROR();
}

/// Creates the terrain and adds it to the scene graph
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

/// Creates the water/beer stream and adds it to the scene graph
void createStream() {
	TransformNode* stream_transform = new TransformNode("streamTranf", rootNode_p);
	stream_transform->translate(glm::vec3(0.0f, 70.0f, 0.0f)+AnimNode::config.points()[0]);
	stream_transform->scale(glm::vec3(0.5f,100.0f,0.5f));

	MeshGeometry* meshGeom_p = MeshManager::Instance()->get(STREAM_FILE_NAME);
	MeshNode * stream_mesh_p = new MeshNode("stream", stream_transform);
	stream_mesh_p->setGeometry(meshGeom_p);

	/*stream.texture = pgr::createTexture(STREAM_TEXTURE_FILE_NAME);
	stream.shader = reloadShader(stream.shader, "stream.vert", "stream.frag");
	glGenVertexArrays(1, &stream.vao);
	glGenBuffers(1, &stream.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, stream.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(streamVertexData), streamVertexData, GL_STATIC_DRAW);
	setupShaderAttribsTexture(stream.vao, stream.vbo, stream.shader);
	stream.matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.7f, -6.5f));
	stream.matrix = glm::scale(stream.matrix, glm::vec3(2.0f, 3.0f, 1.0f));*/
}

/// Creates the bottle and adds it to the scene graph
/// \param index Numeric identification of the bottle
/// \param offset Time offset of the bottle animation, where 1 represens movement between two points duration
void createBottle(int index = 0, float offset = 0.0f) {
	// Index the names so more bottles are possible
	std::stringstream ss; ss << index << std::flush;
	AnimNode* bottle_anim = new AnimNode("bottleAnim"+ss.str(),offset,rootNode_p);

	TransformNode* bottle_transform = new TransformNode("bottleTranf"+ss.str(), bottle_anim);
	bottle_transform->translate(glm::vec3(0.0, -12.5, 0.0));
	bottle_transform->scale(glm::vec3(4));

	MeshGeometry* meshGeom_p = MeshManager::Instance()->get(BOTTLE_FILE_NAME);
	MeshNode * bottle_mesh_p = new MeshNode("bottle"+ss.str(), bottle_transform);
	bottle_mesh_p->setGeometry(meshGeom_p);
}

/// Used to calculate camera direction from angles
void calculateState(void) {
	state.cameraDirection = glm::vec3(cos(state.cameraYaw),tan(state.cameraPitch),sin(state.cameraYaw));
	//state.cameraDirection = glm::vec3(cos(state.cameraYaw)*cos(state.cameraPitch),sin(state.cameraPitch),sin(state.cameraYaw)*cos(state.cameraPitch));
	// both works and acts the same, the second should be more right, but I like the forst one more
	// both acts wierd when paged up/down over the top/bottom, so it is limited by PITCHLIM constant (little less than pi/2)
}

/// Debug thing to flush camera position to the terminal in syntax usable in the code
void flushState(void) {
	std::cout << "state.cameraPosition = glm::vec3(" << state.cameraPosition.x << "f, " << state.cameraPosition.y << "f, " << state.cameraPosition.x << "f);"  << std::endl;
	std::cout << "state.cameraPitch = " << state.cameraPitch << "f;"  << std::endl;
	std::cout << "state.cameraYaw = " << state.cameraYaw << "f;"  << std::endl;
}

/// Switches the camera
/// \param cam Numeric identification of the camera
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

/// Event processing of the menu commands
/// \param item Numeric identification of the menu command
void myMenu(int item) {
	switch(item) {
	case 1:
	case 2:
	case 3:
		switchCam(item);
		break;
	case 66:
		animationSwitch();
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

/// Menu preparation
void createMenu(void) {
	int submenuID = glutCreateMenu(myMenu);
	glutAddMenuEntry("Static 1         [B]", 1);
	glutAddMenuEntry("Static 2         [N]", 2);
	glutAddMenuEntry("Free camera      [F]", 3);
	
	glutCreateMenu(myMenu);
	glutAddSubMenu("Camera", submenuID);

	glutAddMenuEntry("Animation on/off [A]", 66);
	glutAddMenuEntry("Reflector on/off [R]", 77);
	glutAddMenuEntry("Debug info       [D]", 88);
	glutAddMenuEntry("Exit           [Esc]", 99);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/// Putts objects to the scene, creates screene graph
void initializeScene() {
	// create scene root node
	rootNode_p = new SceneNode("root");
	createTerrain();
	createStream();
	// calculate the offset so bottles are evanly positioned
	for (int i=0; i < AnimNode::config.bottles(); i++) {
		createBottle(i,i*float(AnimNode::config.fragments())/AnimNode::config.bottles()); // casting to float has to be done at least on one of those integers
	}
	// dump our scene graph tree for debug
	rootNode_p->dump();
}

/// OpenGL crap doing magic
void display() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	functionDraw();
	glutSwapBuffers();
}

/// Used when window size is changed
/// \param w New window width
/// \param h New window heigh
void reshape (int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	g_aspect_ratio = (float)w/(float)h;
	g_win_w = w;
	g_win_h = h;
}

/// Handles pressing normal keys on the keyboard
/// \param key Pressed key value
/// \param x Guess it's mouse coursor X coordinate, not used here
/// \param y Guess it's mouse coursor Y coordinate, not used here
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
	case'a':
	case'A':
		animationSwitch();
		break;
	}
}

/// Handles pressing special keys on the keyboard
/// \param specKey Pressed key special code
/// \param x Guess it's mouse coursor X coordinate, not used here
/// \param y Guess it's mouse coursor Y coordinate, not used here
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


/// Initialise the program
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}

/// Program starts here, might be mixed with init()
/// I have no idea why something is here and something there
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
