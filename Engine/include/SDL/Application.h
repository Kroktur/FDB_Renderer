#ifndef FDB_SDL_APPLICATIONF
#define FDB_SDL_APPLICATIONF

#include <stdexcept>
#include "SDL/sdl.h"

namespace FDB::Sdl
{
	struct Application
	{
		Application(SDL_InitFlags flags);
		~Application();
		template<void(*fn)(SDL_Event*) >
		static void PollEvent()
		{
			while (SDL_PollEvent(&m_event))
			{
				fn(&m_event);
			}
		}
		template<typename T, void(T::* fn)(SDL_Event*)>
		static void PollEvent(T* obj)
		{
			while (SDL_PollEvent(&m_event))
			{
				if (obj)
					(obj->*fn)(&m_event);
			}
		}
		template<typename fn> requires(std::is_invocable_v<fn, SDL_Event*>)
			static void PollEvent(fn* obj)
		{
			while (SDL_PollEvent(&m_event))
			{
				if (obj)
					obj(&m_event);
			}
		}

		static void AddSDLListener(SDL_EventFilter filter, void* userData);
		static void RemoveSDLListener(SDL_EventFilter filter, void* userData);
	private:
		static SDL_Event m_event;
	};

	inline SDL_Event Application::m_event = {};
}

#endif
