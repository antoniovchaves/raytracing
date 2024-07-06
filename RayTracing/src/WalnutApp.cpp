#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		Material& firstSphere = m_Scene.Materials.emplace_back();
		firstSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		firstSphere.Roughness = 0.0f;

		Material& secondSphere = m_Scene.Materials.emplace_back();
		secondSphere.Albedo = { 0.2f, 0.3f, 1.0f };
		secondSphere.Roughness = 0.0f;

		Material& firstBox = m_Scene.Materials.emplace_back();
		firstBox.Albedo = { 1.0f, 0.0f, 1.0f };
		firstBox.Roughness = 0.0f;

		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;

			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 3.0f, 0.0f, 0.0f };
			sphere.Radius = 2.0f;
			sphere.MaterialIndex = 1;

			m_Scene.Spheres.push_back(sphere);
		}

		Box& box = m_Scene.Boxes.emplace_back();
		box.Position = { 1.0f, 2.0f, 1.0f };
		box.Width = 10.0f;
		box.Height = 0.1f;
		box.Depth = 10.0f;
		box.MaterialIndex = 2;

		// Configurando os planos da caixa
		Plane p1(1, 0, 0, -box.Position.x);
		Plane p2(-1, 0, 0, box.Position.x + box.Width);
		Plane p3(0, 1, 0, -box.Position.y);
		Plane p4(0, -1, 0, box.Position.y + box.Height);
		Plane p5(0, 0, 1, -box.Position.z);
		Plane p6(0, 0, -1, box.Position.z + box.Depth);

		// Inicializando a caixa com os seis planos
		box.SetPlanes(p1, p2, p3, p4, p5, p6);


	}
	virtual void OnUpdate(float ts) override 
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();

		ImGui::End();

		

		ImGui::Begin("Scene");

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0.0f, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}
		for (size_t i = 0; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i];

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metalic", &material.Metalic, 0.05f, 0.0f, 1.0f);

			ImGui::Separator();

			ImGui::PopID();
		}
		
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if(image)
			ImGui::Image(image->GetDescriptorSet(), 
				{ (float)image->GetWidth(), (float)image->GetHeight() }, 
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		if (m_Renderer.GetFrameIndex() < 100)
			Render();
	}
	void Render() 
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;


	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}