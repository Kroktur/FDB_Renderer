
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
#include "Core/Text.h"
#include "Core/ThreadPool.h"
#include "Core/Transform2d.h"
#include "Render/PBRFragment.h"
#include "Render/PBRShaders.h"
#include "Render/Sample.h"

#include "KTR/Tools/KTR_Chrono.h"
#include "Render/UiFragment.h"
#include "Render/UiShaders.h"

	static constexpr std::int32_t renderWidth = 640 / 2;
	static constexpr std::int32_t renderHeight = 360 / 2;

	static constexpr std::int32_t windowWidth = 1920;
	static constexpr std::int32_t windowHeight = 1080;

	namespace FDB::Core
	{
		struct ChangeSceneEvent
		{
			std::uint64_t sceneId;
		};

		struct GameLoop;

		struct BasicScene
		{
		public:
			friend GameLoop;
			virtual ~BasicScene() = default;
			virtual void HandleInput(FDB::Core::JobPool& pool)
			{
			}
			virtual void Update(float dt, FDB::Core::JobPool& pool)
			{
				
			}
			virtual void Render(FDB::Core::JobPool& pool)
			{
				
			}
			virtual  Color::RGBA8888 ClearColor()
			{
				return  Color::alpha8;
			}
			[[nodiscard]] std::uint64_t GetSceneId() const
			{
				return m_sceneId;
			}
		protected:
			virtual void InitScene()
			{
				
			}
			FDB::Render::SDLGraphicPipeline* m_renderer = nullptr;
			FDB::Sdl::Window* m_window = nullptr;
		private:
			void Init(FDB::Sdl::Window* window, FDB::Render::SDLGraphicPipeline* renderer,std::uint64_t id)
			{
				m_renderer = renderer;
				m_window = window;
				m_sceneId = id;
				InitScene();
			}
			std::uint64_t m_sceneId = 0;

		};
	}	
	#include <vector>
	namespace FDB::Core
	{
		struct GameLoop
		{
		public:
			GameLoop(const std::string& name,const glm::uvec2& size,const glm::vec2& renderSize) : m_window(name.c_str(), size, SDL_WINDOW_RESIZABLE),m_pool()
			{
				m_renderer = std::make_unique<FDB::Render::SDLGraphicPipeline>(&m_window, renderSize);
				FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
				FDB::Audio::Soloud::Init();

				m_handle = FDB::Pipeline::EventBus<FDB::Core::ChangeSceneEvent>::SubScribe<GameLoop,&GameLoop::ChangeSceneEvent>(this);
			}
			void AddScene(std::unique_ptr<BasicScene> scene,bool main = false)
			{
				scene->Init(&m_window, m_renderer.get(), m_scenes.size());
				m_scenes.push_back(std::move(scene));
				if (main)
				{
					m_currentScene = m_scenes.back().get();
					m_currentSceneId = m_scenes.size() - 1;
				}
			}
			~GameLoop()
			{
				FDB::Pipeline::EventBus<FDB::Core::ChangeSceneEvent>::UnSubscribe(m_handle);
				m_window.CloseWindow();
				FDB::Sdl::Application::Destroy();
				FDB::Audio::Soloud::Destroy();

			}
			void Loop()
			{
				if (!m_currentScene)
					return;

				std::uint64_t frameCount = 0;
				KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution,KTR::duration::milli_seconds_type<double>> fpsTimer;
				fpsTimer.Resume();
				auto lastFps = fpsTimer.GetTime();

				KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution,KTR::duration::seconds_type<float>> timer;
				timer.Resume();
				auto last = timer.GetTime();
				while (!FDB::Sdl::Application::ShouldQuit() && !m_window.ShouldClose())
				{
					this->ChangeScene();
					FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();
					{
						++frameCount;
						auto currentFps = fpsTimer.GetTime();

						auto delta = (currentFps - lastFps).count();
						if (delta >= 1000)
						{
							const double fps = static_cast<double>(frameCount) * 1000.0 / delta;

							std::string title = "FPS: " + std::to_string(static_cast<int>(fps));
							m_window.SetTitle(title.c_str());

							frameCount = 0;
							lastFps = currentFps;
						}
					}


					m_currentScene->HandleInput(m_pool);

					auto current = timer.GetTime();
					m_currentScene->Update((current - last).count(), m_pool);
					last = current;

					m_renderer->ClearColor(m_currentScene->ClearColor());
					m_renderer->BeginRender();
					m_currentScene->Render(m_pool);
					m_renderer->PresentRender(m_pool);
				}
			}
		private:
			void ChangeSceneEvent(const FDB::Core::ChangeSceneEvent& event)
			{
				m_currentSceneId = event.sceneId;
				m_sceneChanged = true;
			}
			void ChangeScene()
			{
				if (!m_currentScene)
					return;
				m_sceneChanged = false;
				m_currentScene = m_scenes[m_currentSceneId].get();
			}

			std::vector<std::unique_ptr<BasicScene>> m_scenes;
			BasicScene* m_currentScene = nullptr;
			std::uint64_t m_currentSceneId = 0;
			bool m_sceneChanged = false; 
			FDB::Sdl::Window m_window;
			FDB::Pipeline::EventBus<FDB::Core::ChangeSceneEvent>::BusHandle m_handle;
			FDB::Core::JobPool m_pool;
			std::unique_ptr<FDB::Render::SDLGraphicPipeline> m_renderer;
		};
	}





	
	
	

#include "KTR/Tools/KTR_Chrono.h"
#include "KTR/Tools/KTR_Random.h"

	
#include "atomic"



	

	
	//static float angle = (43.0f * 3.14) / 180.f;
	
	using vertIn_t = FDB::Core::Vertex;
	using vertOut_t = FDB::Render::PBRFragment;
	using vertData_t = FDB::Render::PushConstant;
	using pathInData_t = FDB::Render::vertexShaderData;
	using pathOutData_t = FDB::Render::fragmentShaderData;




	struct Pixel
	{
		float z;
		glm::vec4 color;
	};

	static_assert(std::is_trivially_copyable_v<Pixel>);

	using atomicPixel_t = std::atomic<Pixel>;

		
	struct SampleScene : public FDB::Core::BasicScene
	{
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
			FDB::Pipeline::rasterizerOpt_t<FDB::Pipeline::default_cullData_t, FDB::Pipeline::blendData_t<FDB::Pipeline::BlendFactor::SrcAlpha,FDB::Pipeline::BlendFactor::OneMinusSrcAlpha,FDB::Pipeline::BlendOp::Add>, FDB::Pipeline::depthData_t<false,false,FDB::Pipeline::DepthOp::Never>, FDB::Pipeline::rgba_writeMaskData_t>>;
	protected:
		void InitScene() override
		{
			m_transform = FDB::Core::Transform();
			m_transform.ScaleUp({ 2.f,2.f,2.f });
			m_transform.Rotate(glm::pi<float>() / 2.0f, { 1,0,0 });
			m_camera = FDB::Core::Camera();
			m_camTransform = FDB::Core::Transform();
			m_camTransform.LookAtDir({ 0,0,-1 });
			m_camTransform.SetPosition({ 0,0,5 });
			m_camera.Update(m_camTransform.GetModelM());

			m_light = FDB::Core::Light();
			m_light.SetType(FDB::Core::LightType::Point);
			m_light.SetPos({ 0,0,3 });
			m_light.SetDir(glm::normalize(glm::vec3{ -2,0,-1 }));
			m_light.SetColor(FDB::Color::whiteFloat);
			m_light.SetIntensity(350.f);

			m_transform2 = FDB::Core::Transform();
			m_transform2.SetPosition(glm::vec3{ 0,0,1 });
			std::uint32_t frameCount = 0;
		//std::uint32_t frameCount = 0;

			m_light2 = FDB::Core::Light();
			m_light2.SetType(FDB::Core::LightType::Dir);
			m_light2.SetColor(FDB::Color::blueFloat);
			
			
			//Sphere function


			m_SphereMesh = std::make_unique<FDB::Core::Mesh>();
			auto p3d = RelaxAlgo::createSphere();

			std::vector<glm::vec3> unit;
			for (auto& p : p3d) 
				unit.push_back(glm::normalize(p));

			auto sub = CreateSphere::TransformPointToTriangle(p3d);
			sub = CreateSphere::applyNoise(unit, sub.indices, 20.0f, 5.0f, 5.5f);
			m_SphereMesh->subMeshes.push_back(std::move(sub));
			FDB::Core::MeshTools::ComputeTangent(*m_SphereMesh);

			m_transform3.SetPosition(glm::vec3{ 0,0,-10 });
			m_transform3.ScaleUp(glm::vec3{ 1.f/3.f });

			m_light2.SetPos({ 0,0,5 });
			m_light2.SetDir(glm::normalize(glm::vec3{ -2,-1,0 }));

			m_light2.SetIntensity(5.f);
			//lDatas.push_back(light2.GetData());
			m_lDatas.push_back(m_light.GetData());
		
		//	glm::vec4 p1,
		//	glm::vec4 p2,
			m_baseMaterials.baseColor = FDB::Core::textureLoader_t::Load("damier.jpg");
			m_baseMaterials.normalMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_normal_map.png");
			m_baseMaterials.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_emissive_map.png");
			m_baseMaterials.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_OMR_map.png");
		//	glm::vec4 p1,
			m_material.baseColor = FDB::Core::textureLoader_t::Load("Textures/BaseColor.png");
			m_material.normalMap = FDB::Core::textureLoader_t::Load("Textures/Normal.png");
			m_material.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Emissive.png");
			m_material.OMRMap = FDB::Core::textureLoader_t::Load("Textures/ORM.png");


			m_text.SetText("Hello, World!");
			m_text.SetAlign(FDB::Core::Text::Align::Center);
			m_text.SetFont(FDB::Core::fontLoader_t::Load("Lazer84.ttf",6));
			m_text.SetFont(FDB::Core::fontLoader_t::Load("Lazer84.ttf", 6));

			m_text.SetTexture(FDB::Core::textureLoader_t::Load("PBC.png"));

			m_ui.SetPosition({ 1920 / 2.f,1080 / 2.f });
			m_ui.SetScale({ 1920 ,1080 })	;


			m_mesh = FDB::Core::meshLoader_t::Load("bloc_1x1.obj");
			m_wav.SetWav(*FDB::Audio::wavManager_t::Load("chevre.mp3"));
			m_wav.SetVolume(10);
			m_wav.SetLoop(true);
			m_wav.Play();
		}
	public:
		void Render(FDB::Core::JobPool& pool) override
		{
			

			m_pipeline.BindVInData(m_camera.GetData());
			FDB::Render::fragmentShaderData outD{ m_lDatas };
			m_pipeline.BindVOutData(outD);

			auto image = m_renderer->GetFrameBuffer();
			auto viewPort = m_renderer->GetViewPort();
		//			});
			auto subs = m_mesh->subMeshes;
			m_pipeline.BindVertexData({m_transform.GetModelM(),m_material});
			for (auto& sub : subs)
				m_pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(), m_renderer->GetViewPort(), image, pool);

			m_pipeline.BindVertexData({m_transform2.GetModelM(),m_material});
			for (auto& sub : subs)
				m_pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(), m_renderer->GetViewPort(), image, pool);
			m_pipeline.BindVOutData(outD);

			subs = m_SphereMesh->subMeshes;
			m_pipeline.BindVertexData({m_transform3.GetModelM(),m_baseMaterials});
			for (auto& sub : subs)
				m_pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(), viewPort, image, pool);
			pool.WaitIdle();

			FDB::Render::UiShaders::PushConstant push;
			float aspectRatio = static_cast<float>(viewPort.width) / static_cast<float>(viewPort.height);
			push.aspectRatio = aspectRatio;
			push.inColor =m_ui.GetColor();
			push.texture1 = m_text.GetFont()->GetTexture();
			push.texture2 = m_text.GetTexture();

			m_textTransform.SetPosition(m_ui.GetPosNdc(aspectRatio));
			m_textTransform.SetScale(m_ui.GetScaleNdc(aspectRatio));
			push.model = m_textTransform.GetModel();
			m_pipeline2D.BindVertexData(push);
			auto data = m_text.GetBufferData();
			m_pipeline2D.Draw(data.vertices, data.indices, data.indicesCount, viewPort, image, pool);
			pool.WaitIdle();

		}
		void Update(float dt, FDB::Core::JobPool& pool) override
		{
			m_transform.Rotate(dt * 0.5f, { 0,0,-1 });
		}
	private:
		
		FDB::Core::Transform m_transform;
		FDB::Core::Transform m_transform2;
		FDB::Core::Transform m_transform3;

		FDB::Core::Transform m_camTransform;
		FDB::Core::Camera m_camera;
		std::vector<FDB::Core::lightData_t> m_lDatas;
		FDB::Core::Light m_light;
		FDB::Core::Light m_light2;

		FDB::Core::Material m_baseMaterials;
		FDB::Core::Material m_material;
		FDB::Core::Mesh* m_mesh = nullptr;
		std::unique_ptr<FDB::Core::Mesh> m_SphereMesh;


		FDB::Core::Text m_text;
		FDB::Core::Transform2D m_textTransform;
		FDB::Core::Ui m_ui;

		renderPath_t m_pipeline;
		renderPath2D_t m_pipeline2D;
		FDB::Audio::Wav m_wav;
	};
		//	P(1, 1, 1), P(-1, 1, 1),
		//	{ 0.f, 1.f, 0.f, 1.f });
	int main(int argc, char* argv[])
	{
		std::filesystem::path path = argv[0];
		FDB::Core::FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		FDB::Core::GameLoop loop("test", { windowWidth,windowHeight }, { renderWidth, renderHeight });
		loop.AddScene(std::make_unique<SampleScene>(), true);
		loop.AddScene(std::make_unique<FDB::Core::BasicScene>(), false);
		//// Face gauche : bleu
		loop.Loop();

	}