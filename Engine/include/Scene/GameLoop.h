#ifndef FDBGAMELOOPF
#define FDBGAMELOOPF
#include <memory>
#include <string>
#include <KTR/Tools/KTR_Chrono.h>

#include "BaseScene.h"
#include "Audio/Audio.h"
#include "Render/GraphicPipeline.h"
#include "SDL/Application.h"

namespace FDB::Scene
{
	struct GameLoop
	{
	public:
		GameLoop(const std::string& name, const glm::uvec2& size, const glm::vec2& renderSize);

		void AddScene(std::unique_ptr<BasicScene> scene, bool main = false);

		~GameLoop();

		void Loop();

	private:
		void ChangeSceneEvent(const FDB::Scene::ChangeSceneEvent& event);

		void ChangeScene();

		std::vector<std::unique_ptr<BasicScene>> m_scenes;
		BasicScene* m_currentScene = nullptr;
		std::uint64_t m_currentSceneId = 0;
		bool m_sceneChanged = false;
		FDB::Sdl::Window m_window ;
		FDB::Pipeline::EventBus<FDB::Scene::ChangeSceneEvent>::BusHandle m_handle;
		FDB::Core::JobPool m_pool;
		std::unique_ptr<FDB::Render::SDLGraphicPipeline> m_renderer;
	};
}


#endif
