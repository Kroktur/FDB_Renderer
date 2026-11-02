#ifndef RELAXATION
#define RELAXATION
#include <vector>
#include <GLM/vec2.hpp>


class RelaxAlgo {
public:
	static std::vector<glm::vec3> relaxationAlgo(std::vector<glm::vec3> points, float& totalRelaxation);
	static std::vector<glm::vec3> createSphere();

};

#endif // !RELAXATION
