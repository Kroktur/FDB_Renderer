#ifndef FDB_SDL_APPLICATIONF
#define FDB_SDL_APPLICATIONF

#include <stdexcept>

#include "Event.h"
#include "Core/EventBus.h"
#include "SDL/sdl.h"

namespace FDB::Sdl
{
	struct Application
	{
		Application() = delete;
		~Application() = delete;
		static void Init(SDL_InitFlags flags);

		static void Destroy();

		template<void(*fn)(SDL_Event*) >
		static void PollEvent()
		{
			if (!m_init)
				throw std::logic_error("SDL_not Init");
			while (SDL_PollEvent(&m_event))
			{
				fn(&m_event);
			}
		}
		template<typename T, void(T::* fn)(SDL_Event*)>
		static void PollEvent(T* obj)
		{
			if (!m_init)
				throw std::logic_error("SDL_not Init");
			while (SDL_PollEvent(&m_event))
			{
				if (obj)
					(obj->*fn)(&m_event);
			}
		}
		template<typename fn> requires(std::is_invocable_v<fn, SDL_Event*>)
			static void PollEvent(fn* obj)
		{
			if (!m_init)
				throw std::logic_error("SDL_not Init");
			while (SDL_PollEvent(&m_event))
			{
				if (obj)
					obj(&m_event);
			}
		}
		static void AddSDLListener(SDL_EventFilter filter, void* userData);
		static void RemoveSDLListener(SDL_EventFilter filter, void* userData);
		static  bool ShouldQuit();

	private:
		
		static void CallBack(const Event::Quit& quit);
		static SDL_Event m_event;
		static bool m_init;
		static Core::EventBus<Event::Quit>::BusHandle m_handle;
		static bool m_shouldQuit;
	};

	inline SDL_Event Application::m_event = {};
	inline bool Application::m_init = false;
	inline bool Application::m_shouldQuit = false;
	inline  Core::EventBus<Event::Quit>::BusHandle Application::m_handle = {};
	
}

#endif
