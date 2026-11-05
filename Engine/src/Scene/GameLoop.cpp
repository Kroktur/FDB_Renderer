#include "Scene/GameLoop.h"

FDB::Scene::GameLoop::GameLoop(const std::string& name, const glm::uvec2& size, const glm::vec2& renderSize): m_window(name.c_str(), size, SDL_WINDOW_RESIZABLE), m_pool()
{
	m_renderer = std::make_unique<FDB::Render::SDLGraphicPipeline>(&m_window, renderSize);
	FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
	FDB::Audio::Soloud::Init();

	m_handle = FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::SubScribe<GameLoop, &GameLoop::ChangeSceneEvent>(this);
}

void FDB::Scene::GameLoop::AddScene(std::unique_ptr<BasicScene> scene, bool main)
{
	scene->Init(&m_window, m_renderer.get(), m_scenes.size());
	m_scenes.push_back(std::move(scene));
	if (main)
	{
		m_currentScene = m_scenes.back().get();
		m_currentSceneId = m_scenes.size() - 1;
	}
}

FDB::Scene::GameLoop::~GameLoop()
{
	FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::UnSubscribe(m_handle);
	m_window.CloseWindow();
	FDB::Sdl::Application::Destroy();
	FDB::Audio::Soloud::Destroy();

}

void FDB::Scene::GameLoop::Loop()
{
	if (!m_currentScene)
		return;

	std::uint64_t frameCount = 0;
	KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution, KTR::duration::milli_seconds_type<double>> fpsTimer;
	fpsTimer.Resume();
	auto lastFps = fpsTimer.GetTime();

	KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution, KTR::duration::seconds_type<float>> timer;
	timer.Resume();
	auto last = timer.GetTime();
	while (!FDB::Sdl::Application::ShouldQuit() && !m_window.ShouldClose())
	{
		this->ChangeScene();
		FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();
		{
			++frameCount;
			auto currentFps = fpsTimer.GetTime();

			auto delta = (currentFps - lastFps).count();
			if (delta >= 1000)
			{
				const double fps = static_cast<double>(frameCount) * 1000.0 / delta;
				std::string title = "FPS: " + std::to_string(static_cast<int>(fps));
				m_window.SetTitle(title.c_str());

				frameCount = 0;
				lastFps = currentFps;
			}
		}


		m_currentScene->HandleInput(m_pool);

		auto current = timer.GetTime();
		m_currentScene->Update((current - last).count(), m_pool);
		last = current;

		m_renderer->ClearColor(m_currentScene->ClearColor());
		m_renderer->BeginRender();
		m_currentScene->Render(m_pool);
		m_renderer->PresentRender(m_pool);
	}
}

void FDB::Scene::GameLoop::ChangeSceneEvent(const FDB::Scene::ChangeSceneEvent& event)
{
	m_currentSceneId = event.sceneId;
	m_sceneChanged = true;
}

void FDB::Scene::GameLoop::ChangeScene()
{
	if (!m_currentScene)
		return;
	m_sceneChanged = false;
	m_currentScene = m_scenes[m_currentSceneId].get();
}
