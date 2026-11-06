#ifndef ASTEROIDFIELD
#define ASTEROIDFIELD

#include <cstdint>
#include <cmath>
#include <memory>
#include <random>
#include <vector>
#include "Scene/BaseRender.h"
#include "Scene/Sun.h"
#include "Scene/SplineCurve.h"
#include "Scene/BaseScene.h"
#include "Core/Ecs.h"
#include "Core/Transform.h"
#include "Core/Transform2d.h"
#include "Core/Light.h"
#include "Core/relaxation.h"
#include "Core/CreateSphere.h"

namespace FDB::Demo2 {

	struct polaris {};

	struct Asteroid
	{
		glm::vec3 velocity{ 0.f };
		glm::vec3 spinAxis{ 0.f, 1.f, 0.f };
		float     spinSpeed = 0.5f;
		float     side = 1.f;  
	};

	struct AsteroidField : public FDB::Scene::BaseRenderScene {
	private:
		struct ShipPath
		{
			SplineCurve curve;
			float distance = 0.f;
			float speed = 3.f;
			std::uint64_t entity = 0;  
		};
		std::vector<ShipPath> m_paths;
		bool  m_followSpline = true;
		bool  m_toggleWasDown = false;

		static constexpr int   kAsteroidCount = 5;
		static constexpr float kHalfWidth = 3.f;   
		static constexpr float kHalfHeight = 1.f;   
		static constexpr float kMargin = 2.f;  
		static constexpr float kRange = kHalfWidth + kMargin;
		static constexpr float kHeight = kHalfHeight;

		glm::vec3 m_camStart{ -1.f, 0.f, 1.f };
		glm::vec3 m_dir = glm::normalize(glm::vec3{ 1.f, 0.f, -1.f });
		glm::vec3 m_right = glm::normalize(glm::vec3{ 1.f, 0.f, 1.f });
		glm::vec3 m_up{ 0.f, 1.f, 0.f };
		std::mt19937 m_rng{ 1234 };

		float Rand(float a, float b)
		{
			return std::uniform_real_distribution<float>(a, b)(m_rng);
		}

		void ResetAsteroid(FDB::Core::Transform& t, Asteroid& a, bool initial, const glm::vec3& origin)
		{
			a.side = (Rand(0.f, 1.f) < 0.5f) ? 1.f : -1.f;

			float x = initial ? Rand(-kRange, kRange) : -a.side * kRange;
			float y = Rand(0.4f, kHeight) * ((Rand(0.f, 1.f) < 0.5f) ? 1.f : -1.f);
			float depth = Rand(0.8f, 2.2f);
			t.SetPosition(origin + m_right * x + m_up * y + m_dir * depth);

			float speed = Rand(0.8f, 2.5f);
			a.velocity = m_right * (a.side * speed) + m_up * Rand(-0.2f, 0.2f);
			a.spinAxis = glm::normalize(glm::vec3{ Rand(-1.f, 1.f), Rand(-1.f, 1.f), Rand(-1.f, 1.f) } + glm::vec3{ 0.01f });
			a.spinSpeed = Rand(0.3f, 1.5f);
		}

	public:
	protected:
		void InitScene() override
		{
			//TODO DO NOT FORGET TO CALL BASE INIT SCENE
			BaseRenderScene::InitScene();
			FDB::Core::Material material;
			material.baseColor = FDB::Core::textureLoader_t::Load("Textures/BaseColor.png");
			material.normalMap = FDB::Core::textureLoader_t::Load("Textures/Normal.png");
			material.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Emissive.png");
			material.OMRMap = FDB::Core::textureLoader_t::Load("Textures/ORM.png");

			// Polaris
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;

				transform.ScaleUp({ 0.5f,0.5f,0.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Polaris.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(material);
				transform.SetRotation({ 0, -1, 0 });
				polaris r{};
				m_reg.AddComponents(e, std::move(transform), std::move(mesh), std::move(r));
			}

			// sun
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ -60, 30.f,-70 });
				transform.ScaleUp({ 5.f,5.f,5.f });
				FDB::Core::MeshComp mesh;
				SunStruct sun;
				sun.dist = 99.f;
				mesh.mesh = FDB::Core::meshLoader_t::Load("sphere.obj");
				FDB::Core::Material mat;
				mat = m_baseMaterials;
				mat.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/SunEmissive.png");
				mat.baseColor = FDB::Core::textureLoader_t::Load("Textures/SunEmissive.png");
				mat.emissiveFactor = 5;
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(mat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh), std::move(sun));
			}

			// procedural mesh
			m_SphereMesh = std::make_unique<FDB::Core::Mesh>();
			auto p3d = RelaxAlgo::createSphere();
			std::vector<glm::vec3> unit;
			for (auto& p : p3d)
				unit.push_back(glm::normalize(p));
			auto sub = CreateSphere::TransformPointToTriangle(p3d);
			sub = CreateSphere::applyNoise(unit, sub.indices, 2.f, 2.f, 1.f);
			m_SphereMesh->subMeshes.push_back(std::move(sub));
			FDB::Core::MeshTools::ComputeTangent(*m_SphereMesh);

			// astéroïdes
			for (int i = 0; i < kAsteroidCount; ++i)
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				FDB::Core::MeshComp mesh;
				mesh.mesh = m_SphereMesh.get();

				float s = Rand(0.05f, 0.15f);
				transform.ScaleUp(glm::vec3{ s });

				Asteroid a;
				ResetAsteroid(transform, a, true, m_camStart);

				m_reg.AddComponents(e, std::move(transform), std::move(mesh), std::move(a));
			}

			// camera
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Camera camera;
				FDB::Core::Transform transform;
				camera.OrthoGraphic();
				transform.LookAtDir({ 1.f,0,-1 });
				transform.SetPosition(m_camStart);
				m_reg.AddComponents<FDB::Core::Camera, FDB::Core::Transform>(e, std::move(camera), std::move(transform));
			}
			// light
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Dir);
				light.SetPos(-m_dir * 1.5f + m_up * 0.8f);   // devant Polaris, côté caméra, un peu en hauteur
				light.SetColor(FDB::Color::whiteFloat);
				light.SetIntensity(20.f);
				m_reg.AddComponent(e, std::move(light));
			}
			// Light for ship
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos({ 0,0,-5 });
				light.SetDir(glm::normalize(glm::vec3{ 0,0,1 }));
				light.SetColor(FDB::Color::whiteFloat);
				light.SetSpotData(glm::pi<float>() / 6.f, 1.f);
				light.SetIntensity(300.f);

				ShipsLight shipLight;

				m_reg.AddComponents(e, std::move(light), std::move(shipLight));
			}
			// light sun
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Dir);
				light.SetDir(glm::normalize(glm::vec3{ 0,0,1 }));
				light.SetColor(FDB::Color::RGBAFloat(0.922, 0.906, 0.039, 1.0f));
				light.SetIntensity(100.f);
				SunLight sunLight;
				m_reg.AddComponents(e, std::move(light), std::move(sunLight));
			}
			//// ui
			//{
			//	FDB::Core::entity e = m_reg.CreateEntity();

			//	FDB::Core::Mesh2D uiMesh;
			//	uiMesh.data = &FDB::Core::square;
			//	uiMesh.texture = FDB::Core::textureLoader_t::Load("PBC.png");

			//	FDB::Core::Ui ui;
			//	ui.SetPosition({ 300,300 });
			//	ui.SetScale({ 150 ,150 });

			//	FDB::Core::Transform2D transform;

			//	m_reg.AddComponents<>(e, std::move(ui), std::move(uiMesh), std::move(transform));
			//}
		}

	public:
		void Update(float dt, FDB::Core::JobPool& pool) override
		{
			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();

			std::uint64_t polarisE = m_reg.GetEntitiesView<FDB::Core::Transform, polaris>().front();
			std::uint64_t sun = m_reg.GetEntitiesView<FDB::Core::Transform, SunStruct>().front();
			std::uint64_t sunLight = m_reg.GetEntitiesView<FDB::Core::Light, SunLight>().front();
			std::uint64_t camera = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>().front();

			auto& sunTransform = transforms[sun];
			auto& camTransform = transforms[camera];
			auto& polarisTransform = transforms[polarisE];
			(void)polarisTransform;
			auto& sunOffset = m_reg.GetContainer<SunStruct>()[sun];
			auto& sunLightComp = m_reg.GetContainer<FDB::Core::Light>()[sunLight];

			sunTransform.SetPosition(camTransform.GetPosition() - sunOffset.dist * sunLightComp.GetDir());

			{
				auto asteroids = m_reg.GetEntitiesView<FDB::Core::Transform, Asteroid>();
				auto& asts = m_reg.GetContainer<Asteroid>();

				for (auto e : asteroids)
				{
					auto& t = transforms[e];
					auto& a = asts[e];

					t.SetPosition(t.GetPosition() + a.velocity * dt);
					t.Rotate(a.spinSpeed * dt, a.spinAxis);

					glm::vec3 rel = t.GetPosition() - camTransform.GetPosition();
					float x = glm::dot(rel, m_right);
					float y = glm::dot(rel, m_up);

					bool outX = x * a.side > kRange;
					bool outY = std::abs(y) > kHalfHeight + kMargin;

					if (outX || outY)
						ResetAsteroid(t, a, false, camTransform.GetPosition());
				}
			}

			const bool* keys = SDL_GetKeyboardState(NULL);

			if (keys[SDL_SCANCODE_UP]) {
				camTransform.Rotate(-0.5f * dt, { 1,0,0 });
			}
			if (keys[SDL_SCANCODE_DOWN]) {
				camTransform.Rotate(0.5f * dt, { 1,0,0 });
			}
			if (keys[SDL_SCANCODE_LEFT]) {
				camTransform.Rotate(-0.5f * dt, { 0,1,0 });
			}
			if (keys[SDL_SCANCODE_RIGHT]) {
				camTransform.Rotate(0.5f * dt, { 0,1,0 });
			}
			if (keys[SDL_SCANCODE_W]) {
				camTransform.Move(glm::vec3{ 0, 0, -0.5f * dt });
			}
			if (keys[SDL_SCANCODE_S]) {
				camTransform.Move(glm::vec3{ 0, 0, 0.5f * dt });
			}
			if (keys[SDL_SCANCODE_A]) {
				camTransform.Move(glm::vec3{ -0.5f * dt, 0, 0 });
			}
			if (keys[SDL_SCANCODE_D]) {
				camTransform.Move(glm::vec3{ 0.5f * dt, 0, 0 });
			}
			if (keys[SDL_SCANCODE_SPACE]) {
				camTransform.Move(glm::vec3{ 0, -0.5f * dt, 0 });
			}
			if (keys[SDL_SCANCODE_E]) {
				camTransform.Move(glm::vec3{ 0, 0.5f * dt, 0 });
			}
			if (keys[SDL_SCANCODE_P])
			{
				FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::Notify(FDB::Scene::ChangeSceneEvent{ 1 });
			}
		}

		void Render(FDB::Core::JobPool& pool) override
		{
			BaseRenderScene::Render(pool);
		}

	private:
		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;
	};
}
#endif // !ASTEROIDFIELD