#ifndef RELAXATION
#define RELAXATION
#include <vector>
#include <GLM/vec2.hpp>


class RelaxAlgo {
public:
	static std::vector<glm::vec2> relaxationAlgo(std::vector<glm::vec2> points, int height, int width);
};

#endif // !RELAXATION
