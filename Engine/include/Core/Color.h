#ifndef FDB_CORE_COLORF
#define FDB_CORE_COLORF

#include <GLM/detail/type_vec4.hpp>
namespace FDB
{
	struct Color
	{
		using RGBA8888 = glm::vec<4, std::uint8_t>;
		using RGBA32 = std::uint32_t;
		using RGBAFloat = glm::vec<4, float>;

		static constexpr std::uint8_t max8 = 255;
		static constexpr float max8Float = static_cast<float>(max8);
		static constexpr float invMax8Float = 1.f / max8Float;

		static constexpr RGBA8888 default8 = { 0,0,0,0 };
		static constexpr RGBA8888 alpha8 = {0,0,0,255};
		static constexpr RGBA8888 red8{ 255,   0,   0, 255 };
		static constexpr RGBA8888 green8{ 0, 255,   0, 255 };
		static constexpr RGBA8888 blue8{ 0,   0, 255, 255 };
		static constexpr RGBA8888 black8{ 0,   0,   0, 255 };
		static constexpr RGBA8888 white8{ 255, 255, 255, 255 };

		static constexpr RGBAFloat defaultFloat = { 0.f,0.f,0.f,0.f };
		static constexpr RGBA8888 alphaFloat = { 0,0,0,1.f };
		static constexpr RGBAFloat redFloat{ 1.f, 0.f, 0.f, 1.f };
		static constexpr RGBAFloat greenFloat{ 0.f, 1.f, 0.f, 1.f };
		static constexpr RGBAFloat blueFloat{ 0.f, 0.f, 1.f, 1.f };
		static constexpr RGBAFloat blackFloat{ 0.f, 0.f, 0.f, 1.f };
		static constexpr RGBAFloat whiteFloat{ 1.f, 1.f, 1.f, 1.f };

		static RGBA8888 ToARGB8888(const RGBAFloat& argFloat)
		{
			return RGBA8888{ argFloat.r * max8,argFloat.g * max8 , argFloat.b * max8 , argFloat.a * max8 };
		}
		
		static RGBAFloat ToARGBFloat(const RGBA8888& arg8)
		{
			return RGBAFloat{ static_cast<float>(arg8.r) * invMax8Float,static_cast<float>(arg8.g) * invMax8Float,static_cast<float>(arg8.b) * invMax8Float , static_cast<float>(arg8.a) * invMax8Float };
		}
	};
}

#endif
