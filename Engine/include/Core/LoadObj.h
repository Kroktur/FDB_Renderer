#ifndef LOADOBJ
#define LOADOBJ
#include "Pipeline/Transform.h"
#include <string>
#include <vector>

struct LoadObjFile
{
public:
	static void LoadObj(std::vector<Transform::Vertex>& vertexList, std::vector<std::uint32_t> &indices, std::string ObjFile, std::string exePath);

};

#endif // !1
