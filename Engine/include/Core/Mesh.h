#ifndef FDBCOREMESHF
#define FDBCOREMESHF
#include <filesystem>
#include <memory>
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>

#include "Material.h"
#include "ResourceManager.h"
#include "Pipeline/Atribute.h"

namespace FDB::Core
{
    struct Vertex
    {
        glm::vec4 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec<4, float> color;
        glm::vec4 tangent;
     [[nodiscard]]   static  constexpr  FDB::Pipeline::Attribute PosAttribute()
        {
            return FDB::Pipeline::Attribute{ FDB::Pipeline::format::Float4,offsetof(Vertex, pos) };
        }
     [[nodiscard]]   static constexpr  std::array<FDB::Pipeline::Attribute, 5> AllAttributes()
        {
            return { FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(Vertex, pos)},
                FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(Vertex, normal)},
                FDB::Pipeline::Attribute{FDB::Pipeline::format::Float2,offsetof(Vertex, uv)},
                   FDB::Pipeline::Attribute{FDB::Pipeline::format::Float3,offsetof(Vertex, tangent)},
                FDB::Pipeline::Attribute{FDB::Pipeline::format::Float4,offsetof(Vertex, color)}
            };
        }
    };
    using vertex_t = Vertex;
    struct SubMesh
    {
        SubMesh() = default;
        SubMesh(SubMesh&&) noexcept = default;
        SubMesh& operator=(SubMesh&&) noexcept = default;
        SubMesh(const SubMesh&) = default;
        SubMesh& operator=(const SubMesh&) = default;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Material material;
    };
    using subMesh_t = SubMesh;
    struct Mesh
    {

        Mesh() = default;
        Mesh(Mesh&&) noexcept = default;
        Mesh& operator=(Mesh&&) noexcept = default;
        Mesh(const Mesh&) = default;
        Mesh& operator=(const Mesh&) = default;
        std::vector<SubMesh> subMeshes;
    };
    using mesh_t = Mesh;

    struct MeshTools
    {
       [[nodiscard]] static std::unique_ptr<Mesh> LoadMesh(const std::filesystem::path& ObjFile);
       static void ComputeTangent(Mesh& mesh);
    };

    using meshLoader_t = resourceManager_t< FDB::Core::Mesh, TypeWrapper<>, FDB::Core::MeshTools::LoadMesh>;

}


#endif
