#pragma once
#include "GLM/vec4.hpp"
#include "GLM/vec3.hpp"
#include "GLM/mat4x4.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "Pipeline/Transform.h"
#include <vector>
#include "Core/Rect.h"


class InputAssembler {
private:
	std::vector<Transform::Mesh> triangle;

public:
	explicit InputAssembler(const Transform::Mesh& mesh);

	const std::vector<Transform::Mesh>& getTriangles() const
	{
		return triangle;
	}
};


struct ProcessedVertex
{
	glm::vec4 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec4 color;
};


class VertexProcessing
{
public:
	using Triangle = std::vector<Transform::Vertex>;
	using ProcessedTriangle = std::vector<ProcessedVertex>;

	static std::vector<Transform::Vertex> process(std::vector<Transform::Vertex>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	static std::vector<Transform::Vertex> Clipping(std::vector<Transform::Vertex>& processedTriangles);
	static std::vector<Transform::Vertex> perspectiveDivide(std::vector<Transform::Vertex>& processedTriangles);
	static std::vector<Transform::Vertex> viewportTransform(std::vector<Transform::Vertex>& ndcTriangles, float width, float height);
	static std::vector<Transform::Vertex> backFaceCulling(std::vector<Transform::Vertex>& triangles);


};


class Pipeline {
public:

	static std::vector<Transform::Vertex> createSommetList(std::vector<Transform::Vertex>& vertexList, std::vector<unsigned int> indices, FDB::Rect<std::int32_t>& port);
    Pipeline(glm::vec4 position, glm::vec3 colorPixel) : pos(position), color(colorPixel),view(glm::mat4(1.0f)), near(0.1f), far(100.0f), fov(45.0f) {}
	static bool backFaceCulling(glm::vec4 a, glm::vec4 b, glm::vec4 c);

	
	void getView() 
	{
		view = glm::perspective(glm::radians(fov), 800.0f / 600.0f, near, far);
	}
	

private:
	glm::mat4 view;

	glm::vec4 pos;
	glm::vec3 color;

	float near;
	float far;
	float fov;
};

