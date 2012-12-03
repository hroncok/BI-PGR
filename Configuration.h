#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <fstream>
#include <iostream>
#include <string>
#include "pgr.h"

class Configuration {
public:
	Configuration(const std::string & filename = "config.txt");
	~Configuration();
	int bottles() {return m_bottles;}
	int fragments() {return m_fragments;}
	glm::vec3 * points() {return m_points;}
	glm::vec3 * vectors() {return m_vectors;}
protected:
	std::ifstream * m_config;
	int m_bottles;
	int m_fragments;
	glm::vec3 * m_points;
	glm::vec3 * m_vectors;
};

#endif
