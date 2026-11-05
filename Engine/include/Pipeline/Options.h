#ifndef PIPELINEOPTIONF
#define PIPELINEOPTIONF

#include <GLM/common.hpp>
#include <GLM/vec4.hpp>
#include <algorithm>
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
	
		float w;
		float wDx;
		float wDy;

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

	template<typename fragmentT, typename FN, typename... Args> requires(ValidFragmentFunction<fragmentT, FN,Args...>)
		static constexpr FragmentState fragOutput_v = GetFragOutput<fragmentT,FN,Args...>::state;
	
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
		static std::uint32_t FactorToColor(std::uint32_t srcColor, std::uint32_t dstColor)
		{
			static constexpr std::uint32_t zero = 0;
			static  constexpr  std::uint32_t one = 0xFFFFFFFF;

			if constexpr (dataT == BlendFactor::Zero)
			{
				return zero;
			}
			else if constexpr (dataT == BlendFactor::One)
			{
				return one;
			}
			else if constexpr (dataT == BlendFactor::SrcColor)
			{
				return srcColor;
			}
			else if constexpr (dataT == BlendFactor::OneMinusSrcColor)
			{
				std::uint16_t a = static_cast<std::uint16_t>(srcColor & 0xFF);
				std::uint16_t b = static_cast<std::uint16_t>((srcColor >> 8) & 0xFF);
				std::uint16_t g = static_cast<std::uint16_t>((srcColor >> 16) & 0xFF);
				std::uint16_t r = static_cast<std::uint16_t>((srcColor >> 24) & 0xFF);

				r = 255 - r;
				g = 255 - g;
				b = 255 - b;
				a = 255 - a;

				return (r << 24) | (g << 16) | (b << 8) | a;
			}
			else if constexpr (dataT == BlendFactor::DstColor)
			{
				return dstColor;
			}
			else if constexpr (dataT == BlendFactor::OneMinusDstColor)
			{
				std::uint16_t a = static_cast<std::uint16_t>(dstColor & 0xFF);
				std::uint16_t b = static_cast<std::uint16_t>((dstColor >> 8) & 0xFF);
				std::uint16_t g = static_cast<std::uint16_t>((dstColor >> 16) & 0xFF);
				std::uint16_t r = static_cast<std::uint16_t>((dstColor >> 24) & 0xFF);

				r = 255 - r;
				g = 255 - g;
				b = 255 - b;
				a = 255 - a;

				return (r << 24) | (g << 16) | (b << 8) | a;
			}
			else if constexpr (dataT == BlendFactor::SrcAlpha)
			{
				std::uint16_t alpha = static_cast<std::uint16_t>((srcColor) & 0xFF);
				return  alpha | (alpha << 8) | (alpha << 16) | (alpha << 24);
			}
			else if constexpr (dataT == BlendFactor::OneMinusSrcAlpha)
			{
				std::uint16_t alpha = static_cast<std::uint16_t>((srcColor) & 0xFF);
				std::uint16_t r = 255 - alpha;
				std::uint16_t g = 255 - alpha;
				std::uint16_t b = 255 - alpha;
				std::uint16_t a = 255 - alpha;

				return (r << 24) | (g << 16) | (b << 8) | a;
			}
			else if constexpr (dataT == BlendFactor::DstAlpha)
			{
				std::uint16_t alpha = static_cast<std::uint16_t>((dstColor) & 0xFF);
				return  alpha | (alpha << 8) | (alpha << 16) | (alpha << 24);
			}
			else
			{
				std::uint16_t alpha = static_cast<std::uint16_t>((dstColor) & 0xFF);
				std::uint16_t r = 255 - alpha;
				std::uint16_t g = 255 - alpha;
				std::uint16_t b = 255 - alpha;
				std::uint16_t a = 255 - alpha;

				return (r << 24) | (g << 16) | (b << 8) | a;
			}
		}
		template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT>
		static std::uint32_t ColorToBlend(std::uint32_t srcColor, std::uint32_t dstColor)
		{
			if constexpr (opT == BlendOp::None)
			{
				return srcColor;
			}
			else if constexpr (opT == BlendOp::Add)
			{
				return ProcessFactor<true>(srcColor , ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor) , dstColor , ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor));
			}
			else if constexpr (opT == BlendOp::Sub)
			{
				return ProcessFactor<false>(srcColor , ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor) , dstColor , ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor));
			}
			else if constexpr (opT == BlendOp::RSub)
			{
				return ProcessFactor<false>(dstColor , ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor) , srcColor , ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor));
			}
			else if constexpr (opT == BlendOp::Min)
			{
				return std::min(srcColor, dstColor);
			}
			else
			{
				return std::max(srcColor, dstColor);
			}
		}
	private:
		template<bool plus>
		static std::uint32_t ProcessFactor(std::uint32_t lhsColor, std::uint32_t lhsFactor, std::uint32_t rhsColor, std::uint32_t rhsFactor )
		{
			std::int32_t lhsColorR = static_cast<std::int32_t>((lhsColor >> 24) & 0xFF);
			std::int32_t lhsColorG = static_cast<std::int32_t>((lhsColor >> 16) & 0xFF);
			std::int32_t lhsColorB = static_cast<std::int32_t>((lhsColor >> 8) & 0xFF);
			std::int32_t lhsColorA = static_cast<std::int32_t>((lhsColor) & 0xFF);
			std::int32_t lhsFactorR = static_cast<std::int32_t>((lhsFactor >> 24) & 0xFF);
			std::int32_t lhsFactorG = static_cast<std::int32_t>((lhsFactor >> 16) & 0xFF);
			std::int32_t lhsFactorB = static_cast<std::int32_t>((lhsFactor >> 8) & 0xFF);
			std::int32_t lhsFactorA = static_cast<std::int32_t>((lhsFactor) & 0xFF);
			std::int32_t rhsColorR = static_cast<std::int32_t>((rhsColor >> 24) & 0xFF);
			std::int32_t rhsColorG = static_cast<std::int32_t>((rhsColor >> 16) & 0xFF);
			std::int32_t rhsColorB = static_cast<std::int32_t>((rhsColor >> 8) & 0xFF);
			std::int32_t rhsColorA = static_cast<std::int32_t>((rhsColor) & 0xFF);
			std::int32_t rhsFactorR = static_cast<std::int32_t>((rhsFactor >> 24) & 0xFF);
			std::int32_t rhsFactorG = static_cast<std::int32_t>((rhsFactor >> 16) & 0xFF);
			std::int32_t rhsFactorB = static_cast<std::int32_t>((rhsFactor >> 8) & 0xFF);
			std::int32_t rhsFactorA = static_cast<std::int32_t>((rhsFactor) & 0xFF);

			if constexpr (plus)
			{
				std::uint8_t r = std::clamp((lhsColorR * lhsFactorR + rhsColorR * rhsFactorR)/ 255, 0, 255);
				std::uint8_t g = std::clamp((lhsColorG * lhsFactorG + rhsColorG * rhsFactorG)/ 255, 0, 255);
				std::uint8_t b = std::clamp((lhsColorB * lhsFactorB + rhsColorB * rhsFactorB)/ 255, 0, 255);
				std::uint8_t a = std::clamp((lhsColorA * lhsFactorA + rhsColorA * rhsFactorA)/ 255, 0, 255);
				return  r << 24 | g << 16 | b << 8 | a;
			}
			else
			{
				std::uint8_t r = std::clamp((lhsColorR * lhsFactorR - rhsColorR * rhsFactorR)/ 255, 0, 255);
				std::uint8_t g = std::clamp((lhsColorG * lhsFactorG - rhsColorG * rhsFactorG)/ 255, 0, 255);
				std::uint8_t b = std::clamp((lhsColorB * lhsFactorB - rhsColorB * rhsFactorB)/ 255, 0, 255);
				std::uint8_t a = std::clamp((lhsColorA * lhsFactorA - rhsColorA * rhsFactorA)/ 255, 0, 255);
				return  r << 24 | g << 16 | b << 8 | a;
				
			}
		}
		
	};

	
	template<BlendFactor factorSrcT, BlendFactor factorDstT, BlendOp opT >
	struct BlendData
	{
		using color_t =std::uint32_t;
	
		static color_t Blend( color_t srcColor,  color_t dstColor)
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
		static std::uint32_t Apply(std::uint32_t srcColor, std::uint32_t dstColor)
		{
			static constexpr std::uint32_t rMask = 0xFF000000u;
			static constexpr std::uint32_t gMask = 0x00FF0000u;
			static constexpr std::uint32_t bMask = 0x0000FF00u;
			static constexpr std::uint32_t aMask = 0x000000FFu;
			static constexpr std::uint32_t allMask = 0xFFFFFFFFu;

			std::uint32_t result = dstColor;

			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::R))
				result = (result & (allMask & ~rMask)) | (srcColor & rMask);
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::G))
				result = (result & (allMask & ~gMask)) | (srcColor & gMask);
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::B))
				result = (result & (allMask & ~bMask)) | (srcColor & bMask);
			if constexpr (mask & static_cast<std::uint8_t>(writeMaks::A))
				result = (result & (allMask & ~aMask)) | (srcColor & aMask);

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
