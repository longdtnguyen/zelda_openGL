#pragma once

#include <glm/glm.hpp>

// Material surface properties to be used as input into a local illumination model
// (e.g. the Phong Reflection Model).
struct Material {
	Material()
			: kd(glm::vec3(0.0f)),
			  ks(glm::vec3(0.0f)),
			  shine(0.0f),
			  opaque(0.0f),status(0) { }

	// Diffuse reflection coefficient
	glm::vec3 kd;
	glm::vec3 ks;
	float shine;
	float opaque;
	int status;
};
