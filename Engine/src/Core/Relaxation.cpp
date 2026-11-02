#include <vector>
#include <cmath>
#include <GLM/glm.hpp>
#include "Core/Relaxation.h"
#include <KTR/Tools/KTR_Random.h>
#include "KTR/Tools/KTR_Chrono.h"
#include <glm/gtc/matrix_transform.hpp>



std::vector<glm::vec3> RelaxAlgo::createSphere()
{
    KTR::chrono::Timer<KTR::chrono::native_clock_type::high_resolution> timer;

    KTR::Random::default_value_type seed{ KTR::Random::default_seed_gen_type{}() };
    KTR::Random::default_bit_gen_type gen(seed);
    KTR::distrib::Distribution<float> distZ(-1.f, 1.f);
    KTR::distrib::Distribution<float> distA(0.f, glm::pi<float>() * 2.0f);
    std::vector<glm::vec3> p3d;

    // points aléatoires uniformes sur la sphère unité
    std::vector<glm::vec3> points;
    for (size_t i = 0; i < 1024; ++i)
    {
        float z = distZ(gen);
        float a = distA(gen);
        float r = glm::sqrt(1.0f - z * z);
        points.push_back(glm::vec3{ r * glm::cos(a), z, r * glm::sin(a) });
    }

    float totalRelaxation;
    do
    {
        points = relaxationAlgo(points, totalRelaxation);
    } while (totalRelaxation > 0.5f);

    constexpr float kRADIUS = 200.0f;

    for (const auto& p : points) {
        p3d.push_back(p * kRADIUS);
    }
    return p3d;

}

std::vector<glm::vec3> RelaxAlgo::relaxationAlgo(std::vector<glm::vec3> points, float& totalRelaxation)
{
    std::vector<glm::vec3> newPoint = points;

    totalRelaxation = 0.0f;

    for (size_t x = 0; x < points.size(); x++) {
        glm::vec3 totalForce(0.0f);

        for (size_t y = 0; y < points.size(); y++) {
            if (x == y)
                continue;

            float     dist = glm::distance(points[x], points[y]);
            glm::vec3 dir = glm::normalize(points[x] - points[y]);

            totalForce += dir / (dist * dist * 1024.0f);
        }

        // on enlève la composante radiale : seule la force tangente à la sphère compte
        totalForce -= points[x] * glm::dot(totalForce, points[x]);

        totalRelaxation += glm::length(totalForce) * 0.1f;
        newPoint[x] = glm::normalize(points[x] + totalForce * 0.1f);
    }

    return newPoint;
}