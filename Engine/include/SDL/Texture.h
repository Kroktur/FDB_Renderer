#ifndef FDB_SDL_TEXTUREF
#define FDB_SDL_TEXTUREF
#include <GLM/detail/type_vec2.hpp>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include "Core/Rect.h"
#include <SDL3/SDL_render.h>

namespace FDB::Sdl
{
	struct Renderer;

	struct Texture
	{
		Texture(Renderer& renderer, SDL_PixelFormat format, SDL_TextureAccess access, glm::vec<2, std::int32_t> size);
		Texture(const Texture&) = delete;
		Texture(Texture&& other) noexcept;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&& other) noexcept;

		~Texture();

		void Update(const void* pixel, std::int32_t pitch) const;

		void Update(const void* pixel, std::int32_t pitch, const Rect<std::int32_t>& rect) const;

		void Destroy();

		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const;

		template<typename FnType> requires(std::is_invocable_v<FnType, SDL_Texture*>)
			[[nodiscard]] std::invoke_result_t<FnType, SDL_Texture*> UnsafeExecuteOnSDLTexture(FnType&& fn)
		{
			return std::forward<FnType>(fn)(m_texture);
		}

		template<auto(*fn)(SDL_Texture*)>
		[[nodiscard]] std::invoke_result_t<decltype(fn), SDL_Texture*> UnsafeExecuteOnSDLTexture()
		{
			return fn(m_texture);
		}

		[[nodiscard]] SDL_Texture* GetSdlTexture() const;

		[[nodiscard]] bool IsValid() const;

		[[nodiscard]] operator bool() const;

	private:
		SDL_Texture* m_texture;
		glm::vec<2, std::int32_t> m_size;
	};
}
#endif
