#include "SDL/Application.h"

namespace FDB::Sdl
{
	void Application::Init(SDL_InitFlags flags)
	{
		if (!SDL_Init(flags))
			throw std::logic_error("SDL_failed");
		m_handle = Pipeline::EventBus<Event::Quit>::SubScribe<&Application::CallBack>();
		m_init = true;
	}

	void Application::Destroy()
	{
		SDL_Quit();
		m_handle = Pipeline::EventBus<Event::Quit>::UnSubscribe(m_handle);
		m_init = false;
	}

	void Application::AddSDLListener(SDL_EventFilter filter, void* userData)
	{
		if (!m_init)
			throw std::logic_error("SDL_not Init");
		if (!SDL_AddEventWatch(filter, userData))
			throw std::logic_error("ADD listener failed");
	}

	void Application::RemoveSDLListener(SDL_EventFilter filter, void* userData)
	{
		if (!m_init)
			throw std::logic_error("SDL_not Init");
		SDL_RemoveEventWatch(filter, userData);
	}

	bool Application::ShouldQuit()
	{
		return m_shouldQuit;
	}

	void Application::CallBack(const Event::Quit& quit)
	{
		m_shouldQuit = true;
	}
}
