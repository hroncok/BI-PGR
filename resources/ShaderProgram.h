
#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "pgr.h"

class SceneNode;

class BasicShaderProgram
{
public:
  BasicShaderProgram(GLuint prId);
  virtual ~BasicShaderProgram();

  virtual void initLocations();
  virtual void updateUniforms(SceneNode * nd);

  GLuint m_programId;

  GLint m_PVMmatrix;
  /// shader view                  matrix ation
  GLint m_Vmatrix;
  /// shader model                 matrix ation
  GLint m_Mmatrix;
  /// inverse transposed VMmatrix
  GLint m_NormalMatrix;

  /// elapsed time in miliseconds ation
  GLint m_time;

};

class MeshShaderProgram: public BasicShaderProgram
{
public:
  MeshShaderProgram(GLuint prId);

  void initLocations();
  void updateUniforms(SceneNode * nd);

  /// material ations
  GLint m_diffuse;
  GLint m_ambient;
  GLint m_specular;
  GLint m_shininess;

  GLint m_texSampler;

  // attributes
  /// position attribute ation
  GLint m_pos;
  GLint m_normal;
  GLint m_texCoord;
  GLint m_useTexture;
  // cubemap
  GLint m_cubeMapTex;
  //GLint m_worldCameraPosition;
  //GLint m_reflectFactor;
};

#endif // SHADERPROGRAM_H
