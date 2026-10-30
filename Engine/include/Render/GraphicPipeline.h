#ifndef FDBRENDERGRAPHICPIPELINEF
#define FDBRENDERGRAPHICPIPELINEF
#include "Pipeline/GraphicPipeline.h"
#include "SDL/Renderer.h"
#include "SDL/Window.h"
#include "SDL/Texture.h"

namespace FDB::Render
{
	enum class ResolutionMode : std::uint8_t
	{
		Manual,
		Adaptative
	};

	struct SDLGraphicPipeline
	{
		SDLGraphicPipeline(FDB::Sdl::Window* windowPtr, const glm::vec<2, std::int32_t>& resolution);

		SDLGraphicPipeline(FDB::Sdl::Window* windowPtr);

		~SDLGraphicPipeline();

		void ClearColor(const Color::RGBA8888& color);

		void BeginRender(const FDB::Rect<std::int32_t>& viewPort, const Color::RGBAFloat& color = Color::defaultFloat, float z = 1.f);

		void BeginRender(const Color::RGBAFloat& color = Color::defaultFloat, float z = 1.f);

		void PresentRender();

		void SetRenderResolution(const glm::vec<2, std::int32_t>& resolution);

		void SetResolutionMode(ResolutionMode mode);

		[[nodiscard]] ResolutionMode GetResolutionMode() const;

		[[nodiscard]] FDB::Rect<std::int32_t> GetViewPort() const;

		[[nodiscard]] glm::vec<2, std::int32_t> GetResolution() const;

		[[nodiscard]] Pipeline::FrameBuffer GetFrameBuffer() const;


		FDB::Sdl::Renderer* GetRenderer()
		{
			return &m_renderer;
		}
	private:
		void PrivateBegin(const Color::RGBAFloat& color, float z);

		void SyncResolution();

		void CallBack(const Sdl::Event::WindowResize& event);
		FDB::Sdl::Window* m_windowPtr = nullptr;
		FDB::Pipeline::GraphicsPipeline m_pipeline = {};
		FDB::Sdl::Texture m_texture = {};
		FDB::Sdl::Renderer m_renderer = {};
		FDB::Rect<std::int32_t> m_viewPort = {};
		Pipeline::FrameBuffer m_frameBuffer = {};


		ResolutionMode m_mode;
		glm::vec<2, std::int32_t> m_currentResolution = {};
		glm::vec<2, std::int32_t> m_nextResolution = {};
		Pipeline::EventBus<Sdl::Event::WindowResize>::BusHandle m_handle = {};
		bool m_dirtyResolution = false;
	};
}


#endif
