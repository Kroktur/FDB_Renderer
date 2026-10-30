#ifndef FDBPIPELINERENDERPATHF
#define FDBPIPELINERENDERPATHF
#include "FrameBuffer.h"
#include "Options.h"
#include "Rasterizer.h"
#include "Core/Rect.h"

namespace FDB::Pipeline
{
	template<ValidCullData cullT, ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT >
	struct RasterizerOpt
	{
		using cull_t = cullT;
		using blend_t = blendT;
		using depth_t = depthT;
		using write_t = writeT;

	};
	template<typename T>
	struct is_rasterizerOpt : std::false_type {};
	template<ValidCullData cullT, ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT >
	struct is_rasterizerOpt<RasterizerOpt<cullT, blendT, depthT, writeT>> : std::true_type {};

	template<ValidCullData cullT, ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT >
	using rasterizerOpt_t = RasterizerOpt<cullT, blendT, depthT, writeT>;
	using default_rasterizerOpt_t = rasterizerOpt_t<default_cullData_t, default_blendData_t, default_depthData_t, rgba_writeMaskData_t>;

	template<typename T>
	static constexpr bool is_rasterizerOpt_v = is_rasterizerOpt<T>::value;


	template<FDB::Pipeline::ValidVertexT T>
	struct AttributePositionPipeline
	{
		using value_type = T;
		using interpret_t = FDB::Pipeline::Interpret<value_type>;
		using viewPort_t = Rect<std::int32_t>;

		static void Perspective(T& vertexOut,float w)
		{
			using info_t = interpret_t::posInfo_t;
			if constexpr (info_t::n == 0)
				return;
			else
			{
				std::byte* rawData = reinterpret_cast<std::byte*>(&vertexOut);
				glm::vec<info_t::n, float> wV(w);
				glm::vec<info_t::n, float> v;
				std::memcpy(&v, rawData + info_t::offset, sizeof(v));
				v /= wV;
				std::memcpy(rawData + info_t::offset, &v, sizeof(v));
			}
		}
		static void Screen(T& vertexOut,float x, float y,const viewPort_t& viewPort)
		{
			using info_t = interpret_t::posInfo_t;
			if constexpr (info_t::n == 0)
				return;
			else
			{
				std::byte* rawData = reinterpret_cast<std::byte*>(&vertexOut);
				glm::vec<info_t::n, float> v;
				std::memcpy(&v, rawData + info_t::offset, sizeof(v));
				v.x = static_cast<float>(viewPort.x) + (v.x + 1.0f) * 0.5f * static_cast<float>(viewPort.width);
				v.y = static_cast<float>(viewPort.y) + (1.0f - v.y) * 0.5f * static_cast<float>(viewPort.height);
				std::memcpy(rawData + info_t::offset, &v, sizeof(v));
			}
		}

	private:

		
	};






	template<typename T>
	concept ValidRasterizerOpt = is_rasterizerOpt_v<T>;
	template<ValidVertexT VertIn, ValidVertexT VertOut, typename VertexData, typename VInData, typename VOutData, VertOut(*vertexShader)(const VertIn&, const VertexData&, const VInData&), auto(*fragmentShader)(const fragment_input_t<VertOut>&, const VOutData&), ValidRasterizerOpt RasterizerOpt = default_rasterizerOpt_t>
		requires(ValidFragmentFunction<VertOut, decltype(fragmentShader), const VOutData&>)
	struct RenderPath
	{


		using rasterizerOpt = RasterizerOpt;
		using viewPort_t = Rect<std::int32_t>;
		using vertex = VertIn;
		using vertexOut = VertOut;
		using vertex_buffer_t = vertex*;
		using index_buffer_t = std::uint32_t*;
		using vertex_data_t = VertexData;
		using vIn_data_t = VInData;
		using vOut_data_t = VOutData;
		using outTriangle_t = std::array<vertexOut, 3>;
		using pos_array_t = std::array<glm::vec<4, float>, 3>;
		using interpret_t = Interpret<vertexOut>;



		using rasterizer_t = TriangleRasterizer<vertexOut>;
		using blendT = RasterizerOpt::blend_t;
		using DepthT = RasterizerOpt::depth_t;
		using cullType = RasterizerOpt::cull_t;
		using writeT = RasterizerOpt::write_t;


		struct VertexOutData
		{
			glm::vec<4, float> position;
			vertexOut vertex;
		};
		struct TriangleOutData
		{
			pos_array_t positions;
			outTriangle_t triangles;
			float area;
		};


		void BindVInData(vIn_data_t _buffer)
		{
			m_vInBuffer = _buffer;
		}
		void BindVOutData(vOut_data_t _buffer)
		{
			m_vOutBuffer = _buffer;
		}
		void BindVertexData(vertex_data_t _data)
		{
			m_vertexData = _data;
		}

		void Draw(vertex_buffer_t vBuffer, index_buffer_t iBuffer, std::size_t indexCount,const viewPort_t& viewPort, const FrameBuffer& imageData)
		{
			using fragmentInput_t = FDB::Pipeline::FragmentInput<vertexOut>;

			auto fragmentShader2 = [&](const fragmentInput_t& input)
				{
					return  fragmentShader(input, m_vOutBuffer);
				};
			m_memory.clear();
			// vertex shader 

			for (std::size_t i = 0; i + 2 < indexCount; i += 3)
			{
				VertexOutData vertexOut[3];
				vertexOut[0].vertex = vertexShader(vBuffer[iBuffer[i]], m_vertexData, m_vInBuffer);
				vertexOut[0].position = interpret_t::GetPosition(vertexOut[0].vertex);
				vertexOut[1].vertex = vertexShader(vBuffer[iBuffer[i + 1]],m_vertexData, m_vInBuffer);
				vertexOut[1].position = interpret_t::GetPosition(vertexOut[1].vertex);
				vertexOut[2].vertex = vertexShader(vBuffer[iBuffer[i + 2]], m_vertexData, m_vInBuffer);
				vertexOut[2].position = interpret_t::GetPosition(vertexOut[2].vertex);


				std::vector<VertexOutData> poly = RenderPath::Clip(vertexOut[0], vertexOut[1], vertexOut[2]);
				for (std::size_t i = 1; i + 1 < poly.size(); ++i) {
					// swap because y inverse
					VertexOutData a = RenderPath::Screen(RenderPath::Perspective(poly[0]), viewPort);
					VertexOutData b = RenderPath::Screen(RenderPath::Perspective(poly[i]), viewPort);
					VertexOutData c = RenderPath::Screen(RenderPath::Perspective(poly[i + 1]), viewPort);

					auto result = cullType::ValidArea(std::array<glm::vec<4, float>, 3>{ a.position, b.position, c.position });
					if (!result.valid)
						continue;
					if (result.needSwap)
					{
						std::swap(b.position, c.position);
					}
					rasterizer_t::template Compute< cullType, blendT, DepthT, writeT>({ a.vertex,b.vertex,c.vertex }, { a.position,b.position,c.position }, viewPort, imageData, fragmentShader2);
				}
			}
		}

	private:

		[[nodiscard]] static bool CullPass(float area, pos_array_t& position)
		{
			area = rasterizerOpt::cull_t::GetArea(position);
			return rasterizerOpt::cull_t::ValidArea(area, position);
		}
		static constexpr std::size_t planeCount = 6;
		enum class Plane : std::uint8_t
		{
			Far = 0,
			Near = 1,
			Left = 2,
			Right = 3,
			Top = 4,
			Bottom = 5

		};
		template<std::uint8_t N>
		[[nodiscard]] static bool VertexInPlane(const glm::vec<4, float>& pos)
		{
			constexpr Plane plane = static_cast<Plane>(N);

			if constexpr (plane == Plane::Far)
			{
				return  pos.w - pos.z >= 0;
			}
			else if constexpr (plane == Plane::Near)
			{
				return  pos.z >= 0;
			}
			else if constexpr (plane == Plane::Left)
			{
				return  pos.w + pos.x >= 0;
			}
			else if constexpr (plane == Plane::Right)
			{
				return  pos.w - pos.x >= 0;
			}
			else if constexpr (plane == Plane::Top)
			{
				return  pos.w - pos.y >= 0;
			}
			else
			{
				return  pos.w + pos.y >= 0;
			}
		}
		template<std::uint8_t N>
		[[nodiscard]] static float  VertexDistPlane(const glm::vec<4, float>& pos)
		{
			constexpr Plane plane = static_cast<Plane>(N);

			if constexpr (plane == Plane::Far)
			{
				return   pos.w - pos.z;
			}
			else if constexpr (plane == Plane::Near)
			{
				return  pos.z;
			}
			else if constexpr (plane == Plane::Left)
			{
				return  pos.w + pos.x;
			}
			else if constexpr (plane == Plane::Right)
			{
				return  pos.w - pos.x;
			}
			else if constexpr (plane == Plane::Top)
			{
				return  pos.w - pos.y;
			}
			else
			{
				return  pos.w + pos.y;
			}
		}

		template<std::uint8_t I>
		static void ClipAttribute(std::vector<VertexOutData>& poly)
		{
			const std::vector<VertexOutData> copy = std::move(poly);
			poly.clear();
			for (std::size_t i = 0; i < copy.size(); ++i)
			{
				VertexOutData P = copy[i];
				VertexOutData Q = copy[(i + 1) % copy.size()];
				glm::vec<4, float> p = P.position, q = Q.position;

				bool pIn = RenderPath::VertexInPlane<I>(p), qIn = RenderPath::VertexInPlane<I>(q);
				if (pIn)
				{
					poly.push_back(P);
				}

				if (pIn != qIn)
				{
					float dp = RenderPath::VertexDistPlane<I>(p), dq = RenderPath::VertexDistPlane<I>(q);
					VertexOutData result;
					result.vertex = AttributeMix<vertexOut>::Mix(P.vertex, Q.vertex, dp / (dp - dq));
					result.position = interpret_t::GetPosition(result.vertex);
					poly.push_back(result);
				}

			}
		}

		[[nodiscard]] static std::vector<VertexOutData> Clip(const VertexOutData& A, const VertexOutData& B, const VertexOutData& C)
		{
			std::vector<VertexOutData> poly = { A,B,C };
			[&] <std::uint8_t...N>(std::integer_sequence<std::uint8_t, N...>)
			{
				(RenderPath::ClipAttribute<N>(poly), ...);

			}(std::make_integer_sequence<std::uint8_t, planeCount>());

			return poly;
		}

		[[nodiscard]] static VertexOutData Perspective(const  VertexOutData& v)
		{
			VertexOutData copy = v;
			float w = v.position.w;
			AttributePositionPipeline<vertexOut>::Perspective(copy.vertex,w);
			copy.position = v.position / glm::vec<4, float>(w);
			copy.position.w = w;
			return copy;
		}
		[[nodiscard]] static VertexOutData Screen(const VertexOutData& v, const viewPort_t& viewPort)
		{
			VertexOutData copy = v;
			AttributePositionPipeline<vertexOut>::Screen(copy.vertex,copy.position.x,copy.position.y,viewPort);
			copy.position.x = static_cast<float>(viewPort.x) + (v.position.x + 1.0f) * 0.5f * static_cast<float>(viewPort.width);
			copy.position.y = static_cast<float>(viewPort.y) + (1.0f - v.position.y) * 0.5f * static_cast<float>(viewPort.height);
			return copy;
		}

		vIn_data_t m_vInBuffer = {};
		vOut_data_t m_vOutBuffer = {};
		vertex_data_t m_vertexData = {};
		std::vector<TriangleOutData> m_memory = {};

	};



}

#endif
