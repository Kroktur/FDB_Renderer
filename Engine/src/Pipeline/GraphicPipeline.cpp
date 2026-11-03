#include "Pipeline/GraphicPipeline.h"

#include <bit>

FDB::Pipeline::GraphicsPipeline::GraphicsPipeline(const glm::vec<2, std::int32_t>& size) : m_size(size)
{
	this->Resize(m_size);
}



void FDB::Pipeline::GraphicsPipeline::SetSize(const glm::vec<2, std::int32_t>& size)
{

	if (m_size == size)
		return;
	Resize(size);

}

void FDB::Pipeline::GraphicsPipeline::BeginRender(const Color::RGBA8888& color, float z)
{
	std::uint32_t zbits = std::bit_cast<std::uint32_t>(z);
	m_colorBuffer.Clear(color.r << 24 | (color.g << 16) | (color.b << 8) | (color.a));
	m_pixelBuffer.Clear( static_cast<std::uint64_t>(zbits) << 32|static_cast<std::uint64_t>(color.r) << 24 | static_cast<std::uint64_t>(color.g) << 16 | static_cast<std::uint64_t>(color.b) << 8 | static_cast<std::uint64_t>(color.a));
}

glm::vec<2, std::int32_t> FDB::Pipeline::GraphicsPipeline::GetSize() const
{
	return m_size;
}



FDB::Pipeline::FrameBuffer FDB::Pipeline::GraphicsPipeline::GetBuffer()
{
	return FrameBuffer{m_colorBuffer.GetView(),m_pixelBuffer.GetView() };
}

void FDB::Pipeline::GraphicsPipeline::Resize(const glm::vec<2, std::int32_t>& size)
{
	m_size = size;
	m_colorBuffer.Resize(m_size);
	m_pixelBuffer.Resize(m_size);
}
