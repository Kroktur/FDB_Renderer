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
		std::vector<glm::vec<4, std::uint8_t>> tPixels(globSize);
		for (std::int32_t i = 0; i < globSize; ++i)
		{
			tPixels[i].r = pixels[i * 4 + 0];
			tPixels[i].g = pixels[i * 4 + 1];
			tPixels[i].b = pixels[i * 4 + 2];
			tPixels[i].a = pixels[i * 4 + 3];
		}
		stbi_image_free(pixels);
		return TextureTool::CreateTexture(tPixels, {width, height});
	}


	std::unique_ptr<Texture> TextureTool::CreateTexture(const std::vector<glm::vec<4, std::uint8_t>>& pixels_,
		const glm::vec<2, std::uint32_t>& size_)
	{
		std::unique_ptr<Texture> texture = std::make_unique<Texture>();

		std::vector<glm::vec<4, std::uint8_t>> pixels = pixels_;
		std::vector<glm::vec<4, std::uint8_t>> nextPixels;
		nextPixels.reserve(pixels.size());
		glm::vec<2, std::uint32_t> size = size_;
		std::size_t offset = 0;

		texture->data.reserve(static_cast<std::size_t>(size.x) * size.y * 1.5);
		texture->mipLevels.reserve(std::max(size.x, size.y));

		for (;;)
		{
			MipLevel lvl;
			lvl.offset = offset;
			lvl.size = size;

			texture->mipLevels.push_back(lvl);
			texture->data.insert(texture->data.end(), pixels.begin(), pixels.end());
			offset += pixels.size();

			glm::vec<2, std::uint32_t> nextSize{
				std::max(static_cast<std::uint32_t>(1), (size.x + 1) / 2),
				std::max(static_cast<std::uint32_t>(1), (size.y + 1) / 2) };


			if (size.x == 1 && size.y == 1)
				break;

			nextPixels.clear();


			for (std::uint32_t y = 0; y < nextSize.y; ++y)
			{
				for (std::uint32_t x = 0; x < nextSize.x; ++x)
				{
					glm::vec<4, std::uint32_t> sum(0);
					std::uint32_t count = 0;

					for (std::uint32_t dy = 0; dy < 2; ++dy)
					{
						for (std::uint32_t dx = 0; dx < 2; ++dx)
						{
							std::uint32_t sx = x * 2 + dx;
							std::uint32_t sy = y * 2 + dy;

							if (sx < size.x && sy < size.y)
							{
								sum += glm::uvec4(pixels[sy * size.x + sx]);
								++count;
							}
						}
					}

					nextPixels.emplace_back(sum / count);
				}
			}
			pixels = nextPixels;
			size = nextSize;
		}

		return std::move(texture);
	}
}
