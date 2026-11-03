#ifndef FDBFRAMEBUFFERF
#define FDBFRAMEBUFFERF
#include <atomic>

#include "Buffer.h"

namespace FDB::Pipeline 
{
	using pixel = std::uint64_t;
	

	using atomicPixel_t = pixel;

	struct FrameBuffer
	{
		buffer_view_t<std::uint32_t> colorBuffer;
		buffer_view_t<atomicPixel_t> atomicBuffer;
	};
}

#endif
