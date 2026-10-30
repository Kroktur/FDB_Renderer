
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

#include "GLM/detail/type_vec1.hpp"
#include "Pipeline/Atribute.h"
#include "Pipeline/RenderPath.h"
#include "Core/LoadObj.h" 

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
	

	

	// TODO MOVE TO FILE ATTRIBUTE

	// TODO MOVE TO FILE ATTRIBUTE
#include <span>

#include "Pipeline/Buffer.h"
#include "Pipeline/FrameBuffer.h"
#include "Pipeline/GraphicPipeline.h"

	//#include "Pipeline/Buffer.h"
#include "Core/Axis.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "Core/Mesh.h"
#include "Core/ResourceManager.h"
#include "Core/Texture.h"
#include "Core/Transform.h"
#include "Render/GraphicPipeline.h"

#include "Core/Material.h"
#include "Render/PBRFragment.h"
#include "Render/PBRShaders.h"
#include "Render/Sample.h"

	static constexpr std::int32_t windowWidth = 800;
	static constexpr std::int32_t windowHeight = 600;

	static constexpr std::int32_t renderWidth = 320;

	static constexpr std::int32_t renderHeight = 240;





	static constexpr size_t sizeLong = sizeof(long);
    // TODO bouger dans un fichier a part 
	// TODO zoption bias blend 
	namespace test
	{
		
	

	inline float AtomicCompareExchange(float* ptr,float exchange,float comparand) noexcept
	{
		auto* p = reinterpret_cast<volatile LONG*>(ptr);

		return std::bit_cast<float>(static_cast<std::uint32_t>(InterlockedCompareExchange(
			p,
			static_cast<LONG>(std::bit_cast<std::uint32_t>(exchange)),
			static_cast<LONG>(std::bit_cast<std::uint32_t>(comparand))
		)));
	}
	
	class SpinLock {
		std::atomic_flag flag = ATOMIC_FLAG_INIT;

	public:
		void lock() noexcept {
			while (flag.test_and_set(std::memory_order_acquire)) {
				// Spin
			}
		}
		void unlock() noexcept {
			flag.clear(std::memory_order_release);
		}
	};

	static constexpr  auto size = sizeof(std::atomic_flag);
}

	
#include "KTR/Tools/KTR_Chrono.h"
#include "KTR/Tools/KTR_Random.h"

	



	

	
	//static float angle = (43.0f * 3.14) / 180.f;
	
	using vertIn_t = FDB::Core::Vertex;
	using vertOut_t = FDB::Render::PBRFragment;
	using vertData_t = FDB::Render::PushConstant;
	using pathInData_t = FDB::Render::vertexShaderData;
	using pathOutData_t = FDB::Render::fragmentShaderData;






	int main(int argc, char* argv[])
	{
		FDB::Pipeline::buffer_t<int> intBuffer({ 10,10 });
		intBuffer.Resize({ 40,40 });
		intBuffer.Clear(0);

		FDB::Pipeline::buffer_view_t<int> data = intBuffer.GetView();
		++data.data;
		*(data.data + 1) = 5;

		std::filesystem::path path = argv[0];
		FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		std::cout << argv[0] << "\n";

		FDB::Core::Material baseMaterial;
		baseMaterial.baseColor = FDB::Core::textureLoader_t::Load("Textures/Base/base_color_map.png");
		baseMaterial.normalMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_normal_map.png");
		baseMaterial.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_emissive_map.png");
		baseMaterial.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_OMR_map.png");

		FDB::Core::Material material;
		material.baseColor = FDB::Core::textureLoader_t::Load("Textures/BaseColor.png");
		material.normalMap = FDB::Core::textureLoader_t::Load("Textures/Normal.png");
		material.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Emissive.png");
		material.OMRMap = FDB::Core::textureLoader_t::Load("Textures/ORM.png");

		auto* texture = FDB::Core::textureLoader_t::Load("image.png");

		auto* mesh = FDB::Core::meshLoader_t::Load("bloc_1x1.obj");

		//auto tes2t = textureLoad::Load("README.md");
		auto transfrom = FDB::Core::Transform();
		transfrom.ScaleUp({ 2.2f,2.2f,2.2f });
		transfrom.Rotate(glm::pi<float>() / 2.0f, { 1,0,0 });
		auto camera = FDB::Core::Camera();
		auto transformCam = FDB::Core::Transform();
		transformCam.LookAtDir({ 0,0,-1 });
		transformCam.SetPosition({ 0,0,5 });
		camera.Update(transformCam.GetModelM());

		std::vector<FDB::Core::lightData_t> lDatas;
		auto light = FDB::Core::Light();
		light.SetType(FDB::Core::LightType::Point);
		light.SetPos({ 0,0,3});
		light.SetDir(glm::normalize(glm::vec3{-2,0,-1}));
		light.SetColor(FDB::Color::whiteFloat);
		light.SetIntensity(150.f);



		auto light2 = FDB::Core::Light();
		light2.SetType(FDB::Core::LightType::Dir);
		light2.SetColor(FDB::Color::blueFloat);

		light2.SetPos({ 0,0,5 });
		light2.SetDir(glm::normalize(glm::vec3{ -2,-1,0 }));

		light2.SetIntensity(5.f);
		//lDatas.push_back(light2.GetData());
		lDatas.push_back(light.GetData());

		using renderPath_t = FDB::Pipeline::RenderPath<
			vertIn_t,
			vertOut_t,
			vertData_t,
			pathInData_t,
			pathOutData_t,
		&FDB::Render::PBRShader::VertexShader,
			&FDB::Render::PBRShader::FragmentShader<false>,
			FDB::Pipeline::default_rasterizerOpt_t>;





		renderPath_t pipeline;
		static constexpr float value = 3.4028230e+38f;
		static constexpr float result = 1.1754944e-38f;
		float i = value;
		test::AtomicCompareExchange(&i, result, value);
	/*	if (i == result)
			throw;*/


		FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
		FDB::Sdl::Window window("test", { windowWidth,windowHeight }, SDL_WINDOW_RESIZABLE);

		glm::vec<2, std::int32_t> size = { renderWidth,renderHeight };

		FDB::Render::SDLGraphicPipeline Grenderer(&window,size);
		//Grenderer.SetResolutionMode(FDB::Render::ResolutionMode::Adaptative);
		//Grenderer.GetRenderer()->SetVSync(1);


		Uint64 lastTime = SDL_GetTicks();
		std::uint32_t frameCount = 0;

		KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;
		//timer.Resume();
		while (!FDB::Sdl::Application::ShouldQuit() && !window.ShouldClose())
		{
			FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();
			// TODO refact fps
			{
				++frameCount;

				Uint64 currentTime = SDL_GetTicks();

				if (currentTime - lastTime >= 1000)
				{
					float fps = static_cast<float>(frameCount) * 1000.f /
						static_cast<float>(currentTime - lastTime);

					std::string title = std::to_string(static_cast<int>(fps));

					SDL_SetWindowTitle(
						window.GetSdlWindow(),
						title.c_str());

					frameCount = 0;
					lastTime = currentTime;
				}
			}
			Grenderer.ClearColor({ 0, 0, 0, 255 });
			Grenderer.BeginRender();
			pipeline.BindVInData(camera.GetData());
			FDB::Render::fragmentShaderData outD{ material  ,lDatas};
			pipeline.BindVOutData(outD);
			transfrom.Rotate(0.001f,  FDB::Core::Axis::forward);


			auto subs = mesh->subMeshes;
			pipeline.BindVertexData(transfrom.GetModelM());
			
			for(auto& sub : subs)
				pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());
			Grenderer.PresentRender();
		}
		FDB::Sdl::Application::Destroy();
		
	}




