#include "Core/Light.h"


namespace FDB::Core
{
	void Light::SetColor(const Color::RGBAFloat& color, const Color::RGBAFloat& specColor)
	{
		m_data.color = color;
		m_data.specColor = specColor;
	}

	Color::RGBAFloat Light::GetColor() const
	{ return m_data.color; }

	Color::RGBAFloat Light::GetSpecColor() const
	{ return m_data.specColor; }

	void Light::SetType(LightType type)
	{ m_data.type = type; }

	LightType Light::GetType() const
	{ return m_data.type; }

	void Light::SetIntensity(float i)
	{ m_data.intensity = i; }

	float Light::GetIntensity() const
	{ return m_data.intensity; }

	void Light::SetPos(const glm::vec3& pos)
	{ m_data.pos = pos; }

	glm::vec3 Light::GetPos() const
	{ return m_data.pos; }

	void Light::SetDir(const glm::vec3& dir)
	{ m_data.dir = dir; }

	glm::vec3 Light::GetDir() const
	{ return m_data.dir; }

	void Light::SetSpotData(float openAngle, float additionalSmooth)
	{
		m_data.openAngle = openAngle;
		m_data.AdditionalSmooth = additionalSmooth;
	}

	float Light::GetOpenAngle() const
	{ return m_data.openAngle; }

	float Light::GetAdditionalSmooth() const
	{ return m_data.AdditionalSmooth; }

	lightData_t Light::GetData() const
	{ return m_data; }
}
