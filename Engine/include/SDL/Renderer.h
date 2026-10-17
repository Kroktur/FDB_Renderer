#ifndef FDB_SDL_RENDERERF
#define FDB_SDL_RENDERERF
#include <stdexcept>
#include <SDL3/SDL_render.h>
#include "Rect.h"
#include "Core/Color.h"
#include "Core/Rect.h"

struct SDL_Renderer;

namespace FDB::Sdl
{
	struct Window;
	struct Texture;
	struct Renderer
	{
		Renderer(Window& window, const char* name = nullptr, const Color::RGBA8& clearColor = Color::black8);

		Renderer& operator=(Renderer&& other) noexcept;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&& other)noexcept;
		Renderer(const Renderer&) = delete;
		~Renderer();

		void Destroy();

		void RenderTexture(Texture& texture, const Rect<float>& srcRect, const Rect<float> dstRect) const;

		void RenderTexture(Texture& texture) const;

		void RenderTextureReg(Texture& texture, const Rect<float>& srcRect) const;

		void RenderTextureTo(Texture& texture, const Rect<float>& dstRect) const;

		void Clear(const Color::RGBA8& color);

		void Present() const;


		void SetVSync(int vSync) const;

		[[nodiscard]] int GetVSync() const;

		void SetViewPort(const Rect<std::int32_t>& viewPort) const;

		void ResetViewPort() const;

		[[nodiscard]] Rect<std::int32_t> GetViewPort() const;

		template<typename FnType> requires(std::is_invocable_v<FnType, SDL_Renderer*>)
			[[nodiscard]] std::invoke_result_t<FnType, SDL_Renderer*> UnsafeExecuteOnSDLRenderer(FnType&& fn)
		{
			return std::forward<FnType>(fn)(m_renderer);
		}
		template<auto(*fn)(SDL_Renderer*)>
		[[nodiscard]] std::invoke_result_t<decltype(fn), SDL_Renderer*> UnsafeExecuteOnSDLRenderer()
		{
			return fn(m_renderer);
		}

		[[nodiscard]] SDL_Renderer* GetSdlRenderer() const;

		[[nodiscard]] bool IsValid() const;

		[[nodiscard]] operator bool() const;

	private:
		SDL_Renderer* m_renderer;
		Color::RGBA8 m_clearColor;
	};
}
#endif
