
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

	namespace OLD
	{
		
	

	constexpr int SubPixelBits = 8;
	constexpr int SubPixelScale = 1 << SubPixelBits;

	static int32_t ToFixed(float v)
	{
		return static_cast<int32_t>(std::round(v * SubPixelScale));
	}

	struct RasterizerData
	{
		RasterizerData(const glm::vec<4, float>(&pos)[3], const  box2_t<int>& box)
		{
			glm::vec<2, float> minF = { static_cast<float>(box.min.x) , static_cast<float>(box.min.y) };

			edgesFn = { RasterizerData::EdgeFunction(pos[0].x, pos[0].y, pos[1].x,pos[1].y, minF.x, minF.y)
				,RasterizerData::EdgeFunction(pos[1].x, pos[1].y, pos[2].x, pos[2].y,minF.x, minF.y)
				,RasterizerData::EdgeFunction(pos[2].x, pos[2].y, pos[0].x,pos[0].y, minF.x, minF.y) };
			

			baseIncrement[0] = glm::vec<2, float>{ (pos[1].y - pos[0].y),-(pos[1].x - pos[0].x) };
			baseIncrement[1] = glm::vec<2, float>{ (pos[2].y - pos[1].y),-(pos[2].x - pos[1].x) };
			baseIncrement[2] = glm::vec<2, float>{ (pos[0].y - pos[2].y),-(pos[0].x - pos[2].x) };
		}
		RasterizerData(const RasterizerData& other, const glm::vec<2, float>& step, const glm::vec<2, float> blockStep)
		{

			edgesFn = {
				other.edgesFn[0] + other.baseIncrement[0].y * step.y + other.baseIncrement[0].x * step.x
				, other.edgesFn[1] + other.baseIncrement[1].y * step.y + other.baseIncrement[1].x * step.x
				,other.edgesFn[2] + other.baseIncrement[2].y * step.y + other.baseIncrement[2].x * step.x
			};

			baseIncrement[0] = { other.baseIncrement[0] * blockStep };
			baseIncrement[1] = { other.baseIncrement[1] * blockStep };
			baseIncrement[2] = { other.baseIncrement[2] * blockStep };
		}
		glm::vec<3, float> edgesFn;
		glm::vec<2, float> baseIncrement[3];
		//https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf
		static float EdgeFunction(float Ax, float Ay, float Bx, float By, float Dx, float Dy)
		{
			return (Dx - Ax) * (By - Ay) - (Dy - Ay) * (Bx - Ax);
		}
	};

	template<typename VertexType, typename VertexInterpolationType>
	concept IsValidInterpolation = requires(VertexType vertex, VertexInterpolationType interpolation, const VertexType(&triangle)[3], glm::vec< 3, float> invW, const RasterizerData & data, glm::vec<4, float>(&pos)[3])
	{
		VertexInterpolationType{ triangle,invW };
		interpolation.Process(data, data,invW[0]);
		interpolation.StartData();
		interpolation.IncrementY();
		interpolation.IncrementX();
		interpolation.ApplyFragment(vertex);
		VertexInterpolationType::GetInvW(triangle, invW);
		VertexInterpolationType::GetPos(triangle, pos);
	};

	template<std::size_t N>
	struct FloatInterpolation
	{
		FloatInterpolation() = default;
		~FloatInterpolation() = default;
		void Init(const glm::vec<N,float>(&pos)[3], const glm::vec<3, float>& invW)
		{
			dataOverW[0] = pos[0]* invW[0];
			dataOverW[1] = pos[1] * invW[1];
			dataOverW[2] = pos[2] * invW[2];
		}
		void Init(const glm::vec<3, float>& invW)
		{
			dataOverW[0] = glm::vec<N,float>(1.0f)  * invW[0];
			dataOverW[1] = glm::vec<N, float>(1.0f) * invW[1];
			dataOverW[2] = glm::vec<N, float>(1.0f) * invW[2];
		}
		void Process(const RasterizerData& data, const RasterizerData& base , float invArea)
		{
			startData = (data.edgesFn.x * dataOverW[2]
				+ data.edgesFn.y * dataOverW[0]
				+ data.edgesFn.z * dataOverW[1]) * invArea;
			dataIncrement[0] = (base.baseIncrement[0].x * dataOverW[2]
				+ base.baseIncrement[1].x * dataOverW[0]
				+ base.baseIncrement[2].x * dataOverW[1]) * invArea;
			dataIncrement[1] = (base.baseIncrement[0].y * dataOverW[2]
				+ base.baseIncrement[1].y * dataOverW[0]
				+ base.baseIncrement[2].y * dataOverW[1]) * invArea;
		}
		void StartData()
		{
			currentData = startData;
		}
		void IncrementX()
		{
			currentData += dataIncrement[0];
		}
		void IncrementY()
		{
			startData += dataIncrement[1];
		}
		void ApplyValue(glm::vec<N,float>& data) const
		{
			data = currentData;
		}
	private:
		glm::vec<N, float> dataOverW[3];
		glm::vec<N, float>  startData;
		glm::vec<N,float> dataIncrement[2];
		glm::vec<N, float> currentData;
	};


	struct Rasterizer
	{
		// TODO passer en fix point 
		// TODO simd
		static  constexpr  glm::vec<2, int> blockDim = { 64,64 };


		template<typename VertexInput, typename InterpolationT> requires(IsValidInterpolation<VertexInput, InterpolationT>)
			static void Compute(const VertexInput(&tri)[3], const FDB::Rect<std::int32_t>& viewPort, std::uint32_t* buffer, float* zbuffer, std::int32_t bufferWidth)
		{
			glm::vec<4, float> pos[3];
			InterpolationT::GetPos(tri, pos);
			float area = Rasterizer::GetArea(pos);
			if (area <= 0)
				return;
			float invArea = 1.f / area;

			box2_t<int> box = Rasterizer::GetBox(pos, viewPort);
			
			glm::vec<3, float> baseInvW;
			InterpolationT::GetInvW(tri, baseInvW);
			FloatInterpolation<1> valW;
			valW.Init(baseInvW);
			InterpolationT inter(tri, baseInvW);
			RasterizerData baseData(pos, box);
			glm::vec<3, float> baseEdgeIncrement[2] = { glm::vec<3, float>(baseData.baseIncrement[0].x, baseData.baseIncrement[1].x, baseData.baseIncrement[2].x), glm::vec<3, float>(baseData.baseIncrement[0].y, baseData.baseIncrement[1].y, baseData.baseIncrement[2].y) };


			for (int blockY = box.min.y; blockY <= box.max.y; blockY += blockDim.y)
			{
				glm::vec<2, int> blockSize = {};
				blockSize.y = std::min(blockDim.y, box.max.y - blockY + 1);
				for (int blockX = box.min.x; blockX <= box.max.x; blockX += blockDim.x)
				{
					blockSize.x = std::min(blockDim.x, box.max.x - blockX + 1);
					glm::vec<2, float> step = { blockX - box.min.x,blockY - box.min.y };
					glm::vec<2, float> blockStep = { blockSize.x - 1 ,blockSize.y - 1 };
					RasterizerData currentData(baseData, step, blockStep);

					if (IsBlockOut(currentData.edgesFn, currentData.baseIncrement))
						continue;

					valW.Process(currentData, baseData, invArea);
					inter.Process(currentData, baseData,invArea);
					if (IsBlockFullyInside(currentData.edgesFn, currentData.baseIncrement))
					{
						for (int y = 0; y < blockSize.y; ++y)
						{
							valW.StartData();
							inter.StartData();
							std::uint32_t* pixelPtr = buffer + ((blockY + y) * bufferWidth + blockX);
							float* zBufferPtr = zbuffer + ((blockY + y) * bufferWidth + blockX);
							for (int x = 0; x < blockSize.x; ++x)
							{
								{
									glm::vec<1, float> test;
									valW.ApplyValue(test);
									float currentW = 1.f / test.x;
									VertexInput fragment = tri[0];
									inter.ApplyFragment(fragment);
									glm::vec<4, float> finalColor = fragment.color * 255.f * currentW;
									std::uint32_t r = static_cast<std::uint32_t>(finalColor.r);
									std::uint32_t g = static_cast<std::uint32_t>(finalColor.g);
									std::uint32_t b = static_cast<std::uint32_t>(finalColor.b);
									std::uint32_t a = static_cast<std::uint32_t>(finalColor.a);
									*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
								}
								++pixelPtr;
								++zBufferPtr;
								valW.IncrementX();
								inter.IncrementX();
							}
							valW.IncrementY();
							inter.IncrementY();
						}

					}
					else
					{
						for (int y = 0; y < blockSize.y; ++y)
						{
							glm::vec<3, float> edgeFn = currentData.edgesFn;
							valW.StartData();
							inter.StartData();
							std::uint32_t* pixelPtr = buffer + ((blockY + y) * bufferWidth + blockX);
							for (int x = 0; x < blockSize.x; ++x)
							{
								if (edgeFn[0] >= 0 && edgeFn[1] >= 0 && edgeFn[2] >= 0)
								{
									{
										glm::vec<1, float> test;
										valW.ApplyValue(test);
										float currentW = 1.f / test.x;
										VertexInput fragment = tri[0];
										inter.ApplyFragment(fragment);
										glm::vec<4, float> finalColor = fragment.color * 255.f * currentW;
										std::uint32_t r = static_cast<std::uint32_t>(finalColor.r);
										std::uint32_t g = static_cast<std::uint32_t>(finalColor.g);
										std::uint32_t b = static_cast<std::uint32_t>(finalColor.b);
										std::uint32_t a = static_cast<std::uint32_t>(finalColor.a);
										*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
									}
								}
								++pixelPtr;
								edgeFn += baseEdgeIncrement[0];
								valW.IncrementX();
								inter.IncrementX();
							}

							currentData.edgesFn += baseEdgeIncrement[1];
							valW.IncrementY();
							inter.IncrementY();
						}
					}
				}
			}
		}
	private:
		static float GetArea(const glm::vec<4, float>(&pos)[3])
		{
			return RasterizerData::EdgeFunction(
				pos[0].x, pos[0].y,
				pos[1].x, pos[1].y,
				pos[2].x,
				pos[2].y);
		}
		static box2_t<int> GetBox(const glm::vec<4, float>(&pos)[3], const FDB::Rect<std::int32_t>& viewPort)
		{
			box2_t<int> box;
			box.min.x = std::max(viewPort.x, static_cast<int>(std::min({ pos[0].x, pos[1].x, pos[2].x })));
			box.max.x = std::min(viewPort.x + viewPort.width - 1, static_cast<int>(std::max({ pos[0].x, pos[1].x, pos[2].x })));
			box.min.y = std::max(viewPort.y, static_cast<int>(std::min({ pos[0].y, pos[1].y, pos[2].y })));
			box.max.y = std::min(viewPort.y + viewPort.height - 1, static_cast<int>(std::max({ pos[0].y, pos[1].y, pos[2].y })));
			return box;
		}
		static bool IsBlockOut(const glm::vec<3, float>& blockEdgesFn, const glm::vec<2, float>(&increment)[3])
		{

			float abBlockEdgeMax = blockEdgesFn[0] + std::max(increment[0].x, 0.f) + std::max(increment[0].y, 0.f);
			float bcBlockEdgeMax = blockEdgesFn[1] + std::max(increment[1].x, 0.f) + std::max(increment[1].y, 0.f);
			float caBlockEdgeMax = blockEdgesFn[2] + std::max(increment[2].x, 0.f) + std::max(increment[2].y, 0.f);

			if (abBlockEdgeMax < 0.f || bcBlockEdgeMax < 0.f || caBlockEdgeMax < 0.f)
				return true;
			return false;
		}
		static bool IsBlockFullyInside(const glm::vec<3, float>& blockEdgesFn, const glm::vec<2, float>(&increment)[3])
		{

			float abBlockEdgeMin = blockEdgesFn[0] + std::min(increment[0].x, 0.f) + std::min(increment[0].y, 0.f);
			float bcBlockEdgeMin = blockEdgesFn[1] + std::min(increment[1].x, 0.f) + std::min(increment[1].y, 0.f);
			float caBlockEdgeMin = blockEdgesFn[2] + std::min(increment[2].x, 0.f) + std::min(increment[2].y, 0.f);

			if (abBlockEdgeMin >= 0.f && bcBlockEdgeMin >= 0.f && caBlockEdgeMin >= 0.f)
				return true;
			return false;
		}
	};


	namespace Test::Attribute
	{
		
	}

	

	//struct VertexInterpolation
	//{
	//	 VertexInterpolation(const triangle& tri, const glm::vec<3, float>& invW)
	//	 {
	//		glm::vec<4,float> poss[3] = {tri[0].pos,tri[1].pos,tri[2].pos};
	//		glm::vec<4, float> colors[3] = { tri[0].color,tri[1].color,tri[2].color };

	//		color.Init(colors, invW);
	//		position.Init(poss, invW);

	//	 }
	//	 void Process(const RasterizerData& data,const RasterizerData& base,float invArea)
	//	 {
	//		color.Process(data,base,invArea);
	//		position.Process(data, base,invArea);
	//	 }
	//	 void StartData()
	//	 {
	//		 color.StartData();
	//		 position.StartData();
	//	 }
	//	 void IncrementY()
	//	 {
	//		 color.IncrementY();
	//		 position.IncrementY();
	//	 }
	//	 void IncrementX()
	//	 {
	//		 color.IncrementX();
	//		 position.IncrementX();
	//	 }
	//	 void ApplyFragment(Vertex& fragment) const 
	//	 {
	//		 color.ApplyValue(fragment.color);
	//		 position.ApplyValue(fragment.pos);
	//	 }
	//	
	//	 static void GetPos(const triangle& tri, glm::vec<4, float>(&pos)[3])
	//	 {
	//		 pos[0] = tri[0].pos;
	//		 pos[1] = tri[1].pos;
	//		 pos[2] = tri[2].pos;
	//	 }
	//	 static void GetInvW(const triangle& tri,glm::vec<3,float>& vector)
	//	 {
	//		 vector = { 1.f / tri[0].pos.w ,1.f / tri[1].pos.w, 1.f / tri[2].pos.w };
	//	 }
	//private:
	//	FloatInterpolation<4> color;
	//	FloatInterpolation<4> position;
	//};

	}
#include <type_traits>
#include <array>
	namespace FDB::Pipeline
	{
		template<typename T>
		struct Image
		{
			using value_type = T;
			value_type* buffer;
			std::int32_t bufferWidth;
			std::int32_t bufferHeight;
		};
	}



	// TODO MOVE TO FILE ATTRIBUTE
	#include <array>
	#include <limits>
	namespace FDB::Pipeline::Attribute
	{
			enum class format : std::uint32_t
			{
				Float1,
				Float2,
				Float3,
				Float4,
				None
			};
			struct Data
			{
				format format = format::None;
				std::size_t offset = std::numeric_limits<std::size_t>::max();
			};
			template<typename T>
			struct is_attributes_array : std::false_type {};
			template<std::size_t N>
			struct is_attributes_array<std::array<FDB::Pipeline::Attribute::Data, N>> : std::true_type {};
			template<typename T>
			static constexpr bool is_attributes_array_v = is_attributes_array<T>::value;

			template<typename T>
			concept ValidVertexT = requires()
			{
				{ T::PosAttribute() } -> std::same_as<FDB::Pipeline::Attribute::Data>;
			}&& is_attributes_array_v<decltype(T::AllAttributes())> && (T::AllAttributes().size() > 0);


			template<ValidVertexT T>
			struct Interpret
			{
				using value_type = T;
				static constexpr std::size_t structSize = sizeof(value_type);
				static constexpr auto attributes = value_type::AllAttributes();
				static constexpr FDB::Pipeline::Attribute::Data posAttribute = value_type::PosAttribute();
				static constexpr std::size_t attributesSize = attributes.size();
				
				template<FDB::Pipeline::Attribute::Data AttribV>
				struct InfoDataFloat
				{
					static constexpr FDB::Pipeline::Attribute::Data attrib = AttribV;
					static constexpr std::size_t offset = attrib.offset;
					static constexpr std::int32_t n =
						attrib.format == FDB::Pipeline::Attribute::format::Float1 ? 1 :
						attrib.format == FDB::Pipeline::Attribute::format::Float2 ? 2 :
						attrib.format == FDB::Pipeline::Attribute::format::Float3 ? 3 :
						attrib.format == FDB::Pipeline::Attribute::format::Float4 ? 4 : 0;
				};

				template<std::size_t i>
				using infoFloat_t = InfoDataFloat<attributes[i]>;

				using posInfo_t = InfoDataFloat<posAttribute>;
			};

		
	}
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
		enum class BlendOp;

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


		template<typename fragmentT,FragmentState state,typename FN> 
		concept ValidFragmentFunction = std::is_invocable_r_v<fragment_output_t<state>, FN, const fragmentT&, const glm::vec<2, std::int32_t>&, float>;
	
		
		enum class BlendFactor
		{
			Zero,
			rOne,
			SrcColor,
			OneMinusSrcColor,
			DstColor,
			OneMinusDstColor,
			SrcAlpha,
			OneMinusSrcAlpha,
			DstAlpha,
			OneMinusDstAlpha
		};

		enum class BlendOp
		{
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
				else if constexpr (dataT == BlendFactor::rOne)
				{
					return glm::vec<4, float>(1.f);
				}
				else if constexpr (dataT == BlendFactor::SrcColor)
				{
					return srcColor;
				}
				else if constexpr (dataT == BlendFactor::OneMinusSrcColor)
				{
					static constexpr  glm::vec<4, float> vec1 = glm::vec<4, float>(1.f);
					return  vec1 - srcColor;
				}
				else if constexpr (dataT == BlendFactor::DstColor)
				{
					return dstColor;
				}
				else if constexpr (dataT == BlendFactor::OneMinusDstColor)
				{
					static constexpr  glm::vec<4, float> vec1 = glm::vec<4, float>(1.f);
					return vec1 - dstColor;
				}
				else if constexpr (dataT == BlendFactor::SrcAlpha)
				{
					return glm::vec<4, float>(srcColor.a);
				}
				else if constexpr (dataT == BlendFactor::OneMinusSrcAlpha)
				{
					static constexpr  glm::vec<4, float> vec1 = glm::vec<4, float>(1.f);
					return vec1 - glm::vec<4, float>(srcColor.a);
				}
				else if constexpr (dataT == BlendFactor::DstAlpha)
				{
					static constexpr  glm::vec<4, float> vec1 = glm::vec<4, float>(1.f);
					return glm::vec<4, float>(dstColor.a);
				}
				else
				{
					static constexpr  glm::vec<4, float> vec1 = glm::vec<4, float>(1.f);
					return vec1 - glm::vec<4, float>(dstColor.a);
				}
			}
			template<BlendOp opT>
			static glm::vec<4,float> ColorToBlend(const glm::vec<4, float>& srcColor, const glm::vec<4, float>& srcFactor, const glm::vec<4, float>& dstColor, const glm::vec<4, float>& dstFactor)
			{
				if constexpr (opT == BlendOp::Add)
				{
					return srcColor* srcFactor + dstColor * dstFactor;
				}
				else if constexpr (opT == BlendOp::Sub)
				{
					return srcColor * srcFactor - dstColor * dstFactor;
				}
				else if constexpr (opT == BlendOp::RSub)
				{
					return   dstColor * dstFactor - srcColor * srcFactor;
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
	
		template<BlendFactor factorSrcT,BlendFactor factorDstT,BlendOp opT>
		struct BlendData
		{
			using color_t = glm::vec<4, float>;
			static color_t Blend(const color_t& srcColor ,const color_t& dstColor)
			{
				return ColorHelper::ColorToBlend<opT>(srcColor, ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor), dstColor, ColorHelper::FactorToColor<factorSrcT>(srcColor, dstColor));
			}
		private:
			

		};
		
	
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
			template< fragmentState_t state,typename fragmentShaderT> requires(ValidFragmentFunction<fragment,state,fragmentShaderT>)
			static void Compute(const triangle_t& triangle, const viewPort_t& viewPort, Image<std::uint32_t>& image,Image<float>& zImage,const fragmentShaderT& fragmentShader)
			{
				// position
				const std::array<glm::vec<4, float>, 3> position = TriangleRasterizer::GetPosition(triangle);
				// positon
				// area
				float area = TriangleRasterizer::GetArea(position);
				if (!TriangleRasterizer::IsValidArea(area))
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

				TriangleRasterizer::Loop<state,fragmentShaderT>(setup, image,zImage,fragmentShader);
			}
		private:

			template<fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void Loop(const TriangleSetup& setup, Image<std::uint32_t>& image, Image<float>& zImage,const fragmentShaderT& fragmentShader)
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
							TriangleRasterizer::PixelLoopInside<state,fragmentShaderT>(setup.triangle, blockSetup, image,zImage,fragmentShader);
						}
						else
						{
							TriangleRasterizer::PixelLoopClassic<state,fragmentShaderT>(setup.triangle,setup.topLeftRule, blockEdgeData, blockSetup, image,zImage,fragmentShader);
						}
					}
				}
			}




			template< fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void PixelLoopInside(const triangle_t& triangle, BlockSetup& setup, Image<std::uint32_t>& image, Image<float>& zImage,const fragmentShaderT& fragmentShader)
			{
				for (std::int32_t y = 0; y < setup.block.blockSize.y; ++y)
				{
					float w = setup.wData.data;
					float z = setup.zData.data;
					attributeData_t inter = setup.interData.data;
					glm::vec<2, std::int32_t> pixel{ setup.block.blockOrigin.x,setup.block.blockOrigin.y + y };
					std::uint32_t* imagePtr = image.buffer + (pixel.y * image.bufferWidth + pixel.x);
					float* zImagePtr = zImage.buffer + (pixel.y * zImage.bufferWidth + pixel.x);
					for (std::int32_t x = 0; x < setup.block.blockSize.x; ++x)
					{
						TriangleRasterizer::ApplyFragment<state, fragmentShaderT>(triangle, inter, w, z, pixel, imagePtr, zImagePtr, fragmentShader);
						++zImagePtr;
						++imagePtr;
						++pixel.x;
						w += setup.wData.delta[0];
						z += setup.zData.delta[0];
						inter += setup.interData.delta[0];
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

			template<fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>)
			static void PixelLoopClassic(const triangle_t& triangle,const std::array<bool,3>& topLeftRule, edgesData_t& currentEdgeData, BlockSetup& setup, Image<std::uint32_t>& image, Image<float>& zImage,const fragmentShaderT& fragmentShader)
			{
				for (std::int32_t y = 0; y < setup.block.blockSize.y; ++y)
				{
					glm::vec<3, float> edgesFn = currentEdgeData.edgesFn;
					float w = setup.wData.data;
					float z = setup.zData.data;
					attributeData_t inter = setup.interData.data;

					glm::vec<2, std::int32_t> pixel { setup.block.blockOrigin.x,setup.block.blockOrigin.y + y };
					std::uint32_t* imagePtr = image.buffer + (pixel.y * image.bufferWidth + pixel.x);
					float* zImagePtr = zImage.buffer + (pixel.y * zImage.bufferWidth + pixel.x);

					for (std::int32_t x = 0; x < setup.block.blockSize.x; ++x)
					{
						if (TriangleRasterizer::IsInside(edgesFn,topLeftRule))
						{
							
							TriangleRasterizer::ApplyFragment<state, fragmentShaderT>(triangle, inter, w, z, pixel, imagePtr, zImagePtr, fragmentShader);
						}
						++imagePtr;
						++zImagePtr;
						++pixel.x;
						edgesFn += setup.edgeIncrement[0];
						w += setup.wData.delta[0];
						z += setup.zData.delta[0];
						inter += setup.interData.delta[0];
					}
					currentEdgeData.edgesFn += setup.edgeIncrement[1];
					setup.wData.data += setup.wData.delta[1];
					setup.interData.data += setup.interData.delta[1];
					setup.zData.data += setup.zData.delta[1];
				}
			}
		private:
			template<fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT> && state == fragmentState_t::Keep)
			static void ApplyFragment(const triangle_t& triangle,const attributeData_t& inter,float w,float z,const glm::vec<2,float>& pixel, std::uint32_t* imagePtr, float* zImagePtr,const fragmentShaderT& fragmentShader)
			{
				if (z >= *zImagePtr)
					return;
				float currentW = 1.f / w;
				vertex_t fragment = triangle[0];
				inter.Apply(fragment, currentW);
			    fragment_output_t<state> out = fragmentShader(fragment, pixel, z);
				*zImagePtr = z;
				glm::vec4 fragColor = out.color * 255.f;
				std::uint32_t r = static_cast<std::uint32_t>(fragColor.x);
				std::uint32_t g = static_cast<std::uint32_t>(fragColor.y);
				std::uint32_t b = static_cast<std::uint32_t>(fragColor.z);
				std::uint32_t a = static_cast<std::uint32_t>(fragColor.a);
				*imagePtr = (r) | (g << 8) | (b << 16) | (a << 24);
				
			}
			template< fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>&& state == fragmentState_t::Discard)
			static void ApplyFragment(const triangle_t& triangle, const attributeData_t& inter, float w, float z, const glm::vec<2, float>& pixel, std::uint32_t* imagePtr, float* zImagePtr, const fragmentShaderT& fragmentShader)
			{
				if (z >= *zImagePtr)
					return;
				float currentW = 1.f / w;
				vertex_t fragment = triangle[0];
				inter.Apply(fragment, currentW);
				fragment_output_t<state> out = fragmentShader(fragment, pixel, z);
				if (out.discard)
					return;
				*zImagePtr = z;
				glm::vec4 fragColor = out.color * 255.f;
				std::uint32_t r = static_cast<std::uint32_t>(fragColor.x);
				std::uint32_t g = static_cast<std::uint32_t>(fragColor.y);
				std::uint32_t b = static_cast<std::uint32_t>(fragColor.z);
				std::uint32_t a = static_cast<std::uint32_t>(fragColor.a);
				*imagePtr = (r) | (g << 8) | (b << 16) | (a << 24);
			}
			template< fragmentState_t state, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, state, fragmentShaderT>&& state == fragmentState_t::DepthChange)
			static void ApplyFragment(const triangle_t& triangle, const attributeData_t& inter, float w, float z, const glm::vec<2, float>& pixel, std::uint32_t* imagePtr, float* zImagePtr, const fragmentShaderT& fragmentShader)
			{
				vertex_t fragment = triangle[0];
				float currentW = 1.f / w;
				inter.Apply(fragment, currentW);
				fragment_output_t<state> out = fragmentShader(fragment, pixel, z);
				if (out.discard || out.z >= *zImagePtr)
					return;
				*zImagePtr = out.z;
				glm::vec4 fragColor = out.color * 255.f;
				std::uint32_t r = static_cast<std::uint32_t>(fragColor.x);
				std::uint32_t g = static_cast<std::uint32_t>(fragColor.y);
				std::uint32_t b = static_cast<std::uint32_t>(fragColor.z);
				std::uint32_t a = static_cast<std::uint32_t>(fragColor.a);
				*imagePtr = (r) | (g << 8) | (b << 16) | (a << 24);
				
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
			static bool IsValidArea(float area)
			{
				return area > 0;
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
		zBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));
		frameBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y));

		Vertex a,b,c,d;

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
			//TODO vertex gestion
			{
				// Rouge : loin, en haut à gauche
				a.pos.x =0;
				a.pos.y = 0;
				a.pos.w = 2.0f;
				a.pos.z = 1.f;
				a.color = { 1, 0, 0, 1 };

				// Vert : devant, en bas à gauche
				b.pos.x = 0.0f;
				b.pos.y = static_cast<float>(size.y - 1);
				b.pos.w = 1.0f;
				b.pos.z =0.f;
				b.color = { 0, 1, 0, 1 };

				// Bleu : devant, en bas à droite
				c.pos.x = static_cast<float>(size.x - 1);
				c.pos.y = static_cast<float>(size.y - 1);
				c.pos.w = 1.0f;
				c.pos.z = 0.f;
				c.color = { 0, 0, 1, 1 };

				// Blanc : loin, en haut à droite
				d.pos.x = static_cast<float>(size.x - 1);
				d.pos.y = 0.0f;
				d.pos.w =  2.0f;
				d.pos.z = 1.0f;
				d.color = { 1, 1, 1, 1 };
			}


			FDB::Rect<std::int32_t> port;

			renderer.Clear(FDB::Color::black8);
			port.x = 0;
			port.y =0;
			port.width = size.x;
			port.height = size.y ;

			FDB::Pipeline::Image<std::uint32_t> image;
			image.buffer = frameBuffer.data();
			image.bufferHeight = size.y;
			image.bufferWidth = size.x;

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
			using rasterizer_t = FDB::Pipeline::Rasterizer::TriangleRasterizer<Vertex>;
			using fragmentState_t = FDB::Pipeline::Rasterizer::FragmentState;
			using fragmentOutput = FDB::Pipeline::Rasterizer::fragment_output_t<fragmentState_t::Keep>;
			auto fragmentShader = [&](const Vertex& vertex, const glm::vec<2, std::int32_t>& pixel, float) { return  fragmentOutput::Color(vertex.color);};
			rasterizer_t::Compute<fragmentState_t::Keep>({ a,b,c }, port, image, z_image, fragmentShader);
			rasterizer_t::Compute<fragmentState_t::Keep>({ c,d,a }, port, image, z_image, fragmentShader);
			texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
			renderer.RenderTexture(texture);
			renderer.Present();
		}
		FDB::Sdl::Application::Destroy();
	}




