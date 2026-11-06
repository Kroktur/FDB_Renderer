
	#include <iostream>
#include <mutex>
#include <string>
#include "tiny_obj_loader.h"
	#include "Core/Rect.h"
	#include "GLM/vec4.hpp"
	#include "GLM/vec3.hpp"
	#include "SDL/Application.h"
	#include "SDL/Renderer.h"
	#include "SDL/Window.h"
	#include "SDL/Texture.h"
	#include "SDL3/SDL_timer.h"
#include "Core/Font.h"
#include "GLM/detail/type_vec1.hpp"
#include "Pipeline/Atribute.h"
#include "Pipeline/RenderPath.h"
#include "Core/LoadObj.h" 
#include "Core/Relaxation.h"
#include "Core/CreateSphere.h"
//#include "Audio/Audio.h"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Pipeline/Rasterizer.h"

#include "GLM/glm.hpp"
#include "GLM/mat4x4.hpp"

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



#include <type_traits>
#include <array>
#include <print>

#include "Scene/GameLoop.h"
	

	// TODO MOVE TO FILE ATTRIBUTE

	// TODO MOVE TO FILE ATTRIBUTE
#include <span>

#include "Pipeline/Buffer.h"
#include "Pipeline/FrameBuffer.h"
#include "Pipeline/GraphicPipeline.h"

	//#include "Pipeline/Buffer.h"
#include <deque>
#include <KTR/KTR_assert.h>

#include "Audio/Audio.h"
#include "Core/Axis.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "Core/Mesh.h"
#include "Core/ResourceManager.h"
#include "Core/Texture.h"
#include "Core/Transform.h"
#include "Render/GraphicPipeline.h"

#include "Core/Material.h"
#include "Core/Mesh2d.h"
#include "Core/Text.h"
#include "Core/ThreadPool.h"
#include "Core/Transform2d.h"
#include "Render/PBRFragment.h"
#include "Render/PBRShaders.h"
#include "Render/Sample.h"

#include "KTR/Tools/KTR_Chrono.h"
#include "Render/UiFragment.h"
#include "Render/UiShaders.h"
#include "Scene/BaseScene.h"
#include "Scene/BaseSceneRenderer.h"
#include "Scene/DogFIghtDemo.h"
#include "Scene/PBRScene.h"
#include "Scene/Sun.h"
#include "Scene/SplineCurve.h"

	static constexpr std::int32_t renderWidth = 640 * 2;
	static constexpr std::int32_t renderHeight = 360 * 2;

	static constexpr std::int32_t windowWidth = 1920 / 2;
	static constexpr std::int32_t windowHeight = 1080  /2;

#include "KTR/Tools/KTR_RTTI.h"

#include "Core/Ecs.h"





	
	#include <vector>
	




	
	
	

#include "KTR/Tools/KTR_Chrono.h"
#include "KTR/Tools/KTR_Random.h"

	
#include "atomic"


	

	
	//static float angle = (43.0f * 3.14) / 180.f;





		
	
		//	P(1, 1, 1), P(-1, 1, 1),
		//	{ 0.f, 1.f, 0.f, 1.f });


	int main(int argc, char* argv[])
	{
		std::filesystem::path path = argv[0];
		FDB::Core::FilePathSystem::AddFilePath(path.parent_path().parent_path().parent_path().parent_path().parent_path() / "Resources");
		FDB::Scene::GameLoop loop("test", { windowWidth,windowHeight }, { renderWidth, renderHeight });
		loop.AddScene(std::make_unique<FDB::Demo::PBRScene>(), true);
		loop.AddScene(std::make_unique<FDB::Scene::BasicScene>(), false);
		loop.Loop();
	}


