#include "Core/Transform.h"
#include "Core/Axis.h"

namespace FDB::Core
{
	Transform::Transform(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& quat):m_position(pos),m_scale(scale),m_quat(quat), m_dirty(true)
	{}

	void Transform::SetPosition(const glm::vec3& pos)
	{
		m_position = pos;
		m_dirty = true;
	}

	void Transform::Move(const glm::vec3& move)
	{
		m_position += move;
		m_dirty = true;
	}

	glm::vec3 Transform::GetPosition() const
	{
		return m_position;
	}

	void Transform::SetScale(const glm::vec3& scale)
	{
		m_scale = scale;
		m_dirty = true;
	}

	void Transform::ScaleUp(const glm::vec3& scale)
	{
		m_scale *= scale;
		m_dirty = true;
	}

	glm::vec3 Transform::GetScale() const
	{
		return m_scale;
	}

	void Transform::SetOrientation(const glm::quat& quat)
	{
		m_quat = quat;
		m_dirty = true;
	}

	glm::quat Transform::GetOrientation() const
	{
		return m_quat;
	}

	void Transform::SetRotation(const glm::vec3& rotation)
	{
		m_quat = glm::quat(rotation);
		m_dirty = true;
	}

	glm::vec3 Transform::GetRotation() const
	{
		return glm::eulerAngles(m_quat);
	}

	void Transform::Rotate(float angleRad, const glm::vec3& axis)
	{
		glm::quat deltaRotation = glm::angleAxis(angleRad, axis);
		m_quat = glm::normalize(m_quat * deltaRotation);
		m_dirty = true;
	}

	void Transform::Rotate(const glm::vec3& rotation)
	{
		m_quat = glm::normalize(
			m_quat * glm::quat(rotation)
		);
		m_dirty = true;
	}

	void Transform::LookAt(const glm::vec3& target)
	{
		glm::vec3 forward = glm::normalize(target - m_position);
		auto up = Axis::up;
		if (abs(glm::dot(forward, up)) > 0.999f)
			up = Axis::forward;
		m_quat = glm::quatLookAt(forward, up);
		m_dirty = true;
	}

	void Transform::LookAtDir(const glm::vec3& target)
	{
		glm::vec3 forward = glm::normalize(target);
		auto up = Axis::up;
		if (abs(glm::dot(forward, up)) > 0.999f)
			up = Axis::forward;
		m_quat = glm::quatLookAt(forward, up);
		m_dirty = true;
	}

	glm::mat4 Transform::GetModelM()
	{
		if (m_dirty)
		{
			m_mat = glm::translate(glm::mat4(1.0f), m_position) * glm::scale(glm::mat4(1.0f), m_scale) * glm::toMat4(m_quat);
			m_dirty = false;
		}
		return m_mat;
	}
}
