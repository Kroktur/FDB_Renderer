#ifndef KTR_FNV_1A_STRING_F
#define KTR_FNV_1A_STRING_F

#include "KTR_FNV_1ADef.h"
#include <string>
namespace KTR
{
	namespace hash
	{
		/**
		 * @brief FNV-1A hash specialization for std::string.
		 *
		 * This specialization provides hashing support for standard strings
		 * by processing each character as an individual byte of input.
		 *
		 * The string content is hashed directly. The memory address,
		 * capacity, and internal representation of the string are not used.
		 *
		 * @tparam HashBits Hash size type used by the algorithm
		 *                  (std::uint32_t or std::uint64_t).
		 */
		template<KTR_ValidHashBits HashBits>
			struct FNV_1A<std::string, HashBits>
		{
			using hash_type = std::string;
			using FNV_1A_INFO_type = FNV_1A_INFO<HashBits>;
			using result_type = typename FNV_1A_INFO_type::result_type;
			/**
			 * @brief Computes the FNV-1A hash of a string.
			 *
			 * Each character contained in the string is processed
			 * sequentially and mixed into the hash state using the FNV-1A
			 * algorithm.
			 *
			 * @param val String value to hash.
			 * @return Computed FNV-1A hash value.
			 */
			[[nodiscard]] static constexpr result_type Hash(const hash_type& val);
			/**
			 * @brief Functor interface for hashing a string.
			 *
			 * @param val String value to hash.
			 * @return Computed FNV-1A hash value.
			 */
			[[nodiscard]] constexpr result_type operator()(const hash_type& val) const;
		};


		template<KTR_ValidHashBits HashBits>
			constexpr typename FNV_1A<std::string, HashBits>::result_type FNV_1A<std::string, HashBits>::Hash(
				const hash_type& val)
		{
			result_type hash = FNV_1A_INFO_type::Init();
			for (std::size_t i = 0; i < val.size(); ++i)
			{
				FNV_1A_INFO_type::ComputeByte(hash, static_cast<std::uint8_t>(val[i]));
			}
			return hash;
		}

		template<KTR_ValidHashBits HashBits>
			constexpr typename FNV_1A<std::string, HashBits>::result_type FNV_1A<std::string, HashBits>::operator()(
				const hash_type& val) const
		{
			return FNV_1A::Hash(val);
		}

		template<KTR_ValidHashBits HashBits>
			struct FNV_1A<std::string_view, HashBits>
		{
			using hash_type = std::string_view;
			using FNV_1A_INFO_type = FNV_1A_INFO<HashBits>;
			using result_type = typename FNV_1A_INFO_type::result_type;
			[[nodiscard]] static constexpr result_type Hash(const hash_type val);

			[[nodiscard]] constexpr result_type operator()(const hash_type val) const;
		};

		template<KTR_ValidHashBits HashBits>
			constexpr typename FNV_1A<std::string_view, HashBits>::result_type FNV_1A<std::string_view, HashBits>::Hash(
				const hash_type val)
		{
			result_type hash = FNV_1A_INFO_type::Init();
			for (std::size_t i = 0; i < val.size(); ++i)
			{
				FNV_1A_INFO_type::ComputeByte(hash, static_cast<std::uint8_t>(val[i]));
			}
			return hash;
		}

		template<KTR_ValidHashBits HashBits>
			constexpr typename FNV_1A<std::string_view, HashBits>::result_type FNV_1A<std::string_view, HashBits>::operator
			()(const hash_type val) const
		{
			return FNV_1A::Hash(val);
		}
	}
}

#endif
