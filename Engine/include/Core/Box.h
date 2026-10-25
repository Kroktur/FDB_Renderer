#ifndef FDBCOREBOXF
#define FDBCOREBOXF
#include <GLM/detail/type_vec1.hpp>
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>


template<typename T, size_t N> requires(N == 2 || N == 3)
struct Box
{
	using value_type = T;
	glm::vec<N, value_type> min;
	glm::vec<N, value_type> max;
};
template<typename T>
using box2_t = Box<T, 2>;
template<typename T>
using box3_t = Box<T, 3>;


#endif
