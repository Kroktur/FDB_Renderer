#include "Render/UiShaders.h"


namespace FDB::Render
{
	FDB::Core::VSOutput UiShaders::VertexShader(const FDB::Core::VertexUi& input, const PushConstant& data,
		const ubo& camData)
	{
		FDB::Core::VSOutput output;
		glm::vec3 wPos = data.model * glm::vec3(input.position, 1.0f);
		glm::vec2 pos;
		pos.x = wPos.x / data.aspectRatio;
		pos.y = wPos.y;
		output.pos = glm::vec4(pos, 0.0f, 1.0f);
		output.color = data.inColor;
		output.uv = { input.uv.x, input.uv.y };
		output.uv2 = { input.uv2.x, input.uv2.y };
		return output;
	}

	UiShaders::FragmentShaderR_t UiShaders::FragmentShader(
		const FDB::Pipeline::fragment_input_t<FDB::Core::VSOutput>& out, const PushConstant& pushConstant,
		const ubo& data)
	{
		glm::vec4 color = glm::pow(FDB::Render::Sample::Execute(pushConstant.texture1, out.vertex.uv, out.vertexDx.uv, out.vertexDy.uv), glm::vec4(2.2f));
		glm::vec4 color2 = glm::pow(FDB::Render::Sample::Execute(pushConstant.texture2, out.vertex.uv2, out.vertexDx.uv2, out.vertexDy.uv2), glm::vec4(2.2f));
		auto finalColor = glm::pow(color2 * color * out.vertex.color, glm::vec4(1.f / 2.2f));
		return FragmentShaderR_t::Color({ finalColor });
	}
}
