#ifndef FDBRENDERSAMPLEF
#define FDBRENDERSAMPLEF
#include <GLM/vec2.hpp>
#include <GLM/vec4.hpp>

namespace FDB::Core
{
	struct Texture;
}

namespace FDB::Render
{
	struct Sample
	{
		static glm::vec4 Execute(Core::Texture* textureMap, const glm::vec2& uv);
		
	};
}

#endif
