#ifndef QUICKHULL_HPP_
#define QUICKHULL_HPP_
#include "Core/Mesh.h"
#include <vector>
#include <GLM/glm.hpp>
#include "Core/Mesh.h"

struct CreateSphere 
{
public :
	static FDB::Core::SubMesh TransformPointToTriangle(const std::vector<glm::vec3>& pts);
    static FDB::Core::SubMesh applyNoise(const std::vector<glm::vec3>& unitPoints, const std::vector<uint32_t>& idx, float radius, float amplitude, float frequency);




    struct HullFace
    {
        uint32_t  a, b, c;
        glm::vec3 normal;
        float     d;
        bool      alive = true;
    };

};
#endif