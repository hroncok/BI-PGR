//----------------------------------------------------------------------------------------
/**
 * \file    sceneGraph.cpp
 * \author  Tomas Barak, Vlastimil Havran, Jaroslav Sloup
 * \date    2012
 * \brief   Scene Graph Seminar
 */
//----------------------------------------------------------------------------------------

#include <string>
#include <vector>
#include <cstdio>
#include <string.h>
#include <iostream>

#include "pgr.h"   // includes all PGR libraries, like shader, glm, assimp ...

// scene graph includes
#include "sceneGraph/SceneNode.h"       // superclass of all scene graph nodes
#include "sceneGraph/TransformNode.h"   // model transformation
#include "sceneGraph/MeshNode.h"        // model loaded from the file
#include "sceneGraph/Resources.h"
#include "sceneGraph/MeshGeometry.h"
#include "sceneGraph/AxesNode.h"        // coordinate axes

// nodes derived from scene graph nodes
#include "EllipseAnimNode.h" // animation - change of scale
#include "RotationAnimNode.h"// animation transformation

#define TITLE "Seminar 6 - Scene Graph"

// file name used during the scene graph creation
#define TERRAIN_FILE_NAME "./data/terrain"
#define MIG_FILE_NAME "./data/MiG25/MiG25.obj"
#define CESSNA_FILE_NAME pgr::frameworkRoot() + "data/cessna.obj"
#define JEEP_FILE_NAME pgr::frameworkRoot() + "data/Jeep/Jeep.obj"

// scene graph root node
SceneNode *        rootNode_p      = NULL; // scene root
// jeep model node
MeshNode*          jeep_mesh_p    = NULL;    // used for getting to know the jeep_transformation
RotationAnimNode*  jeep_rot       = NULL;

// TASK 2 and 3 - variables and constants
#define   RADIUS_A   25.0
#define   RADIUS_B   15.0

const glm::quat ORIENTATION_FROM_Z_PLUS =  glm::quat(-0.994f, -0.030f,   0.027f,  -0.014f );

// global variables
float     g_aspect_ratio = 1.0f;
bool      g_rotationOn   = false;   // mouse draging rotation
int       g_mouse_old_x;            // updated in myMotion4
int       g_mouse_old_y;
int       g_win_w, g_win_h;         // windowSize
float     g_scale;
glm::vec3 g_translation;

glm::quat g_curOrientation,         // current camera Orientation   
          g_incQuat;                // increment quaternion (during mouse rotation)

void FuncTimerCallback(int)
{
  double timed = 0.001 * (double)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds
  // ELAPSED_TIME is number of milliseconds since glutInit called 

  if(rootNode_p)
     rootNode_p->update(timed);

  glutTimerFunc(33, FuncTimerCallback, 0);
  glutPostRedisplay();
}

void dumpMatrix( const char* title, const glm::mat4 m )
{
  printf("%s\n", title );
  for( int i = 0; i < 4; i++ )
  {
    //cout << endl;
    printf( " %7.3f, %7.3f, %7.3f, %7.3f \n", m[0][i], m[1][i], m[2][i], m[3][i] );
  }
}

void dumpQuat( const char* title, glm::quat q)
{
  printf("%s\n", title );
  printf( "Quat: %s: = [ %7.3f, %7.3f, %7.3f, %7.3f ]\n", title, q.x, q.y, q.z, q.w );
}

void functionDraw()
{
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 projection =  glm::perspective(60.0f, g_aspect_ratio, 1.0f, 10000.0f);

  glm::mat4 view = glm::mat4(1.0);
  view  = glm::translate( view, g_translation );
  view *= glm::mat4_cast(g_curOrientation);  // function converts a quaternion into a 4x4 rotation matrix

  //dumpQuat( "g_curOrientation", g_curOrientation );

  if(rootNode_p)
    rootNode_p->draw(view, projection);

}

#define TRACKBALLSIZE  (0.8f)

// Project a 2D point in screen space [pixels] onto a deformed sphere to get a 3D point on the unit sphere
glm::vec3 projectToSphere( int p_x, int p_y, int w, int h )
{
  //Project the point to sphere
  glm::vec3 v;

  // point in the xy plane [-1..1]^2
  v.x = (float)(2 * p_x   - w) / w;  // window [x,y] position to intrerval <-1,1> relatively to the window size
  v.y = (float)(h - 2.0 * p_y) / h;  // convert to interval -1..1
  v.z = 0;                           // is recomputed below!!

  //Point on a deformed unit sphere     d^2 + (v.z)^2 = R^2, R=1
  float d = glm::length( v );        // distance from point to the center of the circle in xy plane
#if 0
  if( d > 1.0 )                      // point will be in the circle (d must be < R => z > 0), or on  it (d=R => z=0)
    d = 1.0;                         // point out put back to the R=1 circle in xy plane
  v.z = (float)sqrt( 1.00 - d*d);    // compute z of the 3D point on the sphere 
#else
  float r = TRACKBALLSIZE;           // without uncomfort movement in the screen corners
  if (d < r * 0.70710678118654752440f) { /* Inside sphere */
    v.z = sqrt(r*r - d*d);
  } else {                              /* On hyperbola */
    float t = r / 1.41421356237309504880f;
    v.z = t*t / d;
  }
#endif
  //change the point to a unit vector
  v /= glm::length( v );  //normalize - normalization of points outside the circle (the othes have already length one)

  return v;
}

/*
 *  Given an axis and angle, compute quaternion.
 */
glm::quat axisAngleToQuat( const float angle, const glm::vec3 axis )
{
  float s = (float) sin( angle / 2.0);
  float c = (float) cos( angle / 2.0);
  glm::vec3 dir = glm::normalize( axis );
  dir *= s;
  return glm::quat( c, dir); 
}


glm::quat trackball( int p1_x, int p1_y, int p2_x, int p2_y, int w, int h )
{
  glm::vec3 v1, v2;
  float     t;
  float     phi;

  if (p1_x == p2_x && p1_y == p2_y) {
    return glm::quat(); // (1.0, 0.0, 0.0, 0.0);         // Zero rotation
  }
  else
  {
    v1 = projectToSphere( p1_x, p1_y, w, h );
    v2 = projectToSphere( p2_x, p2_y, w, h );

    glm::vec3 axis = glm::cross( v1, v2 );  // should be normaized, but it isn't...

    // determine, what angle to rotate
    t = glm::length(v1 - v2) / 2.0f; 

    if( t >  1.0 )  t =  1.0; 
    if( t < -1.0 )  t = -1.0;

    phi = 2.0f * asin(t);

    return axisAngleToQuat( phi, axis );
  }
}

void myMouse( int button, int state, int x, int y)
{
  if( button == GLUT_LEFT_BUTTON )
  {
    if( state == GLUT_DOWN )
    {
      g_rotationOn = true;
      g_mouse_old_x = x;       // updated in myMotion
      g_mouse_old_y = y;
    }
    else
    {
      g_rotationOn = false;
    }

    glutPostRedisplay();
  }
}

// trackball pomoci kvaternionu / viz MesaDemos, soubory trackball.h a trackball.c
// Vzdy pocita mouse_old_xy jen od predchoziho volani myMotion, cili inkrementuje malicke kvaterniony
//   odpovidajici posunuti od prechoziho volani myMotion (a ne od pozice pri zahajeni interakce)
void myMotion( int x, int y)
{
  if( g_rotationOn ) {
    g_incQuat = trackball( g_mouse_old_x, g_mouse_old_y, x, y, g_win_w, g_win_h);
    g_curOrientation = g_incQuat * g_curOrientation;

    g_mouse_old_x = x;
    g_mouse_old_y = y;
  }

  glutPostRedisplay();
}

/// helper function - creates AxesNode and attaches is to supplied parent
SceneNode * createAxes(const std::string & name = "axes", float scale = 1, SceneNode * parent = NULL)
{
  TransformNode * axes_transform = new TransformNode(name + "_scale", parent);
  axes_transform->scale(glm::vec3(scale));
  return new AxesNode(name, axes_transform);
}

void createTerrain()
{
  TransformNode* terrain_transform = new TransformNode("terrainTranf", rootNode_p);

  terrain_transform->translate(glm::vec3(0.0, -17, 0.0));
  terrain_transform->scale(glm::vec3(80.0, 20.0, 80.0));

  if(!MeshManager::Instance()->exists(TERRAIN_FILE_NAME))
    MeshManager::Instance()->insert(TERRAIN_FILE_NAME, MeshGeometry::LoadRawHeightMap(TERRAIN_FILE_NAME));
  MeshGeometry * mesh_p = MeshManager::Instance()->get(TERRAIN_FILE_NAME);

  MeshNode* terrain_mesh_p = new MeshNode(TERRAIN_FILE_NAME, terrain_transform);
  terrain_mesh_p->setGeometry(mesh_p);
}

void createJeep()
{
  // ======== BEGIN OF SOLUTION - TASK 1-1 ======== //
  // create new TransformNode, use its rotate and translate methods and use it as a parent of the jeep node
  TransformNode* jeep_transform = new TransformNode("jeepTranf", rootNode_p);
  jeep_transform->translate(glm::vec3(0.0, -14, 0.0));
  jeep_transform->rotate(-90,glm::vec3(1,0,0));
  jeep_transform->scale(glm::vec3(4));

  
  MeshGeometry* meshGeom_p = MeshManager::Instance()->get(JEEP_FILE_NAME);
  jeep_mesh_p = new MeshNode("jeep", jeep_transform);
  jeep_mesh_p->setGeometry(meshGeom_p);
  // ========  END OF SOLUTION - TASK 1-1  ======== //
}

void createCessna()
{
  const float A = 25;
  const float B = 15;

  // ======== BEGIN OF SOLUTION - TASK 1-2 ======== //
  EllipseAnimNode* cessna_ellipse = new EllipseAnimNode("cessnaEllipse", rootNode_p);
  cessna_ellipse->setAB(A,B);
  cessna_ellipse->setAxis(glm::vec3(1,0,0),glm::vec3(0,0,1));

  TransformNode* cessna_tran = new TransformNode("cessnaTranf", cessna_ellipse);
  cessna_tran->rotate(90,glm::vec3(1,0,0));
  cessna_tran->scale(glm::vec3(8));

  MeshGeometry * meshGeom_p = MeshManager::Instance()->get(CESSNA_FILE_NAME);
  MeshNode * cessna_mesh_p = new MeshNode("cessna", cessna_tran );
  cessna_mesh_p->setGeometry(meshGeom_p);
  // ========  END OF SOLUTION - TASK 1-2  ======== //
}

void createMig()
{
  // ======== BEGIN OF SOLUTION - TASK 1-3 ======== //
  RotationAnimNode* mig_rotation = new RotationAnimNode("migRotation", rootNode_p);
  mig_rotation->setSpeed(1);
  mig_rotation->setAxis(glm::vec3(0,1,0));

  TransformNode* mig_tran = new TransformNode("migTranf", mig_rotation);
  mig_tran->translate(glm::vec3(-1,0,0));

  MeshGeometry * meshGeom_p = MeshManager::Instance()->get(MIG_FILE_NAME);
  MeshNode * mig_mesh_p = new MeshNode("mig", mig_tran);
  mig_mesh_p->setGeometry(meshGeom_p);
  // ========  END OF SOLUTION - TASK 1-3  ======== //
}

void initializeScene()
{
  // create scene root node
  rootNode_p = new SceneNode("root");

  // you can uncomment this to display origin of the scene
  //createAxes("scene_axes", 1.0f, rootNode_p);

  createTerrain();

  createJeep();
  createCessna();
  createMig();

  // dump our scene graph tree for debug
  rootNode_p->dump();
}

//Called to update the display.
//You should call glutSwapBuffers after all of your rendering to display what you rendered.
//If you need continuous updates of the screen, call glutPostRedisplay() at the end of the function.
void display()
{
  functionDraw();
  glutSwapBuffers();
}

//Called whenever the window is resized. The new window size is given, in pixels.
//This is an opportunity to call glViewport or glScissor to keep up with the change in size.
void reshape (int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  g_aspect_ratio = (float)w/(float)h;
  g_win_w = w;
  g_win_h = h;

  // good place for perspective setup
  // put to functionDraw(), based on g_aspect_ratio
}

//Called whenever a key on the keyboard was pressed.
//The key is given by the ''key'' parameter, which is in ASCII.
//It's often a good idea to have the escape key (ASCII value 27) call glutLeaveMainLoop() to
//exit the program.
void myKeyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27:
      exit(0);//glutLeaveMainLoop();
      break;
    case'x':
      g_translation.x += 0.5;
      glutPostRedisplay();
      break;
    case'X':
      g_translation.x -= 0.5;
      glutPostRedisplay();
      break;
    case'y':
      g_translation.y += 0.5;
      glutPostRedisplay();
      break;
    case'Y':
      g_translation.y -= 0.5;
      glutPostRedisplay();
      break;
    case'z':
      g_translation.z += 0.5;
      glutPostRedisplay();
      break;
    case'Z':
      g_translation.z -= 0.5;
      glutPostRedisplay();
      break;
    case 's':
      jeep_rot->setSpeed(0.0f);
      break;
    case 'S': 
      jeep_rot->setSpeed(M_PI / 10.0f);
      //glutPostRedisplay();
      break;
    case 'r':
    case 'R':   // reset of the view
      g_curOrientation = ORIENTATION_FROM_Z_PLUS;
      glutPostRedisplay();
      break;
  }
}

// Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void init()
{
  initializeScene();
  g_curOrientation = ORIENTATION_FROM_Z_PLUS; 

  g_translation    = glm::vec3(0.0f, 0.0f, -51.0f);

  //glDisable(GL_CULL_FACE); // draw both back and front faces
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE); // draw front faces  only
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
}

void mySpecialKeyboard(int specKey, int x, int y)
{
  switch (specKey)
  {
    case GLUT_KEY_UP:
      break;
    case GLUT_KEY_DOWN:
      break;
  }
}

/*
 * Entry point
 */
int main(int argc, char** argv)
{
  // initialize windonwing system
  glutInit(&argc, argv);

  glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  // initial window size
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(800,100);
  glutCreateWindow(TITLE);

  // register callback for drawing a window contents
  glutDisplayFunc(display);
  // register callback for change of window
  glutReshapeFunc(reshape);
  // register callback for keyboard
  glutKeyboardFunc(myKeyboard);
  glutSpecialFunc(mySpecialKeyboard);
  glutMouseFunc(myMouse);
  glutMotionFunc(myMotion);
  glutTimerFunc(33, FuncTimerCallback, 0);

  if(!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
    pgr::dieWithError("pgr init failed, required OpenGL not supported?");

  init();

  glutMainLoop();
  return 0;
}
