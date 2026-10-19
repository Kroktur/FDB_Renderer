#pragma	once
#include <vector>
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

class Transform {
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 color;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
};