#ifndef FDB_SDL_EVENTF
#define FDB_SDL_EVENTF

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include "Core/EventBus.h"
#include "GLM/vec2.hpp"

namespace FDB::Sdl::Event
{

	struct Quit {};
	struct WindowClose { SDL_WindowID id;};
	struct WindowResize { SDL_WindowID id; glm::vec<2, std::int32_t> size; };
	struct WindowResizePixel { SDL_WindowID id; glm::vec<2, std::int32_t> pixels; };
	struct WindowMinimize { SDL_WindowID id; bool minimize; };
	struct WindowFocus { SDL_WindowID id; bool focus;};

	struct DefaultManager
	{
		static void Process(SDL_Event* event)
		{
			switch (event->type)
			{
			case SDL_EVENT_QUIT:
				Pipeline::EventBus<Quit>::Notify(Quit{});
				break;
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				Pipeline::EventBus<WindowClose>::Notify(WindowClose{ event->window.windowID });
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				Pipeline::EventBus<WindowResize>::Notify(WindowResize{ event->window.windowID ,{event->window.data1,event->window.data2} });
				break;
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				Pipeline::EventBus<WindowResizePixel>::Notify(WindowResizePixel{ event->window.windowID ,{event->window.data1,event->window.data2} });
				break;
			case SDL_EVENT_WINDOW_MINIMIZED:
				Pipeline::EventBus<WindowMinimize>::Notify(WindowMinimize{ event->window.windowID,true });
				break;
			case SDL_EVENT_WINDOW_RESTORED:
				Pipeline::EventBus<WindowMinimize>::Notify(WindowMinimize{ event->window.windowID,false });
				break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				Pipeline::EventBus<WindowFocus>::Notify(WindowFocus{ event->window.windowID,true });
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				Pipeline::EventBus<WindowFocus>::Notify(WindowFocus{ event->window.windowID,false });
				break;
			default:
				break;
			}
		}
	};
}
#endif

