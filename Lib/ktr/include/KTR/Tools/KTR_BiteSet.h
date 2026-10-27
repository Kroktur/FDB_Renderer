#ifndef KTR_BITSET_F
#define KTR_BITSET_F

#include <iostream>
#include <type_traits>
#include "META/KTR_MetaTools.h"

namespace KTR::Bit
{
	/**
	 * @brief Defines the types supported by the bit manipulation utilities.
	 *
	 * Integral types are supported except for bool, as well as enumeration
	 * types whose underlying integral representation can be used for bit
	 * operations.
	 *
	 * @tparam T Type to validate.
	 */
	template<typename T>
	concept ValidBitType = (std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_enum_v<T>;

	/**
	 * @brief Provides the integral type used to perform bit operations on a type.
	 *
	 * For integral types, the type itself is used. For enumeration types,
	 * the underlying integral type of the enumeration is used.
	 *
	 * @tparam T Type to inspect.
	 */
	template<typename T>
	struct bit_type;
	
	template<typename T> requires(std::is_integral_v<T>)
		struct bit_type<T> : Meta::type_container<T> {};

	template<typename T> requires(std::is_enum_v<T>)
		struct bit_type<T> : Meta::type_container<std::underlying_type_t<T>> {};

	/**
	 * @brief Alias retrieving the integral type used for bit operations.
	 *
	 * @tparam T Valid bit-manipulation type.
	 */
	template<ValidBitType T>
	using bit_type_t = typename bit_type<T>::value_type;

	/**
	 * @brief Converts a valid bit type to its integral mask representation.
	 *
	 * For enumeration types, the value is converted to its underlying type.
	 *
	 * @tparam T Valid bit-manipulation type.
	 *
	 * @param value Value to convert.
	 * @return Integral representation of the value.
	 */
	template<ValidBitType T>
	[[nodiscard]] static constexpr bit_type_t<T> to_mask(T value)
	{
		return static_cast<bit_type_t<T>>(value);
	}

	/**
	 * @brief Prints the binary representation of a value.
	 *
	 * The bits are printed from the most significant bit to the least
	 * significant bit.
	 *
	 * @tparam T Valid bit-manipulation type.
	 *
	 * @param value Value whose bits are printed.
	 */
	template<ValidBitType T>
	static void print_bits( T value)
	{
		using m_bit_type = bit_type_t<T>;
		m_bit_type size = sizeof(m_bit_type) * 8;
		m_bit_type real_value = to_mask(value);
		for (m_bit_type i = 0; i < size; ++i)
			std::cout << ((real_value >> (size - 1 - i)) & 1);
		std::cout << "\n";
	}
	
	/**
	 * @brief Provides compile-time bitwise operations for valid bit types.
	 *
	 * The operations use the integral representation of the provided values
	 * and require all operands to share the same underlying bit type.
	 */
	struct BitOpp
	{
		/**
		 * @brief Performs a bitwise OR operation between two values.
		 *
		 * @tparam LhsT Left-hand operand type.
		 * @tparam RhsT Right-hand operand type.
		 *
		 * @param lhs Left-hand operand.
		 * @param rhs Right-hand operand.
		 * @return Result of the bitwise OR operation.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
		[[nodiscard]] static constexpr bit_type_t<LhsT> BitOr(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) | to_mask<RhsT>(rhs);
		}
		/**
		 * @brief Performs a bitwise OR operation on multiple values.
		 *
		 * @tparam LhsT First operand type.
		 * @tparam RhsT Remaining operand types.
		 *
		 * @param lhs First operand.
		 * @param rhs Remaining operands.
		 * @return Result of applying bitwise OR to all operands.
		 */
		template<ValidBitType LhsT, ValidBitType... RhsT> requires((std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>> &&...))
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitOrAll(LhsT lhs, RhsT... rhs)
		{
			bit_type_t<LhsT> result = to_mask<LhsT>(lhs);
			((result |= to_mask<RhsT>(rhs)), ...);
			return result;
		}
		/**
		 * @brief Performs a bitwise XOR operation between two values.
		 *
		 * @tparam LhsT Left-hand operand type.
		 * @tparam RhsT Right-hand operand type.
		 *
		 * @param lhs Left-hand operand.
		 * @param rhs Right-hand operand.
		 * @return Result of the bitwise XOR operation.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
		[[nodiscard]] static constexpr bit_type_t<LhsT> BitXor(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) ^ to_mask<RhsT>(rhs);
		}
		/**
		 * @brief Performs a bitwise XOR operation on multiple values.
		 *
		 * @tparam LhsT First operand type.
		 * @tparam RhsT Remaining operand types.
		 *
		 * @param lhs First operand.
		 * @param rhs Remaining operands.
		 * @return Result of applying bitwise XOR to all operands.
		 */
		template<ValidBitType LhsT, ValidBitType... RhsT> requires((std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>> &&...))
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitXorAll(LhsT lhs, RhsT... rhs)
		{
			bit_type_t<LhsT> result = to_mask<LhsT>(lhs);
			((result ^= to_mask<RhsT>(rhs)), ...);
			return result;
		}
		/**
		 * @brief Performs a bitwise AND NOT operation between two values.
		 *
		 * @tparam LhsT Left-hand operand type.
		 * @tparam RhsT Right-hand operand type.
		 *
		 * @param lhs Left-hand operand.
		 * @param rhs Right-hand operand.
		 * @return Result of applying AND NOT to the operands.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitAndNot(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) & ~to_mask<RhsT>(rhs);
		}
		/**
		 * @brief Performs a bitwise AND NOT operation on multiple values.
		 *
		 * @tparam LhsT First operand type.
		 * @tparam RhsT Remaining operand types.
		 *
		 * @param lhs First operand.
		 * @param rhs Remaining operands.
		 * @return Result of applying AND NOT to all operands.
		 */
		template<ValidBitType LhsT, ValidBitType... RhsT> requires((std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>> &&...))
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitAndNotAll(LhsT lhs, RhsT... rhs)
		{
			bit_type_t<LhsT> result = to_mask<LhsT>(lhs);
			((result &= ~to_mask<RhsT>(rhs)), ...);
			return result;
		}
		/**
		 * @brief Performs a bitwise AND operation between two values.
		 *
		 * @tparam LhsT Left-hand operand type.
		 * @tparam RhsT Right-hand operand type.
		 *
		 * @param lhs Left-hand operand.
		 * @param rhs Right-hand operand.
		 * @return Result of the bitwise AND operation.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitAnd(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) & to_mask<RhsT>(rhs);
		}
		/**
		 * @brief Performs a bitwise AND operation on multiple values.
		 *
		 * @tparam LhsT First operand type.
		 * @tparam RhsT Remaining operand types.
		 *
		 * @param lhs First operand.
		 * @param rhs Remaining operands.
		 * @return Result of applying bitwise AND to all operands.
		 */
		template<ValidBitType LhsT, ValidBitType... RhsT> requires((std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>> &&...))
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitAndAll(LhsT lhs, RhsT... rhs)
		{
			bit_type_t<LhsT> result = to_mask<LhsT>(lhs);
			((result &= to_mask<RhsT>(rhs)), ...);
			return result;
		}
		/**
		 * @brief Performs a bitwise NOT operation on a value.
		 *
		 * @tparam T Valid bit-manipulation type.
		 *
		 * @param value Value to invert.
		 * @return Bitwise complement of the value.
		 */
		template<ValidBitType T>
		[[nodiscard]] static constexpr bit_type_t<T> BitNot(T value)
		{
			return ~to_mask<T>(value);
		}
		/**
		 * @brief Performs a left bit shift.
		 *
		 * @tparam LhsT Value type.
		 * @tparam RhsT Shift amount type.
		 *
		 * @param lhs Value to shift.
		 * @param rhs Number of positions to shift.
		 * @return Result of the left shift operation.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitShiftLeft(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) << to_mask<RhsT>(rhs);
		}
		/**
		 * @brief Performs a right bit shift.
		 *
		 * @tparam LhsT Value type.
		 * @tparam RhsT Shift amount type.
		 *
		 * @param lhs Value to shift.
		 * @param rhs Number of positions to shift.
		 * @return Result of the right shift operation.
		 */
		template<ValidBitType LhsT, ValidBitType RhsT> requires(std::is_same_v<bit_type_t<LhsT>, bit_type_t<RhsT>>)
			[[nodiscard]] static constexpr bit_type_t<LhsT> BitShiftRight(LhsT lhs, RhsT rhs)
		{
			return to_mask<LhsT>(lhs) >> to_mask<RhsT>(rhs);
		}

	};

		
}
#endif
