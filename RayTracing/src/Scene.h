#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{	
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metalic = 0.0f;

	bool isReflective = false;
};

struct Sphere
{
	glm::vec3 Position{ 0.0f };
	float Radius = 0.5f;

	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};