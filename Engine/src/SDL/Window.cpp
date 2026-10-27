#include "SDL/Window.h"
#include "SDL3/SDL_video.h"
namespace FDB::Sdl
{


	Window::Window(const char* title, glm::vec<2, std::uint32_t> size, SDL_WindowFlags flags) : m_window(nullptr), m_shouldClose(false)
	{
		m_window = SDL_CreateWindow(title, static_cast<int>(size.x), static_cast<int>(size.y), flags);
		m_handle = Core::EventBus<Event::WindowClose>::SubScribe<Window,&Window::CallBack>(this);
	}



	Window::Window(Window&& other) noexcept : m_window(other.m_window),m_shouldClose(other.m_shouldClose),m_handle(other.m_handle)
	{
		other.m_window = nullptr;
		other.m_shouldClose = false;
		m_handle = Core::EventBus<Event::WindowClose>::SubScribe<Window,&Window::CallBack>(this);
		Core::EventBus<Event::WindowClose>::UnSubscribe(other.m_handle);
		other.m_handle = {};
	}

	Window::~Window()
	{
		this->CloseWindow();
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this == &other)
			return *this;

		CloseWindow();
		
		m_window = other.m_window;
		other.m_window = nullptr;
		m_shouldClose = other.m_shouldClose;
		other.m_shouldClose = false;
		m_handle = Core::EventBus<Event::WindowClose>::SubScribe<Window,&Window::CallBack>(this);
		Core::EventBus<Event::WindowClose>::UnSubscribe(other.m_handle);
		other.m_handle = {};

		return *this;
	}

	void Window::SetPosition(glm::vec<2, std::int32_t> pos) const
	{
		if (!SDL_SetWindowPosition(m_window, static_cast<int>(pos.x), static_cast<int>(pos.y)))
			throw std::logic_error("SetPosition failed");
	}

	glm::vec<2, std::int32_t> Window::GetPosition() const
	{
		int x, y;
		if (!SDL_GetWindowPosition(m_window, &x, &y))
			throw std::logic_error("GetPosition failed");
		return glm::vec<2, std::int32_t>{static_cast<std::int32_t>(x), static_cast<std::int32_t>(y)};
	}

	void Window::SetSize(glm::vec<2, std::int32_t> pos) const
	{
		if (!SDL_SetWindowSize(m_window, static_cast<int>(pos.x), static_cast<int>(pos.y)))
			throw std::logic_error("SetSize failed");
	}

	glm::vec<2, std::int32_t> Window::GetSize() const
	{
		int x, y;
		if (!SDL_GetWindowSize(m_window, &x, &y))
			throw std::logic_error("GetSize failed");
		return glm::vec<2, std::int32_t>{static_cast<std::int32_t>(x), static_cast<std::int32_t>(y)};
	}

	glm::vec<2, std::int32_t> Window::GetPixelSize() const
	{
		int x, y;
		if (!SDL_GetWindowSizeInPixels(m_window, &x, &y))
			throw std::logic_error("GetSize failed");
		return glm::vec<2, std::int32_t>{static_cast<std::int32_t>(x), static_cast<std::int32_t>(y)};
	}

	float Window::GetAspectRatio() const
	{
		glm::vec<2, std::uint32_t> size = this->GetSize();
		if (size.y == 0)
			return 0;
		return static_cast<float>(size.x) / static_cast<float>(size.y);
	}

	void Window::SetTitle(const char* title) const
	{
		if (!SDL_SetWindowTitle(m_window, title))
			throw std::logic_error("SetTitle failed");
	}

	const char* Window::GetTitle() const
	{
		const char* title = SDL_GetWindowTitle(m_window);
		if (!title)
			throw std::logic_error("SetTitle failed");
		return title;
	}

	bool Window::HasWindowFlag(SDL_WindowFlags flags) const
	{
		return (this->GetWindowFlags() & flags) != 0;
	}

	SDL_WindowFlags Window::GetWindowFlags() const
	{
		return SDL_GetWindowFlags(m_window);
	}

	void Window::SetMode(WindowMode mode) const
	{
		switch (mode)
		{
		case WindowMode::Fullscreen:
			if (!SDL_SetWindowFullscreen(m_window, true))
				throw std::logic_error("FullScreen failed");
			break;
		case WindowMode::Borderless:
			if (!SDL_SetWindowFullscreen(m_window, false))
				throw std::logic_error("Borderless failed");
			if (!SDL_SetWindowBordered(m_window, false))
				throw std::logic_error("Borderless failed");
			break;
		case WindowMode::Windowed:
			if (!SDL_SetWindowFullscreen(m_window, false))
				throw std::logic_error("Windowed failed");
			if (!SDL_SetWindowBordered(m_window, true))
				throw std::logic_error("Windowed failed");
			break;
		}
	}

	WindowMode Window::GetMode() const
	{
		SDL_WindowFlags flag = this->GetWindowFlags();
		if ((flag & SDL_WINDOW_FULLSCREEN) != 0)
			return WindowMode::Fullscreen;
		if ((flag & SDL_WINDOW_BORDERLESS) != 0)
			return WindowMode::Borderless;
		return WindowMode::Windowed;
	}






	void Window::CloseWindow()
	{
		if (!m_window)
			return;
		Core::EventBus<Event::WindowClose>::UnSubscribe(m_handle);
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}

	bool Window::IsValid() const
	{
		return m_window != nullptr;
	}

	Window::operator bool() const
	{
		return m_window != nullptr;
	}

	SDL_WindowID Window::GetId() const
	{
		return SDL_GetWindowID(m_window);
	}

	bool Window::ShouldClose() const
	{
		return m_shouldClose;
	}

	SDL_Window* Window::GetSdlWindow()
	{
		return m_window;
	}

	void Window::CallBack(const Event::WindowClose& event)
	{
		if (event.id == this->GetId())
			m_shouldClose = true;
	}
}
