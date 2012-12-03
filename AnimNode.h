#ifndef ANIM_NODE_H
#define ANIM_NODE_H

#include "resources/SceneNode.h"


class AnimNode : public SceneNode {
public:
	AnimNode(const std::string & name = "<AnimNode>", SceneNode* parent = NULL);
	~AnimNode() {}

	void setSpeed(float speed) { m_speed = speed; }
	void setAxis(const glm::vec3 & axis) { m_axis = axis; }

	void update(double elapsed_time);
	
protected:
	glm::vec3 m_axis;
	float m_angle;
	float m_speed;
};

#endif
