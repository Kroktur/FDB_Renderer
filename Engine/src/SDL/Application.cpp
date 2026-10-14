#include "SDL/Application.h"

namespace FDB::Sdl
{


	Application::Application(SDL_InitFlags flags)
	{
		if (!SDL_Init(flags))
			throw std::logic_error("SDL_failed");
	}

	void Application::AddSDLListener(SDL_EventFilter filter, void* userData)
	{
		if (!SDL_AddEventWatch(filter, userData))
			throw std::logic_error("ADD listener failed");
	}

	void Application::RemoveSDLListener(SDL_EventFilter filter, void* userData)
	{
		SDL_RemoveEventWatch(filter, userData);
	}

	Application::~Application()
	{
		SDL_Quit();
	}
}