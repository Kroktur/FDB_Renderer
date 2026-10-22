#pragma	once
#include <vector>
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

class Transform {
public:
    struct Vertex
    {
        glm::vec4 pos;
        glm::vec<4, float> color;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
};