#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "Pipeline/Pipeline.h"
#include "Pipeline/Transform.h"
#include <vector>
//#include "Core/Rect.h"
#include <iostream>

void Pipeline::createSommetList(std::vector<Transform::Vertex>& triangles, FDB::Rect<std::int32_t>& port, std::function<void(Transform::Vertex, Transform::Vertex, Transform::Vertex)> callback)
{

    glm::mat4 model(1.0f); // set model matrice
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // set the view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10000.0f); // set the projection

    VertexProcessing::process(triangles, model, view, projection); // place all this value for the vertexProcessing
	VertexProcessing::Clipping(triangles, callback, port);
    std::vector<Transform::Vertex> culledTriangles;
}

void VertexProcessing::process(std::vector<Transform::Vertex>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
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

}

void VertexProcessing::Clipping(std::vector<Transform::Vertex>& processedTriangles, std::function<void(Transform::Vertex, Transform::Vertex, Transform::Vertex)> callback, FDB::Rect<std::int32_t>& port)
{
    auto planeDistance = [](const Transform::Vertex& v, int plane) -> float
        {
            switch (plane)
            {
            case 0: return v.pos.x + v.pos.w;
            case 1: return v.pos.w - v.pos.x;
            case 2: return v.pos.y + v.pos.w;
            case 3: return v.pos.w - v.pos.y;
            case 4: return v.pos.z;
            case 5: return v.pos.w - v.pos.z;
            default: return -1.0f;
            }
        };

    auto intersect = [planeDistance](const Transform::Vertex& A, const Transform::Vertex& B, int plane) -> Transform::Vertex {
            float dA = planeDistance(A, plane);
            float dB = planeDistance(B, plane);

            float t = dA / (dA - dB);

            Transform::Vertex I;
            I.pos = A.pos + t * (B.pos - A.pos);
            I.color = A.color + t * (B.color - A.color);

            return I;
        };
    for (std::size_t i = 0; i + 2 < processedTriangles.size(); i += 3)
    {
        std::vector<Transform::Vertex> polygon = { processedTriangles[i], processedTriangles[i + 1], processedTriangles[i + 2]};

        for (int plane = 0; plane < 6; ++plane)
        {
            if (polygon.empty())
                break;
            std::vector<Transform::Vertex> clippedPolygon;
            Transform::Vertex previous = polygon.back();
            float previousDistance = planeDistance(previous, plane);
            bool previousInside = previousDistance >= 0.0f;
            for (const Transform::Vertex& current : polygon)
            {
                float currentDistance = planeDistance(current, plane);
                bool currentInside = currentDistance >= 0.0f;

                if (previousInside && currentInside)
                {
                    clippedPolygon.push_back(current);
                }
                else if (previousInside && !currentInside)
                {
                    clippedPolygon.push_back(intersect(previous, current, plane));
                }
                else if (!previousInside && currentInside)
                {
                    clippedPolygon.push_back(intersect(previous, current, plane));
                    clippedPolygon.push_back(current);
                }
                previous = current;
                previousDistance = currentDistance;
                previousInside = currentInside;
            }
            polygon = std::move(clippedPolygon);
        }

        for (std::size_t j = 1; j + 1 < polygon.size(); ++j)
        {
            std::vector<Transform::Vertex> triangle = {polygon[0], polygon[j], polygon[j + 1]};
            perspectiveDivide(triangle);
            viewportTransform(triangle, port.width, port.height);
            if (!Pipeline::backFaceCulling(triangle[0].pos, triangle[1].pos, triangle[2].pos)) {
                continue;
            }
            callback(triangle[0], triangle[1], triangle[2]);
        }
    }
}

void VertexProcessing::perspectiveDivide(std::vector<Transform::Vertex>& processedTriangles)
{
	if (processedTriangles.empty()) {
        return;
    }

    for (auto& triangle : processedTriangles) {
        if (triangle.pos.w == 0.0f) {
            std::cerr << "Warning: w component is zero during perspective divide." << std::endl;
            continue;
		}
        triangle.pos.x /= triangle.pos.w;
        triangle.pos.y /= triangle.pos.w;
        triangle.pos.z /= triangle.pos.w;
	}
}

void VertexProcessing::viewportTransform(std::vector<Transform::Vertex>& ndcTriangles, float width, float height)
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
}

bool Pipeline::backFaceCulling(glm::vec4 a, glm::vec4 b, glm::vec4 c)
{
    bool isBackFace = false;
    float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (area <= 0.0f) {
        isBackFace = true;
    }
    return isBackFace;
}