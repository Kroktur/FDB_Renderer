#include "Core/Mesh.h"

#include <filesystem>
#include <stdexcept>
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
        return Mesh;
	}
}
