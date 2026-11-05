#ifndef FDBPIPELINERENDERPATHF
#define FDBPIPELINERENDERPATHF
#include "FrameBuffer.h"
#include "Options.h"
#include "Rasterizer.h"
#include "Core/Rect.h"
#include "Core/ThreadPool.h"

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
	template<ValidVertexT VertIn, ValidVertexT VertOut, typename VertexData, typename VInData, typename VOutData, VertOut(*vertexShader)(const VertIn&, const VertexData&, const VInData&), auto(*fragmentShader)(const fragment_input_t<VertOut>&, const VertexData&, const VOutData&), ValidRasterizerOpt RasterizerOpt = default_rasterizerOpt_t>
		requires(ValidFragmentFunction<VertOut, decltype(fragmentShader), const VertexData&,const VOutData&>)
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



		using blendT = RasterizerOpt::blend_t;
		using DepthT = RasterizerOpt::depth_t;
		using cullType = RasterizerOpt::cull_t;
		using writeT = RasterizerOpt::write_t;
		using rasterizer_t = TriangleRasterizer < vertexOut, vertex_data_t, vOut_data_t, fragmentShader, 128, cullType, blendT, DepthT, writeT > ;



		struct VertexOutData
		{
			vertexOut vertex;
			glm::vec<4, float> position;
		};
		struct TriangleOutData
		{
			std::array<VertexOutData, 3> triangle;
		};

		struct Tile
		{
			FrameBuffer* buffer;
			viewPort_t viewPort = {};
			std::vector<TriangleOutData> triangles = {};
			std::vector<vertex_data_t> pushConstant = {};
		};

		template<std::size_t tileSizeT = 32>
		struct TileGrid
		{
			TileGrid() = default;
			static constexpr int tileSizeV = tileSizeT;

			glm::ivec2 size;
			glm::ivec2 tileCount;
			std::vector<Tile> tiles;

			TileGrid(viewPort_t viewPort,  FrameBuffer* buffer)
				: size(viewPort.width,viewPort.height)
			
			{
				tileCount.x = (viewPort.width + tileSizeV - 1) / tileSizeV;
				tileCount.y = (viewPort.height + tileSizeV - 1) / tileSizeV;
				tiles.clear();
				tiles.resize(tileCount.x * tileCount.y);

				for (int y = 0; y < tileCount.y; ++y)
				{
					for (int x = 0; x < tileCount.x; ++x)
					{
						Tile& tile = tiles[y * tileCount.x + x];
						tile.viewPort.x = viewPort.x + x * tileSizeV;
						tile.viewPort.y = viewPort.y + y * tileSizeV;
						tile.viewPort.width = std::min(tileSizeV, size.x - x * tileSizeV);
						tile.viewPort.height = std::min(tileSizeV, size.y - y * tileSizeV);
						tile.buffer = buffer;
					}
				}
			}

			Tile& GetTile(int x, int y)
			{
				return tiles[y * tileCount.x + x];
			}
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

	

		void Setup(const viewPort_t& viewPort, FrameBuffer* imageData)
		{
			if (m_viewPort != viewPort)
				GenerateTile(viewPort, imageData);
			ClearTiles();
		}
		void DrawTile(vertex_buffer_t vBuffer, index_buffer_t iBuffer, std::size_t indexCount, const viewPort_t& viewPort,  FrameBuffer* imageData, Core::jobPool_t& pool)
		{
			

			using fragmentInput_t = FDB::Pipeline::FragmentInput<vertexOut>;

			for (std::size_t i = 0; i + 2 < indexCount; i += 3)
			{
				m_poly.clear();
				m_copy.clear();
				m_poly.push_back(VertexOutData{ vertexShader(vBuffer[iBuffer[i]], m_vertexData, m_vInBuffer) ,{} });
				m_poly.push_back(VertexOutData{ vertexShader(vBuffer[iBuffer[i + 1]], m_vertexData, m_vInBuffer) ,{} });
				m_poly.push_back(VertexOutData{ vertexShader(vBuffer[iBuffer[i + 2]], m_vertexData, m_vInBuffer) ,{} });

				m_poly[0].position = interpret_t::GetPosition(m_poly[0].vertex);
				m_poly[1].position = interpret_t::GetPosition(m_poly[1].vertex);
				m_poly[2].position = interpret_t::GetPosition(m_poly[2].vertex);

				auto cA = GetOutCode(m_poly[0]);
				auto cB = GetOutCode(m_poly[1]);
				auto cC = GetOutCode(m_poly[2]);

				if ((cA & cB & cC) != 0)
					continue;
				if ((cA | cB | cC) != 0)
					RenderPath::Clip(m_poly, m_copy);
				

				for (std::size_t j = 1; j + 1 < m_poly.size(); ++j) {
					VertexOutData a = RenderPath::Screen(RenderPath::Perspective(m_poly[0]), viewPort);
					VertexOutData b = RenderPath::Screen(RenderPath::Perspective(m_poly[j]), viewPort);
					VertexOutData c = RenderPath::Screen(RenderPath::Perspective(m_poly[j + 1]), viewPort);

					auto result = cullType::ValidArea(std::array<glm::vec<4, float>, 3>{ a.position, b.position, c.position });
					if (!result.valid)
						continue;
					if (result.needSwap)
					{
						std::swap(b.position, c.position);
					}

					TriangleOutData triangle;
					triangle.triangle = { a,b,c };


					const int viewportMinX = m_viewPort.x;
					const int viewportMinY = m_viewPort.y;

					const int viewportMaxX = m_viewPort.x + m_viewPort.width - 1;
					const int viewportMaxY = m_viewPort.y + m_viewPort.height - 1;

					FDB::Core::box2_t<float> clamp;
					clamp.min = { std::min({ a.position.x, b.position.x, c.position.x }) ,
					std::min({ a.position.y, b.position.y, c.position.y }) };
					
					clamp.max = { std::max({ a.position.x, b.position.x, c.position.x })
						,std::max({ a.position.y, b.position.y, c.position.y }) };


					if (clamp.max.x <= viewportMinX ||
						clamp.max.y <= viewportMinY ||
						clamp.min.x >= m_viewPort.x + m_viewPort.width ||
						clamp.min.y >= m_viewPort.y + m_viewPort.height)
					{
						continue;
					}

					FDB::Core::box2_t<int> clampInt;

					clampInt.min = {
						std::clamp(static_cast<int>(std::floor(clamp.min.x)),
							viewportMinX, viewportMaxX),

						std::clamp(static_cast<int>(std::floor(clamp.min.y)),
							viewportMinY, viewportMaxY)
					};

					clampInt.max = {
						std::clamp(static_cast<int>(std::ceil(clamp.max.x)) - 1,
							viewportMinX, viewportMaxX),

						std::clamp(static_cast<int>(std::ceil(clamp.max.y)) - 1,
							viewportMinY, viewportMaxY)
					};

					

					int minTileX = (clampInt.min.x - m_viewPort.x) / m_grid.tileSizeV;
					int minTileY = (clampInt.min.y - m_viewPort.y) / m_grid.tileSizeV;

					int maxTileX = (clampInt.max.x - m_viewPort.x) / m_grid.tileSizeV;
					int maxTileY = (clampInt.max.y - m_viewPort.y) / m_grid.tileSizeV;
					for (int y = minTileY; y <= maxTileY; ++y)
					{
						for (int x = minTileX; x <= maxTileX; ++x)
						{
							auto& tile = m_grid.GetTile(x, y);
							tile.triangles.push_back(triangle);
							tile.pushConstant.push_back(m_vertexData);
						}
					}
				}
			}
		}
		void Rasterize(Core::jobPool_t& pool)
		{
			
			for (int y = 0; y < m_grid.tileCount.y; ++y) {
				for (int x = 0; x < m_grid.tileCount.x; ++x) {


					pool.PushJob([this, x, y, &pool]()
						{
							auto& tile = m_grid.GetTile(x, y);
							auto* push = tile.pushConstant.data();
							for (auto& t : tile.triangles)
							{
								rasterizer_t::template Compute({ t.triangle[0].vertex,t.triangle[1].vertex,t.triangle[2].vertex }, { t.triangle[0].position,t.triangle[1].position,t.triangle[2].position }, tile.viewPort, tile.buffer, *push, m_vOutBuffer, pool);
								++push;
							}
						});
				}
			}

			pool.WaitIdle();
		}
	private:

		void GenerateTile(const viewPort_t& viewPort, FrameBuffer* frameBuffer)
		{
			m_viewPort = viewPort;
			m_grid = TileGrid<32>(viewPort, frameBuffer);

		}
		void ClearTiles()
		{
			for (Tile& tile : m_grid.tiles)
			{
				tile.triangles.clear();
				tile.pushConstant.clear();
			}
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

		static std::uint8_t GetOutCode(const VertexOutData& vertex)
		{
			return RenderPath::GetOutCodeN<0>(vertex)
				| RenderPath::GetOutCodeN<1>(vertex)
				| RenderPath::GetOutCodeN<2>(vertex)
				| RenderPath::GetOutCodeN<3>(vertex) 
				| RenderPath::GetOutCodeN<4>(vertex) 
				| RenderPath::GetOutCodeN<5>(vertex);
	
		}

		template<std::uint8_t N>
		static std::uint8_t GetOutCodeN(
			const VertexOutData& vertex)
		{
			return (static_cast<std::uint8_t>(!VertexInPlane<N>(vertex.position)) << N);
		}
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
		static void ClipAttribute(std::vector<VertexOutData>& poly,std::vector<VertexOutData>& copy)
		{
			if (poly.empty())
				return;

			copy.swap(poly);
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
		
		static void Clip( std::vector<VertexOutData>& poly, std::vector<VertexOutData>& copy)
		{
			RenderPath::ClipAttribute<0>(poly, copy);
			RenderPath::ClipAttribute<1>(poly, copy);
			RenderPath::ClipAttribute<2>(poly, copy);
			RenderPath::ClipAttribute<3>(poly, copy);
			RenderPath::ClipAttribute<4>(poly, copy);
			RenderPath::ClipAttribute<5>(poly, copy);
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

		std::vector<VertexOutData> m_poly;
		std::vector<VertexOutData> m_copy;



		viewPort_t m_viewPort;
		TileGrid<32> m_grid = {};
	};



}

#endif
