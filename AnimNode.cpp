
#include "AnimNode.h"
#define FRAGMENTS 2


AnimNode::AnimNode(const std::string &name, SceneNode* parent):
	SceneNode(name, parent), m_axis(1, 0, 0), m_angle(0), m_speed(0) {
}

void AnimNode::update(double elapsed_time) {
	float mytime = elapsed_time/3.0f; // make it slower
	float dec = mytime - floor(mytime); // decimal part
	int seconds = int(mytime - dec); // full part

	glm::vec4 F = glm::vec4(0.0f);
	F.p = 2*pow(dec,3)-3*pow(dec,2)+1;
	F.q = -2*pow(dec,3)+3*pow(dec,2);
	F.r = pow(dec,3)-2*pow(dec,2)+dec;
	F.s = pow(dec,3)-pow(dec,2);

	// TODO add more points and do more fragments
	const glm::vec3 point1 = glm::vec3(10.0f,0.0f,10.0f);
	const glm::vec3 point2 = glm::vec3(30.0f,0.0f,30.0f);
	const glm::vec3 offset1 = glm::vec3(0.0f,0.0f,35.0f);
	const glm::vec3 offset2 = glm::vec3(0.0f,0.0f,45.0f);
	glm::vec3 start, end, startv, endv;
	
	switch (seconds%FRAGMENTS) {
	case 0:
		start = point1; startv = start + offset1;
		end = point2; endv = end + offset2;
		break;
	case 1:
		start = point2; startv = start - offset2;
		end = point1; endv = end - offset1;
		break;
	}
	glm::vec3 mat3 = glm::vec3(0.0f);
	mat3.x = start.x*F.p + end.x*F.q + startv.x*F.r + endv.x*F.s;
	mat3.z = start.z*F.p + end.z*F.q + startv.z*F.r + endv.z*F.s;
	m_local_mat = glm::translate(glm::mat4(1.0f),mat3);
	/// call inherited update (which calculates global matrix and updates children)
	SceneNode::update(elapsed_time);
}
