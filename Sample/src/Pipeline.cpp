#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "../header_file/Pipeline.h"
#include "../header_file/Transform.h"
#include <vector>
#include "Core/Rect.h"
#include <iostream>

std::vector<VertexProcessing::ProcessedTriangle> Pipeline::createSommetList(const Transform::Vertex& a, const Transform::Vertex& b, const Transform::Vertex& c, FDB::Rect<std::int32_t> port)
{
	Transform transform;
	Transform::Mesh mesh;
	std::vector<std::vector<Transform::Vertex>> triangleList;

	mesh.vertices.push_back(a);
	mesh.vertices.push_back(b);
	mesh.vertices.push_back(c);
    
    InputAssembler inputAssembler(mesh);
    const auto& triangles = inputAssembler.getTriangles();
    std::cout << "Nombre de triangles : " << triangles.size() << '\n';
    
    for (size_t i = 0; i < triangles.size(); ++i) {
        std::cout << "Triangle " << i << '\n';
        for (const auto& vertex : triangles[i])
        {
            std::cout << "  Position : (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")\n";
        }
    }

    glm::mat4 model(1.0f); // set model matrice
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); // set the projection
    auto processedTriangles = VertexProcessing::process(triangles, model, view, projection); // place all this value for the vertexProcessing

    for (size_t i = 0; i < processedTriangles.size(); ++i) {
        std::cout << "Triangle " << i << '\n';

        for (size_t j = 0; j < processedTriangles[i].size(); ++j) {
            const glm::vec4& p = processedTriangles[i][j].pos;

            std::cout << "  Sommet " << j  << " : (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
        }
    }
	auto clippedTriangles = VertexProcessing::Clipping(processedTriangles);
	auto ndcTriangles = VertexProcessing::perspectiveDivide(clippedTriangles);
	auto screenTriangles = VertexProcessing::viewportTransform(ndcTriangles, port.width, port.height);
	return screenTriangles;
}

std::vector<VertexProcessing::ProcessedTriangle> VertexProcessing::process(const std::vector<Triangle>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    std::vector<ProcessedTriangle> output;

    for (const Triangle& triangle : triangles) {
        ProcessedTriangle processedTriangle;

        for (const Transform::Vertex& vertex : triangle) {
            glm::vec4 position(vertex.pos);
            glm::vec4 worldPosition = model * position;
            glm::vec4 viewPosition = view * worldPosition;
            glm::vec4 clipPosition = projection * viewPosition;
            
            ProcessedVertex processedVertex;

            processedVertex.pos = clipPosition;
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
            const glm::vec4& p = vertex.pos;
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
            glm::vec4& p = vertex.pos;
            if (p.w != 0.0f) {
                p.x /= p.w;
                p.y /= p.w;
                p.z /= p.w;
				std::cout << "Vertex after perspective divide: (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
            }
        }
		output.push_back(triangle);
	}
    return output;
}

std::vector<VertexProcessing::ProcessedTriangle> VertexProcessing::viewportTransform(const std::vector<VertexProcessing::ProcessedTriangle>& ndcTriangles, float width, float height)
{
	std::vector<ProcessedTriangle> output;

	for (const auto& triangle : ndcTriangles) {
        VertexProcessing::ProcessedTriangle transformedTriangle;
        for (const auto& vertex : triangle) {
            ProcessedVertex transformedVertex = vertex;
            // Convert NDC to screen coordinates
            transformedVertex.pos.x = (vertex.pos.x + 1.0f) * 0.5f * width;
            transformedVertex.pos.y = (1.0f - vertex.pos.y) * 0.5f * height; // Invert Y for screen space
            transformedVertex.pos.z = vertex.pos.z; // Z remains the same
            transformedTriangle.push_back(transformedVertex);
			std::cout << "Vertex after viewport transform: (" << transformedVertex.pos.x << ", " << transformedVertex.pos.y << ", " << transformedVertex.pos.z << ", " << transformedVertex.pos.w << ")\n";
        }
        output.push_back(transformedTriangle);
    }
	return output;
}