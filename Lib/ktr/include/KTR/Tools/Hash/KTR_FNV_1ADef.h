#ifndef KTR_FNV_1ADEF_F
#define KTR_FNV_1ADEF_F

#include <cstdint>
#include <type_traits>
#include "KTR/KTR_congif_compiler.h"
namespace KTR
{
	namespace hash
	{

		#define KTR_CHAR_BIT 8

		/**
		 * @brief Provides the FNV-1A constants associated with a hash size.
		 *
		 * This traits structure maps a hash type to the corresponding
		 * FNV-1A offset basis and prime values defined by the algorithm.
		 */
		template<typename T>
		struct FNV_1A_INFO_TRAITS;


		/**
		 * @brief FNV-1A constants for a 32-bit hash.
		 *
		 * The values defined here are the standard FNV-1A parameters used
		 * to initialize and update a 32-bit hash value.
		 *
		 * @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
		 */
		template<>
		struct FNV_1A_INFO_TRAITS<std::uint32_t>
		{
			using result_type = std::uint32_t;
			static constexpr result_type FNV_basis = 2166136261u;
			static constexpr result_type FNV_prime = 16777619u;
		};

		/**
		 * @brief FNV-1A constants for a 64-bit hash.
		 *
		 * The values defined here are the standard FNV-1A parameters used
		 * to initialize and update a 64-bit hash value.
		 *
		 * @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
		 */
		template<>
		struct FNV_1A_INFO_TRAITS<std::uint64_t>
		{
			using result_type = std::uint64_t;
			static constexpr result_type FNV_basis = 14695981039346656037ull;
			static constexpr result_type FNV_prime = 1099511628211ull;
		};

		/**
		 * @brief Utility class implementing the core FNV-1A operations.
		 *
		 * This class exposes the common initialization value and the byte
		 * processing routine shared by every FNV-1A specialization.
		 *
		 * @tparam T Hash storage type. Must be either std::uint32_t or
		 *           std::uint64_t.
		 */
		template<typename T> requires(std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>)
			struct FNV_1A_INFO
		{
			using result_type = typename FNV_1A_INFO_TRAITS<T>::result_type;
			static constexpr result_type FNV_basis = FNV_1A_INFO_TRAITS<T>::FNV_basis;
			static constexpr result_type FNV_prime = FNV_1A_INFO_TRAITS<T>::FNV_prime;

			/**
			 * @brief Returns the initial FNV-1A offset basis.
			 *
			 * @return The initial hash value.
			 */
			[[nodiscard]] static constexpr  result_type Init();

			/**
			 * @brief Processes a single byte of input.
			 *
			 * The supplied byte is mixed into the current hash value
			 * using the standard FNV-1A algorithm.
			 *
			 * @param hash Current hash value to update.
			 * @param byte Input byte to process.
			 */
			static constexpr  void ComputeByte(result_type& hash, const std::uint8_t byte);
		};


		template <typename T> requires (std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>)
			constexpr typename FNV_1A_INFO<T>::result_type FNV_1A_INFO<T>::Init()
		{
			// Start the computation from the standard FNV-1A offset basis.
			return FNV_basis;
		}

		template <typename T> requires (std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>)
			constexpr void FNV_1A_INFO<T>::ComputeByte(result_type& hash, const std::uint8_t byte)
		{
			// xor_eq : ^= 
			// static_cast<std::uint8_t> get the last byte (8 bits)
			hash xor_eq byte;
			hash *= FNV_prime;
		}


		

		/**
		 * @brief Primary template for FNV-1A hash implementations.
		 *
		 * Individual specializations provide hashing support for
		 * specific data types while sharing the same hashing algorithm.
		 *
		 * @tparam T Type to hash.
		 * @tparam HashBits Hash size (32 or 64 bits).
		 */
		template<typename T, typename  HashBits>
		struct FNV_1A;

	
		/*
		Example specialization:

		template<typename T, typename HashBits>
		struct FNV_1A<T, HashBits>
		{
			using hash_type = T;
			using result_type = ...;

			/// Computes the FNV-1A hash of the given value.
			[[nodiscard]] static constexpr result_type Hash(hash_type value)
			{
				...
				return ...;
			}

			/// Functor interface forwarding to Hash().
			[[nodiscard]] constexpr result_type operator()(hash_type value) const
			{
				return Hash(value);
			}
		};
		*/

		/**
		 * @brief Alias for the 32-bit FNV-1A implementation.
		 *
		 * @tparam T Type to hash.
		 */
		template<typename T>
		using FNV_1A_32 = FNV_1A<T, std::uint32_t>;

		/**
		 * @brief Alias for the 64-bit FNV-1A implementation.
		 *
		 * @tparam T Type to hash.
		 */
		template<typename T>
		using FNV_1A_64 = FNV_1A<T, std::uint64_t>;

#ifdef KTR_64B
		/**
		 * @brief Default FNV-1A implementation for the current platform.
		 *
		 * Uses the native hash width selected at compile time.
		 */
		template<typename T>
		using FNV_1A_type = FNV_1A_64<T>;
#elifdef KTR_32B
		/**
		 * @brief Default FNV-1A implementation for the current platform.
		 *
		 * Uses the native hash width selected at compile time.
		 */
		template<typename T>
		using FNV_1A_type = FNV_1A_32<T>;
#endif
	}

}

/**
 * @brief Concept restricting valid FNV-1A hash sizes.
 *
 * This concept ensures that the selected hash size corresponds to a
 * supported FNV-1A implementation. Currently, only 32-bit and 64-bit
 * unsigned integer hash representations are available.
 *
 * @tparam HB Hash size type to validate.
 */
template<typename HB>
concept KTR_ValidHashBits = std::is_same_v<HB, std::uint32_t> || std::is_same_v<HB, std::uint64_t>;

#endif
