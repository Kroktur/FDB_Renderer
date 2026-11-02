#ifndef FDBCORELIGHTF
#define FDBCORELIGHTF
#include <cstdint>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>
#include <GLM/ext/scalar_constants.hpp>

#include "Color.h"

namespace FDB::Core
{
	enum class LightType : std::uint8_t
	{
		Point,
		Dir,
		Spot
	};

	struct LightData
	{
		// spot / point
		glm::vec3 pos = {0,0,0};
		// spot / dir
		glm::vec3 dir = {0,-1,0};
		// all
		Color::RGBAFloat color = Color::whiteFloat;
		// all
		Color::RGBAFloat specColor = Color::whiteFloat;
		// all
		float intensity = 100.0f;
		// spot
		float openAngle = glm::pi<float>() / 3.f;
		// spot
		float AdditionalSmooth = 1.f;
		// all
		LightType type = LightType::Dir;
	};

	using lightData_t = LightData;
	struct Light
	{
	public:
		Light() = default;
		Light(const Light&) = default;
		Light(Light&&) noexcept = default;
		Light& operator=(const Light&) = default;
		Light& operator=(Light&&) noexcept = default;
		~Light() = default;
		void SetColor(const Color::RGBAFloat& color, const Color::RGBAFloat& specColor = Color::whiteFloat);
		[[nodiscard]] Color::RGBAFloat GetColor() const;
		[[nodiscard]] Color::RGBAFloat GetSpecColor() const;
		void SetType(LightType type);
		[[nodiscard]] LightType GetType() const;
		void SetIntensity(float i);
		[[nodiscard]] float GetIntensity() const;
		void SetPos(const glm::vec3& pos);
		[[nodiscard]] glm::vec3 GetPos() const;
		void SetDir(const glm::vec3& dir);
		[[nodiscard]] glm::vec3 GetDir() const;
		void SetSpotData(float openAngle,float additionalSmooth);
		[[nodiscard]] float GetOpenAngle() const;
		[[nodiscard]] float GetAdditionalSmooth() const;
		[[nodiscard]] lightData_t GetData() const;

	private:
		lightData_t m_data;
	};
}

#endif
