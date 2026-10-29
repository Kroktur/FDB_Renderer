#include <vector>
#include <cmath>
#include <GLM/glm.hpp>
#include <limits>
#include "Core/Relaxation.h"

std::vector<glm::vec2> RelaxAlgo::relaxationAlgo(std::vector<glm::vec2> points, int height, int width)
{
	std::vector<glm::vec2> newPoint = points;

	auto findClosestInAllDirections = [](const glm::vec2& p, const glm::vec2& candidate, const glm::vec2& domainSize)
	{
		glm::vec2 result;
		float     dist = FLT_MAX;

		for (int y = -1; y <= 1; ++y)
		{
			float dOfsY = float(y) * domainSize.y;
			for (int x = -1; x <= 1; ++x)
			{
				float dOfsX = float(x) * domainSize.x;
				glm::vec2 repeatedDomainCandidate = candidate + glm::vec2(dOfsX, dOfsY);

				float d = glm::distance(p, repeatedDomainCandidate);
				if (d < dist)
				{
					dist = d;
					result = repeatedDomainCandidate;
				}
			}
		}

		return std::make_pair(result, dist);
	};

	for (size_t x = 0; x < points.size(); x++) {
		glm::vec2 totalForce(0.0f);

		for (size_t y = 0; y < points.size(); y++) {
			if (x == y)
				continue;

			auto [p, dist] = findClosestInAllDirections(points[x], points[y], { 800, 600 });

			glm::vec2 dir  = glm::normalize(points[x] - p);

			totalForce += dir / (dist * dist * 0.01f);
		}

		newPoint[x] = points[x] + totalForce * 0.2f;
	}

	return newPoint;
}