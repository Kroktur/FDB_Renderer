#ifndef FDBTEXTUREF
#define FDBTEXTUREF
#include <filesystem>
#include <memory>
#include <GLM/vec2.hpp>
#include <GLM/vec4.hpp>

#include "ResourceManager.h"

namespace FDB::Core
{
	

	struct MipTexture
	{
		using value_type = glm::vec<4,std::uint8_t>;
		MipTexture() = default;
		MipTexture(const MipTexture&) = default;
		MipTexture(MipTexture&&) noexcept = default;
		~MipTexture() = default;
		MipTexture& operator=(const MipTexture&) = default;
		MipTexture& operator=(MipTexture&&) noexcept = default;
		MipTexture(std::vector<value_type>&& pixels_, std::uint32_t width_ , std::uint32_t height_) : pixels(std::move(pixels_)),size(width_,height_){}

		glm::vec<2, std::uint32_t> size;
		std::vector<value_type> pixels{};

	private:
	};

	struct Texture
	{
		std::vector<MipTexture> m_mips;
	};


	struct TextureTool
	{
		[[nodiscard]] static std::unique_ptr<Texture> LoadTexture(const std::filesystem::path& TextFile);
		 static void GenerateMipMap(Texture* texture);
 	};
	using textureLoader_t = resourceManager_t< FDB::Core::Texture, TypeWrapper<>, FDB::Core::TextureTool::LoadTexture>;

}



#endif
