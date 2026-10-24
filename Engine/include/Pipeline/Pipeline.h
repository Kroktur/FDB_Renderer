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

	static void process(std::vector<Transform::Vertex>& triangles, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	static void perspectiveDivide(std::vector<Transform::Vertex>& processedTriangles);
	static void backFaceCulling(std::vector<Transform::Vertex>& triangles);
	static void Clipping(std::vector<Transform::Vertex>& processedTriangles, std::function<void(Transform::Vertex, Transform::Vertex, Transform::Vertex)> callback, FDB::Rect<std::int32_t>& port);
	static void viewportTransform(std::vector<Transform::Vertex>& ndcTriangles, float width, float height);


};


class Pipeline {
public:

	static void createSommetList(std::vector<Transform::Vertex>& triangles, FDB::Rect<std::int32_t>& port, std::function<void(Transform::Vertex, Transform::Vertex, Transform::Vertex)> callback);
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

