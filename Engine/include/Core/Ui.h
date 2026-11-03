#ifndef FDBCOREUIF
#define FDBCOREUIF

#include "Pipeline/Atribute.h"

namespace FDB::Core
{
	struct VertexUi {
		glm::vec2 position;
		glm::vec2 uv;
		glm::vec2 uv2;
		[[nodiscard]] static  constexpr  FDB::Pipeline::Attribute PosAttribute()
		{
			return FDB::Pipeline::Attribute{ FDB::Pipeline::format::Float2,offsetof(VertexUi, position) };
		}
		[[nodiscard]] static constexpr  std::array<FDB::Pipeline::Attribute, 3> AllAttributes()
		{
			return { FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VertexUi, position)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VertexUi, uv)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VertexUi, uv2)}
			};
		}
	};


	struct Ui
	{
	public:
		Ui() = default;
		Ui(const Ui&) = default;
		Ui& operator=(const Ui&) = default;
		Ui(Ui&&) noexcept = default;
		Ui& operator=(Ui&&) noexcept = default;
		~Ui() = default;
	public:
		[[nodiscard]] glm::vec4 GetColor() const;

		void SetColor(const glm::vec4& color_);

		void SetVResolution(const glm::vec2& resolution);

		[[nodiscard]] glm::vec2 GetVResolution() const;

		void SetPosition(const glm::vec2& position);

		[[nodiscard]] glm::vec2 GetPosition() const;

		void SetScale(const glm::vec2& scale_);

		[[nodiscard]] glm::vec2 GetScale() const;

		[[nodiscard]] glm::vec2 GetPosNdc(float aspectRatio);

		[[nodiscard]] glm::vec2 GetScaleNdc(float aspectRatio);

	private:
		glm::vec4 m_color = { 1,1,1,1 };
		glm::vec2 m_virtualResolution = { 1920, 1080 };
		glm::vec2 m_pos = { 1,1 };
		glm::vec2 m_scale = { 1,1 };
		bool m_posDirty = true;
		bool m_scaleDirty = true;
		glm::vec2 m_ndcPos = { 1,1 };
		glm::vec2 m_ndcScale = { 1,1 };
	};
}




#endif
