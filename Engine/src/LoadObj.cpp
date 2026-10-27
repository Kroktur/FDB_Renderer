
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Pipeline/Transform.h"
#include "Core/LoadObj.h"
#include <iostream>
#include <unordered_map>
#include <functional>

void LoadObjFile::LoadObj(std::vector<Transform::Vertex>& vertexList, std::vector<std::uint32_t>& indices, std::string ObjFile, std::string exePath)
{
    vertexList.clear();
    indices.clear();

    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = exePath;
    config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(ObjFile, config))
        throw std::runtime_error(reader.Error());

    if (!reader.Warning().empty())
        std::cout << reader.Warning();

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> uniqueVertices;

    for (const auto& shape : shapes)
    {
        for (const auto& idx : shape.mesh.indices)
        {
            VertexKey key{ idx.vertex_index, idx.normal_index, idx.texcoord_index };

            auto it = uniqueVertices.find(key);
            if (it == uniqueVertices.end())
            {
                Transform::Vertex vertex{};

                if (attrib.colors.empty()) {
                    vertex.color = glm::vec4(1.0f);
                }
                else {
                    vertex.color = glm::vec4(attrib.colors[3 * idx.vertex_index + 0], attrib.colors[3 * idx.vertex_index + 1], attrib.colors[3 * idx.vertex_index + 2], 1.0f);
                }

                vertex.pos = glm::vec4( attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2], 1.0f);

                if (idx.normal_index >= 0)
                {
                    vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                    vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                    vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                }

                if (idx.texcoord_index >= 0)
                {
                    vertex.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                    // si ta texture à l'envers (Vulkan) :
                     vertex.uv.y = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                    // si la texture à l'endroit
                    //vertex.uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];
                }

                std::uint32_t newIndex = static_cast<std::uint32_t>(vertexList.size());
                uniqueVertices[key] = newIndex;
                vertexList.push_back(vertex);
                indices.push_back(newIndex);
            }
            else
            {
                indices.push_back(it->second);
            }
        }
    }
}