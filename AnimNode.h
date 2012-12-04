//----------------------------------------------------------------------------------------
/**
 * \file    AnimNode.h
 * \author  Miroslav Hroncok
 * 
 * This class is used to calculete the path of the bottles.
 * It uses the config and offset, but other that that, it's just SceneNode descendant.
 */
//----------------------------------------------------------------------------------------

#ifndef ANIM_NODE_H
#define ANIM_NODE_H

#include "resources/SceneNode.h"
#include "Configuration.h"

class AnimNode : public SceneNode {
public:
	AnimNode(const std::string & name = "<AnimNode>", float offset = 0.0f, SceneNode* parent = NULL);
	~AnimNode() {}

	void update(double elapsed_time);
	static bool animation; // is the animation working
	static Configuration config;
protected:
	float m_offset;
};

#endif
