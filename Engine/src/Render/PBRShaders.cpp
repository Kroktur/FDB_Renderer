#include "Render/PBRShaders.h"

namespace FDB::Render
{
	glm::vec3 BRDFHelper::F0(const glm::vec3& albedo, float metalness)
	{
		return glm::mix(glm::vec3(0.04f), albedo, metalness);
	}

	glm::vec3 BRDFHelper::F(const glm::vec3& F0, float VDH)
	{
		return F0 + (glm::vec3(1.f) - F0) * std::pow(1.f - VDH, 5.0f);
	}

	float BRDFHelper::D(float NDH, float alpha2)
	{
		float NDH2 = glm::pow(NDH, 2.0f);
		float denom = NDH2 * (alpha2 - 1.0f) + 1.0f;
		return alpha2 / (glm::pi<float>() * denom * denom);
	}

	float BRDFHelper::G(float DOT, float k)
	{
		return DOT / (DOT * (1 - k) + k);
	}

	glm::vec3 BRDFHelper::LambertDiffuse(const glm::vec3& albedo)
	{
		return albedo / glm::pi<float>();
	}

	float BRDFHelper::FD90(float roughness, float VDH)
	{
		return  0.5f + 2.f * roughness * VDH * VDH;
	}

	glm::vec3 BRDFHelper::DisneyDiffuse(const glm::vec3& albedo, float FD90, float NDL, float NDV)
	{
		float o1 = 1 + (FD90 - 1) * std::pow((1 - NDL), 5.0f);
		float o2 = 1 + (FD90 - 1) * std::pow((1 - NDV), 5.0f);

		return albedo / glm::pi<float>() * o1 * o2;
	}

	FDB::Render::PBRFragment PBRShader::VertexShader(const FDB::Core::vertex_t& input, const PushConstant& data,
		const vertexShaderData& camData)
	{
		FDB::Render::PBRFragment result;
		glm::vec4 wsPos = data * input.pos;
		result.pos = camData.proj * (camData.view * wsPos);
		result.worldSpacePosition = { wsPos.x,wsPos.y,wsPos.z };
		glm::vec4 pos4 = camData.invView * glm::vec4(0, 0, 0, 1);
		result.eyePos = { pos4.x,pos4.y,pos4.z };
		result.fragTexCord = input.uv;
		result.fragColor = input.color;
		result.normal = glm::normalize(glm::transpose(glm::inverse(data)) * glm::vec4(input.normal.x, input.normal.y, input.normal.z, 0.0f));
		result.tangent = glm::normalize(data * glm::vec4(input.tangent.x, input.tangent.y, input.tangent.z, 0.0f));
		glm::vec3 B = glm::cross(result.normal, result.tangent) * input.tangent.w;
		result.bitangent = glm::normalize(B);
		return result;
	}
}
