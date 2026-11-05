#ifndef FDBSCENEDOGFIGHTDEMOF
#define FDBSCENEDOGFIGHTDEMOF
#include "BaseSceneRenderer.h"
#include "SplineCurve.h"
#include "Sun.h"

namespace FDB::Demo
{



	struct DogFightScene : public FDB::Scene::BaseRenderScene
	{
	private:
		struct ShipPath
		{
			SplineCurve curve;
			float distance = 0.f;
			float speed = 3.f;
			std::uint64_t entity = 0;   // le vaisseau qui suit cette courbe
		};
		std::vector<ShipPath> m_paths;  // remplace m_camCurve / m_camDistance / m_camSpeed
		bool  m_followSpline = true;
		bool  m_toggleWasDown = false;

	public:
	protected:
		void InitScene() override
		{
			BaseRenderScene::InitScene();
			FDB::Core::Material material;
			material.baseColor = FDB::Core::textureLoader_t::Load("Textures/BaseColor.png");
			material.normalMap = FDB::Core::textureLoader_t::Load("Textures/Normal.png");
			material.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Emissive.png");
			material.OMRMap = FDB::Core::textureLoader_t::Load("Textures/ORM.png");
			//// mesh 1
			//{
			//	FDB::Core::entity e = m_reg.CreateEntity();
			//	FDB::Core::Transform transform;
			//	
			//	transform.ScaleUp({ 0.1f,0.1f,0.1f });
			//	//transform.Rotate(glm::pi<float>() / 2.0f, { 0,1,0 });
			//	FDB::Core::MeshComp mesh;
			//	mesh.mesh = FDB::Core::meshLoader_t::Load("Fighter_03.obj");
			//	for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
			//		mesh.mat.push_back(material);
			//	Rotate r{};
			//	m_reg.AddComponents(e, std::move(transform),std::move(mesh),std::move(r));
			//}
			// mesh 2
			//{
			//	FDB::Core::entity e = m_reg.CreateEntity();

			//	FDB::Core::Transform transform;
			//	transform.SetPosition(glm::vec3{ 0,0,1 });
			//	FDB::Core::MeshComp mesh;
			//	mesh.mesh = FDB::Core::meshLoader_t::Load("bloc_1x1.obj");
			//	for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
			//		mesh.mat.push_back(material);
			//	m_reg.AddComponents(e, std::move(transform), std::move(mesh));
			//}


			//sun
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
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				FDB::Core::MeshComp mesh;
				transform.SetPosition(glm::vec3{ 15,-2,30 });
				transform.ScaleUp({ 8.f,8.f,8.f });
				mesh.mesh = FDB::Core::meshLoader_t::Load("sphere.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(m_baseMaterials);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));
			}
			auto makeShip = [&](glm::vec3 scale, std::vector<glm::vec3> points, float speed, float startDistance)
				{
					FDB::Core::entity e = m_reg.CreateEntity();
					FDB::Core::Transform transform;
					transform.ScaleUp(scale);
					FDB::Core::MeshComp mesh;
					mesh.mesh = FDB::Core::meshLoader_t::Load("Fighter_03.obj");
					for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
						mesh.mat.push_back(m_baseMaterials);

					ShipPath p;
					p.curve.Build(points, 0.0f, true);
					p.speed = speed;
					p.distance = startDistance;
					p.entity = e;
					m_reg.AddComponents(e, std::move(transform), std::move(mesh), std::move(p));
				};

			makeShip({ 0.7f,0.7f,0.7f }, {
				{  4.f, 0.f,  10.f }, {  12.f, 2.f,   4.f }, {  12.f, 0.f,  -6.f },
				{ 2.f, 3.f, -12.f }, {-4.f, 1.f,  -4.f }
				}, 5.f, 0.f);

			makeShip({ 0.7f,0.7f,0.7f }, {
				{ -2.f, 1.f,  8.f }, { 4.f, 4.f, 0.f }, { 10.f, 1.f, -8.f }, { 4.f, -2.f, -14.f }
				}, 4.f, 0.f);

			//// procedural Mesh
			//m_SphereMesh = std::make_unique<FDB::Core::Mesh>();
			//auto p3d = RelaxAlgo::createSphere();
			//std::vector<glm::vec3> unit;
			//for (auto& p : p3d)
			//	unit.push_back(glm::normalize(p));
			//auto sub = CreateSphere::TransformPointToTriangle(p3d);
			//sub = CreateSphere::applyNoise(unit, sub.indices, 20.f, 5.f, 5.f);
			//m_SphereMesh->subMeshes.push_back(std::move(sub));
			//FDB::Core::MeshTools::ComputeTangent(*m_SphereMesh);
			//{
			//	FDB::Core::entity e = m_reg.CreateEntity();
			//	FDB::Core::Transform transform;

			//	transform.SetPosition(glm::vec3{ 0,0,-100 });
			//	transform.ScaleUp(glm::vec3{ 1.f / 5.f });
			//	FDB::Core::MeshComp mesh;
			//	mesh.mesh = m_SphereMesh.get();

			//	m_reg.AddComponents(e, std::move(transform), std::move(mesh));
			//}
			// camera 
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Camera camera;
				FDB::Core::Transform transform;
				transform.LookAtDir({ 0.5f,0,-1 });
				transform.SetPosition({ 5,3,30 });

				m_reg.AddComponents<FDB::Core::Camera, FDB::Core::Transform>(e, std::move(camera), std::move(transform));
			}
			// light 
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos({ 0,0,6 });
				light.SetDir(glm::normalize(glm::vec3{ -2,0,-1 }));
				light.SetColor(FDB::Color::whiteFloat);
				light.SetIntensity(1.f);
				m_reg.AddComponent(e, std::move(light));

			}
			// Light for ship
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Spot);
				light.SetPos({ 0,0,0 });
				light.SetDir(glm::normalize(glm::vec3{ 0,0,-1 }));
				light.SetColor(FDB::Color::whiteFloat);
				light.SetSpotData(glm::pi<float>() / 6.f, 1.f);
				light.SetIntensity(300.f);

				ShipsLight shipLight;

				m_reg.AddComponents(e, std::move(light), std::move(shipLight));
			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Spot);
				light.SetPos({ 0,0,0 });
				light.SetDir(glm::normalize(glm::vec3{ 0,0,-1 }));
				light.SetColor(FDB::Color::whiteFloat);
				light.SetSpotData(glm::pi<float>() / 6.f, 1.f);
				light.SetIntensity(300.f);
				ShipsLight shipLight;

				m_reg.AddComponents(e, std::move(light), std::move(shipLight));
			}
			//light sun
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Dir);
				//light.SetPos({ 10, 0, -50 });
				light.SetDir(glm::normalize(glm::vec3{ 0,0,1 }));
				light.SetColor(FDB::Color::RGBAFloat(0.922, 0.906, 0.039, 1.0f));
				light.SetIntensity(30.f);
				SunLight sunLight;
				m_reg.AddComponents(e, std::move(light), std::move(sunLight));

			}
			// ui
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Mesh2D uiMesh;
				uiMesh.data = &FDB::Core::square;
				uiMesh.texture = FDB::Core::textureLoader_t::Load("PBC.png");

				FDB::Core::Ui ui;
				ui.SetPosition({ 300,300 });
				ui.SetScale({ 150 ,150 });

				FDB::Core::Transform2D transform;

				m_reg.AddComponents<>(e, std::move(ui), std::move(uiMesh), std::move(transform));
			}
			// text
			{
				//FDB::Core::entity e = m_reg.CreateEntity();

				//FDB::Core::Text text;
				//text.SetText("Hello, World!");
				//text.SetAlign(FDB::Core::Text::Align::Center);
				//text.SetFont(FDB::Core::fontLoader_t::Load("Lazer84.ttf", 1));
				//text.SetTexture(FDB::Core::textureLoader_t::Load("PBC.png"));

				//FDB::Core::Ui ui;
				//ui.SetPosition({ 1920 / 2.f,1080 / 2.f });
				//ui.SetScale({ 1920 / 5 ,1080 / 5 });

				//FDB::Core::Transform2D transform;

				//m_reg.AddComponents<>(e, std::move(ui), std::move(text), std::move(transform));
			}

		}
	public:
		void Update(float dt, FDB::Core::JobPool& pool) override
		{
			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();
			//auto& tr = transforms[eS[0]];

			//tr.Rotate(-0.5f * dt, { 0,0,1 });

			std::uint64_t sun = m_reg.GetEntitiesView<FDB::Core::Transform, SunStruct>().front();
			std::uint64_t sunLight = m_reg.GetEntitiesView<FDB::Core::Light, SunLight>().front();
			std::uint64_t ShipLight1 = m_reg.GetEntitiesView<FDB::Core::Light, ShipsLight>().front();
			std::uint64_t ShipLight2 = m_reg.GetEntitiesView<FDB::Core::Light, ShipsLight>().back();

			std::uint64_t camera = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>().front();
			std::uint64_t ships1 = m_reg.GetEntitiesView<FDB::Core::Transform, ShipPath>()[0];
			std::uint64_t ships2 = m_reg.GetEntitiesView<FDB::Core::Transform, ShipPath>()[1];

			//std::uint64_t mesh = m_reg.GetEntitiesView<Rotate>().front();

			auto& sunTransform = transforms[sun];
			auto& camTransform = transforms[camera];
			auto& sunOffset = m_reg.GetContainer<SunStruct>()[sun];
			auto& sunLightComp = m_reg.GetContainer<FDB::Core::Light>()[sunLight];

			//auto& meshTransform = transforms[mesh];

			sunTransform.SetPosition(camTransform.GetPosition() - sunOffset.dist * sunLightComp.GetDir());

			auto& shipLight1 = m_reg.GetContainer<FDB::Core::Light>()[ShipLight1];
			auto& shipLight2 = m_reg.GetContainer<FDB::Core::Light>()[ShipLight2];

			auto& spline = m_reg.GetContainer<ShipPath>();
			auto& Ship1 = spline[ships1];
			auto& Ship2 = spline[ships2];
			auto& Ship1Transform = transforms[ships1];
			auto& Ship2Transform = transforms[ships2];
			if (m_followSpline)
			{
				if (Ship2.curve.IsValid())
				{
					Ship2.distance = std::fmod(Ship2.distance + Ship2.speed * dt, Ship2.curve.Length());

					const float t = Ship2.curve.TFromDistance(Ship2.distance);
					Ship2Transform.SetPosition(Ship2.curve.Position(t));
				}

				if (Ship1.curve.IsValid())
				{
					Ship1.distance = std::fmod(Ship1.distance + Ship1.speed * dt, Ship1.curve.Length());

					const float t = Ship1.curve.TFromDistance(Ship1.distance);
					Ship1Transform.SetPosition(Ship1.curve.Position(t));

				}
			}
			Ship1Transform.LookAt(Ship2Transform.GetPosition());
			Ship2Transform.LookAt(Ship1Transform.GetPosition());
			shipLight2.SetPos(Ship2Transform.GetPosition() + Ship2Transform.GetLocalAxis({ 0, 0, -1 }) * 2.f);
			shipLight2.SetDir(Ship2Transform.GetLocalAxis({ 0, 0, -1 }));
			shipLight1.SetPos(Ship1Transform.GetPosition() + Ship1Transform.GetLocalAxis({ 0, 0, -1 }) * 2.f);
			shipLight1.SetDir(Ship1Transform.GetLocalAxis({ 0, 0, -1 }));




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
	private:



		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;
	};
}

#endif
