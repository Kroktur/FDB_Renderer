#ifndef FDBCORECAMERAF
#define FDBCORECAMERAF
#include <GLM/mat4x4.hpp>
#include <GLM/ext/matrix_clip_space.hpp>

namespace FDB::Core
{
	enum class CameraType : std::uint8_t
	{
		Perspective,
		Orthographic
	};
	using cameraType_t = CameraType;
	struct OrthographicSetting
	{
		float left = -400.0f;
		float right = 400.0f;
		float bottom = -300.0f;
		float top = 300.0f;
	};
	using orthographicSetting_t = OrthographicSetting;
	struct PerspectiveSetting
	{
		float fov = glm::pi<float>() /3.f;
		float aspectRatio = 16.0f / 9.0f;
	};
	using perspectiveSetting_t = PerspectiveSetting;
	struct GlobalSetting
	{
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
	};
	using globalSetting_t = GlobalSetting;

	struct CameraData
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 invView;
	};

	struct Camera
	{
		Camera() = default;
		Camera(const Camera&) = default;
		Camera(Camera&&) noexcept = default;
		Camera& operator=(const Camera&) = default;
		Camera& operator=(Camera&&) noexcept = default;
		~Camera() = default;
		static Camera Perspective(const perspectiveSetting_t& perspSetting ={}, const globalSetting_t& globSetting ={});
		static Camera OrthoGraphic(const orthographicSetting_t& orthoSetting ={}, const globalSetting_t& globSetting ={});

	public:
		void Update(const glm::mat4& model);

		void SetOrthoSetting(const orthographicSetting_t& setting);

		[[nodiscard]] orthographicSetting_t GetOrthoSetting() const;

		void SetPerspSetting(const perspectiveSetting_t& setting);

		[[nodiscard]] perspectiveSetting_t GetPerpsSetting() const;

		void SetGloSetting(const globalSetting_t& setting);

		[[nodiscard]] globalSetting_t GetGlobSetting() const;

		[[nodiscard]] glm::mat4 GetProjM();

		[[nodiscard]] glm::mat4 GetViewM();

		[[nodiscard]] glm::mat4 GetModelM() const;
		
		void SetType(cameraType_t type);

		[[nodiscard]] cameraType_t GetType() const;

		[[nodiscard]] CameraData GetData();

	private:
		perspectiveSetting_t m_perspSetting = {};
		orthographicSetting_t m_orthoSetting= {};
		globalSetting_t m_globSetting = {};
		cameraType_t m_type = cameraType_t::Perspective;
		glm::mat4 m_proj = {};
		bool m_projDirty = true;

		glm::mat4 m_model = glm::mat4(1.f);
		glm::mat4 m_view = {};
		bool m_viewDirty = true;
	};
}


#endif
