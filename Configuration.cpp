
#include "Configuration.h"

Configuration::Configuration(const std::string & filename) {
	m_config = new std::ifstream(filename);
	if (!m_config->is_open() || !(*m_config >> m_bottles) || !(*m_config >> m_fragments)) {
		printf("Cannot open/read config file %s\n",filename);
		exit(1);
	}
	m_points = (glm::vec3 *) malloc(sizeof(glm::vec3)*m_fragments);
	m_vectors = (glm::vec3 *) malloc(sizeof(glm::vec3)*m_fragments);
	for (int i=0; i < m_fragments; i++) {
		m_points[i] = glm::vec3(0.0f);
		m_vectors[i] = glm::vec3(0.0f);
		// the config file only works for 2D (add loading Y here, if you want different)
		if (!(*m_config >> m_points[i].x) || !(*m_config >> m_points[i].z) || !(*m_config >> m_vectors[i].x) || !(*m_config >> m_vectors[i].z)) {
			printf("Cannot open/read config file %s on point n. %d\n",filename,i);
			exit(1);
		}
	}
	m_config->close();
	delete(m_config);
}

Configuration::~Configuration() {
	delete(m_points);
	delete(m_vectors);
}

