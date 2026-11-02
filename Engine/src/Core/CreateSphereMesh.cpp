#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <utility>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include "Core/CreateSphere.h"
#include "Core/Mesh.h"

FDB::Core::SubMesh CreateSphere::TransformPointToTriangle(const std::vector<glm::vec3>& pts)
{
    const int k = 12;
    const uint32_t n = (uint32_t)pts.size();

    // points unitaires : les seuils numériques sont prévus pour un rayon de 1
    std::vector<glm::vec3> p(n);
    for (uint32_t i = 0; i < n; ++i) 
        p[i] = glm::normalize(pts[i]);

    FDB::Core::SubMesh subMesh;
    std::set<std::array<uint32_t, 3>> seen;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < n; ++i)
    {
        std::vector<uint32_t> nb;
        for (uint32_t j = 0; j < n; ++j) 
            if (j != i) 
                nb.push_back(j);
        int kk = std::min<int>(k, (int)nb.size());
        std::partial_sort(nb.begin(), nb.begin() + kk, nb.end(), [&](uint32_t a, uint32_t b) {
            return glm::distance2(p[a], p[i]) < glm::distance2(p[b], p[i]);
            });
        nb.resize(kk);

        for (int x = 0; x < kk; ++x)
            for (int y = x + 1; y < kk; ++y)
            {
                uint32_t a = nb[x], b = nb[y];

                glm::vec3 nrm = glm::cross(p[a] - p[i], p[b] - p[i]);
                if (glm::length2(nrm) < 1e-12f) continue;
                nrm = glm::normalize(nrm);

                if (glm::dot(nrm, p[i]) < 0.0f) { 
                    nrm = -nrm; std::swap(a, b); 
                }
                float d = glm::dot(nrm, p[i]);

                bool ok = true;
                for (uint32_t m = 0; m < n && ok; ++m)
                    if (m != i && m != a && m != b && glm::dot(nrm, p[m]) > d + 1e-5f)
                        ok = false;
                if (!ok) 
                    continue;

                std::array<uint32_t, 3> key = { i, a, b };
                std::sort(key.begin(), key.end());
                if (seen.insert(key).second)
                    indices.insert(indices.end(), { i, a, b });
            }
    }

    subMesh.indices = std::move(indices);
    return subMesh;
}

static glm::vec2 sphereUV(const glm::vec3& n)
{
    float u = 0.5f + glm::atan(n.z, n.x) / (2.0f * glm::pi<float>());
    float v = 0.5f - glm::asin(glm::clamp(n.y, -1.0f, 1.0f)) / glm::pi<float>();
    return { u, v };
}

// bruit fractal : plusieurs couches de Perlin, de plus en plus fines et faibles
static float fbm(const glm::vec3& p, float frequency, int octaves = 4)
{
    float sum = 0.0f;
    float amp = 1.0f;
    float norm = 0.0f;

    for (int o = 0; o < octaves; ++o)
    {
        sum += amp * glm::perlin(p * frequency);
        norm += amp;
        amp *= 0.5f;        // chaque couche pèse moitié moins
        frequency *= 2.0f;  // et est deux fois plus détaillée
    }
    return sum / norm;      // reste dans [-1, 1]
}

FDB::Core::SubMesh CreateSphere::applyNoise(const std::vector<glm::vec3>& unitPoints, const std::vector<uint32_t>& idx, float radius, float amplitude, float frequency)
{
    FDB::Core::SubMesh subMesh;
    std::vector<FDB::Core::Vertex> out(unitPoints.size());

    // 1) déplacement radial par le bruit
    for (size_t i = 0; i < unitPoints.size(); ++i)
    {
        const glm::vec3 dir = glm::normalize(unitPoints[i]);

        float n = fbm(dir, frequency);          // [-1, 1]
        float r = radius + amplitude * n;       // rayon local

        out[i].pos = glm::vec4(dir * r, 1.0f);
        out[i].normal = glm::vec3(0.0f);        // on va accumuler dessus
        out[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);        // adapte au nom réel du champ
        out[i].uv = sphereUV(dir);
    }

    // 2) normales : somme des normales de faces (pondérées par l'aire)
    for (size_t i = 0; i + 2 < idx.size(); i += 3)
    {
        uint32_t ia = idx[i], ib = idx[i + 1], ic = idx[i + 2];

        glm::vec3 a(out[ia].pos), b(out[ib].pos), c(out[ic].pos);
        glm::vec3 fn = glm::cross(b - a, c - a);

        out[ia].normal += fn;
        out[ib].normal += fn;
        out[ic].normal += fn;
    }
    for (auto& v : out)
        v.normal = glm::normalize(v.normal);

    subMesh.vertices = std::move(out);
    subMesh.indices = idx;                     // à ne pas oublier
    return subMesh;
}