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
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec<4, float> color;
        static  constexpr  FDB::Pipeline::Data PosAttribute()
        {
            return FDB::Pipeline::Data{ FDB::Pipeline::format::Float4,offsetof(Vertex, pos) };
        }
        static constexpr  std::array<FDB::Pipeline::Data, 4> AllAttributes()
        {
            return { FDB::Pipeline::Data{FDB::Pipeline::format::Float4,offsetof(Vertex, pos)},
                FDB::Pipeline::Data{FDB::Pipeline::format::Float3,offsetof(Vertex, normal)},
                FDB::Pipeline::Data{FDB::Pipeline::format::Float2,offsetof(Vertex, uv)},
                FDB::Pipeline::Data{FDB::Pipeline::format::Float4,offsetof(Vertex, color)}

            };
        }
    };
    struct SubMesh
    {
        SubMesh() = default;
        SubMesh(SubMesh&&) noexcept = default;
        SubMesh& operator=(SubMesh&&) noexcept = default;
        SubMesh(const SubMesh&) = default;
        SubMesh& operator=(const SubMesh&) = default;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
    struct Mesh
    {

        Mesh() = default;
        Mesh(Mesh&&) noexcept = default;
        Mesh& operator=(Mesh&&) noexcept = default;
        Mesh(const Mesh&) = default;
        Mesh& operator=(const Mesh&) = default;
        std::vector<SubMesh> subMeshes;
    };
};