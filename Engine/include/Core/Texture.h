#ifndef FDBTEXTUREF
#define FDBTEXTUREF
#include <filesystem>
#include <memory>
#include <GLM/vec2.hpp>
#include <GLM/vec4.hpp>

#include "ResourceManager.h"

namespace Test
{
	

	

}


namespace FDB::Core
{
	

    struct MipLevel {
        glm::vec<2, std::uint32_t> size;
        std::size_t offset;
    };

    struct Texture {
        std::vector < glm::vec < 4, std::uint8_t>> data;
        std::vector<MipLevel> mipLevels;
    };

	struct TextureTool
	{
		[[nodiscard]] static std::unique_ptr<Texture> LoadTexture(const std::filesystem::path& TextFile);
        [[nodiscard]] static std::unique_ptr<Texture> CreateTexture(const std::vector<glm::vec < 4, std::uint8_t>>& pixels_, const glm::vec<2, std::uint32_t>& size_);
	};
	using textureLoader_t = resourceManager_t< FDB::Core::Texture, TypeWrapper<>, FDB::Core::TextureTool::LoadTexture>;

}



#endif
