#include "EllipseAnimNode.h"

// ======== BEGIN OF SOLUTION - TASK 3-1 ======== //
// implement EllipseAnimNode methods here

EllipseAnimNode::EllipseAnimNode(const std::string &name, SceneNode* parent):
  SceneNode(name, parent), m_majorAxis(1, 0, 0), m_minorAxis(0, 1, 0), m_a(1), m_b(1), m_angle(0), m_speed(1)
{
}

void EllipseAnimNode::update(double elapsed_time)
{
	m_angle = m_speed * elapsed_time;

	float x = m_a * cos(m_angle);
	float y = m_b * sin(m_angle);
	
    m_local_mat = glm::translate(glm::mat4(1.0f), x * m_majorAxis + y * m_minorAxis);

	float angle = atan(-(pow(m_b,2)*x)/(pow(m_a,2)*y));

	if(fmod(m_angle, 2*M_PI) > M_PI)
		angle += M_PI;

	m_local_mat = glm::rotate(m_local_mat,angle * 180 / M_PI,glm::vec3(0,-1,0));

	SceneNode::update(elapsed_time);
}
// ========  END OF SOLUTION - TASK 3-1  ======== //

