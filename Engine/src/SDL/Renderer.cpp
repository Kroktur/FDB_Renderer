#include "SDL/Renderer.h"

#include "SDL/Window.h"
#include "SDL/Texture.h"
namespace FDB::Sdl
{
	Renderer::Renderer(Window& window, const char* name, const Color::RGBA8888& clearColor): m_renderer(nullptr), m_clearColor(clearColor)
	{
		m_renderer = SDL_CreateRenderer(window.GetSdlWindow(), name);
		if (!m_renderer)
		{
			throw std::logic_error("Renderer failed");
		}
		SDL_SetRenderDrawColor(m_renderer, m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		SDL_RenderClear(m_renderer);
	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		if (this == &other)
			return *this;
		this->Destroy();
		m_renderer = other.m_renderer;
		other.m_renderer = nullptr;
		return *this;
	}

	Renderer::Renderer(Renderer&& other) noexcept: m_renderer(other.m_renderer)
	{
		other.m_renderer = nullptr;
	}

	Renderer::~Renderer()
	{
		this->Destroy();
	}

	void Renderer::Destroy()
	{
		if (!m_renderer)
			return;
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	void Renderer::RenderTexture(Texture& texture, const Rect<float>& srcRect, const Rect<float> dstRect) const
	{
		SDL_FRect srcRectSDL = FRectToSDLFRect(srcRect);
		SDL_FRect dstRectSDL = FRectToSDLFRect(dstRect);
		SDL_RenderTexture(m_renderer, texture.GetSdlTexture(), &srcRectSDL, &dstRectSDL);
	}

	void Renderer::RenderTexture(Texture& texture) const
	{
		SDL_RenderTexture(m_renderer, texture.GetSdlTexture(), nullptr, nullptr);
	}

	void Renderer::RenderTextureReg(Texture& texture, const Rect<float>& srcRect) const
	{
		SDL_FRect srcRectSDL = FRectToSDLFRect(srcRect);
		SDL_RenderTexture(m_renderer, texture.GetSdlTexture(), &srcRectSDL, nullptr);
	}

	void Renderer::RenderTextureTo(Texture& texture, const Rect<float>& dstRect) const
	{
		SDL_FRect dstRectSDL = FRectToSDLFRect(dstRect);
		SDL_RenderTexture(m_renderer, texture.GetSdlTexture(), nullptr, &dstRectSDL);
	}

	void Renderer::Clear(const Color::RGBA8888& color)
	{
		if (m_clearColor != color)
		{
			m_clearColor = color;
			SDL_SetRenderDrawColor(m_renderer, m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);

		}
		SDL_RenderClear(m_renderer);
	}

	void Renderer::Present() const
	{
		SDL_RenderPresent(m_renderer);
	}

	void Renderer::SetVSync(int vSync) const
	{
		SDL_SetRenderVSync(m_renderer, vSync);
	}

	int Renderer::GetVSync() const
	{
		int vSync = 0;
		SDL_GetRenderVSync(m_renderer, &vSync);
		return vSync;
	}

	void Renderer::SetViewPort(const Rect<std::int32_t>& viewPort) const
	{
		SDL_Rect rect = RectToSDLRect(viewPort);
		SDL_SetRenderViewport(m_renderer, &rect);
	}

	void Renderer::ResetViewPort() const
	{
		SDL_SetRenderViewport(m_renderer, nullptr);
	}

	Rect<std::int32_t> Renderer::GetViewPort() const
	{
		SDL_Rect rect{};
		SDL_GetRenderViewport(m_renderer, &rect);
		return SDLRectToRect(rect);
	}

	SDL_Renderer* Renderer::GetSdlRenderer() const
	{
		return m_renderer;
	}

	bool Renderer::IsValid() const
	{
		return m_renderer != nullptr;
	}

	Renderer::operator bool() const
	{
		return m_renderer != nullptr;
	}
}
