#include <iostream>
#include <stdexcept>
#include <string>

#include "GLM/vec4.hpp"
#include "GLM/vec3.hpp"


#include "GLM/mat4x4.hpp"


#include <GLM/gtc/matrix_transform.hpp>

#include "SDL/Event.h"


struct Vertex
{
	glm::vec4 pos;
	glm::vec<4,float> color;
};

void SetPixel(
	int x,
	int y,
	const glm::vec<4, std::uint8_t>& color,
	std::vector<glm::vec<4, std::uint8_t>>* frameBuffer,
	glm::ivec2 size)
{
	const int imageY = size.y - 1 - y;
	frameBuffer->at(imageY * size.x + x)= color;
}

// https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
inline static float edge_function(float Ax,float Ay,float Bx,float By,int Px, int Py) 
{
	float dxi = Bx - Ax;
	float dyi = By - Ay;
	return(static_cast<float>(Px) - Ax) * dyi - (static_cast<float>(Py) - Ay) * dxi;
}


inline void Rasterizer(Vertex a,Vertex b,Vertex c,std::vector<glm::vec<4,std::uint8_t>>* frameBuffer,glm::vec<2,std::int32_t> size)
{
	// find bounding box
	int minX = std::floor(std::min(a.pos.x, std::min(b.pos.x, c.pos.x)));
	int maxX = std::ceil(std::max(a.pos.x, std::max(b.pos.x, c.pos.x)));

	int minY = std::floor(std::min(a.pos.y, std::min(b.pos.y, c.pos.y)));
	int maxY = std::ceil(std::max(a.pos.y, std::max(b.pos.y, c.pos.y)));

	// initialise edge functions 
	float w0_row = edge_function(a.pos.x, a.pos.y, b.pos.x, b.pos.y, minX, minY);
	float w1_row = edge_function(b.pos.x, b.pos.y, c.pos.x, c.pos.y, minX, minY);
	float w2_row = edge_function(c.pos.x, c.pos.y, a.pos.x, a.pos.y, minX, minY);

	float delta_w0_x = b.pos.x - a.pos.x;
	float delta_w1_x = c.pos.x - b.pos.x;
	float delta_w2_x = a.pos.x - c.pos.x;

	float delta_w0_y = b.pos.y - a.pos.y;
	float delta_w1_y = c.pos.y - b.pos.y;
	float delta_w2_y = a.pos.y - c.pos.y;

	float w0_row_multi = edge_function(a.pos.x, a.pos.y, b.pos.x, b.pos.y, minX, minY);
	float w1_row_multi = edge_function(b.pos.x, b.pos.y, c.pos.x, c.pos.y, minX, minY);
	float w2_row_multi = edge_function(c.pos.x, c.pos.y, a.pos.x, a.pos.y, minX, minY);

	float area = edge_function(
		a.pos.x, a.pos.y,
		b.pos.x, b.pos.y,
		static_cast<int>(c.pos.x),
		static_cast<int>(c.pos.y));


	for (int pixX = minX; pixX <= maxX ; ++pixX)
	{
		// iterative version
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;
		//
		for (int pixY = minY ; pixY  <= maxY ; ++pixY)
		{
			// multi thread version
			float w0_multithread = w0_row_multi - (static_cast<float>(pixY - minY) * delta_w0_x) + (static_cast<float>(pixX - minX) * delta_w0_y);
			float w1_multithread = w1_row_multi - (static_cast<float>(pixY - minY) * delta_w1_x) + (static_cast<float>(pixX - minX) * delta_w1_y);
			float w2_multithread = w2_row_multi - (static_cast<float>(pixY - minY) * delta_w2_x) + (static_cast<float>(pixX - minX) * delta_w2_y);

			if (w0 <=0 && w1 <= 0 && w2 <= 0)
			{
				float w0_normal = w0 / area;
				float w1_normal = w1 / area;
				float w2_normal = w2 / area;

				glm::vec<4, float> color = w0_normal * c.color + w1_normal * a.color + w2_normal * b.color;

				SetPixel(pixX, pixY, { color.w * 255,color.z * 255,color.y  * 255,color.x * 255 }, frameBuffer, size);
			}
			// iterative version
			w0 -= delta_w0_x;
			w1 -= delta_w1_x;
			w2 -= delta_w2_x;
			//
		}
		// iterative version
		w0_row += delta_w0_y;
		w1_row += delta_w1_y;
		w2_row += delta_w2_y;
		//
	}
}


#include "Core/EventBus.h"
#include "SDL/Application.h"
#include <SDL3/SDL.h>
#include "SDL/Window.h"









int main(int argc, char* argv[])
{
	FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
	FDB::Sdl::Window window("test", { 320, 240	  }, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window.GetSdlWindow(), nullptr);
	if (!renderer)
	{
		std::cout << SDL_GetError() << '\n';
		throw std::logic_error("Renderer failed");
	}
	glm::vec<2,std::int32_t> size = window.GetPixelSize();

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, size.x, size.y);
	if (!texture)
		throw std::logic_error("Texture failed");

	std::vector<glm::vec<4,std::uint8_t>> frameBuffer;
	frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));

	auto lambda = [&frameBuffer, &renderer, &texture,&size,&window](const FDB::Sdl::Event::WindowResizePixel resize)
		{
			if (resize.id != window.GetId())
				return;
			size = resize.pixels;
			frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
			SDL_DestroyTexture(texture);
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, size.x, size.y);
		};
	auto handle = FDB::Core::EventBus<FDB::Sdl::Event::WindowResizePixel>::SubScribe(&lambda);
	Vertex a;

	Vertex b;

	Vertex c;
	

	Uint64 lastTime = SDL_GetTicks();
	std::uint32_t frameCount = 0;

	while (!FDB::Sdl::Application::ShouldQuit() && !window.ShouldClose())
	{
		FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();

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


		//clear
		std::fill(frameBuffer.begin(), frameBuffer.end(), glm::vec<4, std::uint8_t>{255, 0, 0, 0});

		a.pos.x = static_cast<float>(size.x - 1) / 2;
		a.pos.y = static_cast<float>(size.y - 1);
		a.color = { 1,0,0,1 };
		b.pos.x = 0;
		b.pos.y = 0;
		b.color = { 0,1,0,1 };
		c.pos.x = static_cast<float>(size.x - 1);
		c.pos.y = 0;
		c.color = { 0,0,1,1 };
		Rasterizer(a, b, c, &frameBuffer, size);
		SDL_UpdateTexture(texture,nullptr,frameBuffer.data(),size.x * static_cast<int>(sizeof(glm::vec<4, std::uint8_t>)));
		SDL_RenderTexture(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}
	FDB::Core::EventBus<FDB::Sdl::Event::WindowResizePixel>::UnSubscribe(handle);
	FDB::Sdl::Application::Destroy();

}




