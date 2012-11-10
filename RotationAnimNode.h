#ifndef ROTATION_ANIM_NODE_H
#define ROTATION_ANIM_NODE_H

#include "SceneNode.h"

class RotationAnimNode : public SceneNode
{
public:
  RotationAnimNode(const std::string & name = "<RotationAnimNode>", SceneNode* parent = NULL);
  ~RotationAnimNode() {}

  void setSpeed(float speed) { m_speed = speed; }
  void setAxis(const glm::vec3 & axis) { m_axis = axis; }

  void update(double elapsed_time);

protected:
  glm::vec3 m_axis;
  float m_angle;
  float m_speed;
};

#endif
