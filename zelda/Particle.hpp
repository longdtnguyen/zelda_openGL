#pragma once

#include <glm/glm.hpp>

struct Particle{
	glm::vec3 pos, speed;
	float size;
	float life;
	Particle(): life(-1),size(0.5) {}
};