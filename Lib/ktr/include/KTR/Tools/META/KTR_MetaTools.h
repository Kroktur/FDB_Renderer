#ifndef KTR_METATOOLS_F
#define KTR_METATOOLS_F

namespace KTR::Meta
{
	/**
	* @brief Wraps a type into a lightweight compile-time container.
	*
	* This utility is similar to std::type_identity and is commonly used
	* to transport a type through template metaprogramming without
	* instantiating an object of that type.
	*
	* @tparam T Type stored by the container.
	*/
	template<typename T>
	struct type_container
	{
		using value_type = T;
		/**
		 * @brief Implicitly converts the container to the stored type.
		 *
		 * @return A default-constructed instance of the stored type.
		 */
		constexpr operator value_type() const { return value_type{}; }
		/**
		 * @brief Returns a default-constructed instance of the stored type.
		 *
		 * @return A default-constructed value.
		 */
		constexpr value_type operator()() const { return value_type{}; }
	};

	template<typename T>
	using type_container_t = typename type_container<T>::value_type;

	/**
	 * @brief Wraps a compile-time value into a type.
	 *
	 * This utility behaves similarly to std::integral_constant and is
	 * intended for compile-time metaprogramming.
	 *
	 * @tparam T Value type.
	 * @tparam data Compile-time constant value.
	 */
	template<typename T,T data>
	struct data_container
	{
		using value_type = T;
		static constexpr value_type value = data;
		/**
		 * @brief Implicitly converts the container to its stored value.
		 *
		 * @return The wrapped compile-time value.
		 */
		constexpr operator value_type() const { return value; }
		/**
		 * @brief Returns the wrapped compile-time value.
		 *
		 * @return The stored constant.
		 */
		constexpr value_type operator()() const { return value; }
	};

	template<typename T, T data>
	using data_container_t = typename data_container<T, data>::value_type;
	template<typename T, T data>
	static constexpr data_container_t<T,data> data_container_v = data_container<T, data>::value;

	/**
	 * @brief Compile-time representation of the boolean value true.
	 */
	using true_type = data_container<bool, true>;
	/**
	 * @brief Compile-time representation of the boolean value false.
	 */
	using false_type = data_container<bool, false>;

	/**
	 * @brief Alias expanding to void.
	 *
	 * Primarily intended to simplify SFINAE-based type detection.
	 *
	 * @tparam T Ignored type.
	 */
	template<typename T>
	using alias_void_t = void;


	/**
	 * @brief Selects one of two types at compile time.
	 *
	 * @tparam cond Selection condition.
	 * @tparam T Selected when cond is true.
	 * @tparam T2 Selected when cond is false.
	 */
	template<bool cond,typename T,typename T2>
	struct conditional;
	template<typename T,typename T2>
	struct conditional<true, T, T2> : type_container<T> {};
	template<typename T, typename T2>
	struct conditional<false, T, T2> : type_container<T2> {};

	/**
	 * @brief Retrieves the type selected by conditional.
	 *
	 * @tparam cond Selection condition.
	 * @tparam T Selected when cond is true.
	 * @tparam T2 Selected when cond is false.
	 */
	template<bool cond, typename T, typename T2>
	using conditional_t = typename conditional<cond, T, T2>::value_type;

	
	/**
	 * @brief Conditionally exposes a type.
	 *
	 * This utility behaves similarly to std::enable_if and is intended
	 * for SFINAE-based template constraints.
	 *
	 * @tparam cond Compile-time condition.
	 * @tparam T Exposed type when the condition is true.
	 */
	template<bool cond, typename T = void>
	struct enable_if {};
	template<typename T>
	struct enable_if<true,T> : type_container<T> {};

	/**
	 * @brief Retrieves the type exposed by enable_if.
	 *
	 * @tparam cond Compile-time condition.
	 * @tparam T Type returned when the condition is satisfied.
	 */
	template<bool cond, typename T>
	using enable_if_t = enable_if<cond, T>::value_type;
}


/**
 * @brief Generates a trait detecting the presence of a nested type.
 *
 * The generated trait inherits from KTR::Meta::true_type when the
 * specified nested type exists, and from false_type otherwise.
 *
 * Generated symbols:
 * - trait_name
 * - trait_name_v
 * - trait_name_t
 */
#define KTR_DEFINE_HAS_MEMBER_TYPE(trait_name, member_name)           \
    template<typename T, typename T2 = void>                          \
    struct trait_name : KTR::Meta::false_type {};                     \
    template<typename T>                                              \
    struct trait_name<T, KTR::Meta::alias_void_t<                     \
        typename T::member_name>> : KTR::Meta::true_type {};          \
    template<typename T>                                              \
    static constexpr bool trait_name##_v = trait_name<T>::value;      \
	template<typename T>                                              \
	using trait_name##_t = trait_name<T>::value_type;


 /**
  * @brief Generates a trait detecting the presence of a static member.
  *
  * The generated trait inherits from KTR::Meta::true_type when the
  * specified static member exists, and from false_type otherwise.
  *
  * Generated symbols:
  * - trait_name
  * - trait_name_v
  * - trait_name_t
  */
#define KTR_DEFINE_HAS_MEMBER_VALUE(trait_name, member_name)          \
    template<typename T, typename T2 = void>                          \
    struct trait_name : KTR::Meta::false_type {};                     \
    template<typename T>                                              \
    struct trait_name<T, KTR::Meta::alias_void_t<                     \
        decltype(T::member_name)>> : KTR::Meta::true_type {};         \
    template<typename T>                                              \
    static constexpr bool trait_name##_v = trait_name<T>::value;      \
	template<typename T>                                              \
	using trait_name##_t = trait_name<T>::value_type;        \


#endif 
