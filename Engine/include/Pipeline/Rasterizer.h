#ifndef FDBPIPELINERASTERIZERF
#define FDBPIPELINERASTERIZERF


#include <array>
#include <GLM/common.hpp>
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>
#include <GLM/detail/type_vec1.hpp>

#include "Atribute.h"
#include "EdgeFunction.h"
#include "Image.h"
#include "Options.h"
#include "Core/Box.h"
#include "Core/Rect.h"









namespace FDB::Pipeline
{
	using fixed_value_t = std::int64_t;
	using fixed_edge = FDB::Pipeline::EdgeFunction<fixed_value_t>;
	using fixed_point = glm::vec<2, fixed_value_t>;

	static constexpr fixed_value_t subPixelScale = 256;
	static constexpr fixed_value_t fixedDigits = 8;

	static  fixed_value_t ToFixed(float v)
	{
		return static_cast<fixed_value_t>(v * (1 << fixedDigits));
	}

	static  fixed_point ToFixed(const glm::vec<2, float>& p)
	{
		return fixed_point{
			ToFixed(p.x),
			ToFixed(p.y)
		};
	}



	//struct EdgesData
	//{
	//	glm::vec<3, float> edgesFn;
	//	std::array<glm::vec<2, float>, 3> edgesFnDelta;
	//};
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
	template<FDB::Pipeline::ValidVertexT T>
	struct AttributeData
	{
		using value_type = T;
		using interpret_t = FDB::Pipeline::Interpret<value_type>;
		using edgesData_t = FDB::Pipeline::DeltaData<glm::vec<3,float>>;
		using attributebase_t = AttributeBase<value_type>;
		AttributeData() : m_data() {}
		~AttributeData() = default;
		AttributeData(const AttributeData&) = default;
		AttributeData(AttributeData&&)noexcept = default;
		AttributeData& operator=(const AttributeData&) = default;
		AttributeData& operator=(AttributeData&&) noexcept = default;

		void InitW(const T& dataT, float invW)
		{
			const std::byte* rawData = reinterpret_cast<const std::byte*>(&dataT);
			attributebase_t::ForEach([&]<std::size_t i>()
			{
				this->template PrivateInitW<i>(rawData, invW);
			});
		}
		template<typename TParam>
		void InitOrigin(const std::array<AttributeData, 3>& baseInterpol, const  DeltaData<glm::vec<3, TParam>>& edgeData, float invArea)
		{
			const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
			const std::byte* rawData1 = reinterpret_cast<const std::byte*>(&baseInterpol[1]);
			const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);


			attributebase_t::ForEach([&]<std::size_t i>()
			{
				this->template PrivateInitOrigin < i, TParam > (rawData0, rawData1, rawData2, edgeData, invArea);
			});
		}




		template<typename TParam,bool isX>
		void InitGradiant(const std::array<AttributeData, 3>& baseInterpol, const DeltaData<glm::vec<3, TParam>>& edgeData, float invArea)
		{
			const std::byte* rawData0 = reinterpret_cast<const std::byte*>(&baseInterpol[0]);
			const std::byte* rawData1 = reinterpret_cast<const std::byte*>(&baseInterpol[1]);
			const std::byte* rawData2 = reinterpret_cast<const std::byte*>(&baseInterpol[2]);

			attributebase_t::ForEach([&]<std::size_t i>()
			{
				this->template PrivateInitGradiant<i, TParam, isX>(rawData0, rawData1, rawData2, edgeData, invArea);
			});
		}


		AttributeData& operator+=(const AttributeData& other)
		{
			const std::byte* rawData = reinterpret_cast<const std::byte*>(&other);

			attributebase_t::ForEach([&]<std::size_t i>()
			{
				this->template PrivateAdd<i>(rawData);
			});

			return *this;
		}

		void Apply(value_type& fragment, float w) const
		{
			std::byte* rawData = reinterpret_cast<std::byte*>(&fragment);
			attributebase_t::ForEach([&]<std::size_t i>()
			{
				this->template PrivateApply<i>(rawData, w);
			});
		}



	private:


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
		template<std::size_t i,typename  TParam>
		void PrivateInitOrigin(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const  DeltaData<glm::vec<3, TParam>>& edgeData, float invArea)
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
				glm::vec<info_t::n, float> vFinal = v0 * static_cast<float>(edgeData.data[1]) + v1 * static_cast<float>(edgeData.data[2]) + v2 * static_cast<float>(edgeData.data[0]);
				vFinal *= invArea;
				std::memcpy(m_data + info_t::offset, &vFinal, sizeof(vFinal));
			}
		}
		template<std::size_t i,typename TParam, bool isX>
		void PrivateInitGradiant(const std::byte* rawData0, const std::byte* rawData1, const std::byte* rawData2, const DeltaData<glm::vec<3, TParam>>& edgeData, float invArea)
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
					vFinal = v0 * static_cast<float>(edgeData.delta[0][1]) + v1 * static_cast<float>(edgeData.delta[0][2]) + v2 * static_cast<float>(edgeData.delta[0][0]);
				else
					vFinal = v0 * static_cast<float>(edgeData.delta[1][1]) + v1 * static_cast<float>(edgeData.delta[1][2]) + v2 * static_cast<float>(edgeData.delta[1][0]);

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

	


	template<FDB::Pipeline::ValidVertexT VertexT,std::size_t dim = 8>
	struct TriangleRasterizer
	{
		using fragment = VertexT;
		using triangle_t = std::array<fragment, 3>;
		using viewPort_t = Rect<std::int32_t>;
		using attributeData_t = AttributeData<fragment>;
		using interpret_t = Interpret<fragment>;
		template<typename T>
		using deltaData_t = DeltaData<T>;
		using blockData_t = BlockData;
		static  constexpr  glm::vec<2, int> blockDim = { dim,dim };
		using fragmentState_t = FragmentState;



		struct LoopSetup
		{
			triangle_t triangle;
			float fixedInvArea;
			box2_t<std::int32_t> box;
			deltaData_t<glm::vec<3, fixed_value_t>> fixedBaseEdgeData;
			glm::vec<3, float> fixedBaseInvW;
			glm::vec<3, float> fixedBaseZ;
			std::array<attributeData_t, 3> fixedInter;
			std::array<bool, 3> topLeftRule;
		};


		struct BlockSetup
		{
			blockData_t block;
			deltaData_t<float> fixedWData;
			deltaData_t<float> fixedZData;
			deltaData_t<attributeData_t> fixedInterData;
			std::array <glm::vec<3, fixed_value_t>, 2> fixedEdgeIncrement;
		};
		struct FragmentSetup
		{
			attributeData_t fixedInter;
			glm::vec<2, std::int32_t> pixel;
			float fixedW;
			float fixedZ;
			std::uint32_t* imagePtr;
			float* zImagePtr;
			glm::vec<4, float>* colorPtr;
		};
		
		template< ValidCullData cullT = default_cullData_t, ValidBlendData blendT = default_blendData_t, ValidDepthData depthT = default_depthData_t, ValidWriteMaskData writeT = rgba_writeMaskData_t, typename fragmentShaderT> requires(ValidFragmentFunction<fragment, fragmentShaderT>)
			static void Compute(const triangle_t& triangle,const std::array<glm::vec<4, float>, 3>& position, const viewPort_t& viewPort, const ImagesData& image, const fragmentShaderT& fragmentShader)
		{
			float area = cullT::GetArea(position);
			if (area <= 0)
				return;

			LoopSetup setup;
			setup.triangle = triangle;
			setup.box = TriangleRasterizer::GetBox(position, viewPort);
			std::array<glm::vec<2, fixed_value_t>, 3> fixedPos= {
				ToFixed({position[0].x,position[0].y}),
				ToFixed({position[1].x,position[1].y}),
				ToFixed({position[2].x,position[2].y})};

			float fixedArea = cullT::GetArea(fixedPos);
			setup.fixedInvArea = 1.f / fixedArea;
			setup.fixedBaseEdgeData.data = TriangleRasterizer::InitializeEdgeFunction(fixedPos, setup.box, subPixelScale);
			setup.fixedBaseEdgeData.delta = TriangleRasterizer::InitializeEdgeDelta(fixedPos, subPixelScale);



			//float
			setup.fixedBaseInvW = TriangleRasterizer::GetInvW(position) * 1.f / static_cast<float>(subPixelScale);
			

			setup.fixedBaseZ[0] = position[0].z ;
			setup.fixedBaseZ[1] = position[1].z;
			setup.fixedBaseZ[2] = position[2].z;

		

			setup.fixedInter[0].InitW(triangle[0], setup.fixedBaseInvW[0]);
			setup.fixedInter[1].InitW(triangle[1], setup.fixedBaseInvW[1]);
			setup.fixedInter[2].InitW(triangle[2], setup.fixedBaseInvW[2]);


			setup.topLeftRule[0] = TriangleRasterizer::IsTopLeft( fixedPos[0], fixedPos[1]);
			setup.topLeftRule[1] = TriangleRasterizer::IsTopLeft(fixedPos[1], fixedPos[2]);
			setup.topLeftRule[2] = TriangleRasterizer::IsTopLeft(fixedPos[2], fixedPos[0]);

			TriangleRasterizer::Loop<blendT, depthT, writeT, fragmentShaderT>(setup, image, fragmentShader);
		}
	private:
		template<ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT, typename fragmentShaderT>
			static void Loop(const LoopSetup& setup, const ImagesData& image, const fragmentShaderT& fragmentShader)
		{

			BlockSetup blockSetup;
		
			blockSetup.fixedWData = TriangleRasterizer::SetupLinearData(setup.fixedBaseInvW, setup.fixedBaseEdgeData, setup.fixedInvArea);
			blockSetup.fixedInterData = TriangleRasterizer::SetupInterData(setup.fixedInter, setup.fixedBaseEdgeData, setup.fixedInvArea);
			blockSetup.fixedZData = TriangleRasterizer::SetupLinearData(setup.fixedBaseZ, setup.fixedBaseEdgeData, setup.fixedInvArea);


			blockSetup.fixedEdgeIncrement = {
				glm::vec<3, fixed_value_t>(setup.fixedBaseEdgeData.delta[0][0], setup.fixedBaseEdgeData.delta[0][1], setup.fixedBaseEdgeData.delta[0][2]),
				glm::vec<3, fixed_value_t>(setup.fixedBaseEdgeData.delta[1][0], setup.fixedBaseEdgeData.delta[1][1], setup.fixedBaseEdgeData.delta[1][2]) };


			for (std::int32_t blockY = setup.box.min.y; blockY <= setup.box.max.y; blockY += blockDim.y)
			{
				blockSetup.block.blockSize.y = std::min(blockDim.y, setup.box.max.y - blockY + 1);
				for (std::int32_t blockX = setup.box.min.x; blockX <= setup.box.max.x; blockX += blockDim.x)
				{

					blockSetup.block.blockSize.x = std::min(blockDim.x, setup.box.max.x - blockX + 1);

					blockSetup.block.blockOrigin = { blockX, blockY };
					deltaData_t<glm::vec<3, fixed_value_t>> fixedBlockEdgeData = TriangleRasterizer::InitializeStepEdgeData(blockSetup.block.blockOrigin, setup.box, blockSetup.block.blockSize, setup.fixedBaseEdgeData);

					if (TriangleRasterizer::IsBlock<false>(fixedBlockEdgeData))
						continue;

				
					TriangleRasterizer::UpdateLinearData(setup.fixedBaseInvW, fixedBlockEdgeData, setup.fixedInvArea, blockSetup.fixedWData);
					TriangleRasterizer::UpdateInterData(setup.fixedInter, fixedBlockEdgeData, setup.fixedInvArea, blockSetup.fixedInterData);
					TriangleRasterizer::UpdateLinearData(setup.fixedBaseZ, fixedBlockEdgeData, setup.fixedInvArea, blockSetup.fixedZData);


					if (TriangleRasterizer::IsBlock<true>(fixedBlockEdgeData))
					{
						TriangleRasterizer::PixelLoop < blendT, depthT, writeT, fragmentShaderT >(setup.triangle, blockSetup, image, fragmentShader);
					}
					else
					{
						TriangleRasterizer::PixelLoop< blendT, depthT, writeT, fragmentShaderT>(setup.triangle, setup.topLeftRule, fixedBlockEdgeData, blockSetup, image, fragmentShader);
					}
				}
			}
		}
		template< ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT, typename fragmentShaderT> 
			static void PixelLoop(const triangle_t& triangle, BlockSetup& setup, const ImagesData& image, const fragmentShaderT& fragmentShader)
		{
				FragmentSetup applySetup;
				BlockSetup currentSetup = setup;
				for (std::int32_t y = 0; y < currentSetup.block.blockSize.y; ++y)
				{
				

					applySetup.fixedW = currentSetup.fixedWData.data;
					applySetup.fixedZ = currentSetup.fixedZData.data;
					applySetup.fixedInter = currentSetup.fixedInterData.data;

					applySetup.pixel = { currentSetup.block.blockOrigin.x,currentSetup.block.blockOrigin.y + y };
					applySetup.imagePtr = image.finalImage.buffer + (applySetup.pixel.y * image.finalImage.bufferWidth + applySetup.pixel.x);
					applySetup.zImagePtr = image.zBuffer.buffer + (applySetup.pixel.y * image.zBuffer.bufferWidth + applySetup.pixel.x);
					applySetup.colorPtr = image.colorImage.buffer + (applySetup.pixel.y * image.colorImage.bufferWidth + applySetup.pixel.x);


					for (std::int32_t x = 0; x < currentSetup.block.blockSize.x; ++x)
					{
						TriangleRasterizer::ApplyFragment<blendT, depthT, writeT, fragmentShaderT>(triangle, applySetup, currentSetup, fragmentShader);
						++applySetup.imagePtr;
						++applySetup.zImagePtr;
						++applySetup.colorPtr;
						++applySetup.pixel.x;
						
						applySetup.fixedW += currentSetup.fixedWData.delta[0];
						applySetup.fixedZ += currentSetup.fixedZData.delta[0];
						applySetup.fixedInter += currentSetup.fixedInterData.delta[0];

					}

				

					currentSetup.fixedZData.data += currentSetup.fixedWData.delta[1];
					currentSetup.fixedZData.data += currentSetup.fixedZData.delta[1];
					currentSetup.fixedInterData.data += currentSetup.fixedInterData.delta[1];
				}
		}

		template< ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT, typename fragmentShaderT > 
			static void PixelLoop(const triangle_t& triangle, const std::array<bool, 3>& topLeftRule, const deltaData_t<glm::vec<3, fixed_value_t>>& edgeData, BlockSetup& setup, const ImagesData& image, const fragmentShaderT& fragmentShader)
		{
			FragmentSetup applySetup;
			deltaData_t<glm::vec<3, fixed_value_t>> currentEdgeData = edgeData;
			BlockSetup currentSetup = setup;
			for (std::int32_t y = 0; y < currentSetup.block.blockSize.y; ++y)
			{
				glm::vec<3, fixed_value_t> fixedEdgeFn = currentEdgeData.data;
			

				applySetup.fixedW = currentSetup.fixedWData.data;
				applySetup.fixedZ = currentSetup.fixedZData.data;
				applySetup.fixedInter = currentSetup.fixedInterData.data;

				applySetup.pixel = { currentSetup.block.blockOrigin.x,currentSetup.block.blockOrigin.y + y };
				applySetup.imagePtr = image.finalImage.buffer + (applySetup.pixel.y * image.finalImage.bufferWidth + applySetup.pixel.x);
				applySetup.zImagePtr = image.zBuffer.buffer + (applySetup.pixel.y * image.zBuffer.bufferWidth + applySetup.pixel.x);
				applySetup.colorPtr = image.colorImage.buffer + (applySetup.pixel.y * image.colorImage.bufferWidth + applySetup.pixel.x);


				for (std::int32_t x = 0; x < currentSetup.block.blockSize.x; ++x)
				{

					if (TriangleRasterizer::IsInside(fixedEdgeFn, topLeftRule))
					{
						TriangleRasterizer::ApplyFragment<blendT, depthT, writeT, fragmentShaderT>(triangle, applySetup, currentSetup, fragmentShader);
					}
					++applySetup.imagePtr;
					++applySetup.zImagePtr;
					++applySetup.colorPtr;
					++applySetup.pixel.x;
					fixedEdgeFn += currentSetup.fixedEdgeIncrement[0];
				

					applySetup.fixedW += currentSetup.fixedWData.delta[0];
					applySetup.fixedZ += currentSetup.fixedZData.delta[0];
					applySetup.fixedInter += currentSetup.fixedInterData.delta[0];

				}

				currentEdgeData.data += currentSetup.fixedEdgeIncrement[1];

				currentSetup.fixedZData.data += currentSetup.fixedWData.delta[1];
				currentSetup.fixedZData.data += currentSetup.fixedZData.delta[1];
				currentSetup.fixedInterData.data += currentSetup.fixedInterData.delta[1];
			}
		}
	private:
		template< ValidBlendData blendT, ValidDepthData depthT, ValidWriteMaskData writeT, typename fragmentShaderT>
			static void ApplyFragment(const triangle_t& triangle, const FragmentSetup& applySetup, const BlockSetup& blockSetup, const fragmentShaderT& fragmentShader)
		{
			static constexpr fragmentState_t fragOutPut = fragOutput_v<fragment, fragmentShaderT>;
			if constexpr (fragOutPut != fragmentState_t::DepthChange && depthT::zTestEnabled_v)
			{
				if (!depthT::PassDepthTest(applySetup.fixedZ, *applySetup.zImagePtr))
					return;
			}
			fragment_output_t<fragOutput_v<fragment, fragmentShaderT>> out = TriangleRasterizer::ExecuteFragmentShader <fragmentShaderT >(applySetup, blockSetup, fragmentShader);
			if constexpr (fragOutPut != fragmentState_t::Keep)
			{
				if (out.discard)
					return;
			}
			if constexpr (fragOutPut == fragmentState_t::DepthChange && depthT::zTestEnabled_v)
			{
				if (!depthT::PassDepthTest(out.z, *applySetup.zImagePtr))
					return;
			}
			TriangleRasterizer::ApplyColor<blendT, writeT>(out.color, applySetup.imagePtr, applySetup.colorPtr);
			if constexpr (fragOutPut != fragmentState_t::DepthChange && depthT::zWriteEnable_v)
			{
				*applySetup.zImagePtr = applySetup.fixedZ;
			}
			if constexpr (fragOutPut == fragmentState_t::DepthChange && depthT::zWriteEnable_v)
			{
				*applySetup.zImagePtr = out.z;
			}

		}
		template<ValidBlendData blendT, ValidWriteMaskData writeT>
		// ColorUnit
		static void ApplyColor(const glm::vec<4, float>& srcColor, std::uint32_t* imagePtr, glm::vec<4, float>* colorPtr)
		{
			glm::vec<4, float> dstColor = *colorPtr;
			glm::vec<4, float> realColor = glm::clamp(blendT::Blend(srcColor, dstColor),0.f,1.f);
			dstColor = writeT::Apply(realColor, dstColor);
			*colorPtr = dstColor;
			dstColor *= 255.f;
			const std::uint32_t r = static_cast<std::uint32_t>(dstColor.r);
			const std::uint32_t g = static_cast<std::uint32_t>(dstColor.g);
			const std::uint32_t b = static_cast<std::uint32_t>(dstColor.b);
			const std::uint32_t a = static_cast<std::uint32_t>(dstColor.a);
			*imagePtr = r | (g << 8) | (b << 16) | (a << 24);
		}
		template< typename fragmentShaderT>
		static  fragment_output_t<fragOutput_v<fragment, fragmentShaderT>> ExecuteFragmentShader(const FragmentSetup& applySetup, const BlockSetup& blockSetup, const fragmentShaderT& fragmentShader)
		{
			fragment_input_t<fragment> fixedFragInput;
			float fixedW = 1.f / applySetup.fixedW;

			applySetup.fixedInter.Apply(fixedFragInput.vertex, fixedW);
			blockSetup.fixedInterData.delta[0].Apply(fixedFragInput.vertexDx, fixedW);
			blockSetup.fixedInterData.delta[1].Apply(fixedFragInput.vertexDy, fixedW);

			fixedFragInput.pixel = applySetup.pixel;

			fixedFragInput.invW = applySetup.fixedW;
			fixedFragInput.invWDx = blockSetup.fixedWData.delta[0];
			fixedFragInput.invWDy = blockSetup.fixedWData.delta[1];

			fixedFragInput.z = applySetup.fixedZ;
			fixedFragInput.zDx = blockSetup.fixedZData.delta[0];
			fixedFragInput.zDy = blockSetup.fixedZData.delta[1];

			return fragmentShader(fixedFragInput);
		}
		//

		// struct Rasterisation Rule
		template<typename T>
		static bool IsTopLeft(const glm::vec<2, T>& A, const glm::vec<2, T>& B)
		{
			return ((B.y - A.y) > 0.0f || ((B.y - A.y) == 0.f) && ((B.x - A.x) < 0.f));
		}
		template<typename T>
		static bool IsInside(const glm::vec<3, T>& edgesFn, const std::array<bool, 3>& topLeftRule)
		{
			return ((topLeftRule[0] == true ? edgesFn[0] >= 0 : edgesFn[0] > 0) && (topLeftRule[1] == true ? edgesFn[1] >= 0 : edgesFn[1] > 0) && (topLeftRule[2] == true ? edgesFn[2] >= 0 : edgesFn[2] > 0));
		}
		// struct Rasterisation Rule


		static box2_t<int> GetBox(const std::array<glm::vec<4, float>, 3>& pos, const FDB::Rect<std::int32_t>& viewPort)
		{
			box2_t<int> box;
			box.min.x = std::max(viewPort.x, static_cast<int>(std::min({ pos[0].x, pos[1].x, pos[2].x })));
			box.max.x = std::min(viewPort.x + viewPort.width - 1, static_cast<int>(std::max({ pos[0].x, pos[1].x, pos[2].x })));
			box.min.y = std::max(viewPort.y, static_cast<int>(std::min({ pos[0].y, pos[1].y, pos[2].y })));
			box.max.y = std::min(viewPort.y + viewPort.height - 1, static_cast<int>(std::max({ pos[0].y, pos[1].y, pos[2].y })));
			return box;
		}


		template<typename T,std::size_t N>requires(N > 1 && N <= 4)
		static glm::vec<3, T> InitializeEdgeFunction(const std::array<glm::vec<N, T>, 3>& pos, const box2_t<std::int32_t>& box,T scale )
		{
			using edge_fn_t = EdgeFunction<T>;

			glm::vec<2, T> pixelCenter = { static_cast<T>(box.min.x) * scale + scale / static_cast<T>(2), static_cast<T>(box.min.y) * scale + scale / static_cast<T>(2) };
			return {
				edge_fn_t::Compute({pos[0].x, pos[0].y},{ pos[1].x,pos[1].y},pixelCenter)
				,edge_fn_t::Compute({pos[1].x, pos[1].y},{ pos[2].x, pos[2].y},pixelCenter)
				,edge_fn_t::Compute({pos[2].x, pos[2].y},{ pos[0].x,pos[0].y}, pixelCenter) };
		}
		template<typename T,std::size_t N>  requires(N > 1 && N <= 4)
		static std::array < glm::vec<3, T>, 2> InitializeEdgeDelta(const std::array<glm::vec<N, T>, 3>& pos,T scale)
		{
			return { glm::vec<3, T>{(pos[1].y - pos[0].y) * scale,(pos[2].y - pos[1].y) * scale,(pos[0].y - pos[2].y) * scale},

				glm::vec<3, T>{-(pos[1].x - pos[0].x) * scale,-(pos[2].x - pos[1].x) * scale,-(pos[0].x - pos[2].x) * scale}

			};
		}
		template<typename T>
		static deltaData_t<glm::vec<3, T>> InitializeStepEdgeData(const glm::vec<2, std::int32_t>& block, const box2_t<std::int32_t>& box, const glm::vec<2, std::int32_t>& blockSize, const deltaData_t<glm::vec<3, T>>& baseEdge)
		{
			glm::vec<2, T> step = { static_cast<T>(block.x - box.min.x),static_cast<T>(block.y - box.min.y)};
			glm::vec<2, T> blockStep = { static_cast<T>(blockSize.x - 1) , static_cast<T>(blockSize.y - 1 )};

			deltaData_t<glm::vec<3, T>> result;
			result.data[0] = baseEdge.data.x + baseEdge.delta[1][0] * step.y + baseEdge.delta[0][0] * step.x;
			result.data[1] = baseEdge.data.y + baseEdge.delta[1][1] * step.y + baseEdge.delta[0][1] * step.x;
			result.data[2] = baseEdge.data.z + baseEdge.delta[1][2] * step.y + baseEdge.delta[0][2] * step.x;



			result.delta[0] = { baseEdge.delta[0] * blockStep.x };
			result.delta[1] = { baseEdge.delta[1] * blockStep.y };

			return result;
		}
		template<bool In, typename T>
		static bool IsBlock(const deltaData_t<glm::vec<3, T>>& blockEdgeData)
		{
			std::array<float, 3> blockEdge;
			if constexpr (In)
			{
				blockEdge[0] = blockEdgeData.data[0] + std::min(blockEdgeData.delta[0][0], static_cast<T>(0)) + std::min(blockEdgeData.delta[1][0], static_cast<T>(0));
				blockEdge[1] = blockEdgeData.data[1] + std::min(blockEdgeData.delta[0][1], static_cast<T>(0)) + std::min(blockEdgeData.delta[1][1], static_cast<T>(0));
				blockEdge[2] = blockEdgeData.data[2] + std::min(blockEdgeData.delta[0][2], static_cast<T>(0)) + std::min(blockEdgeData.delta[1][2], static_cast<T>(0));
				if (blockEdge[0] >= static_cast<T>(0) && blockEdge[1] >= static_cast<T>(0) && blockEdge[2] >= static_cast<T>(0))
					return true;
				return false;
			}
			else
			{
				blockEdge[0] = blockEdgeData.data[0] + std::max(blockEdgeData.delta[0][0], static_cast<T>(0)) + std::max(blockEdgeData.delta[1][0], static_cast<T>(0));
				blockEdge[1] = blockEdgeData.data[1] + std::max(blockEdgeData.delta[0][1], static_cast<T>(0)) + std::max(blockEdgeData.delta[1][1], static_cast<T>(0));
				blockEdge[2] = blockEdgeData.data[2] + std::max(blockEdgeData.delta[0][2], static_cast<T>(0)) + std::max(blockEdgeData.delta[1][2], static_cast<T>(0));
				if (blockEdge[0] < static_cast<T>(0) || blockEdge[1] < static_cast<T>(0) || blockEdge[2] < static_cast<T>(0))
					return true;
				return false;
			}
		}

		// interpolations
		static glm::vec<3, float> GetInvW(const std::array<glm::vec<4, float>, 3>& position)
		{
			return { 1.f / position[0].w ,1.f / position[1].w, 1.f / position[2].w };
		}
		template<typename T>
		static deltaData_t<float> SetupLinearData(glm::vec<3, float> base, const deltaData_t<glm::vec<3, T>>& baseEdgeData, float invArea)
		{
			deltaData_t<float> result;
			result.data = {};
			result.delta[0] = (static_cast<float>(baseEdgeData.delta[0][0]) * base[2] + static_cast<float>(baseEdgeData.delta[0][1]) * base[0] + static_cast<float>(baseEdgeData.delta[0][2]* base[1])) * invArea;
			result.delta[1] = (static_cast<float>(baseEdgeData.delta[1][0]) * base[2] + static_cast<float>(baseEdgeData.delta[1][1]) * base[0] + static_cast<float>(baseEdgeData.delta[1][2] * base[1])) * invArea;
			return result;
		}
		template<typename T>
		static void UpdateLinearData(glm::vec<3, float> base, const deltaData_t<glm::vec<3, T>>& blockEdgeData, float invArea, deltaData_t<float>& data)
		{
			data.data = (static_cast<float>(blockEdgeData.data[0]) * base[2] + static_cast<float>(blockEdgeData.data[1]) * base[0] + static_cast<float>(blockEdgeData.data[2]) * base[1]) * invArea;
		}
		template<typename T>
		static deltaData_t<attributeData_t> SetupInterData(const  std::array<attributeData_t, 3>& startInter, const deltaData_t<glm::vec<3, T>>& baseEdgeData, float invArea)
		{
			deltaData_t<attributeData_t> result;
			result.data = {};
			result.delta[0].template InitGradiant <T,true > (startInter, baseEdgeData, invArea);
			result.delta[1].template InitGradiant<T,false>(startInter, baseEdgeData, invArea);
			return result;
		}
		template<typename T>
		static void UpdateInterData(const  std::array<attributeData_t, 3>& startInter, const deltaData_t<glm::vec<3, T>>& blockEdgeData, float invArea, deltaData_t<attributeData_t>& interData)
		{
			interData.data.InitOrigin(startInter, blockEdgeData, invArea);
		}
		// interpolations

	};
}

#endif
