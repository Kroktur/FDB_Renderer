#ifndef FDBRENDERUIFRAGMENTF
#define FDBRENDERUIFRAGMENTF
#include "Pipeline/Atribute.h"

namespace FDB::Core
{
	struct VSOutput
	{
		glm::vec4 pos;
		glm::vec2  uv;
		glm::vec2 uv2;
		glm::vec4 color;
		[[nodiscard]] static  constexpr  FDB::Pipeline::Attribute PosAttribute()
		{
			return FDB::Pipeline::Attribute{ FDB::Pipeline::format::Float4,offsetof(VSOutput, pos) };
		}
		[[nodiscard]] static constexpr  std::array<FDB::Pipeline::Attribute, 4> AllAttributes()
		{
			return { FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(VSOutput, pos)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VSOutput, uv)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(VSOutput, uv2)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(VSOutput, color)}
			};
		}
	};
}

#endif
