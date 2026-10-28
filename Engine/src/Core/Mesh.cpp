#include "Core/Mesh.h"

#include <filesystem>
#include <stdexcept>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Core/LoadObj.h"
namespace FDB::Core
{


    struct VertexKey
    {
        int v, n, t;
        bool operator==(const VertexKey& o) const
        {
            return v == o.v && n == o.n && t == o.t;
        }
    };

    struct VertexKeyHash
    {
        static std::size_t operator()(const VertexKey& k) 
        {
            std::size_t h = std::hash<int>()(k.v);
            h ^= std::hash<int>()(k.n) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(k.t) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    struct TangentData
    {
        glm::vec3 tangent = glm::vec3(0.f);
        glm::vec3 bitangent = glm::vec3(0.f);
    };

    void ComputeTangent(Mesh& m)
    {
        for (auto& sub : m.subMeshes)
        {
            std::vector<TangentData> tangents;
            tangents.resize(sub.vertices.size());
            for (std::size_t i = 0; i < sub.indices.size(); i += 3)
            {
                std::size_t i0 = sub.indices[i + 0];
                std::size_t i1 = sub.indices[i + 1];
                std::size_t i2 = sub.indices[i + 2];

                glm::vec3 pos0 = sub.vertices[i0].pos;
                glm::vec3 pos1 = sub.vertices[i1].pos;
                glm::vec3 pos2 = sub.vertices[i2].pos;


                glm::vec2 uv0 = sub.vertices[i0].uv;
                glm::vec2 uv1 = sub.vertices[i1].uv;
                glm::vec2 uv2 = sub.vertices[i2].uv;


                glm::vec2 duv1 = uv1 - uv0;
                glm::vec2 duv2 = uv2 - uv0;

                glm::vec3 dpos1 = pos1 - pos0;
                glm::vec3 dpos2 = pos2 - pos0;

                float det = duv1.x * duv2.y - duv1.y * duv2.x;
                if (std::abs(det) < 1e-8f)
                    continue;

                float invDet = 1.f / det;
                glm::vec3 t = glm::vec3((dpos1 * duv2.y - dpos2 * duv1.y) * invDet);
                glm::vec3 b = glm::vec3((dpos2 * duv1.x - dpos1 * duv2.x) * invDet);


                tangents[i0].tangent += t;
                tangents[i1].tangent += t;
                tangents[i2].tangent += t;

                tangents[i0].bitangent += b;
                tangents[i1].bitangent += b;
                tangents[i2].bitangent += b;

            }
            for (std::size_t i = 0; i < sub.vertices.size(); ++i)
            {
                glm::vec3 t = tangents[i].tangent;
                glm::vec3 b = tangents[i].bitangent;
                glm::vec3 n = sub.vertices[i].normal;

                t = t - n * glm::dot(n, t);

                if (glm::length2(t) < 0.001f)
                {
                    if (glm::abs(n.x) < 0.9f)
                        t = glm::normalize(glm::cross(n, glm::vec3(1.0f, 0.0f, 0.0f)));
                    else
                        t = glm::normalize(glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f)));
                }
                t = glm::normalize(t);
                sub.vertices[i].tangent = t;
                float handedness = glm::dot(glm::cross(t, b), n) > 0.f ? 1.f : -1.f;
                sub.vertices[i].bitangent = glm::cross(n, t) * handedness;
            }

        }
    }

	std::unique_ptr<Mesh> MeshTools::LoadMesh(const std::filesystem::path& ObjFile)
	{

        std::unique_ptr<FDB::Core::Mesh> Mesh = std::make_unique<FDB::Core::Mesh>();
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(ObjFile.string(), config))
            throw std::runtime_error(reader.Error());

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();

        for (const auto& shape : shapes)
        {
            std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> uniqueVertices;
            FDB::Core::SubMesh SubMesh;
            for (const auto& idx : shape.mesh.indices)
            {
                VertexKey key{ idx.vertex_index, idx.normal_index, idx.texcoord_index };

                auto it = uniqueVertices.find(key);
                if (it == uniqueVertices.end())
                {
                    FDB::Core::Vertex vertex{};

                    if (attrib.colors.empty()) {
                        vertex.color = glm::vec4(1.0f);
                    }
                    else {
                        vertex.color = glm::vec4(attrib.colors[3 * idx.vertex_index + 0], attrib.colors[3 * idx.vertex_index + 1], attrib.colors[3 * idx.vertex_index + 2], 1.0f);
                    }

                    vertex.pos = glm::vec4(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2], 1.0f);

                    if (idx.normal_index >= 0)
                    {
                        vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                        vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                        vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        vertex.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                        vertex.uv.y = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                        //vertex.uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    std::uint32_t newIndex = static_cast<std::uint32_t>(SubMesh.vertices.size());
                    uniqueVertices[key] = newIndex;
                    SubMesh.vertices.push_back(vertex);
                    SubMesh.indices.push_back(newIndex);
                }
                else
                {
                    SubMesh.indices.push_back(it->second);
                }
            }
            Mesh->subMeshes.push_back(SubMesh);
        }
        ComputeTangent(*Mesh.get());
        return Mesh;
	}



   
}
