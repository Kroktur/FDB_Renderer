#ifndef FDBCORETRANSFORMF
#define FDBCORETRANSFORM

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <concepts>



namespace FDB::Core
{
	struct Transform
	{
	public:
		Transform() = default;
		Transform(const glm::vec3& pos,const glm::vec3& scale = glm::vec3(1.f),const glm::quat& quat = glm::quat(1, 0, 0, 0));
		Transform(const Transform&) = default;
		Transform(Transform&&)noexcept = default;
		Transform& operator=(const Transform&) = default;
		Transform& operator=(Transform&&) noexcept = default;
		~Transform() = default;
	public:
		void SetPosition(const glm::vec3& pos);

		void Move(const glm::vec3& move);

		[[nodiscard]] glm::vec3 GetPosition() const;

		void SetScale(const glm::vec3& scale);

		void ScaleUp(const glm::vec3& scale);

		[[nodiscard]]glm::vec3 GetScale()const;

		void SetOrientation(const glm::quat& quat);

		[[nodiscard]]glm::quat GetOrientation() const;

		void SetRotation(const glm::vec3& rotation);

		[[nodiscard]] glm::vec3 GetRotation() const;

		void Rotate(float angleRad, const glm::vec3& axis);

		void Rotate(const glm::vec3& rotation);


		void LookAt(const glm::vec3& target);

		void LookAtDir(const glm::vec3& target);

		[[nodiscard]] glm::mat4 GetModelM();

	private:
		glm::vec3 m_position = glm::vec3(0.f);
		glm::vec3 m_scale = glm::vec3(1.f);
		glm::quat m_quat = glm::quat(1,0,0,0);
		glm::mat4 m_mat = glm::mat4(1.0f);
		bool m_dirty = false;
	};
}




#endif
