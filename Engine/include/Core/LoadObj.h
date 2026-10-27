#ifndef LOADOBJ
#define LOADOBJ
#include "Pipeline/Transform.h"
#include <string>
#include <vector>

struct LoadObjFile
{
public:
	static void LoadObj(std::vector<Transform::Vertex>& vertexList, std::vector<std::uint32_t> &indices, std::string ObjFile, std::string exePath);
    struct VertexKey
    {
        int v, n, t;
        bool operator==(const VertexKey& o) const
        {
            return v == o.v && n == o.n && t == o.t;
        }
    };

    struct VertexKeyHash
    {
        std::size_t operator()(const VertexKey& k) const
        {
            std::size_t h = std::hash<int>()(k.v);
            h ^= std::hash<int>()(k.n) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(k.t) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

};

#endif // !1
