#include "Renderer.h"
#include "Walnut/Random.h"

#include <execution>

namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}
void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);

	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;

}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;

	const glm::vec3& rayOrigin = camera.GetPosition();


	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);
					color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
				});
		});


	m_FinalImage->SetData(m_ImageData);

}
glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];


	glm::vec3 color(0.0f);
	float multiplier = 1.0f;
	int repeticoes = 2;

	for (int i = 0; i < repeticoes; i++)
	{	
		for (int j = 0; j < repeticoes; j++)
		{
			Renderer::HitPayload payload = TraceRay(ray);

			if (payload.HitDistance < 0.0f)
			{	
				glm::vec3 skyColor = glm::vec3(0.0f);
				color += skyColor * multiplier;
				break;
			} 

			glm::vec3 randomPoint = Walnut::Random::Vec3(-0.12f, -0.1f);
			glm::vec3 pointOnLight =  glm::vec3((float)i, -1.0f, (float)j);//glm::vec3(-1.0f);

			glm::vec3 lightDir = glm::normalize(randomPoint + pointOnLight);

			float d = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(alngulo entre eles)

			Ray lightRay;
			lightRay.Origin = payload.WorldPosition;
			lightRay.Direction = -lightDir;


			Renderer::HitPayload lightPayload = TraceRay(lightRay);

			const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
			const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

			if (lightPayload.HitDistance < 0.0f)
			{
				glm::vec3 sphereColor = material.Albedo;
				sphereColor *= d;

				color += sphereColor * multiplier;
			}
			else
			{
				color += glm::vec3(0.0f)  * d;
			}

			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;

			ray.Direction = glm::reflect(ray.Direction,
				payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

			
			multiplier *= 0.4f;

			
		}
		
	}

	return glm::vec4(color, 1.0f);
}

std::pair<float, float> Renderer::intersectBox(const Ray& ray, const Box& box)
{
	glm::vec3 invDir = 1.0f / ray.Direction;

	glm::vec3 tMin = (box.Position - ray.Origin) * invDir;
	glm::vec3 tMax = (box.Position + glm::vec3(box.Width, box.Height, box.Depth) - ray.Origin) * invDir;

	glm::vec3 realMin = glm::min(tMin, tMax);
	glm::vec3 realMax = glm::max(tMin, tMax);

	float minVal = glm::max(realMin.x, glm::max(realMin.y, realMin.z));
	float maxVal = glm::min(realMax.x, glm::min(realMax.y, realMax.z));

	return { minVal, maxVal };
}


Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestObject = -1;
	float hitDistance = std::numeric_limits<float>::max(); // também poderia utilizar o FLT_MAX
	int indentifier = -1;

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		float radius = sphere.Radius;
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - radius * radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestObject = (int)i;
			indentifier = 0;
		}

	}

	for (size_t i = 0; i < m_ActiveScene->Boxes.size(); i++)
	{
		const Box& box = m_ActiveScene->Boxes[i];
		// Calcula a interseção do raio com a caixa
		auto [tmin, tmax] = intersectBox(ray, box);

		// Se houver interseção e a distância for menor que a menor encontrada até agora
		if (tmax >= 0 && tmin <= tmax && tmax < hitDistance)
		{
			hitDistance = tmax;
			closestObject = (int)i;
			indentifier = 1;
		}
	}

	if (closestObject < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestObject, indentifier);

	
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex, int indentifier)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	if (indentifier == 0)
	{
		const Sphere& closestObject = m_ActiveScene->Spheres[objectIndex];

		const Material& material = m_ActiveScene->Materials[closestObject.MaterialIndex];

		glm::vec3 origin = ray.Origin - closestObject.Position;

		payload.WorldPosition = origin + ray.Direction * hitDistance;
		payload.WorldNormal = glm::normalize(payload.WorldPosition);

		payload.WorldPosition += closestObject.Position;
	}
	else {
		const Box& closestObject = m_ActiveScene->Boxes[objectIndex];

		const Material& material = m_ActiveScene->Materials[closestObject.MaterialIndex];

		glm::vec3 origin = ray.Origin - closestObject.Position;

		payload.WorldPosition = origin + ray.Direction * hitDistance;
		payload.WorldNormal = glm::normalize(payload.WorldPosition);

		payload.WorldPosition += closestObject.Position + 
			glm::vec3(closestObject.Width, closestObject.Height, closestObject.Depth);
	}


	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}