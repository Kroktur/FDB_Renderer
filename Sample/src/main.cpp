
	#include <iostream>
#include <mutex>
#include <string>
#include "tiny_obj_loader.h"
	#include "Core/Rect.h"
	#include "GLM/vec4.hpp"
	#include "GLM/vec3.hpp"
	#include "SDL/Application.h"
	#include "SDL/Renderer.h"
	#include "SDL/Window.h"
	#include "SDL/Texture.h"
	#include "SDL3/SDL_timer.h"
#include "Core/Font.h"
#include "GLM/detail/type_vec1.hpp"
#include "Pipeline/Atribute.h"
#include "Pipeline/RenderPath.h"
#include "Core/LoadObj.h" 
#include "Core/Relaxation.h"
#include "Core/CreateSphere.h"
//#include "Audio/Audio.h"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Pipeline/Rasterizer.h"

#include "GLM/glm.hpp"
#include "GLM/mat4x4.hpp"

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



#include <type_traits>
#include <array>
#include <print>

#include "Scene/GameLoop.h"
	

	// TODO MOVE TO FILE ATTRIBUTE

	// TODO MOVE TO FILE ATTRIBUTE
#include <span>

#include "Pipeline/Buffer.h"
#include "Pipeline/FrameBuffer.h"
#include "Pipeline/GraphicPipeline.h"

	//#include "Pipeline/Buffer.h"
#include <deque>
#include <KTR/KTR_assert.h>

#include "Audio/Audio.h"
#include "Core/Axis.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "Core/Mesh.h"
#include "Core/ResourceManager.h"
#include "Core/Texture.h"
#include "Core/Transform.h"
#include "Render/GraphicPipeline.h"

#include "Core/Material.h"
#include "Core/Mesh2d.h"
#include "Core/Text.h"
#include "Core/ThreadPool.h"
#include "Core/Transform2d.h"
#include "Render/PBRFragment.h"
#include "Render/PBRShaders.h"
#include "Render/Sample.h"

#include "KTR/Tools/KTR_Chrono.h"
#include "Render/UiFragment.h"
#include "Render/UiShaders.h"
#include "Scene/BaseScene.h"
#include "Scene/Sun.h"
#include "Scene/SplineCurve.h"

	static constexpr std::int32_t renderWidth = 640 * 2;
	static constexpr std::int32_t renderHeight = 360 * 2;

	static constexpr std::int32_t windowWidth = 1920 / 2;
	static constexpr std::int32_t windowHeight = 1080  /2;

#include "KTR/Tools/KTR_RTTI.h"

#include "Core/Ecs.h"





	
	#include <vector>
	




	
	
	

#include "KTR/Tools/KTR_Chrono.h"
#include "KTR/Tools/KTR_Random.h"

	
#include "atomic"



	

	
	//static float angle = (43.0f * 3.14) / 180.f;
	
	using vertIn_t = FDB::Core::Vertex;
	using vertOut_t = FDB::Render::PBRFragment;
	using vertData_t = FDB::Render::PushConstant;
	using pathInData_t = FDB::Render::vertexShaderData;
	using pathOutData_t = FDB::Render::fragmentShaderData;


	struct Rotate
	{
		
	};
	

	struct BaseRenderScene : public FDB::Scene::BasicScene
	{
		using renderPath_t = FDB::Pipeline::RenderPath<
			vertIn_t,
			vertOut_t,
			vertData_t,
			pathInData_t,
			pathOutData_t,
			&FDB::Render::PBRShader::VertexShader,
			&FDB::Render::PBRShader::FragmentShader<false>,
			FDB::Pipeline::default_rasterizerOpt_t>;


		using renderPath2D_t = FDB::Pipeline::RenderPath<
			FDB::Core::VertexUi,
			FDB::Core::VSOutput,
			FDB::Render::UiShaders::PushConstant,
			FDB::Render::UiShaders::ubo,
			FDB::Render::UiShaders::ubo,
			&FDB::Render::UiShaders::VertexShader,
			&FDB::Render::UiShaders::FragmentShader,
			FDB::Pipeline::rasterizerOpt_t<FDB::Pipeline::default_cullData_t
			, FDB::Pipeline::blendData_t<FDB::Pipeline::BlendFactor::SrcAlpha
			, FDB::Pipeline::BlendFactor::OneMinusSrcAlpha
			, FDB::Pipeline::BlendOp::Add>
			, FDB::Pipeline::depthData_t<false, false
			, FDB::Pipeline::DepthOp::Never>
			, FDB::Pipeline::rgba_writeMaskData_t>>;

	public:
		void InitScene() override
		{
			m_baseMaterials.baseColor = FDB::Core::textureLoader_t::Load("Textures/Base/base_color_map.png");
			m_baseMaterials.normalMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_normal_map.png");
			m_baseMaterials.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_emissive_map.png");
			m_baseMaterials.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_OMR_map.png");
			m_baseUi = FDB::Core::textureLoader_t::Load("White.png");
		}
		void Render(FDB::Core::JobPool& pool) override
		{

			auto image = m_renderer->GetFrameBuffer();
			auto viewPort = m_renderer->GetViewPort();
			m_pipeline.Setup(viewPort, &image);
			m_pipeline2D.Setup(viewPort, &image);

			auto& cams = m_reg.GetContainer<FDB::Core::Camera>();
			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();
			auto& meshs = m_reg.GetContainer<FDB::Core::MeshComp>();
			auto& uis = m_reg.GetContainer<FDB::Core::Ui>();
			auto& transform2ds = m_reg.GetContainer<FDB::Core::Transform2D>();
			auto& texts = m_reg.GetContainer<FDB::Core::Text>();
			auto& mesh2ds = m_reg.GetContainer<FDB::Core::Mesh2D>();
			auto& lights = m_reg.GetContainer<FDB::Core::Light>();

			// base Pipeline
			{
				auto& eS = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>();
				cams[eS[0]].Update(transforms[eS[0]].GetModelM());
				m_pipeline.BindVInData(cams[eS[0]].GetData());
				m_reg.GetEntitiesView<FDB::Core::Light>();
				m_lDatas.clear();
				for (auto e : eS)
					m_lDatas.push_back(lights[e].GetData());
				FDB::Render::fragmentShaderData outD{ m_lDatas };
				m_pipeline.BindVOutData(outD);
				m_reg.GetEntitiesView<FDB::Core::MeshComp, FDB::Core::Transform>();


				for (auto e : eS)
				{
					auto& mesh = meshs[e];
					bool defaultMat = mesh.mat.empty();
					FDB::Core::Material* mat = defaultMat ? nullptr : mesh.mat.data();
					for (auto& sub : mesh.mesh->subMeshes)
					{
						if (defaultMat)
						{
							m_pipeline.BindVertexData({ transforms[e].GetModelM(),m_baseMaterials });
							m_pipeline.DrawTile(sub.vertices.data(), sub.indices.data(), sub.indices.size());
						}
						else
						{
							m_pipeline.BindVertexData({ transforms[e].GetModelM(),*mat });
							m_pipeline.DrawTile(sub.vertices.data(), sub.indices.data(), sub.indices.size());
							++mat;
						}
					}
				}
				m_pipeline.End(pool);
			}
			// ui pipeline
			{
				float aspectRatio = static_cast<float>(viewPort.width) / static_cast<float>(viewPort.height);
				FDB::Render::UiShaders::PushConstant push;
				push.aspectRatio = aspectRatio;
				auto& eS = m_reg.GetEntitiesView<FDB::Core::Ui, FDB::Core::Transform2D, FDB::Core::Text>();

				for (auto e : eS)
				{
					auto& ui = uis[e];
					auto& transform = transform2ds[e];
					auto& text = texts[e];
					push.inColor = ui.GetColor();
					push.texture1 = text.GetFont()->GetTexture();
					push.texture2 = text.GetTexture();
					transform.SetPosition(ui.GetPosNdc(aspectRatio));
					transform.SetScale(ui.GetScaleNdc(aspectRatio));
					push.model = transform.GetModel();
					m_pipeline2D.BindVertexData(push);
					auto data = text.GetBufferData();
					m_pipeline2D.DrawTile(data.vertices, data.indices, data.indicesCount);
				}
				m_reg.GetEntitiesView<FDB::Core::Ui, FDB::Core::Transform2D, FDB::Core::Mesh2D>();
				for (auto e : eS)
				{
					auto& ui = uis[e];
					auto& transform = transform2ds[e];
					auto& mesh2d = mesh2ds[e];
					push.inColor = ui.GetColor();
					push.texture1 = m_baseUi;
					push.texture2 = mesh2d.texture;
					transform.SetPosition(ui.GetPosNdc(aspectRatio));
					transform.SetScale(ui.GetScaleNdc(aspectRatio));
					push.model = transform.GetModel();
					m_pipeline2D.BindVertexData(push);
					auto data = mesh2d.GetBufferData();
					m_pipeline2D.DrawTile(data.vertices, data.indices, data.indicesCount);
				}

				m_pipeline2D.End(pool);
			}

		}
	protected:
		FDB::Core::Registry m_reg;
		FDB::Core::Texture* m_baseUi = nullptr;
		FDB::Core::Material m_baseMaterials;

		std::vector<FDB::Core::lightData_t> m_lDatas;
		renderPath_t m_pipeline;
		renderPath2D_t m_pipeline2D;

	};


		
	struct SampleScene : public BaseRenderScene
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
		using renderPath_t = FDB::Pipeline::RenderPath<
			vertIn_t,
			vertOut_t,
			vertData_t,
			pathInData_t,
			pathOutData_t,
			&FDB::Render::PBRShader::VertexShader,
			&FDB::Render::PBRShader::FragmentShader<false>,
			FDB::Pipeline::default_rasterizerOpt_t>;


		using renderPath2D_t = FDB::Pipeline::RenderPath<
			FDB::Core::VertexUi,
			FDB::Core::VSOutput,
			FDB::Render::UiShaders::PushConstant,
			FDB::Render::UiShaders::ubo,
			FDB::Render::UiShaders::ubo,
			&FDB::Render::UiShaders::VertexShader,
			&FDB::Render::UiShaders::FragmentShader,
			FDB::Pipeline::rasterizerOpt_t<FDB::Pipeline::default_cullData_t, FDB::Pipeline::blendData_t<FDB::Pipeline::BlendFactor::SrcAlpha, FDB::Pipeline::BlendFactor::OneMinusSrcAlpha, FDB::Pipeline::BlendOp::Add>, FDB::Pipeline::depthData_t<false, false, FDB::Pipeline::DepthOp::Never>, FDB::Pipeline::rgba_writeMaskData_t>>;
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
			//spline création
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
			auto& eS = m_reg.GetEntitiesView<FDB::Core::Transform, Rotate>();
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
		//	P(1, 1, 1), P(-1, 1, 1),
		//	{ 0.f, 1.f, 0.f, 1.f });


	int main(int argc, char* argv[])
	{
		std::filesystem::path path = argv[0];
		FDB::Core::FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		FDB::Scene::GameLoop loop("test", { windowWidth,windowHeight }, { renderWidth, renderHeight });
		loop.AddScene(std::make_unique<SampleScene>(), true);
		loop.AddScene(std::make_unique<FDB::Scene::BasicScene>(), false);
		loop.Loop();
	}


