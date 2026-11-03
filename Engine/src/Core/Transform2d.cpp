#include "Core/Transform2d.h"

namespace FDB::Core
{
	void Transform2D::SetPosition(const glm::vec2& pos)
	{
		if (pos == m_position)
			return;

		m_position = pos;
		m_dirty = true;
	}

	void Transform2D::Move(const glm::vec2& offset)
	{
		if (offset == glm::vec2(0.0f))
			return;

		m_position += offset;
		m_dirty = true;
	}

	glm::vec2 Transform2D::GetPosition() const
	{
		return m_position;
	}

	void Transform2D::SetScale(const glm::vec2& scale)
	{
		if (m_scale == scale)
			return;
		m_scale = scale;
		m_dirty = true;
	}

	void Transform2D::ScaleUp(const glm::vec2& factor)
	{
		if (factor == glm::vec2(1.0f))
			return;

		m_scale *= factor;
		m_dirty = true;
	}

	glm::vec2 Transform2D::GetScale() const
	{
		return m_scale;
	}

	void Transform2D::SetRotation(float r)
	{
		if (r == m_rotation)
			return;
		m_rotation = r;
		m_dirty = true;
	}

	void Transform2D::Rotate(float r)
	{
		if (r == 0)
			return;
		m_rotation += r;
		m_dirty = true;
	}

	float Transform2D::GetRotation() const
	{
		return m_rotation;
	}

	glm::mat3x3 Transform2D::GetModel()
	{
		if (m_dirty)
		{
			glm::mat3x3 translation = glm::mat3(
				1, 0.0f, 0.0f,
				0.0f, 1, 0.0f,
				m_position.x, m_position.y, 1.0f
			);

			glm::mat3x3 rotation = glm::mat3(
				std::cos(m_rotation), -std::sin(m_rotation), 0.0f,
				std::sin(m_rotation), std::cos(m_rotation), 0.0f,
				0.0f, 0.0f, 1.0f
			);

			glm::mat3x3 scale = glm::mat3(
				m_scale.x, 0.0f, 0.0f,
				0.0f, m_scale.y, 0.0f,
				0.0f, 0.0f, 1.0f
			);


			m_model = translation * rotation * scale;
			m_dirty = false;
		}
		return m_model;
	}
}
