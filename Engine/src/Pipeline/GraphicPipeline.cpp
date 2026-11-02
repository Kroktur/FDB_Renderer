#include "Pipeline/GraphicPipeline.h"

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

void FDB::Pipeline::GraphicsPipeline::BeginRender(const Color::RGBAFloat& color, float z)
{
	Color::RGBA8888 color8 = Color::ToARGB8888(color);
	m_colorBuffer.Clear(color8.r | (color8.g << 8) | (color8.b << 16) | (color8.a << 24));
	m_pixelBuffer.Clear(Pixel{z,color});
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
