#ifndef FDB_PIPELINE_ATTRIBUTE_F
#define FDB_PIPELINE_ATTRIBUTE_F
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
#endif
