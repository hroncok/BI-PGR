//----------------------------------------------------------------------------------------
/**
 * \file    AnimNode.cpp
 * \author  Miroslav Hroncok
 * 
 * This class is used to calculete the path of the bottles.
 * It uses the config and offset, but other that that, it's just SceneNode descendant.
 */
//----------------------------------------------------------------------------------------
#include "AnimNode.h"

AnimNode::AnimNode(const std::string &name, float offset, SceneNode* parent):
	SceneNode(name, parent), m_offset(offset) {}

void AnimNode::update(double elapsed_time) {
	float mytime;
	if (animation) mytime = elapsed_time/3.0f; // make it slower
	else mytime = 0; // time 0
	mytime += m_offset; // add the offset after the deviding
	float dec = mytime - floor(mytime); // decimal part
	int seconds = int(mytime - dec); // full part

	// Calculate value of F1-F4 (p-s is used)
	glm::vec4 F = glm::vec4(0.0f);
	F.p = 2*pow(dec,3)-3*pow(dec,2)+1;
	F.q = -2*pow(dec,3)+3*pow(dec,2);
	F.r = pow(dec,3)-2*pow(dec,2)+dec;
	F.s = pow(dec,3)-pow(dec,2);

	// You can add more points and more vectors here, use the same amount of both!
	// removed when added config file
	//glm::vec3 points[] = {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(20.0f,0.0f,30.0f), glm::vec3(-30.0f,0.0f,0.0f), glm::vec3(15.0f,0.0f,-30.0f)};
	//glm::vec3 vectors[] = {glm::vec3(0.0f,0.0f,-50.0f), glm::vec3(50.0f,0.0f,-10.0f), glm::vec3(0.0f,0.0f,50.0f), glm::vec3(-50.0f,0.0f,0.0f)};
	
	// Set the starting and ending points and vectors from the config
	glm::vec3 start, end, startv, endv;
	start = config.points()[seconds%config.fragments()];
	startv = config.vectors()[seconds%config.fragments()];
	end = config.points()[(seconds+1)%config.fragments()];
	endv = -config.vectors()[(seconds+1)%config.fragments()];
	
	// Calculate current position
	glm::vec3 mat3 = glm::vec3(0.0f);
	mat3.x = start.x*F.p + end.x*F.q + startv.x*F.r + endv.x*F.s;
	//mat3.y = start.y*F.p + end.y*F.q + startv.y*F.r + endv.y*F.s; // if it is not 0 all time, uncomment this line
	mat3.z = start.z*F.p + end.z*F.q + startv.z*F.r + endv.z*F.s;
	m_local_mat = glm::translate(glm::mat4(1.0f),mat3);
	/// call inherited update (which calculates global matrix and updates children)
	SceneNode::update(elapsed_time);
}
