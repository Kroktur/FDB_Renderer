#include "SDL/Texture.h"
#include <stdexcept>
#include "SDL/Renderer.h"
#include "SDL/Rect.h"

namespace FDB::Sdl
{
	Texture::Texture(Renderer& renderer, SDL_PixelFormat format, SDL_TextureAccess access,
		glm::vec<2, std::int32_t> size): m_texture(nullptr), m_size(size)
	{
		m_texture = SDL_CreateTexture(renderer.GetSdlRenderer(), format, access, size.x, size.y);
		if (!m_texture)
		{
			throw std::logic_error("Texture failed");
		}
	}

	Texture::Texture(Texture&& other) noexcept: m_texture(other.m_texture), m_size(other.m_size)
	{
		other.m_size = {};
		other.m_texture = nullptr;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->Destroy();
		m_texture = other.m_texture;
		other.m_texture = nullptr;
		m_size = other.m_size;
		other.m_size = {};
		return *this;
	}

	Texture::~Texture()
	{
		this->Destroy();
	}

	void Texture::Update(const void* pixel, std::int32_t pitch) const
	{
		SDL_UpdateTexture(m_texture, nullptr, pixel, pitch);
	}

	void Texture::Update(const void* pixel, std::int32_t pitch, const Rect<std::int32_t>& rect) const
	{
		SDL_Rect rectSDL = RectToSDLRect(rect);
		SDL_UpdateTexture(m_texture, &rectSDL, pixel, pitch);
	}

	void Texture::Destroy()
	{
		if (!m_texture)
			return;
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
		m_size = {};
	}

	glm::vec<2, std::int32_t> Texture::GetSize() const
	{
		return m_size;
	}

	SDL_Texture* Texture::GetSdlTexture() const
	{
		return m_texture;
	}

	bool Texture::IsValid() const
	{
		return m_texture != nullptr;
	}

	Texture::operator bool() const
	{
		return m_texture != nullptr;
	}
}
