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
	std::vector<std::vector<Transform::Vertex>> triangleList;

public:
	explicit InputAssembler(const Transform::Mesh& mesh);

	const std::vector<std::vector<Transform::Vertex>>& getTriangles() const
	{
		return triangleList;
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

	static std::vector<ProcessedTriangle> process(const std::vector<Triangle>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	static std::vector<VertexProcessing::ProcessedTriangle> Clipping(const std::vector<VertexProcessing::ProcessedTriangle>& processedTriangles);
	static std::vector<VertexProcessing::ProcessedTriangle> perspectiveDivide(std::vector<VertexProcessing::ProcessedTriangle>& processedTriangles);
	static std::vector<VertexProcessing::ProcessedTriangle> viewportTransform(const std::vector<VertexProcessing::ProcessedTriangle>& ndcTriangles, float width, float height);


};


class Pipeline {
public:
	static std::vector<VertexProcessing::ProcessedTriangle> createSommetList(const Transform::Vertex& a, const Transform::Vertex& b, const Transform::Vertex& c, FDB::Rect<std::int32_t>& port);
	Pipeline(glm::vec4 position, glm::vec3 colorPixel) : pos(position), color(colorPixel),view(glm::mat4(1.0f)), near(0.1f), far(100.0f), fov(45.0f) {}
	
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

