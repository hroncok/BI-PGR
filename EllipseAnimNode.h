#ifndef ELLIPSE_ANIM_NODE_H
#define ELLIPSE_ANIM_NODE_H

#include "SceneNode.h"

// ======== BEGIN OF SOLUTION - TASK 3-2 ======== //
class EllipseAnimNode: public SceneNode
{
  // add constructor, declare the update(), setSpeed() and setAxes() methods
public:
  EllipseAnimNode(const std::string & name = "<EllipseAnimNode>", SceneNode* parent = NULL);
  ~EllipseAnimNode() {}

  void setSpeed(float speed) { m_speed = speed; }
  void setAxis(const glm::vec3 & majorAxis,const glm::vec3 & minorAxis) { m_majorAxis = majorAxis; m_minorAxis = minorAxis; }
  void setAB (float a, float b) { m_a = a; m_b = b; }

  void update(double elapsed_time);
  // add m_angle, m_speed, m_majorAxis and m_minorAxis properties
protected:
  glm::vec3 m_majorAxis, m_minorAxis;
  float m_angle;
  float m_speed;
  float m_a, m_b;

};
// ========  END OF SOLUTION - TASK 3-2  ======== //

#endif
