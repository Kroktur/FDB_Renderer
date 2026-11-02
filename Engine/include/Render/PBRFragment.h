#ifndef FDBRENDERBBRFRAGMENTF
#define FDBRENDERBBRFRAGMENTF
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>

#include "Pipeline/Atribute.h"

namespace FDB::Render
{
	struct PBRFragment
	{
		glm::vec4 pos;
		glm::vec3 normal;
		glm::vec3 worldSpacePosition;
		glm::vec3 eyePos;
		glm::vec2 fragTexCord;
		glm::vec4 fragColor;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		[[nodiscard]] static  constexpr  FDB::Pipeline::Attribute PosAttribute()
		{
			return FDB::Pipeline::Attribute{ FDB::Pipeline::format::Float4,offsetof(PBRFragment, pos) };
		}
		[[nodiscard]] static constexpr  std::array<FDB::Pipeline::Attribute, 8> AllAttributes()
		{
			return { FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(PBRFragment, pos)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(PBRFragment, normal)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(PBRFragment, worldSpacePosition)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(PBRFragment, eyePos)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(PBRFragment, fragTexCord)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(PBRFragment, fragColor)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(PBRFragment, tangent)},
				FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(PBRFragment, bitangent)}
			};
		}

	};
}


#endif
