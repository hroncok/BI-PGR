
#include "RotationAnimNode.h"

RotationAnimNode::RotationAnimNode(const std::string &name, SceneNode* parent):
  SceneNode(name, parent), m_axis(1, 0, 0), m_angle(0), m_speed(0)
{
}

void RotationAnimNode::update(double elapsed_time)
{
  // ======== BEGIN OF SOLUTION - TASK 2-1 ======== //
  // use m_speed and elapsed_time to calculate the angle
  m_angle = m_speed * elapsed_time;
  // use glm to create rotation matrix from the angle
  m_local_mat = glm::rotate(glm::mat4(1.0f),m_angle * 180/ M_PI,m_axis);
  // ========  END OF SOLUTION - TASK 2-1  ======== //

  /// call inherited update (which calculates global matrix and updates children)
  SceneNode::update(elapsed_time);
}
