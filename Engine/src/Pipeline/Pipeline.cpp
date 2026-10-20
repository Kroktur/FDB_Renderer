#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "Pipeline/Pipeline.h"
#include "Pipeline/Transform.h"
#include <vector>
//#include "Core/Rect.h"
#include <iostream>

std::vector<Transform::Mesh> Pipeline::createSommetList(std::vector<Transform::Mesh>& triangles, std::vector<unsigned int> indices, FDB::Rect<std::int32_t>& port)
{

    //std::cout << "Nombre de triangles : " << triangles.size() << '\n';
    
    //for (size_t i = 0; i < triangles.size(); ++i) {
    //    //std::cout << "Triangle " << i << '\n';
    //    for (const auto& vertex : triangles[i])
    //    {
    //        //std::cout << "  Position : (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")\n";
    //    }
    //}

    glm::mat4 model(1.0f); // set model matrice
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); // set the projection
    auto processedTriangles = VertexProcessing::process(triangles, model, view, projection); // place all this value for the vertexProcessing

    //for (size_t i = 0; i < processedTriangles.size(); ++i) {
    //    //std::cout << "Triangle " << i << '\n';

    //    for (size_t j = 0; j < processedTriangles[i].size(); ++j) {
    //        const glm::vec4& p = processedTriangles[i][j].pos;

    //        //std::cout << "  Sommet " << j  << " : (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
    //    }
    //}
	auto clippedTriangles = VertexProcessing::Clipping(processedTriangles);
	auto ndcTriangles = VertexProcessing::perspectiveDivide(clippedTriangles);
	auto screenTriangles = VertexProcessing::viewportTransform(ndcTriangles, port.width, port.height);
    std::vector<Transform::Mesh> culledTriangles;
    for (const auto& triangle : screenTriangles) {
        if (!Pipeline::backFaceCulling(triangle.vertices[0].pos, triangle.vertices[1].pos, triangle.vertices[2].pos)) {
            culledTriangles.push_back(triangle);
        }
    }

	return culledTriangles;
}

std::vector<Transform::Mesh> VertexProcessing::process(std::vector<Transform::Mesh>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    std::vector<Transform::Mesh> output;

    for (Transform::Mesh& triangle : triangles) {
        ProcessedTriangle processedTriangle;
        for (auto& vertex : triangle.vertices) {
            glm::vec4 position(vertex.pos);
            glm::vec4 worldPosition = model * position;
            glm::vec4 viewPosition = view * worldPosition;
            glm::vec4 clipPosition = projection * viewPosition;
            
            ProcessedVertex processedVertex;

            vertex.pos = clipPosition;
            vertex.color = vertex.color;
        }

        output.push_back(triangle);
    }

    return output;
}

std::vector<Transform::Mesh> VertexProcessing::Clipping(std::vector<Transform::Mesh>& processedTriangles)
{
    std::vector<Transform::Mesh> output;

    for (const auto& triangle : processedTriangles) {
        for (const auto& vertex : triangle.vertices) {
            const glm::vec4& p = vertex.pos;
            if (p.x < -p.w || p.x > p.w || p.y < -p.w || p.y > p.w || p.z < -p.w || p.z > p.w) {
                //std::cout << "Vertex (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ") is outside the clip space.\n";
            } else {
                //std::cout << "Vertex (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ") is inside the clip space.\n";
            }
        }
		output.push_back(triangle);
    }
    return output;
}

std::vector<Transform::Mesh> VertexProcessing::perspectiveDivide(std::vector<Transform::Mesh>& processedTriangles)
{
    std::vector<Transform::Mesh> output;

    for (auto& triangle : processedTriangles) {
        for (auto& vertex : triangle.vertices) {
            glm::vec4& p = vertex.pos;
            if (p.w != 0.0f) {
                p.x /= p.w;
                p.y /= p.w;
                p.z /= p.w;
				//std::cout << "Vertex after perspective divide: (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")\n";
            }
        }
		output.push_back(triangle);
	}
    return output;
}

std::vector<Transform::Mesh> VertexProcessing::viewportTransform(std::vector<Transform::Mesh>& ndcTriangles, float width, float height)
{
    std::vector<Transform::Mesh> output;

	for (auto& triangle : ndcTriangles) {
        VertexProcessing::ProcessedTriangle transformedTriangle;
        for (auto& vertex : triangle.vertices) {
            // Convert NDC to screen coordinates
            vertex.pos.x = (vertex.pos.x + 1.0f) * 0.5f * width;
            vertex.pos.y = (1.0f - vertex.pos.y) * 0.5f * height; // Invert Y for screen space
            vertex.pos.z = vertex.pos.z; // Z remains the same
        }
        output.push_back(triangle);
    }
	return output;
}

bool Pipeline::backFaceCulling(glm::vec4 a, glm::vec4 b, glm::vec4 c)
{
    float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    return area < 0.0f;
}