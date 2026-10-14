#include <iostream>
#include <stdexcept>

#include "GLM/vec4.hpp"
#include "GLM/vec3.hpp"


#include "GLM/mat4x4.hpp"


#include <GLM/gtc/matrix_transform.hpp>

#include "SDL/Event.h"


struct Vertex
{
	glm::vec4 pos;
	glm::vec3 color;
};



// https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
inline static float edge_function(float Ax,float Ay,float Bx,float By,int Px, int Py) 
{
	float dxi = Bx - Ax;
	float dyi = By - Ay;
	return(static_cast<float>(Px) - Bx) * dyi - (static_cast<float>(Py) - By) * dxi;
}


inline void Rasterizer(Vertex a,Vertex b,Vertex c)
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

	float delta_w0_row = b.pos.x - a.pos.x;
	float delta_w1_row = c.pos.x - b.pos.x;
	float delta_w2_row = a.pos.x - c.pos.x;

	float delta_w0_col = b.pos.y - a.pos.y;
	float delta_w1_col = c.pos.y - b.pos.y;
	float delta_w2_col = a.pos.y - c.pos.y;
	
	for (int pixX = minX; pixX < maxX ; ++pixX)
	{
		// iterative version
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;
		//
		for (int pixY = minY ; pixY  < maxY ; ++pixY)
		{
			// multi thread version
			//float w0_multithread = w0_row - (static_cast<float>(pixY - minY) * delta_w0_row) + (static_cast<float>(pixX - minX) * delta_w0_col);
			//float w1_multithread = w1_row - (static_cast<float>(pixY - minY) * delta_w1_row) + (static_cast<float>(pixX - minX) * delta_w1_col);
			//float w2_multithread = w2_row - (static_cast<float>(pixY - minY) * delta_w2_row) + (static_cast<float>(pixX - minX) * delta_w2_col);
			//

			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				// dans le triangle
			}
			// iterative version
			w0 += delta_w0_col;
			w1 += delta_w1_col;
			w2 += delta_w2_col;
			//
		}
		// iterative version
		w0_row -= delta_w0_row;
		w1_row -= delta_w1_row;
		w2_row -= delta_w2_row;
		//
	}
}


#include "Core/EventBus.h"
#include "SDL/Application.h"
#include <SDL3/SDL.h>
#include "SDL/Window.h"









int main(int argc, char* argv[])
{
	FDB::Sdl::Application application(SDL_INIT_VIDEO);
	FDB::Sdl::Window window("test", { 1920, 1080 }, SDL_WINDOW_RESIZABLE);

	bool running = true;
	auto lambda = [&window,&running](const FDB::Sdl::Event::WindowClose& event)
	{
			if (event.id == window.GetId())
				window.CloseWindow();
	};
	auto lambda2 = [ &running](const FDB::Sdl::Event::Quit& event)
		{
				running = false;
		};
	auto handle = FDB::Core::EventBus<FDB::Sdl::Event::WindowClose>::SubScribe(&lambda);
	auto handle2 = FDB::Core::EventBus<FDB::Sdl::Event::Quit>::SubScribe(&lambda2);
	while (running)
	{
		FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();
	}
	FDB::Core::EventBus<FDB::Sdl::Event::WindowClose>::UnSubscribe(handle);
	FDB::Core::EventBus<FDB::Sdl::Event::Quit>::UnSubscribe(handle2);


}




