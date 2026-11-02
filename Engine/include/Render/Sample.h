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
		static constexpr float convFactor = 1.0f / 255.0f;

		static glm::vec4 Execute(Core::Texture* textureMap, const glm::vec2& uv, const glm::vec2& dUvX ={}, const glm::vec2& dUvY ={});
		static glm::vec4 ExecuteMip(const Core::Texture* texture, const glm::vec2& uv, std::uint32_t id);
		
	};
}

#endif
