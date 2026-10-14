#ifndef FDB_SDL_WINDOWF
#define FDB_SDL_WINDOWF

#include <stdexcept>
#include <GLM/detail/type_vec2.hpp>
#include "SDL/sdl.h"

namespace FDB::Sdl
{
	enum class WindowMode : std::uint8_t
	{
		Windowed,
		Borderless,
		Fullscreen
	};

	struct Window
	{

		Window(const char* title, glm::vec<2, std::uint32_t> size, SDL_WindowFlags flags);
		Window(const Window&) = delete;
		Window(Window&& other) noexcept;

		~Window();
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&& other) noexcept;

		void SetPosition(glm::vec<2, std::int32_t> pos) const;

		[[nodiscard]] glm::vec<2, std::int32_t> GetPosition() const;

		void SetSize(glm::vec<2, std::int32_t> pos) const;

		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const;

		[[nodiscard]] float GetAspectRatio() const;

		void SetTitle(const char* title) const;

		[[nodiscard]] const char* GetTitle() const;

		[[nodiscard]] bool HasWindowFlag(SDL_WindowFlags flags) const;

		[[nodiscard]] SDL_WindowFlags GetWindowFlags() const;

		void SetMode(WindowMode mode) const;

		[[nodiscard]] WindowMode GetMode() const;

		template<typename FnType> requires(std::is_invocable_v<FnType, SDL_Window*>)
			[[nodiscard]] std::invoke_result_t<FnType, SDL_Window*> UnsafeExecuteOnSDLWindow(FnType&& fn)
		{
			return std::forward<FnType>(fn)(m_window);
		}

		template<auto(*fn)(SDL_Window*)>
		[[nodiscard]] std::invoke_result_t<decltype(fn), SDL_Window*> UnsafeExecuteOnSDLWindow()
		{
			return fn(m_window);
		}

		void CloseWindow();

		[[nodiscard]] bool IsValid() const;

		[[nodiscard]] operator bool() const;

		[[nodiscard]] SDL_WindowID GetId() const;
	private:

		SDL_Window* m_window;
	};
}

#endif
