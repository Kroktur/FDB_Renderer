
	#include <iostream>
#include <mutex>
#include <string>

	#include "Core/Rect.h"
	#include "GLM/vec4.hpp"
	#include "GLM/vec3.hpp"
	#include "SDL/Application.h"
	#include "SDL/Renderer.h"
	#include "SDL/Window.h"
	#include "SDL/Texture.h"
	#include "SDL3/SDL_timer.h"

#include "GLM/detail/type_vec1.hpp"
#include "Pipeline/Atribute.h"
#include "Pipeline/Pipeline.h"
#include "Pipeline/Transform.h"


enum format
{
	float1,
	float2,
	float3,
	float4
};

struct Atribut
{
	format format;
	std::size_t offset;
};



#include <array>


	
	template<typename T,size_t N>
	struct Box
	{
		using value_type = T;
		glm::vec<N, value_type> min;
		glm::vec<N, value_type> max;
	};
	template<typename T>
	using box2_t = Box<T, 2>;
	template<typename T>
	using box3_t = Box<T, 3>;


#include <type_traits>
#include <array>
	namespace FDB::Pipeline
	{
		template<typename T>
		struct Image
		{
			using value_type = T;
			value_type* buffer = nullptr;
			std::int32_t bufferWidth = 0;
			std::int32_t bufferHeight = 0;
		};
	}



	// TODO MOVE TO FILE ATTRIBUTE

	// TODO MOVE TO FILE ATTRIBUTE

	struct Vertex
	{
		float test;
		glm::vec4 pos;
		glm::vec<4, float> color;
		static  constexpr  FDB::Pipeline::Attribute::Data PosAttribute()
		{
			
			return FDB::Pipeline::Attribute::Data{ FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, pos) };
		}
		static constexpr  std::array<FDB::Pipeline::Attribute::Data, 2> AllAttributes()
		{
			return {FDB::Pipeline::Attribute::Data{FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, pos)},FDB::Pipeline::Attribute::Data{FDB::Pipeline::Attribute::format::Float4,offsetof(Vertex, color)} };
		}
	};
	using triangle = Vertex[3];



	namespace FDB::Pipeline::Rasterizer
	{

		struct EdgesData
		{
			glm::vec<3, float> edgesFn;
			std::array<glm::vec<2, float>, 3> edgesFnDelta;
		};
		template<typename T>
		struct DeltaData
		{
			using value_type = T;
			value_type data;
			std::array<value_type, 2> delta;
		};
		struct BlockData
		{
			glm::vec<2, std::int32_t> blockSize, blockOrigin;
		};
	
		template<FDB::Pipeline::Attribute::ValidVertexT T>
		struct AttributeData
		{
			using value_type = T;
			using interpret_t = FDB::Pipeline::Attribute::Interpret<value_type>;
			using edgesData_t = FDB::Pipeline::Rasterizer::EdgesData;
			AttributeData() : m_data() {}
			~AttributeData() = default;
			AttributeData(const AttributeData&) = default;
			AttributeData(AttributeData&&)noexcept = default;
			AttributeData& operator=(const AttributeData&) = default;
			AttributeData& operator=(AttributeData&&) noexcept = default;

			void InitW(const T& dataT, float invW)
			{
				const std::byte* rawData = reinterpret_cast<const std::byte*>(&dataT);
				this->ForEach([&]<std::size_t i>()
				{
					this->template PrivateInitW<i>(rawData, invW);
				});
			}

			void InitOrigin(const std::array<AttributeData, 3>& baseInterpol, const  edgesData_t& edgeData, float invArea)
			{
				const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
				const std::byte* rawData1 = reinterpret_cast<const std::byte*>(&baseInterpol[1]);
				const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);


				this->ForEach([&]<std::size_t i>()
				{
					this->template PrivateInitOrigin<i>(rawData0, rawData1, rawData2, edgeData, invArea);
				});
			}




			template<bool isX>
			void InitGradiant(const std::array<AttributeData, 3>& baseInterpol, const edgesData_t& edgeData, float invArea)
			{
				const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
				const std::byte* rawData1 = reinterpret_cast<const std::byte*>(&baseInterpol[1]);
				const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);

				this->ForEach([&]<std::size_t i>()
				{
					this->template PrivateInitGradiant<i, isX>(rawData0, rawData1, rawData2, edgeData, invArea);
				});
			}


			AttributeData& operator+=(const AttributeData& other)
			{
				const std::byte* rawData = reinterpret_cast<const std::byte*>(&other);

				this->ForEach([&]<std::size_t i>()
				{
					this->template PrivateAdd<i>(rawData);
				});

				return *this;
			}

			void Apply(value_type& fragment, float w) const 
			{
				std::byte* rawData = reinterpret_cast<std::byte*>(&fragment);

				this->ForEach([&]<std::size_t i>()
				{
					this->template PrivateApply<i>(rawData, w);
				});
			}

		private:

			template<typename FN>
			void ForEach(const FN& fn) const 
			{
				[&] <std::size_t... i>(std::index_sequence<i...>)
				{
					(fn.template operator() < i > (), ...);
				}(std::make_index_sequence< interpret_t::attributesSize >{});
			}

			template<std::size_t i>
			void PrivateInitW(const std::byte* rawData, float invW)
			{
				using info_t = interpret_t::template infoFloat_t<i>;

				if constexpr (info_t::n == 0)
					return;
				else
				{
					glm::vec<info_t::n, float> v;
					std::memcpy(&v, rawData + info_t::offset, sizeof(v));
					v *= invW;
					std::memcpy(m_data + info_t::offset, &v, sizeof(v));
				}
			}
			template<std::size_t i>
			void PrivateInitOrigin(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const  edgesData_t& edgeData, float invArea)
			{
				using info_t = interpret_t::template infoFloat_t<i>;
				if constexpr (info_t::n == 0)
					return;
				else
				{
					glm::vec<info_t::n, float> v0;
					glm::vec<info_t::n, float> v1;
					glm::vec<info_t::n, float> v2;
					std::memcpy(&v0, rawData0 + info_t::offset, sizeof(v0));
					std::memcpy(&v1, rawData1 + info_t::offset, sizeof(v1));
					std::memcpy(&v2, rawData2 + info_t::offset, sizeof(v2));
					glm::vec<info_t::n, float> vFinal = v0 * edgeData.edgesFn.y + v1 * edgeData.edgesFn.z + v2 * edgeData.edgesFn.x;
					vFinal *= invArea;
					std::memcpy(m_data + info_t::offset, &vFinal, sizeof(vFinal));
				}
			}
			template<std::size_t i, bool isX>
			void PrivateInitGradiant(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const edgesData_t& edgeData, float invArea)
			{
				using info_t = interpret_t::template infoFloat_t<i>;
				if constexpr (info_t::n == 0)
					return;
				else
				{
					glm::vec<info_t::n, float> v0;
					glm::vec<info_t::n, float> v1;
					glm::vec<info_t::n, float> v2;
					std::memcpy(&v0, rawData0 + info_t::offset, sizeof(v0));
					std::memcpy(&v1, rawData1 + info_t::offset, sizeof(v1));
					std::memcpy(&v2, rawData2 + info_t::offset, sizeof(v2));
					glm::vec<info_t::n, float> vFinal;
					if constexpr (isX)
						vFinal = v0 * edgeData.edgesFnDelta[1].x + v1 * edgeData.edgesFnDelta[2].x + v2 * edgeData.edgesFnDelta[0].x;
					else
						vFinal = v0 * edgeData.edgesFnDelta[1].y + v1 * edgeData.edgesFnDelta[2].y + v2 * edgeData.edgesFnDelta[0].y;

					vFinal *= invArea;
					std::memcpy(m_data + info_t::offset, &vFinal, sizeof(vFinal));

				}
			}
			template<std::size_t i>
			void PrivateAdd(const std::byte* otherData)
			{
				using info_t = interpret_t::template infoFloat_t<i>;

				if constexpr (info_t::n == 0)
					return;
				else
				{
					glm::vec<info_t::n, float> v0;
					glm::vec<info_t::n, float> v1;
					std::memcpy(&v0, otherData + info_t::offset, sizeof(v0));
					std::memcpy(&v1, m_data + info_t::offset, sizeof(v1));
					glm::vec<info_t::n, float> vFinal = v0 + v1;
					std::memcpy(m_data + info_t::offset, &vFinal, sizeof(vFinal));

				}
			}
			template<std::size_t i>
			void PrivateApply(std::byte* rawData, float w) const 
			{
				using info_t = interpret_t::template infoFloat_t<i>;
				if constexpr (info_t::n == 0)
					return;
				else
				{
					glm::vec<info_t::n, float> v;
					std::memcpy(&v, m_data + info_t::offset, sizeof(v));
					v *= w;
					std::memcpy(rawData + info_t::offset, &v, sizeof(v));
				}
			}
			std::byte m_data[interpret_t::structSize];
		};
	
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
			static FragmentOutput Color(const glm::vec<4,float>& color_ = {0,0,0,1.f}){return { color_ };}
			glm::vec<4, float> color;
		private:
			FragmentOutput(const glm::vec<4,float>& color_) : color(color_){}
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
			static FragmentOutput Discard(){return { {},true };}
			glm::vec<4, float> color;
			bool discard;
		private:
			FragmentOutput(const glm::vec<4, float>& color_,bool discard_) : color(color_),discard(discard_) {}
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
			static FragmentOutput Color(const glm::vec<4, float>& color_,float z) { return { color_,z,false }; }
			static FragmentOutput Discard() { return { {},0,true }; }
			glm::vec<4, float> color;
			float z;
			bool discard;
		private:
			FragmentOutput(const glm::vec<4, float>& color_,float z_, bool discard_) : color(color_),z(z_), discard(discard_) {}
		};

		template<FragmentState state>
		using fragment_output_t = FragmentOutput<state>;

		template<Attribute::ValidVertexT VertexT>
		struct FragmentInput
		{
			using vertex_t = VertexT;
			vertex_t vertex;
			vertex_t vertexDx ;
			vertex_t vertexDy ;

			 glm::vec<2, std::int32_t> pixel;

			float z;
			float zDx;
			float zDy;

			float invW;
			float invWDx;
			float invWDy;
		};
		template<Attribute::ValidVertexT VertexT>
		using fragment_input_t = FragmentInput<VertexT>;

		template<typename fragmentT,FragmentState state,typename FN>
		concept ValidFragmentFunction = std::is_invocable_r_v<fragment_output_t<state>, FN, const FragmentInput<fragmentT>>;
	
		
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

		enum class BlendOp: std::uint8_t
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
			template<BlendFactor factorSrcT, BlendFactor factorDstT,BlendOp opT>
			static glm::vec<4,float> ColorToBlend(const glm::vec<4, float>& srcColor, const glm::vec<4, float>& dstColor)
			{
				if constexpr (opT == BlendOp::None)
				{
					return srcColor;
				}
				else if constexpr (opT == BlendOp::Add)
				{
					return srcColor* ColorHelper::FactorToColor<factorSrcT>(srcColor,dstColor) + dstColor * ColorHelper::FactorToColor<factorDstT>(srcColor, dstColor);
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
					glm::vec<4, float> result;
					result.r = std::min(srcColor.r, dstColor.r);
					result.g = std::min(srcColor.g, dstColor.g);
					result.b = std::min(srcColor.b, dstColor.b);
					result.a = std::min(srcColor.a, dstColor.a);
					return result;
				}
				else
				{
					glm::vec<4, float> result;
					result.r = std::max(srcColor.r, dstColor.r);
					result.g = std::max(srcColor.g, dstColor.g);
					result.b = std::max(srcColor.b, dstColor.b);
					result.a = std::max(srcColor.a, dstColor.a);
					return result;
				}
			}

		};
	
		template<BlendFactor factorSrcT ,BlendFactor factorDstT ,BlendOp opT >
		struct BlendData
		{
			using color_t = glm::vec<4, float>;

			static color_t Blend(const color_t& srcColor ,const color_t& dstColor)
			{
				return ColorHelper::ColorToBlend<factorSrcT, factorDstT, opT>(srcColor,dstColor);
			}
		};
		template<BlendFactor factorSrcT , BlendFactor factorDstT , BlendOp opT >
		using blendData_t = BlendData < factorSrcT, factorDstT, opT >;

		using default_blend_t = blendData_t<BlendFactor::SrcColor, BlendFactor::DstColor, BlendOp::None>;

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
			static bool DepthTest(float srcZ,float dstZ)
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

		template<bool zTestEnableT ,bool zWriteEnableT ,DepthOp opt>
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
		struct is_depthData : std::false_type{};
		template<bool zTestEnableT, bool zWriteEnableT, DepthOp opt>
		struct is_depthData<depthData_t<zTestEnableT, zWriteEnableT, opt>> : std::true_type{};

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
		template<CullFace faceT, CullMode modeT>
		struct CullData
		{
			static bool ValidArea(float& area,std::array<glm::vec<4,float>,3>& pos)
			{
				if constexpr (modeT == CullMode::None)
				{
					if (area > 0)
						return true;

					if (area == 0)
						return false;

					CullData::InvArea(area, pos);
					return true;
				}
				else if constexpr ((modeT == CullMode::Front && faceT == CullFace::CCW )||(modeT == CullMode::Back && faceT == CullFace::CW))
				{
					if (area >= 0)
						return false;
					InvArea(area, pos);
					return true;
				}
				else 
				{
					return area > 0;
				}
			}
		private:
			static void InvArea(float& area,std::array<glm::vec<4, float>, 3>& pos)
			{
				area = -area;
				glm::vec<4, float> tmpPos = pos[1];
				pos[1] = pos[2];
				pos[2] = tmpPos;
			}	
		};

		template<CullFace faceT, CullMode modeT>
		using cullData_t = CullData<faceT, modeT>;

		using default_cullData_t = cullData_t<CullFace::CCW, CullMode::Back>;


		template<typename T>
		struct is_cullData : std::false_type{};
		template<CullFace faceT, CullMode modeT>
		struct is_cullData<cullData_t<faceT,modeT>> : std::true_type{};


		template<typename T>
		static constexpr bool is_cullData_v = is_cullData<T>::value;

		template<typename T>
		concept ValidCullData = is_cullData_v<T>;

		enum class writeMaks : std::uint8_t
		{
			None = 0,
			R = 1 << 3,
			G = 1 << 3,
			B = 1 << 1,
			A = 1 << 0,
		};
		template<writeMaks... masks> requires (sizeof...(masks) <= 4)
		struct WriteMaskData
		{
			static constexpr std::uint8_t mask = static_cast<std::uint8_t>(writeMaks::None) | (static_cast<std::uint8_t>(masks) | ...);
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
		struct is_writeMaskData : std::false_type{};
		template<writeMaks... masks> requires (sizeof...(masks) <= 4)
		struct is_writeMaskData<writeMaskData_t<masks...>> : std::true_type{};

		template<typename T>
		static constexpr bool is_writeMaskData_v = is_writeMaskData<T>::value;
		
		template<typename T>
		concept ValidWriteData = is_writeMaskData_v<T>;



		template<FDB::Pipeline::Attribute::ValidVertexT VertexT> 
		struct TriangleRasterizer
		{
			using fragment = VertexT;
			using vertex_t = VertexT;
			using triangle_t = std::array<vertex_t, 3>;
			using viewPort_t = FDB::Rect<std::int32_t>;
			using attributeData_t = AttributeData<vertex_t>;
			using interpret_t = Attribute::Interpret<vertex_t>;
			using edgesData_t =EdgesData;
			template<typename T>
			using deltaData_t = DeltaData<T>;
			using blockData_t = BlockData;
			static  constexpr  glm::vec<2, int> blockDim = { 64,64 };
			using fragmentState_t  = FragmentState;
			struct TriangleSetup
			{
				triangle_t triangle;
				float invArea;
				box2_t<std::int32_t> box;
				edgesData_t baseEdgeData;
				glm::vec<3, float> baseInvW;
				glm::vec<3,float> baseZ;
				std::array<attributeData_t, 3> inter;
				std::array<bool, 3> topLeftRule;
			};
			struct BlockSetup
			{
				blockData_t block;
				deltaData_t<float> wData;
				deltaData_t<float> zData;
				deltaData_t<attributeData_t> interData;
				std::array <glm::vec<3, float>, 2> edgeIncrement;
			};
			struct ApplySetup
			{
				attributeData_t inter;
				glm::vec<2, std::int32_t> pixel;
				float w;
				float z;
				std::uint32_t* imagePtr;
				float* zImagePtr;
				glm::vec<4,float>* colorPtr;
			};
			struct ImagesData
			{
				Image<std::uint32_t> finalImage;
				Image<glm::vec<4,float>> colorImage;
				Image<float> zBuffer;
			};
			template< fragmentState_t state, ValidCullData cullT = default_cullData_t, ValidBlendData blendT = default_blend_t,ValidDepthData depthT = default_depthData_t, ValidWriteData writeT = rgba_writeMaskData_t,typename fragmentShaderT> requires(ValidFragmentFunction<fragment,state,fragmentShaderT>)
			static void Compute(const triangle_t& triangle, const viewPort_t& viewPort, const ImagesData& image,const fragmentShaderT& fragmentShader)
			{
				// position
				 std::array<glm::vec<4, float>, 3> position = TriangleRasterizer::GetPosition(triangle);
				// positon
				// area
				float area = TriangleRasterizer::GetArea(position);
				if (!cullT::ValidArea(area,position))
					return;

				TriangleSetup setup;
				setup.triangle = triangle;
				setup.invArea = 1.f / area;
				setup.box = TriangleRasterizer::GetBox(position, viewPort);
			
				
				setup.baseEdgeData.edgesFn = TriangleRasterizer::InitializeEdgeFunction(position, setup.box);
				setup.baseEdgeData.edgesFnDelta = TriangleRasterizer::InitializeEdgeDelta(position);

				setup.baseInvW = TriangleRasterizer::GetInvW(position);
				setup.baseZ[0] = position[0].z;
				setup.baseZ[1] = position[1].z;
				setup.baseZ[2] = position[2].z;

				
				setup.inter[0].InitW(triangle[0], setup.baseInvW[0]);
				setup.inter[1].InitW(triangle[1], setup.baseInvW[1]);
				setup.inter[2].InitW(triangle[2], setup.baseInvW[2]);
				setup.topLeftRule[0] = TriangleRasterizer::IsTopLeft({ position[0].x, position[0].y },{ position[1].x, position[1].y });
				setup.topLeftRule[1] = TriangleRasterizer::IsTopLeft({ position[1].x, position[1].y }, { position[2].x, position[2].y });
				setup.topLeftRule[2] = TriangleRasterizer::IsTopLeft({ position[2].x, position[2].y }, { position[0].x, position[0].y });

				TriangleRasterizer::Loop<state,blendT,depthT,writeT,fragmentShaderT>(setup, image,fragmentShader);
			}
		private:
			template<fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT,typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void Loop(const TriangleSetup& setup, const ImagesData& image,const fragmentShaderT& fragmentShader)
			{

				BlockSetup blockSetup;
				blockSetup.wData = TriangleRasterizer::SetupLinearData(setup.baseInvW, setup.baseEdgeData, setup.invArea);
				blockSetup.interData = TriangleRasterizer::SetupInterData(setup.inter, setup.baseEdgeData, setup.invArea);
				blockSetup.zData  = TriangleRasterizer::SetupLinearData(setup.baseZ, setup.baseEdgeData, setup.invArea);
				
				blockSetup.edgeIncrement = {
					glm::vec<3, float>(setup.baseEdgeData.edgesFnDelta[0].x, setup.baseEdgeData.edgesFnDelta[1].x, setup.baseEdgeData.edgesFnDelta[2].x),
					glm::vec<3, float>(setup.baseEdgeData.edgesFnDelta[0].y, setup.baseEdgeData.edgesFnDelta[1].y, setup.baseEdgeData.edgesFnDelta[2].y) };



				
				for (std::int32_t blockY = setup.box.min.y; blockY <= setup.box.max.y; blockY += blockDim.y)
				{
					blockSetup.block.blockSize.y = std::min(blockDim.y, setup.box.max.y - blockY + 1);
					for (std::int32_t blockX = setup.box.min.x; blockX <= setup.box.max.x; blockX += blockDim.x)
					{

						blockSetup.block.blockSize.x = std::min(blockDim.x, setup.box.max.x - blockX + 1);

						blockSetup.block.blockOrigin = { blockX, blockY };
						edgesData_t blockEdgeData = TriangleRasterizer::InitializeStepEdgeData(blockSetup.block.blockOrigin, setup.box, blockSetup.block.blockSize, setup.baseEdgeData);

						if (TriangleRasterizer::IsBlockOut(blockEdgeData))
							continue;

						TriangleRasterizer::UpdateLinearData(setup.baseInvW, blockEdgeData, setup.invArea, blockSetup.wData);
						TriangleRasterizer::UpdateInterData(setup.inter, blockEdgeData, setup.invArea, blockSetup.interData);
						TriangleRasterizer::UpdateLinearData(setup.baseZ, blockEdgeData, setup.invArea, blockSetup.zData);

						

						if (IsBlockFullyInside(blockEdgeData))
						{
							TriangleRasterizer::PixelLoopInside < state, blendT, depthT,writeT,fragmentShaderT > (setup.triangle, blockSetup, image, fragmentShader);
						}
						else
						{
							TriangleRasterizer::PixelLoopClassic<state,blendT,depthT,writeT,fragmentShaderT>(setup.triangle,setup.topLeftRule, blockEdgeData, blockSetup, image,fragmentShader);
						}
					}
				}
			}
			template< fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT,typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void PixelLoopInside(const triangle_t& triangle, BlockSetup& setup, const ImagesData& image,const fragmentShaderT& fragmentShader)
			{
				ApplySetup applySetup;

				for (std::int32_t y = 0; y < setup.block.blockSize.y; ++y)
				{
					applySetup.w = setup.wData.data;
					applySetup.z = setup.zData.data;
					applySetup.inter = setup.interData.data;
					applySetup.pixel = { setup.block.blockOrigin.x,setup.block.blockOrigin.y + y };
					applySetup.imagePtr = image.finalImage.buffer + (applySetup.pixel.y * image.finalImage.bufferWidth + applySetup.pixel.x);
					applySetup.zImagePtr = image.zBuffer.buffer + (applySetup.pixel.y * image.zBuffer.bufferWidth + applySetup.pixel.x);
					applySetup.colorPtr = image.colorImage.buffer + (applySetup.pixel.y * image.colorImage.bufferWidth + applySetup.pixel.x);
					for (std::int32_t x = 0; x < setup.block.blockSize.x; ++x)
					{
						TriangleRasterizer::ApplyFragment<state, blendT, depthT,writeT, fragmentShaderT>(triangle, applySetup,setup, fragmentShader);
						++applySetup.zImagePtr;
						++applySetup.imagePtr;
						++applySetup.colorPtr;
						++applySetup.pixel.x;
						applySetup.w += setup.wData.delta[0];
						applySetup.z += setup.zData.delta[0];
						applySetup.inter += setup.interData.delta[0];
					}
					setup.wData.data += setup.wData.delta[1];
					setup.interData.data += setup.interData.delta[1];
					setup.zData.data += setup.zData.delta[1];
				}
			}
			static bool IsInside(const glm::vec<3, float>& edgesFn,const std::array<bool, 3>& topLeftRule)
			{
				return ((topLeftRule[0] == true ? edgesFn[0] >= 0 : edgesFn[0] > 0) && (topLeftRule[1] == true ? edgesFn[1] >= 0 : edgesFn[1] > 0) && (topLeftRule[2] == true ? edgesFn[2] >= 0 : edgesFn[2] > 0));
			}
			template<fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT,typename fragmentShaderT > requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void PixelLoopClassic(const triangle_t& triangle,const std::array<bool,3>& topLeftRule, edgesData_t& currentEdgeData, BlockSetup& setup, const ImagesData& image,const fragmentShaderT& fragmentShader)
			{
				ApplySetup applySetup;
				for (std::int32_t y = 0; y < setup.block.blockSize.y; ++y)
				{
					glm::vec<3,float> edgesFn = currentEdgeData.edgesFn;
					applySetup.w = setup.wData.data;
					applySetup.z = setup.zData.data;
					applySetup.inter = setup.interData.data;

					applySetup.pixel = { setup.block.blockOrigin.x,setup.block.blockOrigin.y + y };
					applySetup.imagePtr = image.finalImage.buffer + (applySetup.pixel.y * image.finalImage.bufferWidth + applySetup.pixel.x);
					applySetup.zImagePtr = image.zBuffer.buffer + (applySetup.pixel.y * image.zBuffer.bufferWidth + applySetup.pixel.x);
					applySetup.colorPtr = image.colorImage.buffer + (applySetup.pixel.y * image.colorImage.bufferWidth + applySetup.pixel.x);


					for (std::int32_t x = 0; x < setup.block.blockSize.x; ++x)
					{
						if (TriangleRasterizer::IsInside(edgesFn,topLeftRule))
						{
							TriangleRasterizer::ApplyFragment<state,blendT,depthT,writeT,fragmentShaderT>(triangle, applySetup,setup, fragmentShader);
						}
						++applySetup.imagePtr;
						++applySetup.zImagePtr;
						++applySetup.colorPtr;
						++applySetup.pixel.x;
						edgesFn += setup.edgeIncrement[0];
						applySetup.w += setup.wData.delta[0];
						applySetup.z += setup.zData.delta[0];
						applySetup.inter += setup.interData.delta[0];
					}
					currentEdgeData.edgesFn += setup.edgeIncrement[1];
					setup.wData.data += setup.wData.delta[1];
					setup.interData.data += setup.interData.delta[1];
					setup.zData.data += setup.zData.delta[1];
				}
			}
		private:
			template<fragmentState_t state,typename fragmentShaderT>
			static  fragment_output_t<state> ExecuteFragmentShader(const ApplySetup& applySetup, const BlockSetup& blockSetup,const fragmentShaderT& fragmentShader)
			{
				fragment_input_t<vertex_t> fragInput;
				float realW = 1.f / applySetup.w;

				applySetup.inter.Apply(fragInput.vertex, realW);
				blockSetup.interData.delta[0].Apply(fragInput.vertexDx, realW);
				blockSetup.interData.delta[1].Apply(fragInput.vertexDy, realW);

				fragInput.pixel = applySetup.pixel;

				fragInput.invW = applySetup.w;
				fragInput.invWDx = blockSetup.wData.delta[0];
				fragInput.invWDy = blockSetup.wData.delta[1];

				fragInput.z = applySetup.z;
				fragInput.zDx = blockSetup.zData.delta[0];
				fragInput.zDy = blockSetup.zData.delta[1];

				return fragmentShader(fragInput);
			}
			template<fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT,typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT> && state == fragmentState_t::Keep)
			static void ApplyFragment(const triangle_t& triangle,const ApplySetup& applySetup,const BlockSetup& blockSetup,const fragmentShaderT& fragmentShader)
			{

				if constexpr (depthT::zTestEnabled_v)
				{
					if (!depthT::PassDepthTest(applySetup.z, *applySetup.zImagePtr))
						return;
				}
				fragment_output_t<state> out = TriangleRasterizer::ExecuteFragmentShader < state, fragmentShaderT > (applySetup, blockSetup, fragmentShader);
				TriangleRasterizer::ApplyColor<blendT,writeT>(out.color, applySetup.imagePtr, applySetup.colorPtr);
				if constexpr (depthT::zWriteEnable_v)
				{
					*applySetup.zImagePtr = applySetup.z;
				}
				
			}
			template< fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>&& state == fragmentState_t::Discard)
			static void ApplyFragment(const triangle_t& triangle, const ApplySetup& applySetup,const  BlockSetup& blockSetup, const fragmentShaderT& fragmentShader)
			{
				if constexpr (depthT::zTestEnabled_v)
				{
					if (!depthT::PassDepthTest(applySetup.z, *applySetup.zImagePtr))
						return;
				}
				fragment_output_t<state> out = TriangleRasterizer::ExecuteFragmentShader<state, fragmentShaderT>(applySetup, blockSetup,fragmentShader);
				if (out.discard)
					return;
				if constexpr (depthT::zWriteEnable_v)
				{
					*applySetup.zImagePtr = applySetup.z;
				}
				TriangleRasterizer::ApplyColor<blendT,writeT>(out.color, applySetup.imagePtr, applySetup.colorPtr);

			}
			template< fragmentState_t state, ValidBlendData blendT, ValidDepthData depthT, ValidWriteData writeT,typename fragmentShaderT > requires(ValidFragmentFunction<fragment, state, fragmentShaderT>&& state == fragmentState_t::DepthChange)
			static void ApplyFragment(const triangle_t& triangle, const ApplySetup& applySetup,const  BlockSetup& blockSetup,const fragmentShaderT& fragmentShader)
			{
				fragment_output_t<state> out = TriangleRasterizer::ExecuteFragmentShader<state, fragmentShaderT>(applySetup, blockSetup,fragmentShader);
				if constexpr (depthT::zTestEnabled_v)
				{
					if (!depthT::PassDepthTest(out.z, *applySetup.zImagePtr))
						return;
				}

				if (out.discard)
					return;
				if constexpr (depthT::zWriteEnable_v)
				{
					*applySetup.zImagePtr = out.z;
				}
				TriangleRasterizer::ApplyColor<blendT,writeT>(out.color, applySetup.imagePtr, applySetup.colorPtr);
				
			}
			template<ValidBlendData blendT, ValidWriteData writeT>
			static void ApplyColor(const glm::vec<4,float>& srcColor, std::uint32_t* imagePtr, glm::vec<4, float>* colorPtr)
			{
				glm::vec<4, float> dstColor = *colorPtr;
				glm::vec<4, float> realColor = blendT::Blend(srcColor, dstColor);
				realColor = glm::clamp(realColor, 0.f,1.f);
				if constexpr (writeT::mask & static_cast<std::uint8_t>(writeMaks::R))
					dstColor.r = realColor.r;
				if constexpr (writeT::mask & static_cast<std::uint8_t>(writeMaks::G))
					dstColor.g = realColor.g;
				if constexpr (writeT::mask & static_cast<std::uint8_t>(writeMaks::B))
					dstColor.b = realColor.b;
				if constexpr (writeT::mask & static_cast<std::uint8_t>(writeMaks::A))
					dstColor.a = realColor.a;

				*colorPtr = dstColor;
				dstColor *= 255.f;
				const std::uint32_t r =  static_cast<std::uint32_t>(dstColor.r);
				const std::uint32_t g = static_cast<std::uint32_t>(dstColor.g);
				const std::uint32_t b = static_cast<std::uint32_t>(dstColor.b);
				const std::uint32_t a = static_cast<std::uint32_t>(dstColor.a);
				*imagePtr = r | (g << 8) | (b << 16) | (a << 24);
			}
			static std::array<glm::vec<4, float>, 3> GetPosition(const triangle_t& triangle)
			{
				std::array<glm::vec<4, float>, 3> result;
				result[0] = { 0.f,0.f,0.f,1.f };
				result[1] = { 0.f,0.f,0.f,1.f };
				result[2] = { 0.f,0.f,0.f,1.f };

				if constexpr (interpret_t::posInfo_t::n == 0)
					return result;
				else
				{
					for (std::int32_t i = 0; i < 3; ++i)
					{
						const std::byte* rawData = reinterpret_cast<const std::byte*>(&triangle[i]);
						glm::vec<interpret_t::posInfo_t::n, float> v;
						std::memcpy(&v, rawData + interpret_t::posInfo_t::offset, sizeof(v));
						std::byte* data = reinterpret_cast<std::byte*>(&result[i]);
						std::memcpy(data, &v, sizeof(v));
					}
				}

				return  result;
			}
			static float EdgeFunction(const glm::vec<2, float>& A, const glm::vec<2, float> B, const glm::vec<2, float> D)
			{
				return (D.x - A.x) * (B.y - A.y) - (D.y - A.y) * (B.x - A.x);
			}
			static bool IsTopLeft(const glm::vec<2, float>& A, const glm::vec<2,float>& B)
			{
				return ((B.y - A.y) > 0.0f || ((B.y - A.y) == 0.f) && ((B.x - A.x) < 0.f));
			}
			static float GetArea(const std::array<glm::vec<4, float>, 3>& pos)
			{
				return TriangleRasterizer::EdgeFunction(
					{ pos[0].x, pos[0].y },
					{ pos[1].x, pos[1].y },
					{ pos[2].x,pos[2].y });
			}
			
			static box2_t<int> GetBox(const std::array<glm::vec<4, float>, 3>& pos, const FDB::Rect<std::int32_t>& viewPort)
			{
				box2_t<int> box;
				box.min.x = std::max(viewPort.x, static_cast<int>(std::min({ pos[0].x, pos[1].x, pos[2].x })));
				box.max.x = std::min(viewPort.x + viewPort.width - 1, static_cast<int>(std::max({ pos[0].x, pos[1].x, pos[2].x })));
				box.min.y = std::max(viewPort.y, static_cast<int>(std::min({ pos[0].y, pos[1].y, pos[2].y })));
				box.max.y = std::min(viewPort.y + viewPort.height - 1, static_cast<int>(std::max({ pos[0].y, pos[1].y, pos[2].y })));
				return box;
			}
			static glm::vec<3, float> InitializeEdgeFunction(const std::array<glm::vec<4, float>, 3>& pos, const box2_t<std::int32_t>& box)
			{
				glm::vec<2, float> minFloat = { static_cast<float>(box.min.x) + 0.5f, static_cast<float>(box.min.y) + 0.5f };
				return {
					 TriangleRasterizer::EdgeFunction({pos[0].x, pos[0].y},{ pos[1].x,pos[1].y},{ minFloat.x, minFloat.y})
					,TriangleRasterizer::EdgeFunction({pos[1].x, pos[1].y},{ pos[2].x, pos[2].y},{minFloat.x, minFloat.y})
					,TriangleRasterizer::EdgeFunction({pos[2].x, pos[2].y},{ pos[0].x,pos[0].y}, {minFloat.x, minFloat.y}) };
			}
			static std::array < glm::vec<2, float>, 3> InitializeEdgeDelta(const std::array<glm::vec<4, float>, 3>& pos)
			{
				return { glm::vec<2, float>{pos[1].y - pos[0].y,-(pos[1].x - pos[0].x) }
						  ,glm::vec<2, float>{ pos[2].y - pos[1].y,-(pos[2].x - pos[1].x) }
						  ,glm::vec<2, float>{ pos[0].y - pos[2].y,-(pos[0].x - pos[2].x) }
				};
			}
			static edgesData_t InitializeStepEdgeData(glm::vec<2, std::int32_t> block, const box2_t<std::int32_t>& box, const glm::vec<2, std::int32_t>& blockSize, const edgesData_t& baseEdge)
			{
				glm::vec<2, float> step = { block.x - box.min.x,block.y - box.min.y };
				glm::vec<2, float> blockStep = { blockSize.x - 1 ,blockSize.y - 1 };

				edgesData_t result;
				result.edgesFn[0] = baseEdge.edgesFn.x + baseEdge.edgesFnDelta[0].y * step.y + baseEdge.edgesFnDelta[0].x * step.x;
				result.edgesFn[1] = baseEdge.edgesFn.y + baseEdge.edgesFnDelta[1].y * step.y + baseEdge.edgesFnDelta[1].x * step.x;
				result.edgesFn[2] = baseEdge.edgesFn.z + baseEdge.edgesFnDelta[2].y * step.y + baseEdge.edgesFnDelta[2].x * step.x;

				result.edgesFnDelta[0] = { baseEdge.edgesFnDelta[0] * blockStep };
				result.edgesFnDelta[1] = { baseEdge.edgesFnDelta[1] * blockStep };
				result.edgesFnDelta[2] = { baseEdge.edgesFnDelta[2] * blockStep };

				return result;
			}
			static bool IsBlockOut(const edgesData_t& blockEdgeData)
			{

				float abBlockEdgeMax = blockEdgeData.edgesFn[0] + std::max(blockEdgeData.edgesFnDelta[0].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[0].y, 0.f);
				float bcBlockEdgeMax = blockEdgeData.edgesFn[1] + std::max(blockEdgeData.edgesFnDelta[1].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[1].y, 0.f);
				float caBlockEdgeMax = blockEdgeData.edgesFn[2] + std::max(blockEdgeData.edgesFnDelta[2].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[2].y, 0.f);

				if (abBlockEdgeMax < 0.f || bcBlockEdgeMax < 0.f || caBlockEdgeMax < 0.f)
					return true;
				return false;
			}
			static bool IsBlockFullyInside(const edgesData_t& blockEdgeData)
			{
				float abBlockEdgeMin = blockEdgeData.edgesFn[0] + std::min(blockEdgeData.edgesFnDelta[0].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[0].y, 0.f);
				float bcBlockEdgeMin = blockEdgeData.edgesFn[1] + std::min(blockEdgeData.edgesFnDelta[1].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[1].y, 0.f);
				float caBlockEdgeMin = blockEdgeData.edgesFn[2] + std::min(blockEdgeData.edgesFnDelta[2].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[2].y, 0.f);

				if (abBlockEdgeMin >= 0.f && bcBlockEdgeMin >= 0.f && caBlockEdgeMin >= 0.f)
					return true;
				return false;
			}
			static glm::vec<3, float> GetInvW(const std::array<glm::vec<4, float>, 3>& position)
			{
				return { 1.f / position[0].w ,1.f / position[1].w, 1.f / position[2].w };
			}
			static deltaData_t<float> SetupLinearData(glm::vec<3, float> base, const edgesData_t& baseEdgeData, float invArea)
			{
				deltaData_t<float> result;
				result.data = {};
				result.delta[0] = (baseEdgeData.edgesFnDelta[0].x * base[2] + baseEdgeData.edgesFnDelta[1].x * base[0] + baseEdgeData.edgesFnDelta[2].x * base[1]) * invArea;
				result.delta[1] = (baseEdgeData.edgesFnDelta[0].y * base[2] + baseEdgeData.edgesFnDelta[1].y * base[0] + baseEdgeData.edgesFnDelta[2].y * base[1]) * invArea;
				return result;
			}
			static void UpdateLinearData(glm::vec<3, float> base, const edgesData_t& blockEdgeData, float invArea, deltaData_t<float>& data)
			{
				data.data = (blockEdgeData.edgesFn[0] * base[2] + blockEdgeData.edgesFn[1] * base[0] + blockEdgeData.edgesFn[2] * base[1]) * invArea;
			}
			static deltaData_t<attributeData_t> SetupInterData(const  std::array<attributeData_t, 3>& startInter, const edgesData_t& baseEdgeData, float invArea)
			{
				deltaData_t<attributeData_t> result;
				result.data = {};
				result.delta[0].InitGradiant<true>(startInter, baseEdgeData, invArea);
				result.delta[1].InitGradiant<false>(startInter, baseEdgeData, invArea);
				return result;
			}
			static void UpdateInterData(const  std::array<attributeData_t, 3>& startInter, const edgesData_t& blockEdgeData, float invArea, deltaData_t<attributeData_t>& interData)
			{
				interData.data.InitOrigin(startInter, blockEdgeData, invArea);
			}

		};
	}
	

	


	static constexpr std::int32_t windowWidth = 800;
	static constexpr std::int32_t windowHeight = 600;

	static constexpr std::int32_t renderWidth = 320;

	static constexpr std::int32_t renderHeight = 240;

    // TODO bouger dans un fichier a part 
	// TODO zoption bias blend 
	int main(int argc, char* argv[])
	{
		FDB::Sdl::Application::Init(SDL_INIT_VIDEO);
		FDB::Sdl::Window window("test", { windowWidth,windowHeight }, SDL_WINDOW_RESIZABLE);
		FDB::Sdl::Renderer renderer(window);
		//renderer.SetVSync(1);

		glm::vec<2, std::int32_t> size = { renderWidth,renderHeight };

		FDB::Sdl::Texture texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, size);
		std::vector<std::uint32_t> frameBuffer;
		std::vector<float> zBuffer;
		std::vector<glm::vec<4,float>> colorBuffer;
		zBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		colorBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));

		Transform::Vertex a,b,c,d;

		Uint64 lastTime = SDL_GetTicks();
		std::uint32_t frameCount = 0;
		while (!FDB::Sdl::Application::ShouldQuit() && !window.ShouldClose())
		{
			FDB::Sdl::Application::PollEvent<FDB::Sdl::Event::DefaultManager::Process>();
			// TODO refact fps
			{
				++frameCount;

				Uint64 currentTime = SDL_GetTicks();

				if (currentTime - lastTime >= 1000)
				{
					float fps = static_cast<float>(frameCount) * 1000.f /
						static_cast<float>(currentTime - lastTime);

					std::string title = std::to_string(static_cast<int>(fps));

					SDL_SetWindowTitle(
						window.GetSdlWindow(),
						title.c_str());

					frameCount = 0;
					lastTime = currentTime;
				}
			}

			std::memset(frameBuffer.data(), 0, frameBuffer.size() * sizeof(std::uint32_t));
			std::fill(zBuffer.begin(), zBuffer.end(), 1.f);
			std::fill(colorBuffer.begin(), colorBuffer.end(), glm::vec<4,float>(0,0,0,0));


			std::vector<Transform::Vertex> vertexList;
			std::vector<unsigned int> indices;
			//TODO vertex gestion
			{
				a.pos = glm::vec4(-1.0f, -0.8f, -1.0f, 1.0f);
				b.pos = glm::vec4(0.0f, 0.8f, -1.0f, 1.0f);
				c.pos = glm::vec4(1.0f, -0.8f, -1.0f, 1.0f);
				a.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				b.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				c.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			}
			vertexList.push_back(a);
			vertexList.push_back(b);
			vertexList.push_back(c);
			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(2);


			FDB::Rect<std::int32_t> port;

			renderer.Clear(FDB::Color::black8);
			port.x = 0;
			port.y =0;
			port.width = size.x;
			port.height = size.y;

			FDB::Pipeline::Image<std::uint32_t> image;
			image.buffer = frameBuffer.data();
			image.bufferHeight = size.y;
			image.bufferWidth = size.x;
			FDB::Pipeline::Image<glm::vec<4,float>> color_image;
			color_image.buffer = colorBuffer.data();
			color_image.bufferHeight = size.y;
			color_image.bufferWidth = size.x;
			FDB::Pipeline::Image<float> z_image;
			z_image.buffer = zBuffer.data();
			z_image.bufferHeight = size.y;
			z_image.bufferWidth = size.x;
			auto bench = [&](auto fn, const char* name)
				{
					static constexpr int n = 1000;
					const double freq = double(SDL_GetPerformanceFrequency());
					Uint64 s = SDL_GetPerformanceCounter();
					for (int i = 0; i < n; ++i) fn();
					Uint64 e = SDL_GetPerformanceCounter();

					double totalMs = double(e - s) * 1000.0 / freq;
					printf("%s : %.4f ms/appel (%d iterations, %.2f ms total)\n",
						name, totalMs / n, n, totalMs);
				};
			//bench([&] { FDB::Pipeline::Rasterizer::TriangleRasterizer<Vertex>::Compute<Test2::FragmentState::Keep>({ a,c,b }, port, image, [&](const Vertex& vertex, const glm::vec<2, std::int32_t>& pixel, float) -> Test2::fragment_output_t<Test2::FragmentState::Keep> { return  Test2::fragment_output_t<Test2::FragmentState::Keep>::Color(vertex.color);});}, "Rasterizer V2");
			//bench([&] { Rasterizer::Compute<Vertex, VertexInterpolation>({ a, c, b }, port, frameBuffer.data(), zBuffer.data(), size.x); }, "Rasterizer V1");
			//Rasterizer::Compute<Vertex,VertexInterpolation>({ a, c, b }, port, frameBuffer.data(),zBuffer.data(), size.x);


			using rasterizer_t = FDB::Pipeline::Rasterizer::TriangleRasterizer<Transform::Vertex>;
			using fragmentState_t = FDB::Pipeline::Rasterizer::FragmentState;
			using fragmentOutput = FDB::Pipeline::Rasterizer::fragment_output_t<fragmentState_t::Keep>;
			using fragmentInput_t = FDB::Pipeline::Rasterizer::FragmentInput<Transform::Vertex>;
			Transform::Mesh mesh;
			mesh.vertices = vertexList;
			mesh.indices = indices;
			auto fragmentShader = [&](const fragmentInput_t& input) { return  fragmentOutput::Color(input.vertex.color);};

			auto processedTriangles = Pipeline::createSommetList(vertexList, indices, port);
			rasterizer_t::ImagesData imageData;
			imageData.colorImage = color_image;
			imageData.zBuffer = z_image;
			imageData.finalImage = image;
			using cullType = FDB::Pipeline::Rasterizer::cullData_t<FDB::Pipeline::Rasterizer::CullFace::CCW, FDB::Pipeline::Rasterizer::CullMode::Back>;
			if (!processedTriangles.empty()) {

				rasterizer_t::Compute<fragmentState_t::Keep, cullType>({ processedTriangles[0],processedTriangles[1],processedTriangles[2] }, port, imageData, fragmentShader);
			}
			
			texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
			renderer.RenderTexture(texture);
			renderer.Present();
		}
		FDB::Sdl::Application::Destroy();
	}




