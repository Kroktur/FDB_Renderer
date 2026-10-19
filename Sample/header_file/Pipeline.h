#pragma once
#include "GLM/vec4.hpp"
#include "GLM/vec3.hpp"
#include "GLM/mat4x4.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "../header_file/Transform.h"
#include <vector>

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

class Pipeline {
public:
	static void createSommetList();
	Pipeline(glm::vec4 position, glm::vec3 colorPixel) : pos(position), color(colorPixel),view(glm::mat4(1.0f)), near(0.1f), far(100.0f), fov(45.0f) {}
	
	void getView() 
	{
		view = glm::perspective(glm::radians(fov), 800.0f / 600.0f, near, far);
	}

	void getClip()
	{
		// This function can be used to get the clip space transformation if needed
	}

	

private:
	glm::mat4 view;
	glm::vec4 pos;
	glm::vec3 color;
	float near;
	float far;
	float fov;
};