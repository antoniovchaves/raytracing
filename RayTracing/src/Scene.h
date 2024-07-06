#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <algorithm> // Para std::copy


struct Material
{	
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metalic = 0.0f;
};

struct Sphere
{
	glm::vec3 Position{ 0.0f };
	float Radius = 0.5f;

	int MaterialIndex = 0;
};

struct Plane
{
	float a, b, c, d; // Coeficientes do plano na forma ax + by + cz + d = 0

	// Construtor padrão
	Plane(float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 0.0f)
		: a(a), b(b), c(c), d(d) {}
};


struct Box
{
	glm::vec3 Position{ 0.0f };
	float Width = 0.5f;
	float Height = 0.5f;
	float Depth = 0.5f;
	int MaterialIndex = 0;

	Plane planes[6]; // Um array de seis planos para definir a caixa

	// Construtor que aceita seis planos
	void SetPlanes(Plane p1, Plane p2, Plane p3, Plane p4, Plane p5, Plane p6)
	{
		planes[0] = p1;
		planes[1] = p2;
		planes[2] = p3;
		planes[3] = p4;
		planes[4] = p5;
		planes[5] = p6;
	}

};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
	std::vector<Box> Boxes;
	std::vector<Plane> Planes;
};