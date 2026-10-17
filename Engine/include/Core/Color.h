#ifndef FDB_CORE_COLORF
#define FDB_CORE_COLORF

#include <GLM/detail/type_vec4.hpp>
namespace FDB
{
	struct Color
	{
		using RGBA8 = glm::vec<4, std::uint8_t>;
		using RGBAFloat = glm::vec<4, float>;

		static constexpr std::uint8_t max8 = 255;
		static constexpr float max8Float = static_cast<float>(max8);
		static constexpr float invMax8Float = 1.f / max8Float;

		static constexpr RGBA8 default8 = { 0,0,0,0 };
		static constexpr RGBA8 red8{ 255,   0,   0, 255 };
		static constexpr RGBA8 green8{ 0, 255,   0, 255 };
		static constexpr RGBA8 blue8{ 0,   0, 255, 255 };
		static constexpr RGBA8 black8{ 0,   0,   0, 255 };
		static constexpr RGBA8 white8{ 255, 255, 255, 255 };

		static constexpr RGBAFloat defaultFloat = { 0.f,0.f,0.f,0.f };
		static constexpr RGBAFloat redFloat{ 1.f, 0.f, 0.f, 1.f };
		static constexpr RGBAFloat greenFloat{ 0.f, 1.f, 0.f, 1.f };
		static constexpr RGBAFloat blueFloat{ 0.f, 0.f, 1.f, 1.f };
		static constexpr RGBAFloat blackFloat{ 0.f, 0.f, 0.f, 1.f };
		static constexpr RGBAFloat whiteFloat{ 1.f, 1.f, 1.f, 1.f };

		static RGBA8 ToARG8(const RGBAFloat& argFloat)
		{
			return RGBA8{ argFloat.r * max8,argFloat.g * max8 , argFloat.b * max8 , argFloat.a * max8 };
		}
		static RGBAFloat ToARGFloat(const RGBA8& arg8)
		{
			return RGBAFloat{ static_cast<float>(arg8.r) * invMax8Float,static_cast<float>(arg8.g) * invMax8Float,static_cast<float>(arg8.b) * invMax8Float , static_cast<float>(arg8.a) * invMax8Float };
		}
	};
}

#endif
