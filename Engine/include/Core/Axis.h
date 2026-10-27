#ifndef FDBCOREAXISF
#define FDBCOREAXISF
#include <GLM/vec3.hpp>

namespace FDB::Core
{
	struct Axis
	{
		static constexpr glm::vec<3, float> left = { -1,0,0 };
		static constexpr glm::vec<3, float> right = { 1,0,0 };
		static constexpr glm::vec<3, float> up = { 0,1,0 };
		static constexpr glm::vec<3, float> down = { 0,-1,0 };
		static constexpr glm::vec<3, float> forward = { 0,0,-1 };
		static constexpr glm::vec<3, float> backward = { 0,0,1 };
	};
}



#endif
