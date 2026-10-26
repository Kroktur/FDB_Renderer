
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
#include "Pipeline/Pipeline.h"
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
	
	int main(int argc, char* argv[])
	{
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
		FDB::Sdl::Renderer renderer(window);
		//renderer.SetVSync(1);

		glm::vec<2, std::int32_t> size = { renderWidth,renderHeight };

		FDB::Sdl::Texture texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, size);
		std::vector<std::uint32_t> frameBuffer;
		std::vector<float> zBuffer;
		std::vector<glm::vec<4,float>> colorBuffer;
		zBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		colorBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));


		Transform::Vertex a,b,c,d;
		Uint64 lastTime = SDL_GetTicks();
		std::uint32_t frameCount = 0;

		int iteration = 0;
		int maxiteration = 0;
		int lastiteration = 0;
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

			std::memset(frameBuffer.data(), 0, frameBuffer.size() * sizeof(std::uint32_t));
			std::fill(zBuffer.begin(), zBuffer.end(), 1.f);
			std::fill(colorBuffer.begin(), colorBuffer.end(), glm::vec<4,float>(0,0,0,0));


			std::vector<Transform::Vertex> vertexList;
			std::vector<std::uint32_t> indices;

			Transform::Vertex a, b, c, d, e, f, g, h, i, j, k, l, m, n, o;

			// Cube centré en (0, 0, 0), côté 2
			vertexList.clear();
			indices.clear();

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
				P(1,- 1, 1),
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


			FDB::Rect<std::int32_t> port;

			renderer.Clear(FDB::Color::black8);
			port.x = 0;
			port.y =0;
			port.width = size.x;
			port.height = size.y;

			FDB::Pipeline::Image<std::uint32_t> image;
			image.buffer = frameBuffer.data();
			image.bufferHeight = size.y;
			image.bufferWidth = size.x;
			FDB::Pipeline::Image<glm::vec<4,float>> color_image;
			color_image.buffer = colorBuffer.data();
			color_image.bufferHeight = size.y;
			color_image.bufferWidth = size.x;
			FDB::Pipeline::Image<float> z_image;
			z_image.buffer = zBuffer.data();
			z_image.bufferHeight = size.y;
			z_image.bufferWidth = size.x;
			auto bench = [&](auto fn, const char* name)
				{
					static constexpr int n = 1000;
					const double freq = double(SDL_GetPerformanceFrequency());
					Uint64 s = SDL_GetPerformanceCounter();
					for (int i = 0; i < n; ++i) fn();
					Uint64 e = SDL_GetPerformanceCounter();

					double totalMs = double(e - s) * 1000.0 / freq;
					printf("%s : %.4f ms/appel (%d iterations, %.2f ms total)\n",
						name, totalMs / n, n, totalMs);
				};

			

			//bench([&] { FDB::Pipeline::Rasterizer::TriangleRasterizer<Vertex>::Compute<Test2::FragmentState::Keep>({ a,c,b }, port, image, [&](const Vertex& vertex, const glm::vec<2, std::int32_t>& pixel, float) -> Test2::fragment_output_t<Test2::FragmentState::Keep> { return  Test2::fragment_output_t<Test2::FragmentState::Keep>::Color(vertex.color);});}, "Rasterizer V2");
			//bench([&] { Rasterizer::Compute<Vertex, VertexInterpolation>({ a, c, b }, port, frameBuffer.data(), zBuffer.data(), size.x); }, "Rasterizer V1");
			//Rasterizer::Compute<Vertex,VertexInterpolation>({ a, c, b }, port, frameBuffer.data(),zBuffer.data(), size.x);

			using rasterizer_t = FDB::Pipeline::TriangleRasterizer<Transform::Vertex>;
			using fragmentState_t = FDB::Pipeline::FragmentState;
			using fragmentOutput = FDB::Pipeline::fragment_output_t<fragmentState_t::Keep>;
			using fragmentInput_t = FDB::Pipeline::FragmentInput<Transform::Vertex>;

			FDB::Pipeline::ImagesData imageData;
			imageData.colorImage = color_image;
			imageData.zBuffer = z_image;
			imageData.finalImage = image;

			Transform::Mesh mesh;
			mesh.vertices = vertexList;
			mesh.indices = indices;
			/*auto fragmentShader = [&](const fragmentInput_t& input)
			{
				return  fragmentOutput::Color(input.vertex.color);
			};

		
			
			using blendEnum = FDB::Pipeline::BlendFactor;
			using blendOp = FDB::Pipeline::BlendOp;
			using blendT = FDB::Pipeline::blendData_t < blendEnum::SrcAlpha, blendEnum::OneMinusSrcAlpha, blendOp::Add>;
			using DepthEnum = FDB::Pipeline::DepthOp;
			using DepthT = FDB::Pipeline::depthData_t<false, false, FDB::Pipeline::DepthOp::Never>;
			using cullType = FDB::Pipeline::cullData_t<FDB::Pipeline::CullFace::CCW, FDB::Pipeline::CullMode::Back>;
		auto rasterizer = 	[&](Transform::Vertex a, Transform::Vertex b, Transform::Vertex c) {				
			rasterizer_t::Compute< cullType, blendT, DepthT>({a,b,c }, port, imageData, fragmentShader);
			};
			Pipeline::createSommetList(vertexList, port, rasterizer);*/
			angle += 0.001f;
			pipeline.Draw(vertexList.data(), indices.data(), indices.size(), port, imageData);
			texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
			renderer.RenderTexture(texture);
			renderer.Present();
		}
		FDB::Sdl::Application::Destroy();
		
	}




