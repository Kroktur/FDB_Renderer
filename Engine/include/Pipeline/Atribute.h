#ifndef FDB_PIPELINE_ATTRIBUTE_F
#define FDB_PIPELINE_ATTRIBUTE_F
#include <array>
#include <limits>
#include <GLM/glm.hpp>
namespace FDB::Pipeline
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
	struct is_attributes_array<std::array<FDB::Pipeline::Data, N>> : std::true_type {};
	template<typename T>
	static constexpr bool is_attributes_array_v = is_attributes_array<T>::value;

	template<typename T>
	concept ValidVertexT = requires()
	{
		{ T::PosAttribute() } -> std::same_as<FDB::Pipeline::Data>;
	}&& is_attributes_array_v<decltype(T::AllAttributes())> && (T::AllAttributes().size() > 0);


	template<ValidVertexT T>
	struct Interpret
	{
		using value_type = T;
		static constexpr std::size_t structSize = sizeof(value_type);
		static constexpr auto attributes = value_type::AllAttributes();
		static constexpr FDB::Pipeline::Data posAttribute = value_type::PosAttribute();
		static constexpr std::size_t attributesSize = attributes.size();

		template<FDB::Pipeline::Data AttribV>
		struct InfoDataFloat
		{
			static constexpr FDB::Pipeline::Data attrib = AttribV;
			static constexpr std::size_t offset = attrib.offset;
			static constexpr std::int32_t n =
				attrib.format == FDB::Pipeline::format::Float1 ? 1 :
				attrib.format == FDB::Pipeline::format::Float2 ? 2 :
				attrib.format == FDB::Pipeline::format::Float3 ? 3 :
				attrib.format == FDB::Pipeline::format::Float4 ? 4 : 0;
		};

		template<std::size_t i>
		using infoFloat_t = InfoDataFloat<attributes[i]>;

		using posInfo_t = InfoDataFloat<posAttribute>;

		static glm::vec<4,float> GetPosition(const value_type& vertex)
		{
			glm::vec<4,float> result = { 0.f,0.f,0.f,1.f };
			
			if constexpr (posInfo_t::n == 0)
				return result;
			else
			{
					const std::byte* rawData = reinterpret_cast<const std::byte*>(&vertex);
					glm::vec<posInfo_t::n, float> v;
					std::memcpy(&v, rawData + posInfo_t::offset, sizeof(v));
					std::byte* data = reinterpret_cast<std::byte*>(&result);
					std::memcpy(data, &v, sizeof(v));
			}
			return  result;
		}
	
	};

	template<FDB::Pipeline::ValidVertexT T>
	struct AttributeBase
	{
		using value_type = T;
		using interpret_t = FDB::Pipeline::Interpret<value_type>;

		template<typename FN>
		static void ForEach(const FN& fn)
		{
			[&] <std::size_t... i>(std::index_sequence<i...>)
			{
				(fn.template operator() < i > (), ...);
			}(std::make_index_sequence< interpret_t::attributesSize >{});
		}
	};


	template<FDB::Pipeline::ValidVertexT T>
	struct AttributeMix
	{
		using value_type = T;
		using interpret_t = FDB::Pipeline::Interpret<value_type>;
		using attributebase_t = AttributeBase<value_type>;
		static value_type Mix(const value_type& vertexA, const value_type& vertexB, float t)
		{
			const std::byte* rawDataA = reinterpret_cast<const std::byte*>(&vertexA);
			const std::byte* rawDataB = reinterpret_cast<const std::byte*>(&vertexB);
			value_type result;
			std::byte* resultData = reinterpret_cast<std::byte*>(&result);
			attributebase_t::ForEach([&]<std::size_t i>()
			{
				AttributeMix::template PrivMix<i>(resultData, rawDataA, rawDataB, t);
			});
			return result;
		}

	private:

		template<std::size_t i>
		static void PrivMix(std::byte* resultData, const std::byte* rawDataA, const std::byte* rawDataB, float t)
		{
			using info_t = interpret_t::template infoFloat_t<i>;
			if constexpr (info_t::n == 0)
				return;
			else
			{
				glm::vec<info_t::n, float> vA;
				glm::vec<info_t::n, float> vB;
				std::memcpy(&vA, rawDataA + info_t::offset, sizeof(vA));
				std::memcpy(&vB, rawDataB + info_t::offset, sizeof(vB));
				glm::vec<info_t::n, float> result = glm::mix(vA, vB, t);
				std::memcpy(resultData + info_t::offset, &result, sizeof(result));
			}
		}
	};

}
#endif
