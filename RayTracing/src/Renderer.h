#pragma once

#include "Walnut/Image.h"
#include "Camera.h"
#include "Scene.h"
#include "Ray.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
	};

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }

	Settings& GetSettings() { return m_Settings; }

	uint32_t GetFrameIndex() { return m_FrameIndex; }

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); //RayGen

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex, int indentifier);
	HitPayload Miss(const Ray& ray);

	std::pair<float, float> intersectBox(const Ray& ray, const Box& box);
	std::pair<float, float> Renderer::intersectPlane(const Ray& ray, const Plane& plane);
	uint32_t m_FrameIndex = 1;

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	const Scene*  m_ActiveScene  = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;


	Settings m_Settings;
};
