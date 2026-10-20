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

    InputAssembler inputAssembler(mesh);
    const auto& triangles = inputAssembler.getTriangles();
    std::cout << "Nombre de triangles : " << triangles.size() << '\n';
    
    for (size_t i = 0; i < triangles.size(); ++i) {
        std::cout << "Triangle " << i << '\n';
        for (const auto& vertex : triangles[i])
        {
            std::cout << "  Position : (" << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << ")\n";
        }
    }

    glm::mat4 model(1.0f); // set model matrice
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); // set the projection
    auto processedTriangles = VertexProcessing::process(triangles, model, view, projection); // place all this value for the vertexProcessing

    for (size_t i = 0; i < processedTriangles.size(); ++i) {
        std::cout << "Triangle " << i << '\n';

        for (size_t j = 0; j < processedTriangles[i].size(); ++j) {
            const glm::vec4& p = processedTriangles[i][j].clipPosition;

            std::cout << "  Sommet " << j  << " : (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
        }
    }
	auto clippedTriangles = VertexProcessing::Clipping(processedTriangles);
	auto ndcTriangles = VertexProcessing::perspectiveDivide(clippedTriangles);
}

std::vector<VertexProcessing::ProcessedTriangle> VertexProcessing::process(const std::vector<Triangle>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    std::vector<ProcessedTriangle> output;

    for (const Triangle& triangle : triangles) {
        ProcessedTriangle processedTriangle;

        for (const Transform::Vertex& vertex : triangle) {
            glm::vec4 position(vertex.position,1.0f);
            glm::vec4 worldPosition = model * position;
            glm::vec4 viewPosition = view * worldPosition;
            glm::vec4 clipPosition = projection * viewPosition;
            
            ProcessedVertex processedVertex;

            processedVertex.clipPosition = clipPosition;
            processedVertex.normal = vertex.normal;
            processedVertex.uv = vertex.uv;
            processedVertex.color = vertex.color;

            processedTriangle.push_back(processedVertex);
        }

        output.push_back(processedTriangle);
    }

    return output;
}

std::vector<VertexProcessing::ProcessedTriangle> VertexProcessing::Clipping(const std::vector<VertexProcessing::ProcessedTriangle>& processedTriangles)
{
    std::vector<ProcessedTriangle> output;

    for (const auto& triangle : processedTriangles) {
        for (const auto& vertex : triangle) {
            const glm::vec4& p = vertex.clipPosition;
            // Check if the vertex is inside the clip space
            if (p.x < -p.w || p.x > p.w || p.y < -p.w || p.y > p.w || p.z < -p.w || p.z > p.w) {
                std::cout << "Vertex (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ") is outside the clip space.\n";
            } else {
                std::cout << "Vertex (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ") is inside the clip space.\n";
            }
        }
		output.push_back(triangle); // For now, we just pass the triangle through without clipping
    }
    return output;
}

std::vector<VertexProcessing::ProcessedTriangle> VertexProcessing::perspectiveDivide(std::vector<VertexProcessing::ProcessedTriangle>& processedTriangles)
{
    std::vector<ProcessedTriangle> output;

    for (auto& triangle : processedTriangles) {
        for (auto& vertex : triangle) {
            glm::vec4& p = vertex.clipPosition;
            if (p.w != 0.0f) {
                p.x /= p.w;
                p.y /= p.w;
                p.z /= p.w;
                p.w = 1.0f;
				std::cout << "Vertex after perspective divide: (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
            }
        }
		output.push_back(triangle);
	}
    return output;
}