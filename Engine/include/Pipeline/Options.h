#ifndef PIPELINEOPTIONF
#define PIPELINEOPTIONF

#include <GLM/common.hpp>
#include <GLM/vec4.hpp>

#include "Atribute.h"
#include "EdgeFunction.h"

namespace FDB::Pipeline
{
	enum class FragmentState
	{
		Keep,
		Discard,
		DepthChange
	};
	
	template<FragmentState state>
	struct FragmentOutput;
	
	template<>
	struct FragmentOutput<FragmentState::Keep>
	{
		FragmentOutput() = delete;
		FragmentOutput(const FragmentOutput&) = default;
		FragmentOutput(FragmentOutput&&) noexcept = default;
		~FragmentOutput() = default;
		FragmentOutput& operator=(const FragmentOutput&) = default;
		FragmentOutput& operator=(FragmentOutput&&) noexcept = default;
		static FragmentOutput Color(const glm::vec<4, float>& color_ = { 0,0,0,1.f }) { return { color_ }; }
		glm::vec<4, float> color;
	private:
		FragmentOutput(const glm::vec<4, float>& color_) : color(color_) {}
	};
	
	template<>
	struct FragmentOutput<FragmentState::Discard>
	{
		FragmentOutput() = delete;
		FragmentOutput(const FragmentOutput&) = default;
		FragmentOutput(FragmentOutput&&) noexcept = default;
		~FragmentOutput() = default;
		FragmentOutput& operator=(const FragmentOutput&) = default;
		FragmentOutput& operator=(FragmentOutput&&) noexcept = default;
		static FragmentOutput Color(const glm::vec<4, float>& color_) { return { color_,false }; }
		static FragmentOutput Discard() { return { {},true }; }
		glm::vec<4, float> color;
		bool discard;
	private:
		FragmentOutput(const glm::vec<4, float>& color_, bool discard_) : color(color_), discard(discard_) {}
	};
	
	template<>
	struct FragmentOutput<FragmentState::DepthChange>
	{
		FragmentOutput() = delete;
		FragmentOutput(const FragmentOutput&) = default;
		FragmentOutput(FragmentOutput&&) noexcept = default;
		~FragmentOutput() = default;
		FragmentOutput& operator=(const FragmentOutput&) = default;
		FragmentOutput& operator=(FragmentOutput&&) noexcept = default;
		static FragmentOutput Color(const glm::vec<4, float>& color_, float z) { return { color_,z,false }; }
		static FragmentOutput Discard() { return { {},0,true }; }
		glm::vec<4, float> color;
		float z;
		bool discard;
	private:
		FragmentOutput(const glm::vec<4, float>& color_, float z_, bool discard_) : color(color_), z(z_), discard(discard_) {}
	};
	
	template<FragmentState state>
	using fragment_output_t = FragmentOutput<state>;

	
	template<ValidVertexT VertexT>
	struct FragmentInput
	{
		using vertex_t = VertexT;
		vertex_t vertex;
		vertex_t vertexDx;
		vertex_t vertexDy;
	
		glm::vec<2, std::int32_t> pixel;
	
		float z;
		float zDx;
		float zDy;
	
		float invW;
		float invWDx;
		float invWDy;
	};
	template<ValidVertexT VertexT>
	using fragment_input_t = FragmentInput<VertexT>;
	



	template<typename fragmentT, typename FN,typename... Args>
	concept ValidFragmentFunction = std::is_invocable_r_v<fragment_output_t<FragmentState::Keep>, FN, const FragmentInput<fragmentT>&,Args...> 
	|| std::is_invocable_r_v<fragment_output_t<FragmentState::Discard>, FN, const FragmentInput<fragmentT>&, Args...>
	|| std::is_invocable_r_v<fragment_output_t<FragmentState::DepthChange>, FN, const FragmentInput<fragmentT>&, Args...>;
	
	template<typename fragmentT, typename FN,typename... Args> requires(ValidFragmentFunction<fragmentT, FN,Args...>)
		struct GetFragOutput
	{
		static constexpr FragmentState state = std::is_invocable_r_v<fragment_output_t<FragmentState::Keep>, FN, const FragmentInput<fragmentT>&, Args...> ? FragmentState::Keep :
			std::is_invocable_r_v<fragment_output_t<FragmentState::Discard>, FN, const FragmentInput<fragmentT>&, Args...> ? FragmentState::Discard :
			FragmentState::DepthChange;
	};

	template<typename fragmentT, typename FN> requires(ValidFragmentFunction<fragmentT, FN>)
		static constexpr FragmentState fragOutput_v = GetFragOutput<fragmentT,FN>::state;
	
	enum class BlendFactor : std::uint8_t
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};
	
	enum class BlendOp : std::uint8_t
	{
		None,
		Add,
		Sub,
		RSub,
		Min,
		Max
	};
	
	struct ColorHelper
	{
		template<BlendFactor dataT>
		static glm::vec<4, float> FactorToColor(const glm::vec<4, float>& srcColor, const glm::vec<4, float>& dstColor)
		{
	
			if constexpr (dataT == BlendFactor::Zero)
			{
				return glm::vec<4, float>(0.f);
			}
			else if constexpr (dataT == BlendFactor::One)
			{
				return glm::vec<4, float>(1.f);
			}
			else if constexpr (dataT == BlendFactor::SrcColor)
			{
				return srcColor;
			}
			else if constexpr (dataT == BlendFactor::OneMinusSrcColor)
			{
				return  glm::vec4(1.f) - srcColor;
			}
			else if constexpr (dataT == BlendFactor::DstColor)
			{
				return dstColor;
			}
			else if constexpr (dataT == BlendFactor::OneMinusDstColor)
			{
				return glm::vec4(1.f) - dstColor;
			}
			else if constexpr (dataT == BlendFactor::SrcAlpha)
			{
				return glm::vec<4, float>(srcColor.a);
			}
			else if constexpr (dataT == BlendFactor::OneMinusSrcAlpha)
			{
				return glm::vec4(1.f) - glm::vec<4, float>(srcColor.a);
			}
			else if constexpr (dataT == BlendFactor::DstAlpha)
			{
				return glm::vec<4, float>(dstColor.a);
			}
			else
			{
				return glm::vec4(1.f) - glm::vec<4, float>(dstColor.a);
			}
		}
		template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT>
		static glm::vec<4, float> ColorToBlend(const glm::vec<4, float>& srcColor, const glm::vec<4, float>& dstColor)
		{
			if constexpr (opT == BlendOp::None)
			{
				return srcColor;
			}
			else if constexpr (opT == BlendOp::Add)
			{
				return srcColor * ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor) + dstColor * ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor);
			}
			else if constexpr (opT == BlendOp::Sub)
			{
				return srcColor * ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor) - dstColor * ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor);
			}
			else if constexpr (opT == BlendOp::RSub)
			{
				return   dstColor * ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor) - srcColor * ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor);
			}
			else if constexpr (opT == BlendOp::Min)
			{
				return glm::min(srcColor, dstColor);
			}
			else
			{
				return glm::max(srcColor, dstColor);
			}
		}
	
	};
	
	template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT >
	struct BlendData
	{
		using color_t = glm::vec<4, float>;
	
		static color_t Blend(const color_t& srcColor, const color_t& dstColor)
		{
			return ColorHelper::ColorToBlend<factorSrcT, factorDstT, opT>(srcColor, dstColor);
		}
	};
	template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT >
	using blendData_t = BlendData < factorSrcT, factorDstT, opT >;
	
	using default_blendData_t = blendData_t<BlendFactor::SrcColor, BlendFactor::DstColor, BlendOp::None>;
	
	template<typename T>
	struct is_blendData : std::false_type {};
	
	template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT >
	struct is_blendData<blendData_t<factorSrcT, factorDstT, opT>> : std::true_type {};
	template<typename T>
	static constexpr bool is_blendData_v = is_blendData<T>::value;
	template<typename T>
	concept ValidBlendData = is_blendData_v<T>;
	
	
	enum class DepthOp : std::uint8_t
	{
		Never,
		Less,
		Equal,
		NotEqual,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
		Always
	};
	
	struct DepthHelper
	{
		template<DepthOp opT>
		static bool DepthTest(float srcZ, float dstZ)
		{
			if constexpr (opT == DepthOp::Never)
				return false;
			else if constexpr (opT == DepthOp::Less)
				return srcZ < dstZ;
			else if constexpr (opT == DepthOp::Equal)
				return srcZ == dstZ;
			else if constexpr (opT == DepthOp::NotEqual)
				return srcZ != dstZ;
			else if constexpr (opT == DepthOp::LessOrEqual)
				return srcZ <= dstZ;
			else if constexpr (opT == DepthOp::Greater)
				return srcZ > dstZ;
			else if constexpr (opT == DepthOp::GreaterOrEqual)
				return srcZ >= dstZ;
			else
				return true;
		}
	};
	
	template<bool zTestEnableT, bool zWriteEnableT, DepthOp opt>
	struct DepthData
	{
		static constexpr bool zTestEnabled_v = zTestEnableT;
		static constexpr bool zWriteEnable_v = zWriteEnableT;
	
		static bool PassDepthTest(float srcZ, float dstZ)
		{
			return DepthHelper::DepthTest<opt>(srcZ, dstZ);
		}
	};
	template<bool zTestEnableT, bool zWriteEnableT, DepthOp opt>
	using depthData_t = DepthData<zTestEnableT, zWriteEnableT, opt>;
	using default_depthData_t = DepthData<true, true, DepthOp::Less>;
	
	template<typename T>
	struct is_depthData : std::false_type {};
	template<bool zTestEnableT, bool zWriteEnableT, DepthOp opt>
	struct is_depthData<depthData_t<zTestEnableT, zWriteEnableT, opt>> : std::true_type {};
	
	template<typename T>
	static constexpr bool is_depthData_v = is_depthData<T>::value;
	
	template<typename T>
	concept ValidDepthData = is_depthData_v<T>;
	
	enum class CullFace : std::uint8_t
	{
		CW,
		CCW,
	};
	
	enum class CullMode : std::uint8_t
	{
		None,
		Back,
		Front
	};


	// CULL WITH Y+
	struct cullOut_t
	{
		bool valid = false;
		bool needSwap = false;
	};
	template<CullFace faceT, CullMode modeT>
	struct CullData
	{
		template<typename T,std::size_t N> requires(N >1 && N <= 4)
		static T GetArea(const std::array<glm::vec<N, T>, 3>& pos)
		{
			return EdgeFunction<T>::Compute(
				{ pos[0].x, pos[0].y },
				{ pos[1].x, pos[1].y },
				{ pos[2].x,pos[2].y });
		}
		template<typename T ,std::size_t N> requires(N > 1 && N <= 4)
		static cullOut_t ValidArea( const std::array<glm::vec<N, T>, 3>& pos)
		{
			const float area = CullData::GetArea(pos);
			if (area == 0.f)
				return cullOut_t{ false,false };
			bool isCCW = area > 0;
			if constexpr (modeT == CullMode::None)
			{
				return cullOut_t{true,!isCCW };
			}
			else if constexpr ((modeT == CullMode::Front && faceT == CullFace::CCW) || (modeT == CullMode::Back && faceT == CullFace::CW))
			{
				return cullOut_t{ !isCCW,!isCCW };
			}
			else
			{
				return  cullOut_t{ isCCW,false };
			}
		}

	private:
		
	};
	
	template<CullFace faceT, CullMode modeT>
	using cullData_t = CullData<faceT, modeT>;
	
	using default_cullData_t = cullData_t<CullFace::CCW, CullMode::Back>;
	
	
	template<typename T>
	struct is_cullData : std::false_type {};
	template<CullFace faceT, CullMode modeT>
	struct is_cullData<cullData_t<faceT, modeT>> : std::true_type {};
	
	
	template<typename T>
	static constexpr bool is_cullData_v = is_cullData<T>::value;
	
	template<typename T>
	concept ValidCullData = is_cullData_v<T>;
	
	enum class writeMaks : std::uint8_t
	{
		None = 0,
		R = 1 << 3,
		G = 1 << 2,
		B = 1 << 1,
		A = 1 << 0,
	};
	template<writeMaks... masks> requires (sizeof...(masks) <= 4)
	struct WriteMaskData
	{
		static constexpr std::uint8_t mask = static_cast<std::uint8_t>(writeMaks::None) | (static_cast<std::uint8_t>(masks) | ...);
		static glm::vec<4,float> Apply(const glm::vec<4, float>& srcColor, const glm::vec<4, float>& dstColor)
		{
			glm::vec<4, float> result = dstColor;
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::R))
				result.r = srcColor.r;
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::G))
				result.g = srcColor.g;
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::B))
				result.b = srcColor.b;
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::A))
				result.a = srcColor.a;
			return result;
		}
	};
	
	template<writeMaks... masks> requires (sizeof...(masks) <= 4)
	using writeMaskData_t = WriteMaskData<masks...>;
	
	using r_writeMaskData_t = writeMaskData_t<writeMaks::R>;
	using g_writeMaskData_t = writeMaskData_t<writeMaks::G>;
	using b_writeMaskData_t = writeMaskData_t<writeMaks::B>;
	using a_writeMaskData_t = writeMaskData_t<writeMaks::A>;
	using rgb_writeMaskData_t = WriteMaskData<writeMaks::R, writeMaks::G, writeMaks::B>;
	using rgba_writeMaskData_t = WriteMaskData<writeMaks::R, writeMaks::G, writeMaks::B, writeMaks::A>;
	
	template<typename T>
	struct is_writeMaskData : std::false_type {};
	template<writeMaks... masks> requires (sizeof...(masks) <= 4)
	struct is_writeMaskData<writeMaskData_t<masks...>> : std::true_type {};
	
	template<typename T>
	static constexpr bool is_writeMaskData_v = is_writeMaskData<T>::value;
	
	template<typename T>
	concept ValidWriteMaskData = is_writeMaskData_v<T>;

}
#endif
