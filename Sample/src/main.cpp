
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
#include "Pipeline/Transform.h"

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
#include "Core/ResourceManager.h"
#include "Render/GraphicPipeline.h"


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

	

	
	//static float angle = (43.0f * 3.14) / 180.f;
	static float angle =0;

	using vertIn_t = Transform::Vertex;
	using vertOut_t = Transform::Vertex;

	using vertData_t = int;
	using pathInData_t = int;
	using pathOutData_t = int;
	inline static vertOut_t VShader(const vertIn_t& input,const vertData_t&,const pathInData_t&)
	{

		glm::mat4 model(1.0f);
		model = glm::rotate(
			model, angle, glm::normalize(glm::vec3(1.f, 1.f, 0.f)));
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
		glm::mat4 projection = glm::perspectiveRH_ZO(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10000.0f); // set the projection
		//glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f,  0.1f, 10000.0f);
		vertIn_t result = input;
		glm::vec4 position(result.pos);
		glm::vec4 worldPosition = model * position;
		glm::vec4 viewPosition = view * worldPosition;
		glm::vec4 clipPosition = projection * viewPosition;
		result.pos = clipPosition;
		return result;
	}
	using fragReturn_t = FDB::Pipeline::fragment_output_t<FDB::Pipeline::FragmentState::Keep>;
	inline static fragReturn_t FShader(const FDB::Pipeline::fragment_input_t<vertOut_t>& out,const pathOutData_t& data)
	{
		return fragReturn_t::Color(out.vertex.color);
	}

	struct Texture{};
	std::unique_ptr<Texture> loadTexture(const std::filesystem::path& path)
	{
		return std::make_unique<Texture>();
	}

	int main(int argc, char* argv[])
	{
		FDB::Pipeline::buffer_t<int> intBuffer({ 10,10 });
		intBuffer.Resize({ 40,40 });
		intBuffer.Clear(0);

		FDB::Pipeline::buffer_view_t<int> data = intBuffer.GetView();
		++data.data;
		*(data.data + 1) = 5;



		FilePathSystem::AddFilePath(std::filesystem::path("C:/Users/arthu/Desktop/FDB_Render"));

		using textureLoad = ResourceManager < Texture, TypeWrapper<>, loadTexture>;

		auto test = textureLoad::Load("README.md");
		auto tes2t = textureLoad::Load("README.md");


		using renderPath_t = FDB::Pipeline::RenderPath<
			vertIn_t,
			vertOut_t,
			vertData_t,
			pathInData_t,
			pathOutData_t,
			&VShader,
			&FShader,
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
		//FDB::Sdl::Renderer renderer(window);
		//renderer.SetVSync(1);

		glm::vec<2, std::int32_t> size = { renderWidth,renderHeight };

		//FDB::Sdl::Texture texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, size);
	
		//FDB::Pipeline::GraphicsPipeline basePipeline;

		FDB::Render::SDLGraphicPipeline Grenderer(&window,size);
		//Grenderer.SetResolutionMode(FDB::Render::ResolutionMode::Adaptative);
		Uint64 lastTime = SDL_GetTicks();
		std::uint32_t frameCount = 0;

		int iteration = 0;
		int maxiteration = 0;
		int lastiteration = 0;

		std::vector<Transform::Vertex> vertexList;
		std::vector<std::uint32_t> indices;

		auto addFace = [&](glm::vec4 p0,
			glm::vec4 p1,
			glm::vec4 p2,
			glm::vec4 p3,
			glm::vec4 color)
			{
				unsigned int base = static_cast<unsigned int>(vertexList.size());

				Transform::Vertex v0, v1, v2, v3;

				v0.pos = p0;
				v1.pos = p1;
				v2.pos = p2;
				v3.pos = p3;

				v0.color = { 1.f,0.f,0.f,1.f };
				v1.color = { 0.f,1.f,0.f,1.f };
				v2.color = { 0.f,0.f,1.f,1.f };
				v3.color = { 1.f,1.f,1.f,1.f };

				vertexList.push_back(v0);
				vertexList.push_back(v1);
				vertexList.push_back(v2);
				vertexList.push_back(v3);

				// Deux triangles CCW vus de l'extérieur
				indices.insert(indices.end(), {
					base, base + 1, base + 2,
					base, base + 2, base + 3
					});
			};

		auto P = [](float x, float y, float z) {
			return glm::vec4(x, y, z, 1.f);
			};

		// Face arrière : rouge
		addFace(P(-1, -1, -1), P(-1, 1, -1),
			P(1, 1, -1), P(1, -1, -1),
			{ 1.f, 0.f, 0.f, 1.f });

		// Face avant : vert
		addFace(P(-1, -1, 1), P(1, -1, 1),
			P(1, 1, 1), P(-1, 1, 1),
			{ 0.f, 1.f, 0.f, 1.f });

		// Face gauche : bleu
		addFace(P(-1, -1, -1), P(-1, -1, 1),
			P(-1, 1, 1), P(-1, 1, -1),
			{ 0.f, 0.f, 1.f, 1.f });

		// Face droite : jaune
		addFace(P(1, -1, -1), P(1, 1, -1),
			P(1, 1, 1), P(1, -1, 1),
			{ 1.f, 1.f, 0.f, 1.f });

		// Face du bas : magenta
		// Face du bas (-Y) : normale vers le bas
		addFace(
			P(-1, -1, -1),
			P(1, -1, -1),
			P(1, -1, 1),
			P(-1, -1, 1),
			{ 0.f, 1.f, 1.f, 1.f }
		);

		// Face du haut (+Y) : normale vers le haut
		addFace(
			P(-1, 1, 1),
			P(1, 1, 1),
			P(1, 1, -1),
			P(-1, 1, -1),
			{ 0.f, 1.f, 1.f, 1.f }
		);


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

			Grenderer.ClearColor(FDB::Color::black8);
			Grenderer.BeginRender();




			angle += 0.001f;
			KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;
			timer.Resume();
			auto actual = KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime());

			pipeline.Draw(vertexList.data(), indices.data(), indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());

			auto now = KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(timer.GetTime());

			std::cout << (now - actual).count() << "\n";
			Grenderer.PresentRender();
		}
		FDB::Sdl::Application::Destroy();
		
	}




