#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "Pipeline/Pipeline.h"
#include "Pipeline/Transform.h"
#include <vector>
//#include "Core/Rect.h"
#include <iostream>

std::vector<Transform::Vertex> Pipeline::createSommetList(std::vector<Transform::Vertex>& triangles, std::vector<unsigned int> indices, FDB::Rect<std::int32_t>& port)
{

    glm::mat4 model(1.0f); // set model matrice
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); // set the projection
    auto processedTriangles = VertexProcessing::process(triangles, model, view, projection); // place all this value for the vertexProcessing

	auto ndcTriangles = VertexProcessing::perspectiveDivide(processedTriangles);
	auto clippedTriangles = VertexProcessing::Clipping(ndcTriangles);
	auto screenTriangles = VertexProcessing::viewportTransform(clippedTriangles, port.width, port.height);
    std::vector<Transform::Vertex> culledTriangles;
    if (!Pipeline::backFaceCulling(screenTriangles[0].pos, screenTriangles[1].pos, screenTriangles[2].pos)) {
        culledTriangles.push_back(screenTriangles[0]);
        culledTriangles.push_back(screenTriangles[1]);
        culledTriangles.push_back(screenTriangles[2]);
    }

	return culledTriangles;
}

std::vector<Transform::Vertex> VertexProcessing::process(std::vector<Transform::Vertex>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
    for (Transform::Vertex& triangle : triangles) {
        //ProcessedTriangle processedTriangle;
        glm::vec4 position(triangle.pos);
        glm::vec4 worldPosition = model * position;
        glm::vec4 viewPosition = view * worldPosition;
        glm::vec4 clipPosition = projection * viewPosition;
            
        ProcessedVertex processedVertex;

        triangle.pos = clipPosition;
        triangle.color = triangle.color;
    }

    return triangles;
}

std::vector<Transform::Vertex> VertexProcessing::Clipping(std::vector<Transform::Vertex>& processedTriangles)
{
    for (const auto& triangle : processedTriangles) {
         if (triangle.pos.x < -1.0f || triangle.pos.x > 1.0f ||
             triangle.pos.y < -1.0f || triangle.pos.y > 1.0f ||
             triangle.pos.z < -1.0f || triangle.pos.z > 1.0f)
         {
            // std::cout << "Vertex :" << triangle.pos.x << ", " << triangle.pos.y << ", " << triangle.pos.z << " outside clip space.\n";
         }
         else
         {
            // std::cout << "Vertex :" << triangle.pos.x << ", " << triangle.pos.y << ", " << triangle.pos.z << " inside clip space.\n";
         }

    }
    return processedTriangles;
}

std::vector<Transform::Vertex> VertexProcessing::perspectiveDivide(std::vector<Transform::Vertex>& processedTriangles)
{

    for (auto& triangle : processedTriangles) {
        triangle.pos.x /= triangle.pos.w;
        triangle.pos.y /= triangle.pos.w;
        triangle.pos.z /= triangle.pos.w;
	}
    return processedTriangles;
}

std::vector<Transform::Vertex> VertexProcessing::viewportTransform(std::vector<Transform::Vertex>& ndcTriangles, float width, float height)
{

	for (auto& triangle : ndcTriangles) {
        //VertexProcessing::ProcessedTriangle transformedTriangle;
	    // Convert screen coordinates to NDC space
        triangle.pos.x = (triangle.pos.x + 1.0f) * 0.5f * width;
        triangle.pos.y = (1.0f - triangle.pos.y) * 0.5f * height; // Invert Y for screen space
        triangle.pos.z = triangle.pos.z; // Z remains the same
    }
    auto triangles = ndcTriangles[2];
	ndcTriangles[2] = ndcTriangles[1];
	ndcTriangles[1] = triangles;
   
	return ndcTriangles;
}

bool Pipeline::backFaceCulling(glm::vec4 a, glm::vec4 b, glm::vec4 c)
{
    float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    return area > 0.0f;
}