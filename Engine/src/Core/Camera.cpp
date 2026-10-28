#include "Core/Camera.h"

namespace FDB::Core
{
	Camera Camera::Perspective(const perspectiveSetting_t& perspSetting, const globalSetting_t& globSetting)
	{
		Camera result;
		result.SetPerspSetting(perspSetting);
		result.SetGloSetting(globSetting);
		result.SetType(CameraType::Perspective);
		return result;
	}

	Camera Camera::OrthoGraphic(const orthographicSetting_t& orthoSetting, const globalSetting_t& globSetting)
	{
		Camera result;
		result.SetOrthoSetting(orthoSetting);
		result.SetGloSetting(globSetting);
		result.SetType(CameraType::Orthographic);
		return result;
	}

	void Camera::Update(const glm::mat4& model)
	{
		if (m_data.invView != model)
		{
			m_data.invView = model;
			m_viewDirty = true;
		}
	}

	void Camera::SetOrthoSetting(const orthographicSetting_t& setting)
	{
		m_orthoSetting = setting;
		if (m_type == cameraType_t::Orthographic)
			m_projDirty = true;
	}

	orthographicSetting_t Camera::GetOrthoSetting() const
	{
		return m_orthoSetting;
	}

	void Camera::SetPerspSetting(const perspectiveSetting_t& setting)
	{
		m_perspSetting = setting;
		if (m_type == cameraType_t::Perspective)
			m_projDirty = true;
	}

	perspectiveSetting_t Camera::GetPerpsSetting() const
	{
		return m_perspSetting;
	}

	void Camera::SetGloSetting(const globalSetting_t& setting)
	{
		m_globSetting = setting;
		m_projDirty = true;
	}

	globalSetting_t Camera::GetGlobSetting() const
	{
		return m_globSetting;
	}

	glm::mat4 Camera::GetProjM()
	{
		if (m_projDirty)
		{
			if (m_type == cameraType_t::Perspective)
			{
				m_data.proj = glm::perspectiveRH_ZO(m_perspSetting.fov, m_perspSetting.aspectRatio, m_globSetting.nearPlane, m_globSetting.farPlane);
			}
			else
			{
				m_data.proj = glm::orthoRH_ZO(m_orthoSetting.left,m_orthoSetting.right,m_orthoSetting.bottom,m_orthoSetting.top,m_globSetting.nearPlane,m_globSetting.farPlane);
			}
			m_projDirty = false;
		}
		return m_data.proj;
	}

	glm::mat4 Camera::GetViewM()
	{
		if (m_viewDirty)
		{
			m_data.view = glm::inverse(m_data.invView);
			m_viewDirty = false;
		}
		return m_data.view;
	}

	glm::mat4 Camera::GetModelM() const
	{
		return m_data.invView;
	}

	cameraType_t Camera::GetType() const
	{
		return m_type;
	}

	CameraData Camera::GetData()
	{
		this->PrivUpdate();
		return m_data;
	}

	void Camera::PrivUpdate()
	{
		if (m_viewDirty)
		{
			m_data.view = glm::inverse(m_data.invView);
			m_viewDirty = false;
		}
		if (m_projDirty)
		{
			if (m_type == cameraType_t::Perspective)
			{
				m_data.proj = glm::perspectiveRH_ZO(m_perspSetting.fov, m_perspSetting.aspectRatio, m_globSetting.nearPlane, m_globSetting.farPlane);
			}
			else
			{
				m_data.proj = glm::orthoRH_ZO(m_orthoSetting.left, m_orthoSetting.right, m_orthoSetting.bottom, m_orthoSetting.top, m_globSetting.nearPlane, m_globSetting.farPlane);
			}
			m_projDirty = false;
		}
	}

	void Camera::SetType(cameraType_t type)
	{
		m_type = type;
	}
}
