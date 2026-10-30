#include "Render/Sample.h"

#include "Core/Texture.h"

namespace FDB::Render
{
	glm::vec4 Sample::Execute(Core::Texture* textureMap, const glm::vec2& uv)
	{
		auto& texture = textureMap->m_mips[0];
		glm::vec<2, std::uint32_t> iUv = { uv.x * static_cast<float>(texture.size.x),uv.y * static_cast<float>(texture.size.y) };
		const std::uint32_t index = iUv.y * texture.size.x + iUv.x;
		const Core::MipTexture::value_type pixel = texture.pixels[index];
		static constexpr float convFactor = 1.0f / 255.0f;
		return glm::vec4(pixel) * convFactor;
	}
}
