
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
#include "Core/Transform.h"
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

	struct VertexOut
	{
		glm::vec4 pos ;
		glm::vec3 normal;
		glm::vec3 worldSpacePosition;
		glm::vec3 eyePos;
		glm::vec2 fragTexCoord;
		glm::vec4 fragColor;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		[[nodiscard]] static  constexpr  FDB::Pipeline::Attribute PosAttribute()
		{
			return FDB::Pipeline::Attribute{ FDB::Pipeline::format::Float4,offsetof(VertexOut, pos) };
		}
		[[nodiscard]] static constexpr  std::array<FDB::Pipeline::Attribute, 8> AllAttributes()
		{
			return { FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(VertexOut, pos)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(VertexOut, normal)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(VertexOut, worldSpacePosition)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(VertexOut, eyePos)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VertexOut, fragTexCoord)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(VertexOut, fragColor)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(VertexOut, tangent)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(VertexOut, bitangent)}
			};
		}

	};

	
	//static float angle = (43.0f * 3.14) / 180.f;
	static float angle =0;

	using vertIn_t = FDB::Core::Vertex;
	using vertOut_t = VertexOut;

	using vertData_t = glm::mat4;
	using pathInData_t = FDB::Core::cameraDat_t;
	using pathOutData_t = std::vector<FDB::Core::lightData_t>;
	inline static vertOut_t VShader(const vertIn_t& input,const vertData_t& data,const pathInData_t& camData)
	{
		vertOut_t result;
		glm::vec4 wsPos = data * input.pos;
		result.pos = camData.proj * (camData.view * wsPos);
		result.worldSpacePosition = { wsPos.x,wsPos.y,wsPos.z };
		glm::vec4 pos4 = camData.invView * glm::vec4(0, 0, 0, 1);
		result.eyePos = { pos4.x,pos4.y,pos4.z };
		result.fragTexCoord = input.uv;
		result.fragColor = input.color;
		result.normal = glm::normalize(data * glm::vec4(input.normal.x, input.normal.y, input.normal.z, 0.0f));
		result.tangent= glm::normalize(data * glm::vec4(input.tangent.x, input.tangent.y, input.tangent.z, 0.0f));
		result.bitangent = glm::normalize(data * glm::vec4(input.bitangent.x, input.bitangent.y, input.bitangent.z, 0.0f));
		return result;
	}
	using fragReturn_t = FDB::Pipeline::fragment_output_t<FDB::Pipeline::FragmentState::Keep>;
	inline static fragReturn_t FShader(const FDB::Pipeline::fragment_input_t<vertOut_t>& out,const pathOutData_t& data)
	{
		float occlusion = 0.f;
		float metalness = 0.f;
		float roughness = 0.f;
		const auto& v = out.vertex;
		glm::vec3 normal = v.normal;
		glm::vec4 result =glm::vec4(0.f);

		glm::vec3 baseColor = v.fragColor;
		glm::vec3 albedo = glm::pow(
			glm::vec3(baseColor),
			glm::vec3(2.2f)
		);

		result += glm::vec4(albedo, 1.0f)
			* (1.0f - metalness)
			* occlusion
			* 0.02f;

		for (std::size_t i = 0 ; i < data.size() ; ++i)
		{
			const FDB::Core::lightData_t& light = data[i];


			if (light.type == FDB::Core::LightType::Point)
			{
				glm::vec3 lightPos = light.pos;
				glm::vec3 lightColor = light.color;
				glm::vec3 L = glm::normalize(lightPos - v.worldSpacePosition);
				glm::vec3 N = normal;
				glm::vec3 V = glm::normalize(v.eyePos - v.worldSpacePosition);
				glm::vec3 H = glm::normalize(L + V);


				glm::vec3 Kd = glm::mix(baseColor, glm::vec3(0.f), metalness);
				glm::vec3 Ks = glm::mix(glm::vec3(1.0f), baseColor, metalness);
				float Ns = 1.0f / glm::pow(roughness, 2.0f);
				float intensity = light.intensity;
				float r = glm::length(lightPos - v.worldSpacePosition);
				float attenuation = (intensity / (4.0f * 3.14159265f * glm::pow(r, 2.0f)));
				float diffuse = glm::max(glm::dot(N, L), 0.f);
				glm::vec3 specular = Ks * glm::vec3(light.specColor) *glm::pow(glm::max(glm::dot(N,H),0.f),Ns);
				glm::vec3 Fr = (Kd * 1.f / 3.14159265f) + glm::vec3(specular);
				float Ej = diffuse * attenuation;
				result += glm::vec4((Fr * Ej) * lightColor,1.f);
			}
			
		}

		result = glm::pow(result, glm::vec4(2.2f));
		result.a = 1.0f;
		return fragReturn_t::Color(result);
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

		std::filesystem::path path = argv[0];
		FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path()/ "Resources");
		std::cout << argv[0] << "\n";


		auto* mesh = FDB::Core::meshLoader_t::Load("polaris.obj");

		//auto tes2t = textureLoad::Load("README.md");
		auto transfrom = FDB::Core::Transform();
		auto camera = FDB::Core::Camera();
		auto transformCam = FDB::Core::Transform();
		transformCam.LookAtDir({ 0,0,-1 });
		transformCam.SetPosition({ 0,0,5 });
		camera.Update(transformCam.GetModelM());

		std::vector<FDB::Core::lightData_t> lDatas;
		auto light = FDB::Core::Light();
		light.SetType(FDB::Core::LightType::Point);
		light.SetPos({ 1,0,2});
		light.SetIntensity(100.f);
		lDatas.push_back(light.GetData());


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

		

		//auto addFace = [&](glm::vec4 p0,
		//	glm::vec4 p1,
		//	glm::vec4 p2,
		//	glm::vec4 p3,
		//	glm::vec4 color)
		//	{
		//		unsigned int base = static_cast<unsigned int>(vertexList.size());

		//		Transform::Vertex v0, v1, v2, v3;

		//		v0.pos = p0;
		//		v1.pos = p1;
		//		v2.pos = p2;
		//		v3.pos = p3;

		//		v0.color = { 1.f,0.f,0.f,1.f };
		//		v1.color = { 0.f,1.f,0.f,1.f };
		//		v2.color = { 0.f,0.f,1.f,1.f };
		//		v3.color = { 1.f,1.f,1.f,1.f };

		//		vertexList.push_back(v0);
		//		vertexList.push_back(v1);
		//		vertexList.push_back(v2);
		//		vertexList.push_back(v3);

		//		// Deux triangles CCW vus de l'extérieur
		//		indices.insert(indices.end(), {
		//			base, base + 1, base + 2,
		//			base, base + 2, base + 3
		//			});
		//	};

		//auto P = [](float x, float y, float z) {
		//	return glm::vec4(x, y, z, 1.f);
		//	};

		//// Face arrière : rouge
		//addFace(P(-1, -1, -1), P(-1, 1, -1),
		//	P(1, 1, -1), P(1, -1, -1),
		//	{ 1.f, 0.f, 0.f, 1.f });

		//// Face avant : vert
		//addFace(P(-1, -1, 1), P(1, -1, 1),
		//	P(1, 1, 1), P(-1, 1, 1),
		//	{ 0.f, 1.f, 0.f, 1.f });

		//// Face gauche : bleu
		//addFace(P(-1, -1, -1), P(-1, -1, 1),
		//	P(-1, 1, 1), P(-1, 1, -1),
		//	{ 0.f, 0.f, 1.f, 1.f });

		//// Face droite : jaune
		//addFace(P(1, -1, -1), P(1, 1, -1),
		//	P(1, 1, 1), P(1, -1, 1),
		//	{ 1.f, 1.f, 0.f, 1.f });

		//// Face du bas : magenta
		//// Face du bas (-Y) : normale vers le bas
		//addFace(
		//	P(-1, -1, -1),
		//	P(1, -1, -1),
		//	P(1, -1, 1),
		//	P(-1, -1, 1),
		//	{ 0.f, 1.f, 1.f, 1.f }
		//);

		//// Face du haut (+Y) : normale vers le haut
		//addFace(
		//	P(-1, 1, 1),
		//	P(1, 1, 1),
		//	P(1, 1, -1),
		//	P(-1, 1, -1),
		//	{ 0.f, 1.f, 1.f, 1.f }
		//);
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

			Grenderer.ClearColor(FDB::Color::black8);
			Grenderer.BeginRender();
			pipeline.BindVInData(camera.GetData());
			pipeline.BindVOutData(lDatas);

			transfrom.Rotate(0.001f, FDB::Core::Axis::up);
			transfrom.Rotate(0.001f, FDB::Core::Axis::right);

			//pipeline.Draw(vertexList.data(), indices.data(), indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());



			//auto startP = std::chrono::high_resolution_clock::now();
			//auto actual = timer.GetTime();

			auto subs = mesh->subMeshes;
			pipeline.BindVertexData(transfrom.GetModelM());
			
			for(auto& sub : subs)
				pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());
			/*auto now = timer.GetTime();
				std::cout << KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(now - actual).count() << "\n";*/

			//auto endP = std::chrono::high_resolution_clock::now();

			//auto result = endP - startP;

		  //std::cout << result.count() << " \n";

			Grenderer.PresentRender();
		}
		FDB::Sdl::Application::Destroy();
		
	}




