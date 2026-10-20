#include <string>

#include "Core/Rect.h"
#include "GLM/vec4.hpp"
#include "GLM/vec3.hpp"
#include "SDL/Application.h"
#include "SDL/Renderer.h"
#include "SDL/Window.h"
#include "SDL/Texture.h"
#include "SDL3/SDL_timer.h"
#include "../header_file/Pipeline.h"
#include "../header_file/Transform.h"



struct Rasterizer
{
	static void Compute(std::vector<VertexProcessing::ProcessedTriangle> triangles,const FDB::Rect<std::int32_t>& viewPort,std::uint32_t* buffer)
	{
		auto a = triangles[0][0];
		auto b = triangles[0][1];
		auto c = triangles[0][2];
		// find bounding box
		int minX = std::max(viewPort.x, static_cast<int>(std::min({ a.pos.x, b.pos.x, c.pos.x })));
		int maxX = std::min(viewPort.x + viewPort.width - 1, static_cast<int>(std::max({ a.pos.x, b.pos.x, c.pos.x })));
		int minY = std::max(viewPort.y, static_cast<int>(std::min({ a.pos.y, b.pos.y, c.pos.y })));
		int maxY = std::min(viewPort.y + viewPort.height - 1, static_cast<int>(std::max({ a.pos.y, b.pos.y, c.pos.y })));

		// initialise edge functions 
		float w0_row = Rasterizer::EdgeFunction(a.pos.x, a.pos.y, b.pos.x, b.pos.y, static_cast<float>(minX), static_cast<float>(minY) );
		float w1_row = Rasterizer::EdgeFunction(b.pos.x, b.pos.y, c.pos.x, c.pos.y, static_cast<float>(minX) , static_cast<float>(minY) );
		float w2_row = Rasterizer::EdgeFunction(c.pos.x, c.pos.y, a.pos.x, a.pos.y, static_cast<float>(minX) , static_cast<float>(minY) );

		float delta_w0_x = b.pos.x - a.pos.x;
		float delta_w1_x = c.pos.x - b.pos.x;
		float delta_w2_x = a.pos.x - c.pos.x;

		float delta_w0_y = b.pos.y - a.pos.y;
		float delta_w1_y = c.pos.y - b.pos.y;
		float delta_w2_y = a.pos.y - c.pos.y;

		//float w0_row_multi = Rasterizer::EdgeFunction(a.pos.x, a.pos.y, b.pos.x, b.pos.y, static_cast<float>(minX) , static_cast<float>(minY) );
		//float w1_row_multi = Rasterizer::EdgeFunction(b.pos.x, b.pos.y, c.pos.x, c.pos.y, static_cast<float>(minX) , static_cast<float>(minY) );
		//float w2_row_multi = Rasterizer::EdgeFunction(c.pos.x, c.pos.y, a.pos.x, a.pos.y, static_cast<float>(minX) , static_cast<float>(minY) );

		float area = Rasterizer::EdgeFunction(
			a.pos.x, a.pos.y,
			b.pos.x, b.pos.y,
			c.pos.x,
			c.pos.y);


		float invArea = 1.f / area;

		float Ax = a.color.x * 255.f * invArea;
		float Ay = a.color.y * 255.f * invArea;
		float Az = a.color.z * 255.f * invArea;
		float Aw = a.color.w * 255.f * invArea;
		float Bx = b.color.x * 255.f * invArea;
		float By = b.color.y * 255.f * invArea;
		float Bz = b.color.z * 255.f * invArea;
		float Bw = b.color.w * 255.f * invArea;
		float Cx = c.color.x * 255.f * invArea;
		float Cy = c.color.y * 255.f * invArea;
		float Cz = c.color.z * 255.f * invArea;
		float Cw = c.color.w * 255.f *invArea;

	

			for (int pixY = minY; pixY <= maxY; ++pixY)
			{
				// iterative version
				float w0 = w0_row;
				float w1 = w1_row;
				float w2 = w2_row;
				//

				std::uint32_t* pixelPtr = buffer + (pixY * viewPort.width + minX);
				for (int pixX = minX; pixX <= maxX; ++pixX)
				{
					// multi thread version
					//float w0_multithread = w0_row_multi - (static_cast<float>(pixY - minY) * delta_w0_x) + (static_cast<float>(pixX - minX) * delta_w0_y);
					//float w1_multithread = w1_row_multi - (static_cast<float>(pixY - minY) * delta_w1_x) + (static_cast<float>(pixX - minX) * delta_w1_y);
					//float w2_multithread = w2_row_multi - (static_cast<float>(pixY - minY) * delta_w2_x) + (static_cast<float>(pixX - minX) * delta_w2_y);
					if (pixX == 318)
					{
						float toot = 5;
						toot++;
					}
					if (w0 <= 0 && w1 <= 0 && w2 <= 0)
					{
						// multi thread verion
						//float w0_normal = w0_multithread;
						//float w1_normal = w1_multithread ;
						//float w2_normal = w2_multithread ;

						float w0_normal = w0;
						float w1_normal = w1;
						float w2_normal = w2;

					

						//TODO move 
						{
							float rC = w0_normal * Cx + w1_normal * Ax + w2_normal * Bx;
							float gC = w0_normal * Cy + w1_normal * Ay + w2_normal * By;
							float bC = w0_normal * Cz + w1_normal * Az + w2_normal * Bz;
							float aC = w0_normal * Cw + w1_normal * Aw + w2_normal * Bw;

							std::uint32_t r = static_cast<std::uint32_t>(rC);
							std::uint32_t g = static_cast<std::uint32_t>(gC);
							std::uint32_t b = static_cast<std::uint32_t>(bC);
							std::uint32_t a = static_cast<std::uint32_t>(aC);
							*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
						}
					}
					++pixelPtr;
					// iterative version
					w0 += delta_w0_y;
					w1 += delta_w1_y;
					w2 += delta_w2_y;
					//
				}
				// iterative version
				w0_row -= delta_w0_x;
				w1_row -= delta_w1_x;
				w2_row -= delta_w2_x;
				//
			}
		}
	// https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
	static float EdgeFunction(float Ax, float Ay, float Bx, float By, float Px, float Py)
	{
		return (Px - Ax) * (By - Ay) - (Py - Ay) * (Bx - Ax);
	}
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
	frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));

	Transform::Vertex a,b,c,d;
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
		//TODO vertex gestion
		{
			a.pos = glm::vec4(-1.5f, -1.0f, -100.0f, 1.0f);
			b.pos = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			c.pos = glm::vec4(1.5f, -1.0f, 0.0f, 1.0f);
			a.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			b.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			c.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}


		FDB::Rect<std::int32_t> port;
		port.width = size.x;
		port.height = size.y;

		renderer.Clear(FDB::Color::black8);
		auto processedTriangles = Pipeline::createSommetList(a, b, c, port);
		Rasterizer::Compute(processedTriangles, port, frameBuffer.data());
		

		texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
		renderer.RenderTexture(texture);
		renderer.Present();
	}
	FDB::Sdl::Application::Destroy();
	//Pipeline::createSommetList();
	return 0;
}




