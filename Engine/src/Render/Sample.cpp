#include "Render/Sample.h"

#include <GLM/geometric.hpp>

#include "Core/Texture.h"

namespace FDB::Render
{
	glm::vec4 Sample::Execute(Core::Texture* textureMap, const glm::vec2& uv, const glm::vec2& dUvX, const glm::vec2& dUvY)
	{

		if (textureMap->mipLevels.size() == 1)
			return ExecuteMip(textureMap, uv, 0);
		glm::vec2 size = textureMap->mipLevels[0].size;

		glm::vec2 dx = dUvX * glm::vec2(size);
		glm::vec2 dy = dUvY * glm::vec2(size);

		float rho = std::max(glm::length(dx), glm::length(dy));

		float lod = std::log2(std::max(rho, 1e-8f));
		lod = std::max(0.0f, lod);
		lod = glm::clamp(lod, 0.f, static_cast<float>(textureMap->mipLevels.size() - 1));

		const std::uint32_t i = static_cast<std::uint32_t>(std::floor(lod));
		const std::uint32_t i2 = static_cast<std::uint32_t>(std::ceil(lod));
		return glm::mix(Sample::ExecuteMip(textureMap,uv,i), Sample::ExecuteMip(textureMap,uv,i2), glm::fract(lod));
	}

	glm::vec4 Sample::ExecuteMip(const Core::Texture* texture,const glm::vec2& uv,std::uint32_t id)
	{
		auto& level = texture->mipLevels[id];

		glm::vec2 fTexelCoord = { uv.x * static_cast<float>(level.size.x),uv.y * static_cast<float>(level.size.y) };

		glm::uvec2 iUv00 =  fTexelCoord;
		glm::uvec2 iUv10 = iUv00 + glm::uvec2(1, 0);
		glm::uvec2 iUv01 = iUv00 + glm::uvec2(0, 1);
		glm::uvec2 iUv11 = iUv00 + glm::uvec2(1);


		iUv00 = glm::min(iUv00, level.size - glm::uvec2(1));
		iUv10 = glm::min(iUv10, level.size - glm::uvec2(1));
		iUv01 = glm::min(iUv01, level.size - glm::uvec2(1));
		iUv11 = glm::min(iUv11, level.size - glm::uvec2(1));

		auto lerpFactors = fTexelCoord - glm::vec2(iUv00);


		const std::uint32_t i00 = static_cast<std::uint32_t>(level.offset) + (iUv00.y * level.size.x + iUv00.x);
		const std::uint32_t i10 = static_cast<std::uint32_t>(level.offset) + (iUv10.y * level.size.x + iUv10.x);
		const std::uint32_t i01 = static_cast<std::uint32_t>(level.offset) + (iUv01.y * level.size.x + iUv01.x);
		const std::uint32_t i11 = static_cast<std::uint32_t>(level.offset) + (iUv11.y * level.size.x + iUv11.x);




		const glm::u8vec4 p00 = texture->data[i00];
		const glm::u8vec4 p10 = texture->data[i10];
		const glm::u8vec4 p01 = texture->data[i01];
		const glm::u8vec4 p11 = texture->data[i11];

		auto pA = glm::mix(p00, p10, lerpFactors.x);
		auto pB = glm::mix(p01, p11, lerpFactors.x);
		auto pixel = glm::mix(pA, pB, lerpFactors.y);

		return  glm::vec4(pixel) * convFactor;
	}
}
