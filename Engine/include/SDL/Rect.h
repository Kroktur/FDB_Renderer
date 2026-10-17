#ifndef FDB_SDL_RECTF
#define FDB_SDL_RECTF

#include <SDL3/SDL_rect.h>

#include "Core/Rect.h"

namespace FDB::Sdl
{
		inline static constexpr SDL_Rect RectToSDLRect(const Rect<std::int32_t>& port)
		{
			return SDL_Rect{ port.x,port.y,port.width,port.height };
		}
		inline static constexpr Rect<std::int32_t> SDLRectToRect(const SDL_Rect& port)
		{
			return  Rect<std::int32_t>{ port.x, port.y, port.w, port.h };
		}

		inline static constexpr SDL_FRect FRectToSDLFRect(const Rect<float>& port)
		{
			return SDL_FRect{ port.x,port.y,port.width,port.height };
		}
		inline static constexpr Rect<float> SDLFRectToFRect(const SDL_FRect& port)
		{
			return  Rect<float>{ port.x, port.y, port.w, port.h };
		}
}
#endif

