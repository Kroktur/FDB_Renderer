#pragma	once
#include <vector>

#include "Atribute.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

class Transform {
public:
    struct Vertex
    {
        glm::vec4 pos;
        glm::vec<4, float> color;
        static  constexpr  FDB::Pipeline::Attribute::Data PosAttribute()
        {
            return FDB::Pipeline::Attribute::Data{ FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, pos) };
        }
        static constexpr  std::array<FDB::Pipeline::Attribute::Data, 2> AllAttributes()
        {
            return { FDB::Pipeline::Attribute::Data{FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, pos)},FDB::Pipeline::Attribute::Data{FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, color)} };
        }
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
};