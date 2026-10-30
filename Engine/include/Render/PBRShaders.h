#pragma once
#include <GLM/common.hpp>
#include <GLM/vec3.hpp>
#include <glm/exponential.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "PBRFragment.h"
#include "Sample.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "Core/Mesh.h"
#include "Pipeline/Options.h"

namespace FDB::Render
{
	struct BRDFHelper
	{
		static glm::vec3 F0(const glm::vec3& albedo, float metalness);
		static glm::vec3 F(const glm::vec3& F0, float VDH);
		static float D(float NDH, float alpha2);
		static float G(float DOT, float k);
		static glm::vec3 LambertDiffuse(const glm::vec3& albedo);
		static float FD90(float roughness, float VDH);
		static glm::vec3 DisneyDiffuse(const glm::vec3& albedo, float FD90, float NDL, float NDV);
	};


	using PushConstant = glm::mat4;
	using  vertexShaderData = FDB::Core::cameraDat_t;
	struct fragmentShaderData
	{
		FDB::Core::Material mat;
		std::vector<FDB::Core::lightData_t> lights;
	};
	using FragmentShaderR_t = FDB::Pipeline::fragment_output_t<FDB::Pipeline::FragmentState::Keep>;

	struct PBRShader
	{
		static FDB::Render::PBRFragment VertexShader(const FDB::Core::vertex_t& input, const PushConstant& data, const vertexShaderData& camData);
		template<bool disneyDiff = false >
		 static FragmentShaderR_t FragmentShader(const FDB::Pipeline::fragment_input_t<FDB::Render::PBRFragment>& out, const fragmentShaderData& data)
		 {
			 static constexpr glm::vec3 ambientLight = glm::vec3(0.02f);

			 const auto& v = out.vertex;

			 glm::vec4 baseMap = FDB::Render::Sample::Execute(data.mat.baseColor, v.fragTexCord);
			 glm::vec3 OMRMap = FDB::Render::Sample::Execute(data.mat.OMRMap, v.fragTexCord);
			 glm::vec3 normalMap = FDB::Render::Sample::Execute(data.mat.normalMap, v.fragTexCord) * 2.0f - glm::vec4(1.0f);
			 glm::vec3 emissiveMap = glm::pow(glm::vec3(FDB::Render::Sample::Execute(data.mat.emissiveMap, v.fragTexCord)), glm::vec3(2.2f));
			 float emissiveFactor = data.mat.emissiveFactor;

			 float occlusion = OMRMap.r;
			 float metalness = OMRMap.b;
			 float roughness = OMRMap.g;

			 glm::vec3 albedo = glm::pow(glm::vec3(baseMap) * glm::vec3(v.fragColor), glm::vec3(2.2f));

			 glm::mat3x3 TBN = glm::mat3x3(v.tangent, v.bitangent, v.normal);
			 glm::vec3 normal = glm::normalize(TBN * normalMap);

			 glm::vec3 result = albedo * (1 - metalness) * ambientLight * occlusion;
			 glm::vec3 V = glm::normalize(v.eyePos - v.worldSpacePosition);

			 // BRDF constant

			 // make sure a is not 0 to avoid divide by 0
			 const float a = std::max(roughness * roughness, 0.001f);
			 const float aSquared = a * a;

			 // TODO ask Charles multiple possibilities for k
			 const float k = std::pow(roughness + 1.0f, 2.0f) / 8.0f;
			 glm::vec3 F0 = BRDFHelper::F0(albedo, metalness);

			 // diff = mat albedo / pi
			 glm::vec3 diff = glm::vec3(0.f);

			 if constexpr (!disneyDiff)
			 {
				 diff = BRDFHelper::LambertDiffuse(albedo);
			 }

			 for (const auto& light : data.lights)
			 {
				 float rSquared = std::numeric_limits<float>::max();
				 glm::vec3 L = {};

				 if (light.type == FDB::Core::LightType::Point)
				 {
					 glm::vec3 dir = light.pos - v.worldSpacePosition;
					 rSquared = dot(dir, dir);
					 L = glm::normalize(dir);
				 }
				 else if (light.type == FDB::Core::LightType::Spot)
				 {
					 glm::vec3 dir = light.pos - v.worldSpacePosition;
					 rSquared = dot(dir, dir);
					 L = glm::normalize(dir);
				 }
				 else
				 {
					 L = glm::normalize(-light.dir);
				 }

				 if (rSquared <= glm::epsilon<float>())
					 continue;

				 float NDL = std::max(glm::dot(normal, L), 0.0f);
				 float NDV = std::max(glm::dot(normal, V), 0.0f);

				 if (NDL <= 0.0f || NDV <= 0.0f)
					 continue;

				 glm::vec3 H = glm::normalize(L + V);
				 float VDH = std::max(glm::dot(V, H), 0.0f);
				 float NDH = std::max(glm::dot(normal, H), 0.0f);

				 glm::vec3 F = BRDFHelper::F(F0, VDH);
				 float D = BRDFHelper::D(NDH, aSquared);
				 float G = BRDFHelper::G(NDV, k) * BRDFHelper::G(NDL, k);

				 glm::vec3 Kd = (glm::vec3(1.f) - F) * (1.f - metalness);
				 glm::vec3 spec = (D * G * F) / (4.0f * NDV * NDL);
				 spec *= glm::vec3(light.specColor);


				 if  constexpr (disneyDiff)
				 {
					 float FD90 = BRDFHelper::FD90(roughness, VDH);
					 diff = BRDFHelper::DisneyDiffuse(albedo, FD90, NDL, NDV);
				 }

				 float E = 0.0f;
				 if (light.type == FDB::Core::LightType::Point)
				 {
					 E = light.intensity / (4.0f * glm::pi<float>() * rSquared);
				 }
				 else if (light.type == FDB::Core::LightType::Dir)
				 {
					 E = light.intensity;
				 }
				 else
				 {
					 float spotAngle = glm::dot(glm::normalize(-light.dir), L);
					 float cosInner = std::cos(light.openAngle);
					 float cosOuter = std::cos(light.openAngle + light.AdditionalSmooth);
					 float spotAttenuation = glm::smoothstep(cosOuter, cosInner, spotAngle);
					 float attenuation = light.intensity / (4.0f * glm::pi<float>() * rSquared);
					 E = attenuation * spotAttenuation;
				 }
				 result += (Kd * diff + spec) * glm::vec3(light.color) * NDL * E;
			 }
			 result += emissiveMap * emissiveFactor;
			 result = result / (result + glm::vec3(1.f));
			 result = glm::pow(result, glm::vec3(1.f / 2.2f));
			 return FragmentShaderR_t::Color(glm::vec4(result, 1.0f));
		 }
	};
}