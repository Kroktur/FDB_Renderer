#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "../header_file/Pipeline.h"
#include "../header_file/Transform.h"
#include <vector>
#include <iostream>

void Pipeline::createSommetList()
{
	Transform transform;
	Transform::Mesh mesh;
	std::vector<std::vector<Transform::Vertex>> triangleList;

	mesh.vertices.push_back({ glm::vec3(-2.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) });
	
	mesh.vertices.push_back({ glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ glm::vec3(1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) });
	mesh.vertices.push_back({ glm::vec3(2.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) });

	mesh.indices = { 0, 2, 1, 2, 5, 4 };

	if (mesh.vertices.size() % 3 != 0) {
		printf("Error: The number of vertices is not a multiple of 3.\n");
		return;
	}
    InputAssembler inputAssembler(mesh);
    const auto& triangles = inputAssembler.getTriangles();
    std::cout << "Nombre de triangles : " << triangles.size() << '\n';
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        std::cout << "Triangle " << i << '\n';
        for (const auto& vertex : triangles[i])
        {
            std::cout << "  Position : (" << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << ")\n";
        }
    }
}

InputAssembler::InputAssembler(const Transform::Mesh& mesh)
{
    if (!mesh.indices.empty())
    {
        if (mesh.indices.size() % 3 != 0)
        {
            std::cout << "Erreur : nombre d'indices invalide.\n";
            return;
        }

        for (size_t i = 0; i < mesh.indices.size(); i += 3)
        {
            unsigned int a = mesh.indices[i];
            unsigned int b = mesh.indices[i + 1];
            unsigned int c = mesh.indices[i + 2];

            if (a >= mesh.vertices.size() ||
                b >= mesh.vertices.size() ||
                c >= mesh.vertices.size())
            {
                std::cout << "Erreur : index hors limites.\n";
                triangleList.clear();
                return;
            }

            triangleList.push_back({
                mesh.vertices[a],
                mesh.vertices[b],
                mesh.vertices[c]
                });
        }
    }
    else
    {
        if (mesh.vertices.size() % 3 != 0)
        {
            std::cout << "Erreur : nombre de sommets invalide.\n";
            return;
        }

        for (size_t i = 0; i < mesh.vertices.size(); i += 3)
        {
            triangleList.push_back({ mesh.vertices[i], mesh.vertices[i + 1], mesh.vertices[i + 2]});
        }
    }
}
