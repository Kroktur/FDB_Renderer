
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
#include "Core/Relaxation.h"

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

#include <KTR/Tools/KTR_Random.h>


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

	struct BRDFHelper
	{
		inline  static glm::vec3 F0(const glm::vec3& albedo, float metalness)
		{
			return glm::mix(glm::vec3(0.04f), albedo, metalness);
		}

		inline static glm::vec3 F(const glm::vec3& F0, float VDH)
		{
			return F0 + (glm::vec3(1.f) - F0) * std::pow(1.f - VDH, 5.0f);
		}

		inline static float D(float NDH, float alpha2)
		{
			float NDH2 = glm::pow(NDH, 2.0f);
			float denom = NDH2 * (alpha2 - 1.0f) + 1.0f;
			return alpha2 / (glm::pi<float>() * denom * denom);
		}

		inline static float G(float DOT, float k)
		{
			return DOT / (DOT * (1 - k) + k);
		}

		inline static glm::vec3 LambertDiffuse(const glm::vec3& albedo)
		{
			return albedo / glm::pi<float>();
		}

		inline static float FD90(float roughness, float VDH)
		{
			return  0.5f + 2.f * roughness * VDH * VDH;
		}

		inline static glm::vec3 DisneyDiffuse(const glm::vec3& albedo, float FD90, float NDL, float NDV)
		{
			float o1 = 1 + (FD90 - 1) * std::pow((1 - NDL), 5.0f);
			float o2 = 1 + (FD90 - 1) * std::pow((1 - NDV), 5.0f);

			return albedo / glm::pi<float>() * o1 * o2;
		}
	};

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
		static constexpr glm::vec3 ambientLight = glm::vec3(0.02f);
		static constexpr float emissiveFactor = 3.0f;

		const auto& v = out.vertex;

		// base data
		glm::vec3 emissive = glm::vec3(0.0f);
		float occlusion = 1.f;
		float metalness = 0.25f;
		float roughness = 0.75f;
		glm::vec3 albedo = v.fragColor;

		// load All data
		// TODO base color sRGB

		glm::vec3 normal = glm::normalize(v.normal);
		glm::vec3 result = albedo * (1 - metalness) * ambientLight * occlusion;
		glm::vec3 V = glm::normalize(v.eyePos - v.worldSpacePosition);

		// BRDF constant

		// make sure a is not 0 to avoid divide by 0
		const float a = std::max(roughness * roughness, 0.001f);
		const float aSquared = a * a;

		// TODO ask Charles multiple possibilities for k
		const float k = std::pow(roughness + 1.0f, 2.0f) / 8.0f;
		glm::vec3 F0 = BRDFHelper::F0(albedo,metalness);

		// diff = mat albedo / pi
		glm::vec3 diff = BRDFHelper::LambertDiffuse(albedo);

		for (const auto& light : data)
		{
			float rSquared =std::numeric_limits<float>::max();
			glm::vec3 L = {};

			if (light.type == FDB::Core::LightType::Point)
			{
				glm::vec3 dir = light.pos - v.worldSpacePosition;
				rSquared = dot(dir, dir);
				L = glm::normalize(dir);
			}
			else if (light.type == FDB::Core::LightType::Spot)
			{
				glm::vec3 dir = light.pos - v.worldSpacePosition;
				rSquared = dot(dir, dir);
				L = glm::normalize(dir);
			}
			else 
			{
				L = glm::normalize(-light.dir);
			}

			if (rSquared <= glm::epsilon<float>())
				continue;

			float NDL = std::max(glm::dot(normal, L), 0.0f);
			float NDV = std::max(glm::dot(normal, V), 0.0f);

			if (NDL <= 0.0f || NDV <= 0.0f)
				continue;

			glm::vec3 H = glm::normalize(L + V);
			float VDH = std::max(glm::dot(V, H), 0.0f);
			float NDH = std::max(glm::dot(normal, H), 0.0f);

			glm::vec3 F = BRDFHelper::F(F0, VDH);
			float D = BRDFHelper::D(NDH,aSquared);
			float G = BRDFHelper::G(NDV, k) * BRDFHelper::G(NDL, k);

			glm::vec3 Kd = (glm::vec3(1.f) - F) * (1.f - metalness);
			glm::vec3 spec = (D * G * F) / (4.0f * NDV * NDL);
			spec *= glm::vec3(light.specColor);

			// disney version

		/*	float FD90 = BRDFHelper::FD90(roughness, VDH);
			diff = BRDFHelper::DisneyDiffuse(albedo, FD90, NDL, NDV);*/

			float E = 0.0f;
			if (light.type == FDB::Core::LightType::Point)
			{
				E = light.intensity / (4.0f * glm::pi<float>() * rSquared);
			}
			else if (light.type == FDB::Core::LightType::Dir)
			{
				 E = light.intensity;
			}
			else
			{
				float spotAngle = glm::dot(glm::normalize(-light.dir),L);
				float cosInner = std::cos(light.openAngle);
				float cosOuter = std::cos(light.openAngle + light.AdditionalSmooth);
				float spotAttenuation = glm::smoothstep(cosOuter,cosInner,spotAngle);
				float attenuation = light.intensity / (4.0f * glm::pi<float>() * rSquared);
				E = attenuation * spotAttenuation;
			}
			result += (Kd * diff + spec) * glm::vec3(light.color) * NDL * E;
		}
		result += emissive * emissiveFactor;
		return fragReturn_t::Color(glm::vec4(result, 1.0f));
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
		FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		std::cout << argv[0] << "\n";


		auto* mesh = FDB::Core::meshLoader_t::Load("RSI-Arrastra.obj");

		//auto tes2t = textureLoad::Load("README.md");
		auto transfrom = FDB::Core::Transform();
		transfrom.ScaleUp(glm::vec3{1.f/4.f});
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

		light.SetIntensity(110.f);



		auto light2 = FDB::Core::Light();
		light2.SetType(FDB::Core::LightType::Dir);
		light2.SetPos({ 0,0,5 });
		light2.SetDir(glm::normalize(glm::vec3{ -2,-1,0 }));

		light2.SetIntensity(5.f);
		lDatas.push_back(light2.GetData());
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

		std::vector<glm::vec2> points;

		KTR::Random::default_value_type seed { KTR::Random::default_seed_gen_type{}() };
		KTR::Random::default_bit_gen_type gen(seed);
		KTR::distrib::Distribution<float> distX(0.f, 800.f);
		KTR::distrib::Distribution<float> distY(0.f, 600.f);

		for (size_t i = 0; i < 1024; ++i)
			points.push_back(glm::vec2{ distX(gen), distY(gen) });


		/*std::vector<glm::vec2> points = {
	{120,  80}, {350, 120}, {180, 240}, {600, 100},
	{720, 200}, {400, 300}, {250, 400}, {100, 500},
	{650, 450}, {500, 180}, {300, 200}, {330, 210},
	{340, 205}, {520, 350}, {700, 520}, {150, 150},
	{450, 480}, {580, 250}, {200, 350}, {750,  80}
		};*/
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

			Grenderer.ClearColor({ 173, 216, 230, 255 });
			Grenderer.BeginRender();
			pipeline.BindVInData(camera.GetData());
			pipeline.BindVOutData(lDatas);

			transfrom.Rotate(0.01f,  FDB::Core::Axis::forward);

			//pipeline.Draw(vertexList.data(), indices.data(), indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());



			//auto startP = std::chrono::high_resolution_clock::now();
			//auto actual = timer.GetTime();

			auto subs = mesh->subMeshes;
			pipeline.BindVertexData(transfrom.GetModelM());
			
			//for(auto& sub : subs)
				//pipeline.Draw(sub.vertices.data(), sub.indices.data(), sub.indices.size(),Grenderer.GetViewPort(),Grenderer.GetFrameBuffer());
			points = RelaxAlgo::relaxationAlgo(points, 800, 600);
			
			SDL_SetRenderDrawColor(Grenderer.m_renderer.GetSdlRenderer(), 0, 0, 0, 255);
			SDL_RenderClear(Grenderer.m_renderer.GetSdlRenderer());
			SDL_SetRenderDrawColor(Grenderer.m_renderer.GetSdlRenderer(), 255, 255, 255, 0);

			for (const auto& point : points) {
				SDL_RenderPoint(Grenderer.m_renderer.GetSdlRenderer(), point.x, point.y);
			}

			SDL_RenderPresent(Grenderer.m_renderer.GetSdlRenderer());
			/*auto now = timer.GetTime();
				std::cout << KTR::chrono::Function::DurationCast<KTR::duration::seconds_type<float>>(now - actual).count() << "\n";*/

			//auto endP = std::chrono::high_resolution_clock::now();

			//auto result = endP - startP;

		  //std::cout << result.count() << " \n";
			//Grenderer.PresentRender();
		}
		FDB::Sdl::Application::Destroy();
		
	}




