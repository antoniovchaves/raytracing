#include "Renderer.h"
#include "Walnut/Random.h"

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

}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;

	const glm::vec3& rayOrigin = camera.GetPosition();


	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

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
		
			glm::vec3 randomPoint = glm::vec3(-1.0f, -1.0f, -1.0f); //Walnut::Random::Vec3(-0.12f, -0.1f);
			glm::vec3 pointOnLight =  glm::vec3((float)i, -1.0f, (float)j);//glm::vec3(-1.0f);

			glm::vec3 lightDir = glm::normalize(randomPoint + pointOnLight);

			float d = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(alngulo entre eles)

			const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
			const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

			glm::vec3 sphereColor = material.Albedo;
			sphereColor *= d;

			color += sphereColor * multiplier;

			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
			
			if (material.isReflective)
			{
				ray.Direction = glm::reflect(ray.Direction,
					payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.1f, 0.1f)
				);
				multiplier *= 0.7f;
			}
			else { multiplier *= 0.5f; }
		}
		
	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max(); // tamb�m poderia utilizar o FLT_MAX

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

		// float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // n�o utilizado atualmente
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}

	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);

	
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];
	const Material& material = m_ActiveScene->Materials[closestSphere.MaterialIndex];
	float radius = closestSphere.Radius;
	glm::vec3 origin = ray.Origin - closestSphere.Position;


	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(alngulo entre eles)


	glm::vec3 sphereColor = material.Albedo;
	sphereColor *= d;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}