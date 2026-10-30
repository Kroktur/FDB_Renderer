#include "Core/Texture.h"
#include "Core/LoadImage.h"
namespace FDB::Core
{



	std::unique_ptr<Texture> TextureTool::LoadTexture(const std::filesystem::path& TextFile)
	{
	        std::int32_t width, height, channels;

	        std::uint8_t* pixels = stbi_load(
	            TextFile.string().c_str(),
	            &width,
	            &height,
	            &channels,
	            4
	        );
	        if (!pixels)
	        {
	            throw(std::runtime_error("impossible to load file: " + TextFile.string()));
	        }

	        std::int32_t globSize = width * height;
	        std::vector<MipTexture::value_type> tPixels(globSize);
	        for (std::int32_t i = 0 ; i < globSize ; ++i)
	        {
	            tPixels[i].r = pixels[i * 4 + 0];
				tPixels[i].g = pixels[i * 4 + 1];
				tPixels[i].b = pixels[i * 4 + 2];
				tPixels[i].a = pixels[i * 4 + 3];
	        }
			stbi_image_free(pixels);
			std::unique_ptr<Texture> texture = std::make_unique<Texture>();
			texture->m_mips.emplace_back(std::move(tPixels), width, height);
		return std::move(texture);
	}


	void GenerateMip(Texture* texture)
	{}

	void TextureTool::GenerateMipMap(Texture* texture)
	{
		glm::vec<2, std::uint32_t> size = texture->m_mips[0].size;

		while (size.x >1 || size.y > 1 )
		{
			
		}
	}
}
