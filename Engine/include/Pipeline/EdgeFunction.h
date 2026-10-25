#ifndef FDBPIPELINEEDGEFUNCTIONF
#define FDBPIPELINEEDGEFUNCTIONF
#include <GLM/vec4.hpp>

namespace FDB::Pipeline
{
	template<typename T> requires(std::is_arithmetic_v<T>)
	struct EdgeFunction
	{
		using value_type = T;
		static value_type Compute(const glm::vec<2, value_type>& A, const glm::vec<2, value_type>& B, const glm::vec<2, value_type>& D)
		{
			return (D.x - A.x) * (B.y - A.y) - (D.y - A.y) * (B.x - A.x);
		}
	};
}


#endif
