//----------------------------------------------------------------------------------------
/**
 * \file    Configuration.h
 * \author  Miroslav Hroncok
 * 
 * Courswork for BI-PGR on FIT CTU.
 * This class is used to handle config and load it form file
 */
//----------------------------------------------------------------------------------------
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
	int bottles();
	int fragments();
	glm::vec3 * points();
	glm::vec3 * vectors();
protected:
	int m_bottles;
	int m_fragments;
	glm::vec3 * m_points;
	glm::vec3 * m_vectors;
};

#endif
