#ifndef KTR_FNV_1A_ENUM_F
#define KTR_FNV_1A_ENUM_F

#include "KTR_FNV_1ADef.h"

namespace KTR
{
	namespace hash
	{
		/**
		 * @brief FNV-1A hash specialization for enumeration types.
		 *
		 * This specialization provides hashing support for enum and enum class
		 * types by converting the enumeration value to its underlying integral
		 * representation before applying the FNV-1A algorithm.
		 *
		 * The enum name itself is not used during hashing. Only the numerical
		 * value associated with the enumeration entry is processed.
		 *
		 * @tparam T Enumeration type to hash.
		 * @tparam HashBits Hash size type used by the algorithm
		 *                  (std::uint32_t or std::uint64_t).
		 */
		template<typename T, KTR_ValidHashBits HashBits> requires(std::is_enum_v<T>)
			struct FNV_1A<T, HashBits>
		{

			using hash_type = T;
			using FNV_1A_INFO_type = FNV_1A_INFO<HashBits>;
			using result_type = typename FNV_1A_INFO_type::result_type;
			using underlying_type = std::underlying_type_t<T>;
			/**
			 * @brief Computes the FNV-1A hash of an enumeration value.
			 *
			 * The enumeration value is first converted to its underlying
			 * integral representation. The resulting bytes are then processed
			 * sequentially using the FNV-1A algorithm.
			 *
			 * @param rawVal Enumeration value to hash.
			 * @return Computed FNV-1A hash value.
			 */
			[[nodiscard]] static constexpr result_type Hash(hash_type rawVal)
			{
				underlying_type val = static_cast<underlying_type>(rawVal);
				result_type hash = FNV_1A_INFO_type::Init();
				for (std::size_t i = 0; i < sizeof(underlying_type); ++i)
				{
					FNV_1A_INFO_type::ComputeByte(hash, static_cast<std::uint8_t>(val >> (i * KTR_CHAR_BIT)));
				}
				return hash;
			}

			/**
			 * @brief Functor interface for hashing an enumeration value.
			 *
			 * Provides a callable wrapper around the static Hash() function.
			 *
			 * @param val Enumeration value to hash.
			 * @return Computed FNV-1A hash value.
			 */
			[[nodiscard]] constexpr result_type operator()(hash_type val) const
			{
				return FNV_1A::Hash(val);
			}

		};
	}
}

#endif
