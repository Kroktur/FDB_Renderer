#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

/// Courbe de Hermite (Catmull-Rom avec tension) passant par TOUS les checkpoints.
/// - Aucune dependance a l'ECS : a stocker dans ton composant SplineData.
/// - Les extremites sont gerees automatiquement (points fantomes ou boucle fermee).
/// - Table de longueur d'arc : permet une vitesse constante en unites/seconde.
///
/// Parametre t : de 0 a MaxT() (chaque entier = un checkpoint). Meme convention que l'ancien code.
/// Parametre distance : de 0 a Length() (unites monde).
class SplineCurve
{
public:
    SplineCurve() = default;

    SplineCurve(const std::vector<glm::vec3>& checkpoints, float tension = 0.0f, bool closed = false)
    {
        Build(checkpoints, tension, closed);
    }

    /// (Re)construit la courbe. A appeler seulement quand les checkpoints changent.
    void Build(const std::vector<glm::vec3>& checkpoints, float tension = 0.0f, bool closed = false)
    {
        m_points.clear();
        m_cumul.clear();
        m_segments = 0;
        m_tension = tension;
        m_closed = closed;

        const size_t n = checkpoints.size();
        if (n < 2)
            return;

        if (closed && n >= 3)
        {
            // [dernier, p0, p1, ..., pn-1, p0, p1]  -> n segments, boucle sans couture
            m_points.push_back(checkpoints.back());
            m_points.insert(m_points.end(), checkpoints.begin(), checkpoints.end());
            m_points.push_back(checkpoints[0]);
            m_points.push_back(checkpoints[1]);
        }
        else
        {
            // [fantome, p0, ..., pn-1, fantome] -> n-1 segments, la courbe passe par p0 et pn-1
            m_closed = false;
            m_points.push_back(2.0f * checkpoints[0] - checkpoints[1]);
            m_points.insert(m_points.end(), checkpoints.begin(), checkpoints.end());
            m_points.push_back(2.0f * checkpoints[n - 1] - checkpoints[n - 2]);
        }

        m_segments = m_points.size() - 3;
        BuildLengthTable();
    }

    bool IsValid() const { return m_segments > 0; }
    bool IsClosed() const { return m_closed; }
    size_t GetStepCount() const { return m_segments; }
    float MaxT() const { return static_cast<float>(m_segments); }
    float Length() const { return m_cumul.empty() ? 0.0f : m_cumul.back(); }

    /// Position pour t dans [0, MaxT()]. Clampe au lieu de lancer une exception.
    glm::vec3 Position(float t) const
    {
        if (!IsValid()) return glm::vec3{ 0.0f };
        size_t i; float u;
        Locate(t, i, u);

        const glm::vec3& pPrev = m_points[i];
        const glm::vec3& p0 = m_points[i + 1];
        const glm::vec3& p1 = m_points[i + 2];
        const glm::vec3& pNext = m_points[i + 3];

        const glm::vec3 m0 = Tangent(pPrev, p1);
        const glm::vec3 m1 = Tangent(p0, pNext);

        const float u2 = u * u, u3 = u2 * u;
        return (2 * u3 - 3 * u2 + 1) * p0 + (u3 - 2 * u2 + u) * m0
            + (-2 * u3 + 3 * u2) * p1 + (u3 - u2) * m1;
    }

    /// Tangente normalisee analytique (derivee exacte, pas de differences finies).
    glm::vec3 Forward(float t) const
    {
        if (!IsValid()) return glm::vec3{ 0.0f, 0.0f, -1.0f };
        size_t i; float u;
        Locate(t, i, u);

        const glm::vec3& pPrev = m_points[i];
        const glm::vec3& p0 = m_points[i + 1];
        const glm::vec3& p1 = m_points[i + 2];
        const glm::vec3& pNext = m_points[i + 3];

        const glm::vec3 m0 = Tangent(pPrev, p1);
        const glm::vec3 m1 = Tangent(p0, pNext);

        const float u2 = u * u;
        glm::vec3 d = (6 * u2 - 6 * u) * p0 + (3 * u2 - 4 * u + 1) * m0
            + (-6 * u2 + 6 * u) * p1 + (3 * u2 - 2 * u) * m1;

        const float len = glm::length(d);
        return len > 1e-6f ? d / len : glm::vec3{ 0.0f, 0.0f, -1.0f };
    }

    /// Convertit une distance parcourue (unites monde) en parametre t.
    float TFromDistance(float d) const
    {
        if (m_cumul.size() < 2) return 0.0f;
        d = std::clamp(d, 0.0f, Length());

        auto it = std::upper_bound(m_cumul.begin(), m_cumul.end(), d);
        size_t i = std::clamp<size_t>(static_cast<size_t>(it - m_cumul.begin()), 1, m_cumul.size() - 1);

        const float d0 = m_cumul[i - 1], d1 = m_cumul[i];
        const float f = d1 > d0 ? (d - d0) / (d1 - d0) : 0.0f;
        return (static_cast<float>(i - 1) + f) / static_cast<float>(SamplesPerSegment);
    }

    /// Convertit un parametre t en distance (utile pour placer des spawns / arrets exprimes en t).
    float DistanceFromT(float t) const
    {
        if (m_cumul.size() < 2) return 0.0f;
        t = std::clamp(t, 0.0f, MaxT());
        const float s = t * static_cast<float>(SamplesPerSegment);
        const size_t i = std::min(static_cast<size_t>(s), m_cumul.size() - 2);
        const float f = s - static_cast<float>(i);
        return m_cumul[i] + f * (m_cumul[i + 1] - m_cumul[i]);
    }

    /// Orientation "regarde vers l'avant" ; robuste si la tangente est quasi verticale.
    static glm::quat LookRotation(const glm::vec3& forward, const glm::vec3& worldUp = { 0.0f, 1.0f, 0.0f })
    {
        glm::vec3 up = worldUp;
        if (std::abs(glm::dot(forward, up)) > 0.999f)
            up = glm::vec3{ 0.0f, 0.0f, 1.0f };

        const glm::vec3 right = glm::normalize(glm::cross(forward, up));
        const glm::vec3 realUp = glm::cross(right, forward);
        return glm::quat_cast(glm::mat3(right, realUp, -forward)); // convention -Z = avant (comme l'ancien code)
    }

private:
    static constexpr int SamplesPerSegment = 32;

    glm::vec3 Tangent(const glm::vec3& prev, const glm::vec3& next) const
    {
        return (1.0f - m_tension) * ((next - prev) * 0.5f);
    }

    void Locate(float t, size_t& index, float& u) const
    {
        t = std::clamp(t, 0.0f, MaxT());
        index = std::min(static_cast<size_t>(t), m_segments - 1);
        u = t - static_cast<float>(index);
    }

    void BuildLengthTable()
    {
        const int total = static_cast<int>(m_segments) * SamplesPerSegment;
        m_cumul.reserve(total + 1);
        m_cumul.push_back(0.0f);

        glm::vec3 prev = Position(0.0f);
        for (int k = 1; k <= total; ++k)
        {
            const glm::vec3 p = Position(static_cast<float>(k) / SamplesPerSegment);
            m_cumul.push_back(m_cumul.back() + glm::length(p - prev));
            prev = p;
        }
    }

    std::vector<glm::vec3> m_points;  ///< Checkpoints + points fantomes / de bouclage.
    std::vector<float> m_cumul;       ///< Longueur d'arc cumulee a chaque echantillon.
    size_t m_segments = 0;
    float m_tension = 0.0f;
    bool m_closed = false;
};