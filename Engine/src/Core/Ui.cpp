#include "Core/Ui.h"

namespace FDB::Core
{
	glm::vec4 Ui::GetColor() const
	{
		return m_color;
	}

	void Ui::SetColor(const glm::vec4& color_)
	{
		m_color = color_;
	}

	void Ui::SetVResolution(const glm::vec2& resolution)
	{
		m_virtualResolution = resolution;
		m_posDirty = true;
		m_scaleDirty = true;
	}

	glm::vec2 Ui::GetVResolution() const
	{
		return m_virtualResolution;
	}

	void Ui::SetPosition(const glm::vec2& position)
	{
		m_pos = position;
		m_posDirty = true;
	}

	glm::vec2 Ui::GetPosition() const
	{
		return m_pos;
	}

	void Ui::SetScale(const glm::vec2& scale_)
	{
		m_scale = scale_;
		m_scaleDirty = true;
	}

	glm::vec2 Ui::GetScale() const
	{
		return m_scale;
	}

	glm::vec2 Ui::GetPosNdc(float aspectRatio)
	{
		if (m_posDirty)
		{
			m_ndcPos = { (m_pos.x / m_virtualResolution.x) * (2.0f * aspectRatio) - aspectRatio,
				(m_pos.y / m_virtualResolution.y) * 2.0f - 1.0f };
			m_posDirty = false;
		}
		return m_ndcPos;
	}

	glm::vec2 Ui::GetScaleNdc(float aspectRatio)
	{
		if (m_scaleDirty)
		{
			m_ndcScale = { (m_scale.x / m_virtualResolution.x) * (2.0f * aspectRatio),
				(m_scale.y / m_virtualResolution.y) * 2.0f };
			m_scaleDirty = false;
		}
		return m_ndcScale;
	}
}
