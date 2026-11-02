#ifndef FDBFRAMEBUFFERF
#define FDBFRAMEBUFFERF
#include <atomic>

#include "Buffer.h"

namespace FDB::Pipeline 
{
	struct Pixel
	{
		Pixel(float z,const glm::vec4 color_) : z(z),color(color_){}
		Pixel() = default;
		Pixel(const Pixel&) = default;
		Pixel(Pixel&&) noexcept = default;
		Pixel& operator=(const Pixel&) = default;
		Pixel& operator=(Pixel&&) noexcept = default;
		~Pixel() = default;
		float z = 0;
		glm::vec4 color= glm::vec4(0,0,0,1);
	};

	using atomicPixel_t = std::atomic<Pixel>;

	struct FrameBuffer
	{
		buffer_view_t<std::uint32_t> colorBuffer;
		buffer_view_t<atomicPixel_t> atomicBuffer;
	};
}

#endif
