#ifndef FDBFRAMEBUFFERF
#define FDBFRAMEBUFFERF
#include "Buffer.h"

namespace FDB::Pipeline 
{
	struct FrameBuffer
	{
		buffer_view_t<glm::vec<4, float>> colorFBuffer;
		buffer_view_t<std::uint32_t> colorBuffer;
		buffer_view_t<float> zBuffer;
	};
}

#endif
