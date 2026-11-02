#include "Render/GraphicPipeline.h"

#include "Core/ThreadPool.h"

namespace FDB::Render
{
	SDLGraphicPipeline::SDLGraphicPipeline(FDB::Sdl::Window* windowPtr, const glm::vec<2, std::int32_t>& resolution):m_windowPtr(windowPtr), m_nextResolution(resolution), m_mode(ResolutionMode::Adaptative)
	{
		m_renderer = FDB::Sdl::Renderer(*windowPtr);
		m_pipeline = FDB::Pipeline::GraphicsPipeline(resolution);
		this->SetResolutionMode(ResolutionMode::Manual);
	}

	SDLGraphicPipeline::SDLGraphicPipeline(FDB::Sdl::Window* windowPtr): m_windowPtr(windowPtr), m_mode(ResolutionMode::Manual)
	{
		m_renderer = FDB::Sdl::Renderer(*windowPtr);
		m_pipeline = FDB::Pipeline::GraphicsPipeline();
		this->SetResolutionMode(ResolutionMode::Adaptative);
	}

	SDLGraphicPipeline::~SDLGraphicPipeline()
	{
		if (m_handle)
			m_handle = Pipeline::EventBus<Sdl::Event::WindowResize>::UnSubscribe(m_handle);
	}

	void SDLGraphicPipeline::ClearColor(const Color::RGBA8888& color)
	{
		m_renderer.Clear(color);
	}

	void SDLGraphicPipeline::BeginRender(const FDB::Rect<std::int32_t>& viewPort, const Color::RGBAFloat& color,
		float z)
	{
		this->PrivateBegin(color, z);
		m_viewPort = viewPort;
	}

	void SDLGraphicPipeline::BeginRender(const Color::RGBAFloat& color, float z)
	{
		this->PrivateBegin(color, z);
		m_viewPort.x = 0;
		m_viewPort.y = 0;
		m_viewPort.width = m_currentResolution.x;
		m_viewPort.height = m_currentResolution.y;
	}

	void SDLGraphicPipeline::PresentRender(Core::jobPool_t& pool)
	{
		static constexpr std::size_t blockSize = (64 * 64);
		auto* aCBuffer = m_frameBuffer.atomicBuffer.data;
		auto* cBuffer = m_frameBuffer.colorBuffer.data;
		std::size_t totSize = m_frameBuffer.atomicBuffer.size.x * m_frameBuffer.atomicBuffer.size.y;

		for (std::size_t i = 0 ; i < totSize ; i += blockSize)
		{
			std::size_t count = std::min(blockSize, totSize - i);
			auto* startCaBuffer = aCBuffer + i;
			auto* startCBuffer = cBuffer + i;
			pool.PushJob([startCaBuffer, startCBuffer, count]()mutable {
				for (std::size_t j = 0; j < count; ++j) {
					Pipeline::Pixel pixel = startCaBuffer->load(std::memory_order_acquire);
					pixel.color *= 255.f;
					const std::uint32_t r = static_cast<std::uint32_t>(pixel.color.r);
					const std::uint32_t g = static_cast<std::uint32_t>(pixel.color.g);
					const std::uint32_t b = static_cast<std::uint32_t>(pixel.color.b);
					const std::uint32_t a = static_cast<std::uint32_t>(pixel.color.a);
					*startCBuffer = r | (g << 8) | (b << 16) | (a << 24);
					++startCaBuffer;
					++startCBuffer;
				}
			});

		}
		pool.WaitIdle();
		m_texture.Update(m_frameBuffer.colorBuffer.data, m_currentResolution.x * static_cast<int>(sizeof(std::uint32_t)));
		m_renderer.RenderTexture(m_texture);
		m_renderer.Present();
	}

	void SDLGraphicPipeline::SetRenderResolution(const glm::vec<2, std::int32_t>& resolution)
	{
		if (m_mode == ResolutionMode::Adaptative)
			return;
		if (m_currentResolution == resolution)
			return;
		m_nextResolution = resolution;
		m_dirtyResolution = true;
	}

	void SDLGraphicPipeline::SetResolutionMode(ResolutionMode mode)
	{
		if (m_mode == mode)
			return;

		m_mode = mode;
		m_dirtyResolution = true;

		if (m_mode == ResolutionMode::Adaptative)
		{
			m_nextResolution = m_windowPtr->GetPixelSize();
			m_handle = Pipeline::EventBus<Sdl::Event::WindowResize>::SubScribe<SDLGraphicPipeline, &SDLGraphicPipeline::CallBack>(this);
		}
		else
		{
			m_handle = Pipeline::EventBus<Sdl::Event::WindowResize>::UnSubscribe(m_handle);
		}
	}

	ResolutionMode SDLGraphicPipeline::GetResolutionMode() const
	{
		return m_mode;
	}

	FDB::Rect<std::int32_t> SDLGraphicPipeline::GetViewPort() const
	{
		return m_viewPort;
	}

	glm::vec<2, std::int32_t> SDLGraphicPipeline::GetResolution() const
	{
		return m_currentResolution;
	}

	Pipeline::FrameBuffer SDLGraphicPipeline::GetFrameBuffer() const
	{
		return m_frameBuffer;
	}

	void SDLGraphicPipeline::PrivateBegin(const Color::RGBAFloat& color, float z)
	{
		this->SyncResolution();
		m_pipeline.BeginRender(color, z);
		m_frameBuffer = m_pipeline.GetBuffer();
	}

	void SDLGraphicPipeline::SyncResolution()
	{
		if (!m_dirtyResolution)
			return;
		m_dirtyResolution = false;
		m_currentResolution = m_nextResolution;
		m_pipeline.SetSize(m_currentResolution);
		m_texture = FDB::Sdl::Texture(m_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_currentResolution);
	}

	void SDLGraphicPipeline::CallBack(const Sdl::Event::WindowResize& event)
	{
		if (event.id != m_windowPtr->GetId())
			return;
		m_dirtyResolution = true;
		m_nextResolution = event.size;
	}
}
