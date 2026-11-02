#ifndef FDBCOREMATERIALF
#define FDBCOREMATERIALF
#include "Core/Texture.h"

namespace FDB::Core
{
	struct Material
	{
		Texture* baseColor = nullptr;
		Texture* OMRMap = nullptr;
		Texture* normalMap = nullptr;
		Texture* emissiveMap = nullptr;
		float emissiveFactor = 3.0f;
	};
}

#endif
