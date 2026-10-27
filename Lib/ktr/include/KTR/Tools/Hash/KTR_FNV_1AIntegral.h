#ifndef KTR_FNV_1A_INTEGRAL_F
#define KTR_FNV_1A_INTEGRAL_F

#include "KTR_FNV_1ADef.h"
namespace KTR
{
	namespace hash
	{
		/**
	 * @brief FNV-1A hash specialization for integral types.
	 *
	 * This specialization provides hashing support for all integral
	 * types by processing the raw bytes composing the value.
	 *
	 * The value is decomposed byte by byte and each byte is passed
	 * through the common FNV-1A update routine defined by
	 * FNV_1A_INFO.
	 *
	 * @tparam T Integral type to hash.
	 * @tparam HashBits Hash size type used by the algorithm
	 *                  (std::uint32_t or std::uint64_t).
	 */
		template<typename T, KTR_ValidHashBits HashBits> requires(std::is_integral_v<T>)
			struct FNV_1A<T, HashBits>
		{

			using hash_type = T;
			using FNV_1A_INFO_type = FNV_1A_INFO<HashBits>;
			using result_type = typename FNV_1A_INFO_type::result_type;
			/**
			 * @brief Computes the FNV-1A hash of an integral value.
			 *
			 * The value is processed as a sequence of bytes. Each byte is
			 * extracted from the original value and mixed into the hash state
			 * using the FNV-1A algorithm.
			 *
			 * @param val Integral value to hash.
			 * @return Computed FNV-1A hash value.
			 */
			[[nodiscard]] static constexpr result_type Hash(hash_type val)
			{
				result_type hash = FNV_1A_INFO_type::Init();
				for (std::size_t i = 0; i < sizeof(hash_type); ++i)
				{
					FNV_1A_INFO_type::ComputeByte(hash, static_cast<std::uint8_t>(val >> (i * KTR_CHAR_BIT)));
				}
				return hash;
			}

			/**
			 * @brief Functor interface for hashing an integral value.
			 *
			 * Provides a convenient callable interface forwarding the request
			 * to the static Hash() function.
			 *
			 * @param val Integral value to hash.
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
