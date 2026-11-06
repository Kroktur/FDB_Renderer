#ifndef FDBSCENEPBRSCENEF
#define FDBSCENEPBRSCENEF
#include <KTR/Tools/KTR_Chrono.h>

#include "BaseSceneRenderer.h"
#include "Audio/Audio.h"


namespace FDB::Demo
{


	struct PBRScene : public FDB::Scene::BaseRenderScene
	{
		struct MoveCamera
		{
			float radius;
			float angle;
			glm::vec3 pos;
		};
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


			FDB::Core::Material materialRay;
			materialRay.baseColor = FDB::Core::textureLoader_t::Load("Ray/BaseColor.jpg");
			materialRay.normalMap = FDB::Core::textureLoader_t::Load("Ray/Normal.jpg");
			materialRay.emissiveMap = m_baseMaterials.emissiveMap;
			materialRay.OMRMap = FDB::Core::textureLoader_t::Load("Ray/ORM.jpg");

			FDB::Core::Material materialLion;
			materialLion.baseColor = FDB::Core::textureLoader_t::Load("Lion/BaseColor.jpg");
			materialLion.normalMap = FDB::Core::textureLoader_t::Load("Lion/Normal.jpg");
			materialLion.emissiveMap = m_baseMaterials.emissiveMap;
			materialLion.OMRMap = FDB::Core::textureLoader_t::Load("Lion/ORM.png");

			FDB::Core::Material coneMat;
			coneMat.baseColor = FDB::Core::textureLoader_t::Load("Cone/BaseColor.jpg");
			coneMat.normalMap = FDB::Core::textureLoader_t::Load("Cone/Normal.jpg");
			coneMat.emissiveMap = m_baseMaterials.emissiveMap;
			coneMat.OMRMap = FDB::Core::textureLoader_t::Load("Cone/ORM.jpg");
			FDB::Core::Material flashLightMat;
			flashLightMat.baseColor = FDB::Core::textureLoader_t::Load("FlashLight/BaseColor.jpg");
			flashLightMat.normalMap = FDB::Core::textureLoader_t::Load("FlashLight/Normal.jpg");
			flashLightMat.emissiveMap = m_baseMaterials.emissiveMap;
			flashLightMat.OMRMap = FDB::Core::textureLoader_t::Load("FlashLight/ORM.jpg");

			FDB::Core::Material radioMat;
			radioMat.baseColor = FDB::Core::textureLoader_t::Load("Radio/BaseColor.jpg");
			radioMat.normalMap = FDB::Core::textureLoader_t::Load("Radio/Normal.jpg");
			radioMat.emissiveMap = m_baseMaterials.emissiveMap;
			radioMat.OMRMap = FDB::Core::textureLoader_t::Load("Radio/ORM.jpg");


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
				transform.SetPosition(glm::vec3{ 1.5f,0.f,1.5f });
				transform.Rotate(-3 * glm::pi<float>() / 3.f, { 0,1,0 });

				transform.ScaleUp({ 2.5f ,2.5f ,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Ray/ray.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(materialRay);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ -1.5f,0.f,1.5f });
				transform.Rotate(2 * glm::pi<float>() / 3.f, { 0,1,0 });

				transform.ScaleUp({ 2.5f ,2.5f ,2.5f });
				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Lion/lion.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(materialLion);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0.f,0.f,0.f });
				transform.Rotate(2 * glm::pi<float>() / 3.f, { 0,1,0 });

				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Cone/cone.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(coneMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}
			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 1.5f,0.12f,-1.5f });
				transform.Rotate(-glm::pi<float>() / 4.f, { 0,1,0 });
				transform.ScaleUp({ 2.f ,2.f ,2.f });

				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("FlashLight/flashLight.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(flashLightMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}



			// 2

			{
				FDB::Core::entity e = m_reg.CreateEntity();
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ -1.5f,0.12f,-1.5f });
				transform.Rotate(glm::pi<float>() / 4.f, { 0,1,0 });
				transform.ScaleUp({ 2.f ,2.f ,2.f });

				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("FlashLight/flashLight.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(flashLightMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}


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
				eL = e;
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
				FDB::Core::Transform transform;
				transform.SetPosition(glm::vec3{ 0.f,0.f,-1.f });
				transform.Rotate(glm::pi<float>(), { 0,1,0 });

				FDB::Core::MeshComp mesh;
				mesh.mesh = FDB::Core::meshLoader_t::Load("Radio/radio.obj");
				for (std::size_t i = 0; i < mesh.mesh->subMeshes.size(); ++i)
					mesh.mat.push_back(radioMat);
				m_reg.AddComponents(e, std::move(transform), std::move(mesh));

			}



			neonWav.SetWav(*FDB::Audio::wavManager_t::Load("Sound/neon.mp3"));
			neonWav.SetVolume(50.f);
			lampWav.SetWav(*FDB::Audio::wavManager_t::Load("Sound/lamp.mp3"));
			lampWav.SetVolume(50.f);
			breakWav.SetWav(*FDB::Audio::wavManager_t::Load("Sound/neonBreak.mp3"));
			breakWav.SetVolume(100.f);
			lampOnWav.SetWav(*FDB::Audio::wavManager_t::Load("Sound/lampOn.mp3"));
			lampOnWav.SetVolume(100.f);

			radio.SetWav(*FDB::Audio::wavManager_t::Load("Sound/radio.mp3"));
			radio.SetLoop(true);

			std::cout << radio.GetVolume();
			radioStart.SetWav(*FDB::Audio::wavManager_t::Load("Sound/radioStart.mp3"));

			radioStart.SetVolume(100.f);
		}
		void OnEnable() override
		{
			neonWav.Play();
			timer.Resume();
			timer.Pause();
			radioStart.Play();
		}
		void OnDisable() override
		{
			if (lampWav.IsPlaying())
				lampWav.Stop();

			if (radio.IsPlaying())
				radio.Stop();
			std::cout << "quit pbr scene" << std::endl;
		}
	public:
		void Update(float dt, FDB::Core::JobPool& pool) override
		{

			if (timer.GetState() == KTR::chrono::timer_state::paused)
				timer.Resume();

			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 2 && !radio.IsPlaying())
			{
				radio.Play();
				radio.SetVolume(0.5f);

			}


			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 8 && !elBreak)
			{
				elBreak = true;
				neonWav.Stop();
				breakWav.Play();
				m_reg.DestroyEntity(eL);
			}
			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 10 && !flashLightSpawn)
			{
				flashLightSpawn = true;
				lampOnWav.Play();
				{

					FDB::Core::entity e = m_reg.CreateEntity();

					FDB::Core::Light light;
					light.SetType(FDB::Core::LightType::Spot);
					light.SetPos(glm::vec3{ -1.5f,0.12f + 0.1f,-1.5f } + glm::vec3{ 1,0,1 } * 0.1f);
					light.SetDir(glm::normalize(glm::vec3{ 1,0,1 }));
					light.SetSpotData(glm::pi<float>() / 5.f, 0.25f);
					light.SetColor(FDB::Color::RGBAFloat(1.0f, 0.85f, 0.25f, 1.0f), FDB::Color::whiteFloat);
					light.SetIntensity(250.f);


					m_reg.AddComponent(e, std::move(light));

				}
				{

					FDB::Core::entity e = m_reg.CreateEntity();

					FDB::Core::Light light;
					light.SetType(FDB::Core::LightType::Spot);
					light.SetPos(glm::vec3{ 1.5f,0.12f + 0.1f,-1.5f } + glm::vec3{ -1,0,1 } * 0.1f);
					light.SetDir(glm::normalize(glm::vec3{ -1,0,1 }));
					light.SetSpotData(glm::pi<float>() / 5.f, 0.25f);
					light.SetColor(FDB::Color::RGBAFloat(1.0f, 0.85f, 0.25f, 1.0f), FDB::Color::whiteFloat);
					light.SetIntensity(250.f);


					m_reg.AddComponent(e, std::move(light));

				}

				lampWav.Play();
			}
			// TODO changeScene
			if (KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime()).count() >= 20)
			{
				//FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::Notify({ 1 });
			}


			auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();



			std::uint64_t camera = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>().front();


			auto& camMove = m_reg.GetContainer<MoveCamera>()[camera];
			auto& camTransform = transforms[camera];

			camMove.angle += 0.8f * dt;
			auto pos = camTransform.GetPosition();
			camTransform.SetPosition({ camMove.pos.x + camMove.radius * std::cos(camMove.angle) ,pos.y,  camMove.pos.z + camMove.radius * std::sin(camMove.angle) });
			camTransform.LookAt({ 0,0,0 });



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
		FDB::Audio::Wav neonWav;
		FDB::Audio::Wav lampWav;
		FDB::Audio::Wav lampOnWav;


		FDB::Audio::Wav breakWav;
		FDB::Audio::Wav radioStart;
		FDB::Audio::Wav radio;


		std::uint64_t eL;
		bool elBreak = false;

		bool flashLightSpawn = false;

		KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;
		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;
	};
}

#endif
