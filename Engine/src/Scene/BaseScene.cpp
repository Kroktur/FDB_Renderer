#include "Scene/BaseScene.h"

void FDB::Scene::BasicScene::HandleInput(FDB::Core::JobPool& pool)
{
}

void FDB::Scene::BasicScene::Update(float dt, FDB::Core::JobPool& pool)
{

}

void FDB::Scene::BasicScene::Render(FDB::Core::JobPool& pool)
{

}

FDB::Color::RGBA8888 FDB::Scene::BasicScene::ClearColor()
{
	return  Color::alpha8;
}

std::uint64_t FDB::Scene::BasicScene::GetSceneId() const
{
	return m_sceneId;
}

void FDB::Scene::BasicScene::InitScene()
{

}

void FDB::Scene::BasicScene::Init(FDB::Sdl::Window* window, FDB::Render::SDLGraphicPipeline* renderer, std::uint64_t id)
{
	m_renderer = renderer;
	m_window = window;
	m_sceneId = id;
	InitScene();
}
