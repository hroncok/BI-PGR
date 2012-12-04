//----------------------------------------------------------------------------------------
/**
 * \file    Configuration.cpp
 * \author  Miroslav Hroncok
 * 
 * Courswork for BI-PGR on FIT CTU.
 * This class is used to handle config and load it form file
 */
//----------------------------------------------------------------------------------------
#include "Configuration.h"

/// The constructor loads the values
/// \param filename Filename of teh config file (config.txt is used as default)
Configuration::Configuration(const std::string & filename) {
	std::ifstream * m_config = new std::ifstream(filename);
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

/// Bottles getter
/// \return Number of bottles
int Configuration::bottles() {
	return m_bottles;
}

/// Fragments getter
/// \return Number of fragments on the path
int Configuration::fragments() {
	return m_fragments
;}

/// Points getter
/// \return Array of points on the paths
glm::vec3 * Configuration::points() {
	return m_points;
}

/// Vectors getter
/// \return Array of directional vectors for the path
glm::vec3 * Configuration::vectors() {
	return m_vectors;
}