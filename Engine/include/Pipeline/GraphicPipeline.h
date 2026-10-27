#ifndef FDBPIPELINEGRAPHICPIPELINE
#define FDBPIPELINEGRAPHICPIPELINE
#include "GLM/glm.hpp"
#include "FrameBuffer.h"
#include "Core/Color.h"
#include "Core/Rect.h"

namespace FDB::Pipeline
{
	struct GraphicsPipeline
	{
		GraphicsPipeline(const glm::vec<2, std::int32_t>& size);
		GraphicsPipeline() = default;
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) noexcept = default;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept = default;
		~GraphicsPipeline() = default;

		void SetSize(const glm::vec<2,std::int32_t>& size);
		void BeginRender(const Color::RGBAFloat& color = Color::alphaFloat, float z = 1.f);
		[[nodiscard]] glm::vec<2, std::int32_t> GetSize() const;
		FrameBuffer GetBuffer();
	private:
		void Resize(const glm::vec<2, std::int32_t>& size);
		buffer_t<glm::vec<4, float>> m_colorFBuffer;
		buffer_t<std::uint32_t> m_colorBuffer;
		buffer_t<float> m_zBuffer;
		glm::vec<2, std::int32_t> m_size ={};
 	};
}

#endif
