#ifndef KTR_HASHER_F
#define KTR_HASHER_F
#include <cstdint>
#include <type_traits>

/**
 * @brief Controls inclusion of the default hash implementation.
 *
 * When KTR_DISABLE_DEFAULT_HASH is defined, the default FNV-1A hash headers
 * are not included. This allows users to avoid including unused hash
 * implementations and reduce unnecessary dependencies.
 */
#ifndef KTR_DISABLE_DEFAULT_HASH
#include "KTR_FNV_1A.h"
#endif

#include "Tools/META/KTR_MetaTools.h"

namespace KTR
{
	namespace hash
	{
#ifndef KTR_DISABLE_DEFAULT_HASH
		/**
		 * @brief Default hash implementation used by KTR.
		 *
		 * This alias defines the default hashing algorithm selected by the
		 * library. By default, KTR uses the FNV-1A hash implementation.
		 *
		 * @tparam T Type to hash.
		 */
		template<typename T>
		using default_hasher_type = FNV_1A_type<T>;
#endif

/**
 * @brief Concept validating a hash implementation interface.
 *
 * A valid hash implementation must provide:
 * - A type representing the input value (`hash_type`).
 * - A type representing the returned hash value (`result_type`).
 * - A static Hash() function accepting the hash input type.
 * - A callable operator() accepting the hash input type.
 *
 * The returned hash type must be compatible with the hash sizes supported
 * by KTR, currently limited to 32-bit and 64-bit unsigned integers.
 *
 * @tparam Hash Hash implementation type to validate.
 */
template< typename Hash>
concept ValidHasher = requires(Hash hash,typename Hash::hash_type&& t)
{
	/**
	 * @brief Required hash input and output type declarations.
	 */
	typename Hash::result_type;
	typename Hash::hash_type;
	/**
	 * @brief Required static hash function interface.
	 */
	{ Hash::Hash(t) } -> std::convertible_to<typename Hash::result_type>;
	/**
	 * @brief Required callable hash interface.
	 */
	{ hash(t) }-> std::convertible_to<typename Hash::result_type>;
}
&& (std::is_same_v<typename Hash::result_type, std::uint32_t> || std::is_same_v<typename Hash::result_type, std::uint64_t>);




	template<typename T, ValidHasher hash>
	struct is_hashable_type : KTR::Meta::false_type {};

	template<typename T, ValidHasher hash> requires(std::is_same_v<T, typename hash::hash_type>)
		struct is_hashable_type<T, hash> : KTR::Meta::true_type {};

	template<typename T, ValidHasher hash>
	static constexpr bool  is_hashable_type_v = is_hashable_type < T, hash >::value;
	}
}

#endif
