
	#include <iostream>
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
		enum class format : std::uint32_t
		{
			Float1,
			Float2,
			Float3,
			Float4
		};
		struct Data
		{
			format format;
			std::size_t offset;
		};
	}

	struct Vertex
	{
		glm::vec4 pos;
		glm::vec<4, float> color;
		static  constexpr  Test::Attribute::Data PosAttribute()
		{
			return Test::Attribute::Data{ Test::Attribute::format::Float4,offsetof(Vertex, pos) };
		}
		static constexpr  std::array<Test::Attribute::Data,2> AllAttributes()
		{
			return { Test::Attribute::Data{Test::Attribute::format::Float4,offsetof(Vertex, pos)},Test::Attribute::Data{Test::Attribute::format::Float4,offsetof(Vertex, color)} };
		}
	};


	
	using triangle = Vertex[3];

	struct VertexInterpolation
	{
		 VertexInterpolation(const triangle& tri, const glm::vec<3, float>& invW)
		 {
			glm::vec<4,float> poss[3] = {tri[0].pos,tri[1].pos,tri[2].pos};
			glm::vec<4, float> colors[3] = { tri[0].color,tri[1].color,tri[2].color };

			color.Init(colors, invW);
			position.Init(poss, invW);

		 }
		 void Process(const RasterizerData& data,const RasterizerData& base,float invArea)
		 {
			color.Process(data,base,invArea);
			position.Process(data, base,invArea);
		 }
		 void StartData()
		 {
			 color.StartData();
			 position.StartData();
		 }
		 void IncrementY()
		 {
			 color.IncrementY();
			 position.IncrementY();
		 }
		 void IncrementX()
		 {
			 color.IncrementX();
			 position.IncrementX();
		 }
		 void ApplyFragment(Vertex& fragment) const 
		 {
			 color.ApplyValue(fragment.color);
			 position.ApplyValue(fragment.pos);
		 }
		
		 static void GetPos(const triangle& tri, glm::vec<4, float>(&pos)[3])
		 {
			 pos[0] = tri[0].pos;
			 pos[1] = tri[1].pos;
			 pos[2] = tri[2].pos;
		 }
		 static void GetInvW(const triangle& tri,glm::vec<3,float>& vector)
		 {
			 vector = { 1.f / tri[0].pos.w ,1.f / tri[1].pos.w, 1.f / tri[2].pos.w };
		 }
	private:
		FloatInterpolation<4> color;
		FloatInterpolation<4> position;
	};


	namespace Test
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
			std::array<value_type,2> delta;
		};
	}

	
	template<typename T>
	struct Interpolateur
	{
		using value_type = T;
		static constexpr std::size_t size = sizeof(T) * 8;
		static constexpr auto attributes = T::AllAttributes();
		static constexpr std::size_t attributesSize = attributes.size();
		Interpolateur() : data{}
		{
		}

		void Init(const T& dataT, float invW)
		{
			const std::byte* rawData = reinterpret_cast<const std::byte*>(&dataT);
			[&] <std::size_t... I>(std::index_sequence<I...>)
			{
				(this->template Init<I>(rawData, invW), ...);
			}(std::make_index_sequence<attributesSize>{});
		}

		template<std::size_t i>
		void Init(const std::byte* rawData,float invW)
		{
			static constexpr Test::Attribute::Data attrib = attributes[i];
			static constexpr std::size_t offset = attrib.offset;

			static constexpr std::int32_t n =
				attrib.format == Test::Attribute::format::Float1 ? 1 :
				attrib.format == Test::Attribute::format::Float2 ? 2 :
				attrib.format == Test::Attribute::format::Float3 ? 3 :
				attrib.format == Test::Attribute::format::Float4 ? 4 : 0;

			if constexpr (n == 0)
				return;
			else
			{
				glm::vec<n,float> v;
				std::memcpy(&v, rawData + offset, sizeof(v));
				v *= invW;
				std::memcpy(data + offset, &v, sizeof(v));
			}
		}
		void InitStart(const std::array<Interpolateur,3>& baseInterpol,const Test::EdgesData& edgeData,float invArea)
		{
			const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
			const std::byte* rawData1= reinterpret_cast<const std::byte*>(&baseInterpol[1]);
			const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);

			[&] <std::size_t... I>(std::index_sequence<I...>)
			{
				(this->template InitStart<I>(rawData0, rawData1, rawData2,edgeData,invArea), ...);
			}(std::make_index_sequence<attributesSize>{});
			
		}

		template<std::size_t i>
		void InitStart(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const Test::EdgesData& edgeData, float invArea)
		{
			static constexpr Test::Attribute::Data attrib = attributes[i];
			static constexpr std::size_t offset = attrib.offset;

			static constexpr std::int32_t n =
				attrib.format == Test::Attribute::format::Float1 ? 1 :
				attrib.format == Test::Attribute::format::Float2 ? 2 :
				attrib.format == Test::Attribute::format::Float3 ? 3 :
				attrib.format == Test::Attribute::format::Float4 ? 4 : 0;

			if constexpr (n == 0)
				return;
			else
			{
				glm::vec<n, float> v0;
				glm::vec<n, float> v1;
				glm::vec<n, float> v2;
				std::memcpy(&v0, rawData0 + offset, sizeof(v0));
				std::memcpy(&v1, rawData1 + offset, sizeof(v1));
				std::memcpy(&v2, rawData2 + offset, sizeof(v2));
				glm::vec<n, float> vFinal = v0 * edgeData.edgesFn.y + v1 * edgeData.edgesFn.z + v2 * edgeData.edgesFn.x;
				vFinal *= invArea;
				std::memcpy(data + offset, &vFinal, sizeof(vFinal));
			}
		}


		template<bool isX>
		void InitIncrement(const std::array<Interpolateur, 3>& baseInterpol, const Test::EdgesData& edgeData, float invArea)
		{
			const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
			const std::byte* rawData1 = reinterpret_cast<const std::byte*>(&baseInterpol[1]);
			const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);
			[&] <std::size_t... I>(std::index_sequence<I...>)
			{
				(this->template InitIncrement<I,isX>(rawData0, rawData1, rawData2, edgeData, invArea), ...);
			}(std::make_index_sequence<attributesSize>{});

		}
		
		template<std::size_t i,bool isX>
		void InitIncrement(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const Test::EdgesData& edgeData, float invArea)
		{
			static constexpr Test::Attribute::Data attrib = attributes[i];
			static constexpr std::size_t offset = attrib.offset;

			static constexpr std::int32_t n =
				attrib.format == Test::Attribute::format::Float1 ? 1 :
				attrib.format == Test::Attribute::format::Float2 ? 2 :
				attrib.format == Test::Attribute::format::Float3 ? 3 :
				attrib.format == Test::Attribute::format::Float4 ? 4 : 0;

			if constexpr (n == 0)
				return;
			else
			{
				glm::vec<n, float> v0;
				glm::vec<n, float> v1;
				glm::vec<n, float> v2;
				std::memcpy(&v0, rawData0 + offset, sizeof(v0));
				std::memcpy(&v1, rawData1 + offset, sizeof(v1));
				std::memcpy(&v2, rawData2 + offset, sizeof(v2));
				glm::vec<n, float> vFinal;
				if constexpr (isX)
					 vFinal = v0 * edgeData.edgesFnDelta[1].x + v1 * edgeData.edgesFnDelta[2].x + v2 * edgeData.edgesFnDelta[0].x;
				else
					vFinal = v0 * edgeData.edgesFnDelta[1].y + v1 * edgeData.edgesFnDelta[2].y + v2 * edgeData.edgesFnDelta[0].y;

				vFinal *= invArea;
				std::memcpy(data + offset, &vFinal, sizeof(vFinal));

			}
		}
		Interpolateur& operator+=(const Interpolateur& other)
		{
			const std::byte* rawData = reinterpret_cast<const std::byte*>(&other);
			[&] <std::size_t... I>(std::index_sequence<I...>)
			{
				(this->template Add<I>(rawData), ...);
			}(std::make_index_sequence<attributesSize>{});


			return *this;
		}
		template<std::size_t i>
		void Add(const std::byte* otherData)
		{
			static constexpr Test::Attribute::Data attrib = attributes[i];
			static constexpr std::size_t offset = attrib.offset;

			static constexpr std::int32_t n =
				attrib.format == Test::Attribute::format::Float1 ? 1 :
				attrib.format == Test::Attribute::format::Float2 ? 2 :
				attrib.format == Test::Attribute::format::Float3 ? 3 :
				attrib.format == Test::Attribute::format::Float4 ? 4 : 0;

			if constexpr (n == 0)
				return;
			else
			{
				glm::vec<n, float> v0;
				glm::vec<n, float> v1;
				std::memcpy(&v0, otherData + offset, sizeof(v0));
				std::memcpy(&v1 ,data + offset, sizeof(v1));
				glm::vec<n, float> vFinal = v0 + v1;
				std::memcpy(data + offset, &vFinal, sizeof(vFinal));

			}
		}
		void Apply(value_type& fragment, float w)
		{
			 std::byte* rawData = reinterpret_cast<std::byte*>(&fragment);
			[&] <std::size_t... I>(std::index_sequence<I...>)
			{
				(this->template Apply<I>(rawData,w), ...);
			}(std::make_index_sequence<attributesSize>{});
		}
		template<std::size_t i>
		void Apply(std::byte* rawData,float w)
		{
			static constexpr Test::Attribute::Data attrib = attributes[i];
			static constexpr std::size_t offset = attrib.offset;

			static constexpr std::int32_t n =
				attrib.format == Test::Attribute::format::Float1 ? 1 :
				attrib.format == Test::Attribute::format::Float2 ? 2 :
				attrib.format == Test::Attribute::format::Float3 ? 3 :
				attrib.format == Test::Attribute::format::Float4 ? 4 : 0;

			if constexpr (n == 0)
				return;
			else
			{
				glm::vec<n, float> v;
				std::memcpy(&v, data + offset, sizeof(v));
				v *= w;
				std::memcpy(rawData + offset, &v, sizeof(v));
			}
		}
		std::byte data[size];
	};




	
#include <type_traits>
#include <array>


	namespace Test
	{
		using fragment = Vertex;
		using vertex_t = Vertex;
		using triangle_t = std::array<vertex_t, 3>;
		using viewPort_t = FDB::Rect<std::int32_t>;
		struct Image
		{
			std::uint32_t* buffer;
			std::int32_t bufferWidth;
			std::int32_t bufferHeight;
		};
	
		struct BlockData
		{
			glm::vec<2, std::int32_t> blockSize, blockOrigin;
		};
		struct Rasterizer2
		{
			static  constexpr  glm::vec<2, int> blockDim = { 64,64 };
			static std::array<glm::vec<4, float>, 3> GetPosition(const triangle_t& triangle)
			{
				std::array<glm::vec<4, float>, 3> result;
				result[0] = {0.f,0.f,0.f,1.f};
				result[1] = { 0.f,0.f,0.f,1.f };
				result[2] = { 0.f,0.f,0.f,1.f };


				static constexpr Test::Attribute::Data attrib = vertex_t::PosAttribute();
				static constexpr std::size_t offset = attrib.offset;

				static constexpr std::int32_t n =
					attrib.format == Test::Attribute::format::Float1 ? 1 :
					attrib.format == Test::Attribute::format::Float2 ? 2 :
					attrib.format == Test::Attribute::format::Float3 ? 3 :
					attrib.format == Test::Attribute::format::Float4 ? 4 : 0;


				if constexpr (n == 0)
					return result;
				else
				{
					for (std::int32_t i = 0 ; i < 3 ; ++i)
					{
						const std::byte* rawData = reinterpret_cast<const std::byte*>(&triangle[i]);
						glm::vec<n, float> v;
						std::memcpy(&v, rawData + offset, sizeof(v));
						std::byte* data = reinterpret_cast<std::byte*>(&result[i]);
						std::memcpy(data, &v, sizeof(v));
					}
				}

				return  result;
			
			}
			static float EdgeFunction(const glm::vec<2,float>& A, const glm::vec<2,float> B, const glm::vec<2,float> D)
			{
				return (D.x - A.x) * (B.y - A.y) - (D.y - A.y) * (B.x - A.x);
			}
			static float GetArea(const std::array<glm::vec<4, float>, 3>& pos)
			{
				return Rasterizer2::EdgeFunction(
					{pos[0].x, pos[0].y},
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
			static glm::vec<3, float> InitializeEdgeFunction(const std::array<glm::vec<4, float>, 3>& pos,const box2_t<std::int32_t>& box)
			{
				glm::vec<2, float> minFloat = { static_cast<float>(box.min.x) , static_cast<float>(box.min.y) };
				return { 
					 Rasterizer2::EdgeFunction({pos[0].x, pos[0].y},{ pos[1].x,pos[1].y},{ minFloat.x, minFloat.y})
					,Rasterizer2::EdgeFunction({pos[1].x, pos[1].y},{ pos[2].x, pos[2].y},{minFloat.x, minFloat.y})
					,Rasterizer2::EdgeFunction({pos[2].x, pos[2].y},{ pos[0].x,pos[0].y}, {minFloat.x, minFloat.y}) };
			}
			static std::array < glm::vec<2,float>,3> InitializeEdgeDelta(const std::array<glm::vec<4, float>, 3>& pos)
			{
				return { glm::vec<2, float>{pos[1].y - pos[0].y,-(pos[1].x - pos[0].x) }
					      ,glm::vec<2, float>{ pos[2].y - pos[1].y,-(pos[2].x - pos[1].x) }
					      ,glm::vec<2, float>{ pos[0].y - pos[2].y,-(pos[0].x - pos[2].x) }
				};
			}
			
			static EdgesData InitializeStepEdgeData(glm::vec<2, std::int32_t> block, const box2_t<std::int32_t>& box, const glm::vec<2, std::int32_t>& blockSize, const EdgesData& baseEdge)
			{
				glm::vec<2, float> step = { block.x - box.min.x,block.y - box.min.y };
				glm::vec<2, float> blockStep = { blockSize.x - 1 ,blockSize.y - 1 };

				EdgesData result;
				result.edgesFn[0] = baseEdge.edgesFn.x + baseEdge.edgesFnDelta[0].y * step.y + baseEdge.edgesFnDelta[0].x * step.x;
				result.edgesFn[1] = baseEdge.edgesFn.y + baseEdge.edgesFnDelta[1].y * step.y + baseEdge.edgesFnDelta[1].x * step.x;
				result.edgesFn[2] = baseEdge.edgesFn.z + baseEdge.edgesFnDelta[2].y * step.y + baseEdge.edgesFnDelta[2].x * step.x;

				result.edgesFnDelta[0] = { baseEdge.edgesFnDelta[0] * blockStep };
				result.edgesFnDelta[1] = { baseEdge.edgesFnDelta[1] * blockStep };
				result.edgesFnDelta[2] = { baseEdge.edgesFnDelta[2] * blockStep };

				return result;
			}
			static bool IsBlockOut(const EdgesData& blockEdgeData)
			{

				float abBlockEdgeMax = blockEdgeData.edgesFn[0] + std::max(blockEdgeData.edgesFnDelta[0].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[0].y, 0.f);
				float bcBlockEdgeMax = blockEdgeData.edgesFn[1] + std::max(blockEdgeData.edgesFnDelta[1].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[1].y, 0.f);
				float caBlockEdgeMax = blockEdgeData.edgesFn[2] + std::max(blockEdgeData.edgesFnDelta[2].x, 0.f) + std::max(blockEdgeData.edgesFnDelta[2].y, 0.f);

				if (abBlockEdgeMax < 0.f || bcBlockEdgeMax < 0.f || caBlockEdgeMax < 0.f)
					return true;
				return false;
			}
			static bool IsBlockFullyInside(const EdgesData& blockEdgeData)
			{
				float abBlockEdgeMin = blockEdgeData.edgesFn[0] + std::min(blockEdgeData.edgesFnDelta[0].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[0].y, 0.f);
				float bcBlockEdgeMin = blockEdgeData.edgesFn[1] + std::min(blockEdgeData.edgesFnDelta[1].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[1].y, 0.f);
				float caBlockEdgeMin = blockEdgeData.edgesFn[2] + std::min(blockEdgeData.edgesFnDelta[2].x, 0.f) + std::min(blockEdgeData.edgesFnDelta[2].y, 0.f);

				if (abBlockEdgeMin >= 0.f && bcBlockEdgeMin >= 0.f && caBlockEdgeMin >= 0.f)
					return true;
				return false;
			}
			static glm::vec<3,float> GetInvW(const std::array<glm::vec<4, float>, 3>& position)
			{
				return { 1.f / position[0].w ,1.f / position[1].w, 1.f / position[2].w };
			}
			static DeltaData<float> SetupWData(glm::vec<3, float> baseInvW, const EdgesData& baseEdgeData, float invArea)
			{
				DeltaData<float> result;
				result.data = {};
				result.delta[0] = (baseEdgeData.edgesFnDelta[0].x * baseInvW[2] + baseEdgeData.edgesFnDelta[1].x * baseInvW[0] + baseEdgeData.edgesFnDelta[2].x * baseInvW[1]) * invArea;
				result.delta[1] = (baseEdgeData.edgesFnDelta[0].y * baseInvW[2] + baseEdgeData.edgesFnDelta[1].y * baseInvW[0] + baseEdgeData.edgesFnDelta[2].y * baseInvW[1]) * invArea;
				return result;
			}
			static void UpdateWData(glm::vec<3,float> baseInvW,const EdgesData& blockEdgeData,float invArea, DeltaData<float>& wData)
			{
				wData.data= (blockEdgeData.edgesFn[0] * baseInvW[2] + blockEdgeData.edgesFn[1] * baseInvW[0] + blockEdgeData.edgesFn[2] * baseInvW[1]) * invArea;
			}
			static DeltaData<Interpolateur<vertex_t>> SetupInterData(const  std::array<Interpolateur<vertex_t>, 3>& startInter, const EdgesData& baseEdgeData, float invArea)
			{
				DeltaData<Interpolateur<vertex_t>> result;
				result.data = {};
				result.delta[0].InitIncrement<true>(startInter, baseEdgeData, invArea);
				result.delta[1].InitIncrement<false>(startInter, baseEdgeData, invArea);

				
				return result;
			}
			static void UpdateInterData(const  std::array<Interpolateur<vertex_t>, 3>& startInter, const EdgesData& blockEdgeData, float invArea, DeltaData<Interpolateur<vertex_t>>& interData)
			{
				interData.data.InitStart(startInter, blockEdgeData, invArea);
			}
			static void Compute(const triangle_t& triangle, const viewPort_t& viewPort, Image& image)
			{
				// position
				const std::array<glm::vec<4, float>, 3> position = Rasterizer2::GetPosition(triangle);
				// positon
				// area
				float area = GetArea(position);
				if (!IsValidArea(area))
					return;
				float invArea = 1.f / area;
				// area
				// box
				box2_t<std::int32_t> box = GetBox(position, viewPort);
				// box

				EdgesData data;
				data.edgesFn = InitializeEdgeFunction(position, box);
				data.edgesFnDelta = InitializeEdgeDelta(position);

				glm::vec<3, float> baseInvW = GetInvW(position);

				std::array<Interpolateur<vertex_t>, 3> inter;
				inter[0].Init(triangle[0], baseInvW[0]);
				inter[1].Init(triangle[1], baseInvW[1]);
				inter[2].Init(triangle[2], baseInvW[2]);

 				Loop(triangle,box, data,baseInvW,invArea,inter, image);
			}



			static void Loop(const triangle_t& triangle,const box2_t<std::int32_t>& box,const EdgesData& baseEdgeData,const glm::vec<3,float>& baseInvW,float invArea,const  std::array<Interpolateur<vertex_t>, 3>& startInter,Image& image)
			{

				DeltaData<float> wData = SetupWData(baseInvW, baseEdgeData, invArea);
				DeltaData<Interpolateur<vertex_t>> interData = SetupInterData(startInter, baseEdgeData, invArea);


				std::array <glm::vec<3, float>, 2> edgesFnIncrement = { glm::vec<3, float>(baseEdgeData.edgesFnDelta[0].x, baseEdgeData.edgesFnDelta[1].x, baseEdgeData.edgesFnDelta[2].x),glm::vec<3, float>(baseEdgeData.edgesFnDelta[0].y, baseEdgeData.edgesFnDelta[1].y, baseEdgeData.edgesFnDelta[2].y) };

				glm::vec<2, std::int32_t> blockSize = {};
				for (std::int32_t blockY = box.min.y; blockY <= box.max.y; blockY += blockDim.y)
				{
					blockSize.y = std::min(blockDim.y, box.max.y - blockY + 1);
					for (std::int32_t blockX = box.min.x; blockX <= box.max.x; blockX += blockDim.x)
					{
						blockSize.x = std::min(blockDim.x, box.max.x - blockX + 1);
						glm::vec<2, std::int32_t> blockOrigin = { blockX, blockY };
						EdgesData blockEdgeData = InitializeStepEdgeData(blockOrigin, box, blockSize, baseEdgeData);

						if (IsBlockOut(blockEdgeData))
							continue;

						UpdateWData(baseInvW, blockEdgeData,invArea, wData);
						UpdateInterData(startInter, blockEdgeData, invArea, interData);

						if (IsBlockFullyInside(blockEdgeData))
						{
							PixelLoopInside(triangle, { blockSize, blockOrigin }, wData, interData, image);
						}
						else
						{
							PixelLoopClassic(triangle, { blockSize, blockOrigin }, blockEdgeData, edgesFnIncrement, wData, interData, image);
						}
					}
				}
			}

			static void PixelLoopInside(const triangle_t& triangle, const BlockData& block, DeltaData<float>& wData, DeltaData<Interpolateur<vertex_t>>& interData, Image& image)
			{
				for (std::int32_t y = 0; y < block.blockSize.y; ++y)
				{
					float w = wData.data;
					Interpolateur<vertex_t> inter = interData.data;
					std::uint32_t* pixelPtr = image.buffer + ((block.blockOrigin.y + y) * image.bufferWidth + block.blockOrigin.x);
					for (std::int32_t x = 0; x < block.blockSize.x; ++x)
					{
						{
							float currentW = 1.f / w;
							vertex_t fragment = triangle[0];
							inter.Apply(fragment, currentW);
							glm::vec4 fragColor = fragment.color * 255.f;
							std::uint32_t r = static_cast<std::uint32_t>(fragColor.x);
							std::uint32_t g = static_cast<std::uint32_t>(fragColor.y);
							std::uint32_t b = static_cast<std::uint32_t>(fragColor.z);
							std::uint32_t a = static_cast<std::uint32_t>(fragColor.a);
							*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
						}
						++pixelPtr;
						w += wData.delta[0];
						inter += interData.delta[0];
					}
					wData.data += wData.delta[1];
					interData.data += interData.delta[1];
				}
			}
			static void PixelLoopClassic(const triangle_t& triangle,const BlockData& block,EdgesData& currentEdgeData,const std::array <glm::vec<3, float>, 2>& edgeIncrement, DeltaData<float>& wData, DeltaData<Interpolateur<vertex_t>>& interData, Image& image)
			{
				for (std::int32_t y = 0; y < block.blockSize.y; ++y)
				{
					glm::vec<3, float> edgesFn = currentEdgeData.edgesFn;
					float w = wData.data;
					Interpolateur<vertex_t> inter = interData.data;
					std::uint32_t* pixelPtr = image.buffer + ((block.blockOrigin.y + y) * image.bufferWidth + block.blockOrigin.x);
					for (std::int32_t x = 0; x < block.blockSize.x; ++x)
					{
						if (edgesFn[0] >= 0 && edgesFn[1] >= 0 && edgesFn[2] >= 0)
						{
							float currentW = 1.f / w;
							vertex_t fragment = triangle[0];
							inter.Apply(fragment, currentW);
							glm::vec4 fragColor = fragment.color * 255.f;
							std::uint32_t r = static_cast<std::uint32_t>(fragColor.x);
							std::uint32_t g = static_cast<std::uint32_t>(fragColor.y);
							std::uint32_t b = static_cast<std::uint32_t>(fragColor.z);
							std::uint32_t a = static_cast<std::uint32_t>(fragColor.a);
							*pixelPtr = (r) | (g << 8) | (b << 16) | (a << 24);
						}
						++pixelPtr;
						edgesFn += edgeIncrement[0];
						w += wData.delta[0];
						inter += interData.delta[0];
					}
					currentEdgeData.edgesFn += edgeIncrement[1];
					wData.data += wData.delta[1];
					interData.data += interData.delta[1];
				}
			}
		};
	}
	


	static constexpr std::int32_t windowWidth = 800;
	static constexpr std::int32_t windowHeight = 600;

	static constexpr std::int32_t renderWidth = 320;

	static constexpr std::int32_t renderHeight = 240;

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
			std::memset(zBuffer.data(), 0, zBuffer.size() * sizeof(float));

			//TODO vertex gestion
			{
				a.pos.x = 0;
				a.pos.y = static_cast<float>(size.y - 1) ;
				a.pos.w = 1.0f;
				a.color = { 1,0,0,1 };
				b.pos.x = 0;
				b.pos.y = 0;
				b.pos.w = 1.f;
				b.color = { 0,1,0,1 };
				c.pos.x = static_cast<float>(size.x - 1) ;
				c.pos.y = 0;
				c.pos.w = 1.f;
				c.color = { 0,0,1,1 };
				d.pos.x = static_cast<float>(size.x - 1) ;
				d.pos.y = static_cast<float>(size.y - 1) ;
				d.color = { 1,1,1,1 };
			}


			FDB::Rect<std::int32_t> port;

			renderer.Clear(FDB::Color::black8);
			port.x = 0;
			port.y =0;
			port.width = size.x;
			port.height = size.y ;
			
			Test::Image image;
			image.buffer = frameBuffer.data();
			image.bufferHeight = size.y;
			image.bufferWidth = size.x;

			auto bench = [&](auto fn, const char* name)
				{
					fn(); // échauffement
					Uint64 s = SDL_GetPerformanceCounter();
					for (int i = 0; i < 1000; ++i) fn();
					Uint64 e = SDL_GetPerformanceCounter();
					printf("%s : %.3f ms/appel\n", name,
						double(e - s) * 1000.0 / double(SDL_GetPerformanceFrequency()) / 1000.0);
				};

			bench([&] { Test::Rasterizer2::Compute({ a,c,b }, port, image);}, "Rasterizer V2");
			bench([&] { Rasterizer::Compute<Vertex, VertexInterpolation>({ a, c, b }, port, frameBuffer.data(), zBuffer.data(), size.x); }, "Rasterizer V1");
			//Rasterizer::Compute<Vertex,VertexInterpolation>({ a, c, b }, port, frameBuffer.data(),zBuffer.data(), size.x);
			
			Test::Rasterizer2::Compute({a,c,b}, port, image);
			//Rasterizer::ComputeWithouGLM({ a, b, c }, port, frameBuffer.data(), size.x);


			texture.Update(frameBuffer.data(), size.x * static_cast<int>(sizeof(std::uint32_t)));
			renderer.RenderTexture(texture);
			renderer.Present();
		}
		FDB::Sdl::Application::Destroy();
	}




