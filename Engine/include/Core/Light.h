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
		void SetColor(const Color::RGBAFloat& color, const Color::RGBAFloat& specColor = Color::whiteFloat)
		{
			m_data.color = color;
			m_data.specColor = specColor;
		}
		[[nodiscard]] Color::RGBAFloat GetColor() const { return m_data.color; }
		[[nodiscard]] Color::RGBAFloat GetSpecColor() const { return m_data.specColor; }
		void SetType(LightType type) { m_data.type = type; }
		[[nodiscard]] LightType GetType() const { return m_data.type; }
		void SetIntensity(float i) { m_data.intensity = i; }
		[[nodiscard]] float GetIntensity() const { return m_data.intensity; }
		void SetPos(const glm::vec3& pos) { m_data.pos = pos; }
		[[nodiscard]] glm::vec3 GetPos() const { return m_data.pos; }
		void SetDir(const glm::vec3& dir) { m_data.dir = dir; }
		[[nodiscard]] glm::vec3 GetDir() const { return m_data.dir; }
		void SetSpotData(float openAngle,float additionalSmooth)
		{
			m_data.openAngle = openAngle;
			m_data.AdditionalSmooth = additionalSmooth;
		}
		[[nodiscard]] float GetOpenAngle() const { return m_data.openAngle; }
		[[nodiscard]] float GetAdditionalSmooth() const { return m_data.AdditionalSmooth; }
		[[nodiscard]] lightData_t GetData() const { return m_data; }
	private:
		lightData_t m_data;
	};
}

#endif
