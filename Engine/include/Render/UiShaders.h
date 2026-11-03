#ifndef FDBRENDERUISHADERF
#define FDBRENDERUISHADERF
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>

#include "Sample.h"
#include "UiFragment.h"
#include "Core/Ui.h"
#include "Pipeline/Options.h"

namespace FDB::Render
{
	struct UiShaders
	{
		using FragmentShaderR_t = Pipeline::fragment_output_t<FDB::Pipeline::FragmentState::Keep>;

		struct PushConstant
		{
			glm::vec4 inColor;
			float aspectRatio;
			glm::mat3x3 model;
			FDB::Core::Texture* texture1;
			FDB::Core::Texture* texture2;
		};
		using ubo = void*;

		[[nodiscard]] static FDB::Core::VSOutput VertexShader(const FDB::Core::VertexUi& input, const PushConstant& data, const ubo& camData);
		[[nodiscard]] static UiShaders::FragmentShaderR_t FragmentShader(const FDB::Pipeline::fragment_input_t<FDB::Core::VSOutput>& out, const PushConstant& pushConstant, const ubo& data);
	};

}

#endif
