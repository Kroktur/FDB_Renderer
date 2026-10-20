
	#include <iostream>
	#include <string>

	#include "Core/Rect.h"
	#include "GLM/vec4.hpp"
	#include "GLM/vec3.hpp"
	#include "SDL/Application.h"
	#include "SDL/Renderer.h"
	#include "SDL/Window.h"
	#include "SDL/Texture.h"
	#include "SDL3/SDL_timer.h"

#include "GLM/detail/type_vec1.hpp"

enum format
{
	float1,
	float2,
	float3,
	float4
};

struct Atribut
{
	format format;
	std::size_t offset;
};



#include <array>


	

	template<typename T,size_t N>
	struct Box
	{
		using value_type = T;
		glm::vec<N, value_type> min;
		glm::vec<N, value_type> max;
	};
	template<typename T>
	using box2_t = Box<T, 2>;


	constexpr int SubPixelBits = 8;
	constexpr int SubPixelScale = 1 << SubPixelBits;

	static int32_t ToFixed(float v)
	{
		return static_cast<int32_t>(std::round(v * SubPixelScale));
	}

	struct RasterizerData
	{
		RasterizerData(const glm::vec<4, float>(&pos)[3], const  box2_t<int>& box)
		{
			glm::vec<2, float> minF = { static_cast<float>(box.min.x) , static_cast<float>(box.min.y) };

			edgesFn = { RasterizerData::EdgeFunction(pos[0].x, pos[0].y, pos[1].x,pos[1].y, minF.x, minF.y)
				,RasterizerData::EdgeFunction(pos[1].x, pos[1].y, pos[2].x, pos[2].y,minF.x, minF.y)
				,RasterizerData::EdgeFunction(pos[2].x, pos[2].y, pos[0].x,pos[0].y, minF.x, minF.y) };
			

			baseIncrement[0] = glm::vec<2, float>{ (pos[1].y - pos[0].y),-(pos[1].x - pos[0].x) };
			baseIncrement[1] = glm::vec<2, float>{ (pos[2].y - pos[1].y),-(pos[2].x - pos[1].x) };
			baseIncrement[2] = glm::vec<2, float>{ (pos[0].y - pos[2].y),-(pos[0].x - pos[2].x) };
		}
		RasterizerData(const RasterizerData& other, const glm::vec<2, float>& step, const glm::vec<2, float> blockStep)
		{

			edgesFn = {
				other.edgesFn[0] + other.baseIncrement[0].y * step.y + other.baseIncrement[0].x * step.x
				, other.edgesFn[1] + other.baseIncrement[1].y * step.y + other.baseIncrement[1].x * step.x
				,other.edgesFn[2] + other.baseIncrement[2].y * step.y + other.baseIncrement[2].x * step.x
			};

			baseIncrement[0] = { other.baseIncrement[0] * blockStep };
			baseIncrement[1] = { other.baseIncrement[1] * blockStep };
			baseIncrement[2] = { other.baseIncrement[2] * blockStep };
		}
		glm::vec<3, float> edgesFn;
		glm::vec<2, float> baseIncrement[3];
		static float EdgeFunction(float Ax, float Ay, float Bx, float By, float Px, float Py)
		{
			return (Px - Ax) * (By - Ay) - (Py - Ay) * (Bx - Ax);
		}
	};

	template<typename VertexType, typename VertexInterpolationType>
	concept IsValidInterpolation = requires(VertexType vertex, VertexInterpolationType interpolation, const VertexType(&triangle)[3], glm::vec< 3, float> invW, const RasterizerData & data, glm::vec<4, float>(&pos)[3])
	{
		VertexInterpolationType{ triangle,invW };
		interpolation.Process(data, data,invW[0]);
		interpolation.StartData();
		interpolation.IncrementY();
		interpolation.IncrementX();
		interpolation.ApplyFragment(vertex);
		VertexInterpolationType::GetInvW(triangle, invW);
		VertexInterpolationType::GetPos(triangle, pos);
	};

	template<std::size_t N>
	struct FloatInterpolation
	{
		FloatInterpolation() = default;
		~FloatInterpolation() = default;
		void Init(const glm::vec<N,float>(&pos)[3], const glm::vec<3, float>& invW)
		{
			dataOverW[0] = pos[0]* invW[0];
			dataOverW[1] = pos[1] * invW[1];
			dataOverW[2] = pos[2] * invW[2];
		}
		void Init(const glm::vec<3, float>& invW)
		{
			dataOverW[0] = glm::vec<N,float>(1.0f)  * invW[0];
			dataOverW[1] = glm::vec<N, float>(1.0f) * invW[1];
			dataOverW[2] = glm::vec<N, float>(1.0f) * invW[2];
		}
		void Process(const RasterizerData& data, const RasterizerData& base , float invArea)
		{
			startData = (data.edgesFn.x * dataOverW[2]
				+ data.edgesFn.y * dataOverW[0]
				+ data.edgesFn.z * dataOverW[1]) * invArea;
			dataIncrement[0] = (base.baseIncrement[0].x * dataOverW[2]
				+ base.baseIncrement[1].x * dataOverW[0]
				+ base.baseIncrement[2].x * dataOverW[1]) * invArea;
			dataIncrement[1] = (base.baseIncrement[0].y * dataOverW[2]
				+ base.baseIncrement[1].y * dataOverW[0]
				+ base.baseIncrement[2].y * dataOverW[1]) * invArea;
		}
		void StartData()
		{
			currentData = startData;
		}
		void IncrementX()
		{
			currentData += dataIncrement[0];
		}
		void IncrementY()
		{
			startData += dataIncrement[1];
		}
		void ApplyValue(glm::vec<N,float>& data) const
		{
			data = currentData;
		}
	private:
		glm::vec<N, float> dataOverW[3];
		glm::vec<N, float>  startData;
		glm::vec<N,float> dataIncrement[2];
		glm::vec<N, float> currentData;
	};


	struct Rasterizer
	{
		// TODO passer en fix point 
		// TODO simd
		static  constexpr  glm::vec<2, int> blockDim = { 64,64 };


		template<typename VertexInput, typename InterpolationT> requires(IsValidInterpolation<VertexInput, InterpolationT>)
			static void Compute(const VertexInput(&tri)[3], const FDB::Rect<std::int32_t>& viewPort, std::uint32_t* buffer, float* zbuffer, std::int32_t bufferWidth)
		{
			glm::vec<4, float> pos[3];
			InterpolationT::GetPos(tri, pos);
			float area = Rasterizer::GetArea(pos);
			if (area <= 0)
				return;
			float invArea = 1.f / area;

			box2_t<int> box = Rasterizer::GetBox(pos, viewPort);
			
			glm::vec<3, float> baseInvW;
			InterpolationT::GetInvW(tri, baseInvW);
			FloatInterpolation<1> valW;
			valW.Init(baseInvW);
			InterpolationT inter(tri, baseInvW);
			RasterizerData baseData(pos, box);
			glm::vec<3, float> baseEdgeIncrement[2] = { glm::vec<3, float>(baseData.baseIncrement[0].x, baseData.baseIncrement[1].x, baseData.baseIncrement[2].x), glm::vec<3, float>(baseData.baseIncrement[0].y, baseData.baseIncrement[1].y, baseData.baseIncrement[2].y) };


			for (int blockY = box.min.y; blockY <= box.max.y; blockY += blockDim.y)
			{
				glm::vec<2, int> blockSize = {};
				blockSize.y = std::min(blockDim.y, box.max.y - blockY + 1);
				for (int blockX = box.min.x; blockX <= box.max.x; blockX += blockDim.x)
				{
					blockSize.x = std::min(blockDim.x, box.max.x - blockX + 1);
					glm::vec<2, float> step = { blockX - box.min.x,blockY - box.min.y };
					glm::vec<2, float> blockStep = { blockSize.x - 1 ,blockSize.y - 1 };
					RasterizerData currentData(baseData, step, blockStep);

					if (IsBlockOut(currentData.edgesFn, currentData.baseIncrement))
						continue;

					valW.Process(currentData, baseData, invArea);
					inter.Process(currentData, baseData,invArea);
					if (IsBlockFullyInside(currentData.edgesFn, currentData.baseIncrement))
					{
						for (int y = 0; y < blockSize.y; ++y)
						{
							valW.StartData();
							inter.StartData();
							std::uint32_t* pixelPtr = buffer + ((blockY + y) * bufferWidth + blockX);
							float* zBufferPtr = zbuffer + ((blockY + y) * bufferWidth + blockX);
							for (int x = 0; x < blockSize.x; ++x)
							{
								{
									glm::vec<1, float> test;
									valW.ApplyValue(test);
									float currentW = 1.f / test.x;
									VertexInput fragment = tri[0];
									inter.ApplyFragment(fragment);
									glm::vec<4, float> finalColor = fragment.color * 255.f * currentW;
									std::uint32_t r = static_cast<std::uint32_t>(finalColor.r);
									std::uint32_t g = static_cast<std::uint32_t>(finalColor.g);
									std::uint32_t b = static_cast<std::uint32_t>(finalColor.b);
									std::uint32_t a = static_cast<std::uint32_t>(finalColor.a);
									*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
								}
								++pixelPtr;
								++zBufferPtr;
								valW.IncrementX();
								inter.IncrementX();
							}
							valW.IncrementY();
							inter.IncrementY();
						}

					}
					else
					{
						for (int y = 0; y < blockSize.y; ++y)
						{
							glm::vec<3, float> edgeFn = currentData.edgesFn;
							valW.StartData();
							inter.StartData();
							std::uint32_t* pixelPtr = buffer + ((blockY + y) * bufferWidth + blockX);
							for (int x = 0; x < blockSize.x; ++x)
							{
								if (edgeFn[0] >= 0 && edgeFn[1] >= 0 && edgeFn[2] >= 0)
								{
									{
										glm::vec<1, float> test;
										valW.ApplyValue(test);
										float currentW = 1.f / test.x;
										VertexInput fragment = tri[0];
										inter.ApplyFragment(fragment);
										glm::vec<4, float> finalColor = fragment.color * 255.f * currentW;
										std::uint32_t r = static_cast<std::uint32_t>(finalColor.r);
										std::uint32_t g = static_cast<std::uint32_t>(finalColor.g);
										std::uint32_t b = static_cast<std::uint32_t>(finalColor.b);
										std::uint32_t a = static_cast<std::uint32_t>(finalColor.a);
										*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
									}
								}
								++pixelPtr;
								edgeFn += baseEdgeIncrement[0];
								valW.IncrementX();
								inter.IncrementX();
							}

							currentData.edgesFn += baseEdgeIncrement[1];
							valW.IncrementY();
							inter.IncrementY();
						}
					}
				}
			}
		}
	private:
		static float GetArea(const glm::vec<4, float>(&pos)[3])
		{
			return RasterizerData::EdgeFunction(
				pos[0].x, pos[0].y,
				pos[1].x, pos[1].y,
				pos[2].x,
				pos[2].y);
		}
		static box2_t<int> GetBox(const glm::vec<4, float>(&pos)[3], const FDB::Rect<std::int32_t>& viewPort)
		{
			box2_t<int> box;
			box.min.x = std::max(viewPort.x, static_cast<int>(std::min({ pos[0].x, pos[1].x, pos[2].x })));
			box.max.x = std::min(viewPort.x + viewPort.width - 1, static_cast<int>(std::max({ pos[0].x, pos[1].x, pos[2].x })));
			box.min.y = std::max(viewPort.y, static_cast<int>(std::min({ pos[0].y, pos[1].y, pos[2].y })));
			box.max.y = std::min(viewPort.y + viewPort.height - 1, static_cast<int>(std::max({ pos[0].y, pos[1].y, pos[2].y })));
			return box;
		}
		static bool IsBlockOut(const glm::vec<3, float>& blockEdgesFn, const glm::vec<2, float>(&increment)[3])
		{

			float abBlockEdgeMax = blockEdgesFn[0] + std::max(increment[0].x, 0.f) + std::max(increment[0].y, 0.f);
			float bcBlockEdgeMax = blockEdgesFn[1] + std::max(increment[1].x, 0.f) + std::max(increment[1].y, 0.f);
			float caBlockEdgeMax = blockEdgesFn[2] + std::max(increment[2].x, 0.f) + std::max(increment[2].y, 0.f);

			if (abBlockEdgeMax < 0.f || bcBlockEdgeMax < 0.f || caBlockEdgeMax < 0.f)
				return true;
			return false;
		}
		static bool IsBlockFullyInside(const glm::vec<3, float>& blockEdgesFn, const glm::vec<2, float>(&increment)[3])
		{

			float abBlockEdgeMin = blockEdgesFn[0] + std::min(increment[0].x, 0.f) + std::min(increment[0].y, 0.f);
			float bcBlockEdgeMin = blockEdgesFn[1] + std::min(increment[1].x, 0.f) + std::min(increment[1].y, 0.f);
			float caBlockEdgeMin = blockEdgesFn[2] + std::min(increment[2].x, 0.f) + std::min(increment[2].y, 0.f);

			if (abBlockEdgeMin >= 0.f && bcBlockEdgeMin >= 0.f && caBlockEdgeMin >= 0.f)
				return true;
			return false;
		}
	};


	struct Vertex
	{
		glm::vec4 pos;
		glm::vec<4, float> color;
	};
	using triangle = Vertex[3];

	struct VertexInterpolation
	{
		 VertexInterpolation(const triangle& tri, const glm::vec<3, float>& invW)
		 {
			glm::vec<4,float> poss[3] = {tri[0].pos,tri[1].pos,tri[2].pos};
			glm::vec<4, float> colors[3] = { tri[0].color,tri[1].color,tri[2].color };

			color.Init(colors, invW);
			position.Init(poss, invW);

		 }
		 void Process(const RasterizerData& data,const RasterizerData& base,float invArea)
		 {
			color.Process(data,base,invArea);
			position.Process(data, base,invArea);
		 }
		 void StartData()
		 {
			 color.StartData();
			 position.StartData();
		 }
		 void IncrementY()
		 {
			 color.IncrementY();
			 position.IncrementY();
		 }
		 void IncrementX()
		 {
			 color.IncrementX();
			 position.IncrementX();
		 }
		 void ApplyFragment(Vertex& fragment) const 
		 {
			 color.ApplyValue(fragment.color);
			 position.ApplyValue(fragment.pos);
		 }
		
		 static void GetPos(const triangle& tri, glm::vec<4, float>(&pos)[3])
		 {
			 pos[0] = tri[0].pos;
			 pos[1] = tri[1].pos;
			 pos[2] = tri[2].pos;
		 }
		 static void GetInvW(const triangle& tri,glm::vec<3,float>& vector)
		 {
			 vector = { 1.f / tri[0].pos.w ,1.f / tri[1].pos.w, 1.f / tri[2].pos.w };
		 }
	private:
		FloatInterpolation<4> color;
		FloatInterpolation<4> position;
	};

	
	

	





	static constexpr std::int32_t windowWidth = 800;
	static constexpr std::int32_t windowHeight= 600;

	static constexpr std::int32_t renderWidth = 320;

	static constexpr std::int32_t renderHeight = 240;

	

	
	


	int main(int argc, char* argv[])
	{
		FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
		FDB::Sdl::Window window("test", { windowWidth,windowHeight }, SDL_WINDOW_RESIZABLE);
		FDB::Sdl::Renderer renderer(window);
		//renderer.SetVSync(1);

		glm::vec<2, std::int32_t> size = { renderWidth,renderHeight };

		FDB::Sdl::Texture texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, size);
		std::vector<std::uint32_t> frameBuffer;
		std::vector<float> zBuffer;
		zBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));

		Vertex a,b,c,d;

		Uint64 lastTime = SDL_GetTicks();
		std::uint32_t frameCount = 0;
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
			std::memset(zBuffer.data(), 0, zBuffer.size() * sizeof(float));

			//TODO vertex gestion
			{
				a.pos.x = 0;
				a.pos.y = static_cast<float>(size.y - 1) ;
				a.pos.w = 1.0f;
				a.color = { 1,0,0,1 };
				b.pos.x = 0;
				b.pos.y = 0;
				b.pos.w = 1.f;
				b.color = { 0,1,0,1 };
				c.pos.x = static_cast<float>(size.x - 1) ;
				c.pos.y = 0;
				c.pos.w = 1.f;
				c.color = { 0,0,1,1 };
				d.pos.x = static_cast<float>(size.x - 1) ;
				d.pos.y = static_cast<float>(size.y - 1) ;
				d.color = { 1,1,1,1 };
			}


			FDB::Rect<std::int32_t> port;

			renderer.Clear(FDB::Color::black8);
			port.x = 0;
			port.y =0;
			port.width = size.x;
			port.height = size.y ;
			
			auto bench = [&](auto fn, const char* name)
				{
					fn(); // échauffement
					Uint64 s = SDL_GetPerformanceCounter();
					for (int i = 0; i < 1000; ++i) fn();
					Uint64 e = SDL_GetPerformanceCounter();
					printf("%s : %.3f ms/appel\n", name,
						double(e - s) * 1000.0 / double(SDL_GetPerformanceFrequency()) / 1000.0);
				};

			/*bench([&] { Rasterizer::ComputeWithouGLM({ a, b, c }, port, frameBuffer.data(),size.x); }, "ComputeWithoutGLM");
			bench([&] { Rasterizer::Compute2({ a, c, b }, port, frameBuffer.data(), size.x); }, "Compute2");*/
			Rasterizer::Compute<Vertex,VertexInterpolation>({ a, c, b }, port, frameBuffer.data(),zBuffer.data(), size.x);

			//Rasterizer::ComputeWithouGLM({ a, b, c }, port, frameBuffer.data(), size.x);


			texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
			renderer.RenderTexture(texture);
			renderer.Present();
		}
		FDB::Sdl::Application::Destroy();
	}




