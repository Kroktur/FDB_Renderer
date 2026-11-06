#ifndef BASERENDERSCENE
#define BASERENDERSCENE

#include "Core/Ecs.h"
#include "baseScene.h"
#include "Pipeline/RenderPath.h"
#include "Render/PBRShaders.h"
#include "Render/GraphicPipeline.h"
#include "Render/UIShaders.h"
#include "Core/Transform.h"
#include "Core/UI.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "Core/Material.h"
#include "Core/Texture.h"
#include "Core/Mesh.h"
#include "Core/Mesh2d.h"
#include "Core/Text.h"
#include "Core/Transform2d.h"
#include "Core/ResourceManager.h"

using vertIn_t = FDB::Core::Vertex;
using vertOut_t = FDB::Render::PBRFragment;
using vertData_t = FDB::Render::PushConstant;
using pathInData_t = FDB::Render::vertexShaderData;
using pathOutData_t = FDB::Render::fragmentShaderData;

struct BaseRenderScene : public FDB::Scene::BasicScene
{
	using renderPath_t = FDB::Pipeline::RenderPath<
		vertIn_t,
		vertOut_t,
		vertData_t,
		pathInData_t,
		pathOutData_t,
		&FDB::Render::PBRShader::VertexShader,
		&FDB::Render::PBRShader::FragmentShader<false>,
		FDB::Pipeline::default_rasterizerOpt_t>;


	using renderPath2D_t = FDB::Pipeline::RenderPath<
		FDB::Core::VertexUi,
		FDB::Core::VSOutput,
		FDB::Render::UiShaders::PushConstant,
		FDB::Render::UiShaders::ubo,
		FDB::Render::UiShaders::ubo,
		&FDB::Render::UiShaders::VertexShader,
		&FDB::Render::UiShaders::FragmentShader,
		FDB::Pipeline::rasterizerOpt_t<FDB::Pipeline::default_cullData_t
		, FDB::Pipeline::blendData_t<FDB::Pipeline::BlendFactor::SrcAlpha
		, FDB::Pipeline::BlendFactor::OneMinusSrcAlpha
		, FDB::Pipeline::BlendOp::Add>
		, FDB::Pipeline::depthData_t<false, false
		, FDB::Pipeline::DepthOp::Never>
		, FDB::Pipeline::rgba_writeMaskData_t>>;

public:
	void InitScene() override
	{
		m_baseMaterials.baseColor = FDB::Core::textureLoader_t::Load("Textures/Base/base_color_map.png");
		m_baseMaterials.normalMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_normal_map.png");
		m_baseMaterials.emissiveMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_emissive_map.png");
		m_baseMaterials.OMRMap = FDB::Core::textureLoader_t::Load("Textures/Base/base_OMR_map.png");
		m_baseUi = FDB::Core::textureLoader_t::Load("White.png");
	}
	void Render(FDB::Core::JobPool& pool) override
	{

		auto image = m_renderer->GetFrameBuffer();
		auto viewPort = m_renderer->GetViewPort();
		m_pipeline.Setup(viewPort, &image);
		m_pipeline2D.Setup(viewPort, &image);

		auto& cams = m_reg.GetContainer<FDB::Core::Camera>();
		auto& transforms = m_reg.GetContainer<FDB::Core::Transform>();
		auto& meshs = m_reg.GetContainer<FDB::Core::MeshComp>();
		auto& uis = m_reg.GetContainer<FDB::Core::Ui>();
		auto& transform2ds = m_reg.GetContainer<FDB::Core::Transform2D>();
		auto& texts = m_reg.GetContainer<FDB::Core::Text>();
		auto& mesh2ds = m_reg.GetContainer<FDB::Core::Mesh2D>();
		auto& lights = m_reg.GetContainer<FDB::Core::Light>();

		// base Pipeline
		{
			auto& eS = m_reg.GetEntitiesView<FDB::Core::Camera, FDB::Core::Transform>();
			cams[eS[0]].Update(transforms[eS[0]].GetModelM());
			m_pipeline.BindVInData(cams[eS[0]].GetData());
			m_reg.GetEntitiesView<FDB::Core::Light>();
			m_lDatas.clear();
			for (auto e : eS)
				m_lDatas.push_back(lights[e].GetData());
			FDB::Render::fragmentShaderData outD{ m_lDatas };
			m_pipeline.BindVOutData(outD);
			m_reg.GetEntitiesView<FDB::Core::MeshComp, FDB::Core::Transform>();


			for (auto e : eS)
			{
				auto& mesh = meshs[e];
				bool defaultMat = mesh.mat.empty();
				FDB::Core::Material* mat = defaultMat ? nullptr : mesh.mat.data();
				for (auto& sub : mesh.mesh->subMeshes)
				{
					if (defaultMat)
					{
						m_pipeline.BindVertexData({ transforms[e].GetModelM(),m_baseMaterials });
						m_pipeline.DrawTile(sub.vertices.data(), sub.indices.data(), sub.indices.size());
					}
					else
					{
						m_pipeline.BindVertexData({ transforms[e].GetModelM(),*mat });
						m_pipeline.DrawTile(sub.vertices.data(), sub.indices.data(), sub.indices.size());
						++mat;
					}
				}
			}
			m_pipeline.End(pool);
		}
		// ui pipeline
		{
			float aspectRatio = static_cast<float>(viewPort.width) / static_cast<float>(viewPort.height);
			FDB::Render::UiShaders::PushConstant push;
			push.aspectRatio = aspectRatio;
			auto& eS = m_reg.GetEntitiesView<FDB::Core::Ui, FDB::Core::Transform2D, FDB::Core::Text>();

			for (auto e : eS)
			{
				auto& ui = uis[e];
				auto& transform = transform2ds[e];
				auto& text = texts[e];
				push.inColor = ui.GetColor();
				push.texture1 = text.GetFont()->GetTexture();
				push.texture2 = text.GetTexture();
				transform.SetPosition(ui.GetPosNdc(aspectRatio));
				transform.SetScale(ui.GetScaleNdc(aspectRatio));
				push.model = transform.GetModel();
				m_pipeline2D.BindVertexData(push);
				auto data = text.GetBufferData();
				m_pipeline2D.DrawTile(data.vertices, data.indices, data.indicesCount);
			}
			m_reg.GetEntitiesView<FDB::Core::Ui, FDB::Core::Transform2D, FDB::Core::Mesh2D>();
			for (auto e : eS)
			{
				auto& ui = uis[e];
				auto& transform = transform2ds[e];
				auto& mesh2d = mesh2ds[e];
				push.inColor = ui.GetColor();
				push.texture1 = m_baseUi;
				push.texture2 = mesh2d.texture;
				transform.SetPosition(ui.GetPosNdc(aspectRatio));
				transform.SetScale(ui.GetScaleNdc(aspectRatio));
				push.model = transform.GetModel();
				m_pipeline2D.BindVertexData(push);
				auto data = mesh2d.GetBufferData();
				m_pipeline2D.DrawTile(data.vertices, data.indices, data.indicesCount);
			}

			m_pipeline2D.End(pool);
		}

	}
protected:
	FDB::Core::Registry m_reg;
	FDB::Core::Texture* m_baseUi = nullptr;
	FDB::Core::Material m_baseMaterials;

	std::vector<FDB::Core::lightData_t> m_lDatas;
	renderPath_t m_pipeline;
	renderPath2D_t m_pipeline2D;

};
#endif // !BASERENDERSCENE
