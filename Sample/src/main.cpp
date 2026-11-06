
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
#include "Scene/BaseSceneRenderer.h"
#include "Scene/DogFIghtDemo.h"
#include "Scene/PBRScene.h"
#include "Scene/Sun.h"
#include "Scene/SplineCurve.h"
#include "Scene/AsteroidField.h"

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

	struct ScaleScene : public FDB::Scene::BaseRenderScene
	{
		struct MoveCamera
		{
			float radius;
			float angle;
			glm::vec3 pos;
		};
		struct Rat
		{};
	
	protected:
		void InitScene() override
		{
			BaseRenderScene::InitScene();
			FDB::Core::Material wallMat;
			wallMat.baseColor = FDB::Core::textureLoader_t::Load("Textures/Wall1/BaseColor.jpg");
			wallMat.normalMap = FDB::Core::textureLoader_t::Load("Textures/Wall1/Normal.jpg");
			wallMat.emissiveMap = m_baseMaterials.emissiveMap;
			wallMat.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Wall1/ORM.jpg");
			FDB::Core::Material floorMat;
			floorMat.baseColor = FDB::Core::textureLoader_t::Load("Textures/Floor/BaseColor.jpg");
			floorMat.normalMap = FDB::Core::textureLoader_t::Load("Textures/Floor/Normal.jpg");
			floorMat.emissiveMap = m_baseMaterials.emissiveMap;
			floorMat.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Floor/ORM.jpg");

			FDB::Core::Material ratMat;
			ratMat.baseColor = FDB::Core::textureLoader_t::Load("Rat/BaseColor.jpg");
			ratMat.normalMap = FDB::Core::textureLoader_t::Load("Rat/Normal.jpg");
			ratMat.emissiveMap = m_baseMaterials.emissiveMap;
			ratMat.OMRMap = FDB::Core::textureLoader_t::Load("Rat/ORM.jpg");
			

			FDB::Core::Material ratTrapMat;
			ratTrapMat.baseColor = FDB::Core::textureLoader_t::Load("RatTrap/BaseColor.jpg");
			ratTrapMat.normalMap = FDB::Core::textureLoader_t::Load("RatTrap/Normal.jpg");
			ratTrapMat.emissiveMap = m_baseMaterials.emissiveMap;
			ratTrapMat.OMRMap = FDB::Core::textureLoader_t::Load("RatTrap/ORM.jpg");
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0,0,0 });
				transform.ScaleUp({ 2.5f,1.f,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("plane.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(floorMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}

			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0,2.5f,-2.5f });
				transform.Rotate(glm::pi<float>() / 2.f, { 1,0,0 });
				transform.ScaleUp({ 2.5f,1.f,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("plane.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(wallMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0,2.5f,2.5f });
				transform.Rotate(-glm::pi<float>() / 2.f, { 1,0,0 });
				transform.ScaleUp({ 2.5f,1.f,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("plane.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(wallMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ -2.5f,2.5f,0 });
				transform.Rotate(-glm::pi<float>() / 2.f, { 0,0,1 });
				transform.ScaleUp({ 2.5f,1.f,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("plane.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(wallMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 2.5f,2.5f,0 });
				transform.Rotate(glm::pi<float>() / 2.f, { 0,0,1 });
				transform.ScaleUp({ 2.5f,1.f,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("plane.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(wallMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}


			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0,1,0 });
				transform.ScaleUp({ 10.f,10.f,10.f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Rat/rat.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(ratMat);
				Rat rat;
				m_reg.AddComponents(e, std::move(transform), std::move(mesh),std::move(rat));

			}
			
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0,0,0 });
				transform.ScaleUp({ 10.f,10.f,10.f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("RatTrap/ratTrap.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(ratTrapMat);
				Rat rat;
				m_reg.AddComponents(e, std::move(transform), std::move(mesh), std::move(rat));

			}


			// 2



			// camera 
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Camera camera;
				FDB::Core::Transform transform;
				transform.SetPosition({ 0,1.5,5 });
				transform.LookAt({ 0.f,0.f,0.f });
				MoveCamera m;
				m.radius = 5.0f;
				m.angle = 0.0f;
				m.pos = { 0,0,0 };
				m_reg.AddComponents<FDB::Core::Camera, FDB::Core::Transform>(e, std::move(camera), std::move(transform), std::move(m));
			}
			{

				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos(glm::vec3{ 0,2.5,0 });
				light.SetDir(glm::normalize(glm::vec3{ 0,-1,0 }));
				light.SetSpotData(glm::pi<float>() / 3.f, 0.25f);
				light.SetColor(FDB::Color::whiteFloat, FDB::Color::whiteFloat);
				light.SetIntensity(200.f);


				m_reg.AddComponent(e, std::move(light));

			}



			{

				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos(glm::vec3{ -2.3,0.2f,-2.3 });
				light.SetDir(glm::normalize(glm::vec3{ 0,-1,0 }));
				light.SetSpotData(glm::pi<float>() / 3.f, 0.25f);
				light.SetColor(FDB::Color::redFloat, FDB::Color::whiteFloat);
				light.SetIntensity(100.f);


				m_reg.AddComponent(e, std::move(light));

			}
			{

				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos(glm::vec3{ 2.3,0.2f,-2.3 });
				light.SetDir(glm::normalize(glm::vec3{ 0,-1,0 }));
				light.SetSpotData(glm::pi<float>() / 3.f, 0.25f);
				light.SetColor(FDB::Color::blueFloat, FDB::Color::whiteFloat);
				light.SetIntensity(100.f);


				m_reg.AddComponent(e, std::move(light));

			}
			{

				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos(glm::vec3{ -2.3,0.2f,2.3 });
				light.SetDir(glm::normalize(glm::vec3{ 0,-1,0 }));
				light.SetSpotData(glm::pi<float>() / 3.f, 0.25f);
				light.SetColor(FDB::Color::greenFloat, FDB::Color::whiteFloat);
				light.SetIntensity(100.f);


				m_reg.AddComponent(e, std::move(light));

			}
			{

				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Light light;
				light.SetType(FDB::Core::LightType::Point);
				light.SetPos(glm::vec3{ 2.3,0.2f,2.3 });
				light.SetDir(glm::normalize(glm::vec3{ 0,-1,0 }));
				light.SetSpotData(glm::pi<float>() / 3.f, 0.25f);
				light.SetColor(FDB::Color::RGBAFloat{0.45,0.12,0.89,1}, FDB::Color::whiteFloat);
				light.SetIntensity(100.f);


				m_reg.AddComponent(e, std::move(light));
				ratSound.SetWav(*FDB::Audio::wavManager_t::Load("Sound/rat.mp3"));
			}
		
		}
		void OnEnable() override
		{
			ratSound.SetLoop(true);

			ratSound.Play();
			ratSound.SetVolume(0.15f);

			timer.Resume();
		
		}
		void OnDisable() override
		{
			if (ratSound.IsPlaying())
				ratSound.Stop();
			



			timer.Reset();
			std::cout << "quit pbr scene" << std::endl;
		}
	public:
		static constexpr glm::vec3 minScale{ 10,5,30 };
		static constexpr glm::vec3 maxScale{ 50,70,110 };
		static constexpr glm::vec3 speed{ 100,100,100 };

		void Update(float dt, FDB::Core::JobPool& pool) override
		{

			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();



			std::uint64_t camera = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>().front();
			std::uint64_t rat = m_reg.GetEntitiesView<Rat>().front();

			auto& camMove = m_reg.GetContainer<MoveCamera>()[camera];
			auto& camTransform = transforms[camera];

			camMove.angle += 0.8f * dt;
			auto pos = camTransform.GetPosition();
			camTransform.SetPosition({ camMove.pos.x + camMove.radius * std::cos(camMove.angle) ,pos.y,  camMove.pos.z + camMove.radius * std::sin(camMove.angle) });
			camTransform.LookAt({ 0,0,0 });

			auto& ratTransform = transforms[rat];
			ratTransform.Rotate(2.f * dt ,{ 1,0,0 });
			ratTransform.Rotate(2.5f * dt, { 1,0,1 });

			ratTransform.Rotate(0.5f * dt, { 1,1,0 });

			auto scale = ratTransform.GetScale();
			
			for (int i = 0; i < 3; ++i)
			{
				if (scale[i] >= maxScale[i])
					expand[i] = -1;
				if (scale[i] <= minScale[i])
					expand[i] = 1;
			}

			scale += expand * dt * speed;
			ratTransform.SetScale(scale);

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

			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 10)
			{
				FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::Notify({ 3});
			}
		}
	private:
		glm::vec3 expand{ 1,1,1 };
		KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;
		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;
		FDB::Audio::Wav ratSound;
	};



	struct TextScene : public FDB::Scene::BaseRenderScene
	{
		

	protected:
		void InitScene() override
		{
			BaseRenderScene::InitScene();

			// camera 
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Camera camera;
				FDB::Core::Transform transform;
				transform.SetPosition({ 0,1.5,5 });
				transform.LookAt({ 0.f,0.f,0.f });
				
				m_reg.AddComponents<FDB::Core::Camera, FDB::Core::Transform>(e, std::move(camera), std::move(transform));
			}
			{
		FDB::Core::entity e = m_reg.CreateEntity();

		FDB::Core::Mesh2D uiMesh;
		uiMesh.data = &FDB::Core::square;
		uiMesh.texture = FDB::Core::textureLoader_t::Load("fond.png");

		FDB::Core::Ui ui;
				
		ui.SetPosition({ 1920/2,1080/2 });
		ui.SetScale({ 1920 ,1080 });

		FDB::Core::Transform2D transform;

		m_reg.AddComponents<>(e, std::move(ui), std::move(uiMesh), std::move(transform));
	}

			
			txt = "Il etait une fois, au sein d'un groupe de fous... \nDeux developpeurs, lances dans un projet de quatre semaines,\n se casserent le crane a developper un moteur.\nApres des heures de galere, de bugs et de prises de tete,\n ils finirent par venir a bout de leur projet.\nEt c est ainsi que ce groupe de developpeurs,\n repondant au doux nom de FDB (Fous Du Bus), vous presente :\nSTAR CITICRASH\nET LE RAT\n";
			// text
			{
				FDB::Core::entity e = m_reg.CreateEntity();

				FDB::Core::Text text;
				text.SetText(" ");
				text.SetAlign(FDB::Core::Text::Align::Center);
				text.SetFont(FDB::Core::fontLoader_t::Load("ARIAL.TTF", 20));
				text.SetTexture(FDB::Core::textureLoader_t::Load("White.png"));

				FDB::Core::Ui ui;
				ui.SetPosition({ 1920 / 2.f,1080 / 2.f });
				ui.SetScale({ 1920  ,1080  });

				FDB::Core::Transform2D transform;

				m_reg.AddComponents<>(e, std::move(ui), std::move(text), std::move(transform));
			}
			txtSound.SetWav(*FDB::Audio::wavManager_t::Load("Sound/keys.mp3"));
			txtSound.SetLoop(true);

		}
		void OnEnable() override
		{
			txtSound.Play();
			txtTimer.Resume();

		}
		void OnDisable() override
		{
			txtTimer.Reset();
			timer.Reset();
			std::cout << "quit pbr scene" << std::endl;
		}
	public:

		void Update(float dt, FDB::Core::JobPool& pool) override
		{

			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();



			std::uint64_t camera = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>().front();
			std::uint64_t text = m_reg.GetEntitiesView<FDB::Core::Text>().front();

			auto& textReal = m_reg.GetContainer<FDB::Core::Text>()[text];
			auto& camTransform = transforms[camera];

		


			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(txtTimer.GetTime()).count() >=(0.02f))
			{
				if (index < txt.size())
				{
					std::string t(1, txt[index++]);
					while ((t.back() == '\n' || t.back() == ' ') && index < txt.size())
					{
						t += txt[index++];
					}
					std::string str = textReal.GetText();
					str += t;
					textReal.SetText(str);
					txtTimer.Restart();
				}
				else
				{
					if (txtSound.IsPlaying())
						txtSound.Stop();
				}
			}
			if (index >= txt.size() && timer.GetState() == KTR::chrono::timer_state::idle)
				timer.Resume();
				

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

			if (timer.GetState() != KTR::chrono::timer_state::idle && KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 10)
			{
				FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::Notify({ 1});
			}
		}
	private:
		KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;
		KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> txtTimer;
		FDB::Audio::Wav txtSound;
		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;
		std::string txt;
		std::size_t index = 0;
	};


	int main(int argc, char* argv[])
	{
		std::filesystem::path path = argv[0];
		FDB::Core::FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		FDB::Scene::GameLoop loop("test", { windowWidth,windowHeight }, { renderWidth, renderHeight });
		loop.AddScene(std::make_unique<TextScene>(), true);
		loop.AddScene(std::make_unique<FDB::Demo::PBRScene>(), false);
		loop.AddScene(std::make_unique<ScaleScene>(), false);
		loop.AddScene(std::make_unique<FDB::Demo::DogFightScene>(), false);
		loop.AddScene(std::make_unique<FDB::Demo::AsteroidField>(), false);
		loop.Loop();
	}


