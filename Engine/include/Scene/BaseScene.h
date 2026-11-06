#ifndef FDBSCENEBASESCENEF
#define FDBSCENEBASESCENEF
#include <cstdint>

#include "Core/Color.h"

namespace FDB::Sdl
{
	struct Window;
}

namespace FDB::Render
{
	struct SDLGraphicPipeline;
}

namespace FDB::Core
{
	struct JobPool;
}

namespace FDB::Scene
{
	struct ChangeSceneEvent
	{
		std::uint64_t sceneId;
	};

	struct GameLoop;

	struct BasicScene
	{
	public:
		friend GameLoop;
		virtual ~BasicScene() = default;
		virtual void HandleInput(FDB::Core::JobPool& pool);

		virtual void Update(float dt, FDB::Core::JobPool& pool);

		virtual void Render(FDB::Core::JobPool& pool);

		virtual  Color::RGBA8888 ClearColor();

		[[nodiscard]] std::uint64_t GetSceneId() const;

		virtual void OnEnable()
		{
			
		}
		virtual void OnDisable()
		{
			
		}

	protected:
		virtual void InitScene();
		FDB::Render::SDLGraphicPipeline* m_renderer = nullptr;
		FDB::Sdl::Window* m_window = nullptr;
	private:
		void Init(FDB::Sdl::Window* window, FDB::Render::SDLGraphicPipeline* renderer, std::uint64_t id);
		std::uint64_t m_sceneId = 0;
	};
}


#endif
