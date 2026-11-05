#ifndef FDBCORETRANSFORM2DF
#define FDBCORETRANSFORM2DF
#include <GLM/mat3x3.hpp>
#include <GLM/vec2.hpp>


namespace FDB::Core
{
	struct Transform2D
	{
	public:
		Transform2D() = default;
		Transform2D(const Transform2D&) = default;
		Transform2D& operator=(const Transform2D&) = default;
		Transform2D& operator=(Transform2D&&) noexcept = default;
		Transform2D(Transform2D&&) noexcept = default;
		~Transform2D() = default;
	public:
		void SetPosition(const glm::vec2& pos);

		void Move(const glm::vec2& offset);

		[[nodiscard]] glm::vec2 GetPosition() const;

		void SetScale(const glm::vec2& scale);

		void ScaleUp(const glm::vec2& factor);

		[[nodiscard]] glm::vec2 GetScale() const;

		void SetRotation(float r);

		void Rotate(float r);

		[[nodiscard]] float GetRotation() const;

		[[nodiscard]] glm::mat3x3 GetModel();

	private:
		glm::vec2 m_position = { 0,0 };
		glm::vec2 m_scale = { 0,0 };
		float m_rotation = 0;
		bool m_dirty = true;
		glm::mat3x3 m_model = glm::mat3x3(1.0f);
	};

}

#endif
