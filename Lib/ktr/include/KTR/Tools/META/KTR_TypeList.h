



#ifndef KTR_TYPELIST_F
#define KTR_TYPELIST_F
#include "KTR_assert.h"
#include "KTR_MetaTools.h"
#include <type_traits>
#include <tuple>
#include <variant>

template<typename fn, typename returnT, typename... CallArgs >
concept is_typelist_callable_RT = requires(fn && fn_, CallArgs&&... args)
{
	{ fn_.template operator() < int > (std::forward<CallArgs>(args)...) }->  std::convertible_to<returnT>;
};


namespace KTR::Meta {
	template<typename ... Args>struct typelist {};



	/**
	* @brief Retrieves the first type contained in a typelist.
	*
	* @tparam List Input typelist.
	*/
	template <typename List>
	struct front;
	template<typename Head, typename... Tail>
	struct front<typelist<Head, Tail...>> : type_container<Head> {};
	/**
	* @brief Alias retrieving the first type of a typelist.
	*
	* @tparam List Input typelist.
	*/
	template<typename List>
	using front_t = typename front<List>::value_type;

	/**
	* @brief Inserts a type at the beginning of a typelist.
	*
	* @tparam type Type to insert.
	* @tparam List Target typelist.
	*/
	template<typename type, typename List>
	struct push_front;
	template<typename type, typename... List>
	struct push_front<type, typelist<List...>> : type_container<typelist<type, List...>> {};

	/**
	* @brief Alias retrieving the resulting typelist after a push_front operation.
	*
	* @tparam type Type to insert.
	* @tparam list Target typelist.
	*/
	template<typename type, typename list>
	using push_front_t = typename push_front<type, list>::value_type;

	/**
	* @brief Removes the first type from a typelist.
	*
	* @tparam list Input typelist.
	*/
	template<typename List>
	struct pop_front;
	template<typename first, typename... List>
	struct pop_front< typelist<first, List...>> : type_container<typelist<List...>> {};

	/**
	 * @brief Alias retrieving the typelist after removing its first type.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	using pop_front_t = typename pop_front<list>::value_type;


	/**
	 * @brief Retrieves the last type contained in a typelist.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct back;
	template<typename last>
	struct back<typelist<last>> : type_container<last> {};
	template<typename first, typename... Arg>
	struct back<typelist<first, Arg...>> : back < typelist < Arg... >> {};

	/**
	 * @brief Alias retrieving the last type of a typelist.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	using back_t = typename back<list>::value_type;


	/**
	 * @brief Appends a type to the end of a typelist.
	 *
	 * @tparam type Type to append.
	 * @tparam List Target typelist.
	 */
	template<typename type, typename List>
	struct push_back;
	template<typename type, typename... List>
	struct push_back<type, typelist<List...>> : type_container<typelist<List..., type>> {};

	/**
	 * @brief Alias retrieving the resulting typelist after a push_back operation.
	 *
	 * @tparam type Type to append.
	 * @tparam List Target typelist.
	 */
	template<typename type, typename List>
	using push_back_t = typename push_back<type, List>::value_type;


	/**
	 * @brief Removes the last type from a typelist.
	 *
	 * @tparam List Input typelist.
	 */
	template<typename List>
	struct pop_back;
	template<typename last>
	struct pop_back<typelist<last>> :type_container<typelist<>> {};
	template<typename first, typename... Rest>
	struct pop_back<typelist<first, Rest...>> : type_container<push_front_t<first, typename pop_back<typelist<Rest...>>::value_type>> {};

	/**
	 * @brief Alias retrieving the typelist after removing its last type.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	using pop_back_t = typename pop_back<list>::value_type;


	/**
	 * @brief Reverses the order of the types in a typelist.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct reverse;
	template<>
	struct reverse<typelist<>> : type_container<typelist<>> {};
	template<typename first, typename... Rest>
	struct reverse<typelist<first, Rest...>> : type_container<push_back_t<first, typename reverse<typelist<Rest...>>::value_type>> {};

	/**
	* @brief Alias retrieving the reversed typelist.
	*
	* @tparam list Input typelist.
	*/
	template<typename list>
	using reverse_t = typename  reverse<list>::value_type;

	/**
	 * @brief Computes the number of types contained in a typelist.
	 *
	 * @tparam List Input typelist.
	 */
	template<typename List>
	struct size;
	template<typename... Args>
	struct size<typelist<Args...>> :data_container<std::size_t, sizeof...(Args)> {};

	/**
	 * @brief Compile-time number of types contained in a typelist.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	static constexpr std::size_t size_v = size<list>::value;

	/**
	 * @brief Retrieves the type located at a given index.
	 *
	 * @tparam index Zero-based index.
	 * @tparam List Input typelist.
	 */
	template<std::size_t index, typename List>
	struct at;
	template<std::size_t index>
	struct at<index, typelist<>> { KTR_STATIC_ASSERT(index != index, "at: index out of bounds in typelist"); };
	template<typename... Args>
	struct at<0, typelist<Args...>> : type_container<front_t<typelist<Args...>>> {};
	template<std::size_t index, typename... Args>
	struct at<index, typelist<Args...>> : at<index - 1, pop_front_t<typelist<Args...>>> {};
	/**
	* @brief Alias retrieving the type at the specified index.
	*
	* @tparam index Zero-based index.
	* @tparam list Input typelist.
	*/
	template<std::size_t index, typename list> requires(index < size_v<list>)
	using at_t = typename  at<index, list>::value_type;


	/**
	 * @brief Determines whether a typelist is empty.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct is_empty;
	template<>
	struct is_empty<typelist<>> : true_type {};
	template<typename... Arg>
	struct is_empty<typelist<Arg...>> : false_type {};

	/**
	 * @brief Compile-time result indicating whether a typelist is empty.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	static constexpr bool is_empty_v = is_empty<list>::value;

	/**
	 * @brief Determines whether a typelist contains a given type.
	 *
	 * @tparam type Type to search for.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	struct contains;
	template<typename type>
	struct contains<type, typelist<>> : false_type {};
	template<typename type, typename...Arg>
	struct contains<type, typelist<type, Arg...>> : true_type {};
	template<typename type, typename... Arg>
	struct contains<type, typelist<Arg...>> : contains<type, pop_front_t<typelist<Arg...>>> {};
	/**
	* @brief Compile-time result indicating whether a type is present in a typelist.
	*
	* @tparam type Type to search for.
	* @tparam list Input typelist.
	*/
	template<typename type, typename list>
	static constexpr bool contains_v = contains < type, list >::value;

	/**
	* @brief Retrieves the index of the first occurrence of a type.
	*
	* Produces the zero-based index of the first matching type contained
	* in a typelist.
	*
	* @tparam index Internal recursion index.
	* @tparam type Type to search for.
	* @tparam list Input typelist.
	*/
	template<std::size_t index, typename type, typename list>
	struct get_index;
	template<std::size_t index, typename type>
	struct get_index<index, type, typelist<>> { KTR_STATIC_ASSERT(index != index, "get_index: type not found in typelist"); };
	template<std::size_t index, typename type, typename... Arg>
	struct get_index<index, type, typelist<type, Arg...>> : data_container<std::size_t, index> {};
	template<std::size_t index, typename type, typename... Arg>
	struct get_index<index, type, typelist<Arg...>> : get_index<index + 1, type, pop_front_t<typelist<Arg...>>> {};
	/**
	* @brief Compile-time index of the first occurrence of a type.
	*
	* @tparam type Type to search for.
	* @tparam list Input typelist.
	*/
	template< typename type, typename list>requires contains_v<type, list>
	static constexpr std::size_t get_index_v = get_index<0, type, list>::value;


	/**
	 * @brief Counts the occurrences of a type in a typelist.
	 *
	 * @tparam index Internal recursion counter.
	 * @tparam type Type to count.
	 * @tparam list Input typelist.
	 */
	template<std::size_t index, typename type, typename list>
	struct count;
	template<std::size_t index, typename type>
	struct count<index, type, typelist<>> : data_container<std::size_t, index> {};
	template<std::size_t index, typename type, typename... Arg>
	struct count<index, type, typelist<type, Arg...>> : count<index + 1, type, typelist<Arg...>> {};
	template<std::size_t index, typename type, typename... Arg>
	struct count<index, type, typelist<Arg...>> : count<index, type, pop_front_t<typelist<Arg...>>> {};

	/**
	 * @brief Compile-time number of occurrences of a type.
	 *
	 * @tparam type Type to count.
	 * @tparam list Input typelist.
	 */
	template< typename type, typename list>
	static constexpr std::size_t count_v = count < 0, type, list >::value;


	/**
	 * @brief Determines whether every type in a typelist matches a given type.
	 *
	 * @tparam type Reference type.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	struct is_all_same;
	template<typename type>
	struct is_all_same<type, typelist<type>> :true_type {};
	template<typename type, typename... Arg>
	struct is_all_same<type, typelist<type, Arg...>> :is_all_same<type, typelist<Arg...>> {};
	template<typename type, typename... Arg>
	struct is_all_same<type, typelist<Arg...>> :false_type {};

	/**
	 * @brief Compile-time result indicating whether all types are identical.
	 *
	 * @tparam type Reference type.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	static constexpr bool is_all_same_v = is_all_same<type, list>::value;


	/**
	 * @brief Determines whether two typelists contain the same sequence of types.
	 *
	 * @tparam list First typelist.
	 * @tparam list2 Second typelist.
	 */
	template< typename list, typename list2>
	struct is_same_typeList : false_type {};
	template< typename... Arg>
	struct is_same_typeList<typelist<Arg...>, typelist<Arg...>> :true_type {};

	/**
	 * @brief Compile-time result comparing two typelists.
	 *
	 * @tparam list First typelist.
	 * @tparam list2 Second typelist.
	 */
	template<typename list, typename list2>
	static constexpr bool is_same_typeList_v = is_same_typeList<list, list2>::value;


	/**
	 * @brief Determines whether a specific type appears more than once.
	 *
	 * @tparam type Type to inspect.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	struct has_duplicate_type;
	template<typename type>
	struct has_duplicate_type <type, typelist<>> :false_type {};
	template<typename type, typename... Arg>
	struct has_duplicate_type<type, typelist<type, Arg...>> :contains<type, typelist<Arg...>> {};
	template<typename type, typename... Arg>
	struct has_duplicate_type<type, typelist<Arg...>> :has_duplicate_type<type, pop_front_t<typelist<Arg...>>> {};

	/**
	 * @brief Compile-time result indicating whether a type is duplicated.
	 *
	 * @tparam type Type to inspect.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	static constexpr bool has_duplicate_type_v = has_duplicate_type<type, list>::value;

	/**
	 * @brief Determines whether a typelist contains duplicate types.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct has_duplicate;
	template<>
	struct has_duplicate <typelist<>> :false_type {};
	template<typename first, typename... Arg>
	struct has_duplicate<typelist<first, Arg...>> :conditional_t<contains_v<first, typelist<Arg...>>, true_type, has_duplicate<typelist<Arg...>>> {};

	/**
	 * @brief Compile-time result indicating whether a typelist contains duplicates.
	 *
	 * @tparam list Input typelist.
	 */
	template< typename list>
	static constexpr bool has_duplicate_v = has_duplicate<list>::value;


	/**
	 * @brief Removes the first occurrence of a type from a typelist.
	 *
	 * @tparam type Type to remove.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list >
	struct erase;
	template<typename type>
	struct erase<type, typelist<>> :type_container<typelist<>> {};
	template<typename type, typename ...Arg>
	struct erase<type, typelist<type, Arg...>> :type_container<typelist<Arg...>> {};
	template<typename type, typename first, typename ...Arg >
	struct erase<type, typelist<first, Arg...>> :type_container<push_front_t<first, typename erase<type, typelist<Arg...>>::value_type>> {};

	/**
	 * @brief Alias retrieving the typelist after erasing the first occurrence.
	 *
	 * @tparam type Type to remove.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	using erase_t = typename erase<type, list>::value_type;


	/**
	 * @brief Removes every occurrence of a type from a typelist.
	 *
	 * @tparam type Type to remove.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	struct remove_type;
	template<typename type>
	struct remove_type<type, typelist<>> : type_container<typelist<>> {};
	template<typename type, typename ... Arg>
	struct remove_type < type, typelist<type, Arg...>> : type_container<typename remove_type<type, typelist<Arg...>>::value_type> {};
	template<typename type, typename first, typename ... Arg>
	struct remove_type < type, typelist<first, Arg...>> : type_container<push_front_t<first, typename remove_type<type, typelist<Arg...>>::value_type>> {};

	/**
	 * @brief Alias retrieving the typelist after removing all occurrences.
	 *
	 * @tparam type Type to remove.
	 * @tparam list Input typelist.
	 */
	template<typename type, typename list>
	using remove_type_t = typename remove_type<type, list>::value_type;


	/**
	 * @brief Removes duplicate types from a typelist.
	 *
	 * The first occurrence of each type is preserved while all subsequent
	 * duplicates are removed.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct unique;
	template<>
	struct unique<typelist<>> :type_container<typelist<>> {};
	template<typename First, typename... Rest>
	struct unique<typelist<First, Rest...>> : type_container<push_front_t<First, typename unique<remove_type_t<First, typelist<Rest...>>>::value_type>> {};
	/**
	* @brief Alias retrieving the typelist containing unique types.
	*
	* @tparam List Input typelist.
	*/
	template<typename List>
	using unique_t = typename unique<List>::value_type;


	/**
	 * @brief Determines whether a typelist matches a parameter pack.
	 *
	 * The comparison succeeds only if both contain the same types in
	 * the same order.
	 *
	 * @tparam list Reference typelist.
	 * @tparam Arg Types to compare.
	 */
	template<typename list, typename... Arg>
	struct is_matching_args;
	template<>
	struct is_matching_args <typelist<>> : true_type {};
	template<typename... ts, typename... Arg >
	struct is_matching_args < typelist<ts...>, Arg...> : is_same_typeList<typelist<ts...>, typelist<Arg...>> {};

	/**
	 * @brief Compile-time result indicating whether a typelist matches a parameter pack.
	 *
	 * @tparam list Reference typelist.
	 * @tparam Arg Types to compare.
	 */
	template<typename list, typename... Arg>
	static constexpr bool is_matching_args_v = is_matching_args <list, Arg...>::value;


	/**
	 * @brief Concatenates two typelists.
	 *
	 * Optionally removes duplicate types from the resulting typelist.
	 *
	 * @tparam LhsType Left-hand typelist.
	 * @tparam RhsType Right-hand typelist.
	 * @tparam unique Enables duplicate removal when true.
	 */
	template<typename LhsType, typename RhsType, bool unique>
	struct concat;
	template<typename ...LhsArgs, typename ...RhsArgs, bool unique>
	struct concat<typelist<LhsArgs...>, typelist<RhsArgs...>, unique> : conditional<unique, unique_t<typelist<LhsArgs..., RhsArgs...>>, typelist<LhsArgs..., RhsArgs...>> {};

	/**
	 * @brief Alias retrieving the concatenated typelist.
	 *
	 * @tparam LhsType Left-hand typelist.
	 * @tparam RhsType Right-hand typelist.
	 * @tparam unique Enables duplicate removal when true.
	 */
	template<typename LhsType, typename RhsType, bool unique>
	using  concat_t = typename concat<LhsType, RhsType, unique>::value_type;


	/**
	 * @brief Filters the types of a typelist using a compile-time predicate.
	 *
	 * Only the types satisfying the predicate are kept.
	 *
	 * @tparam Filter Unary type predicate.
	 * @tparam List Input typelist.
	 */
	template<template<typename> class Filter, typename List>
	struct filter;

	template<template<typename> class Filter>
	struct filter<Filter, typelist<>> : type_container<typelist<>> {};


	template<template<typename >class T, typename Test>
	concept typelist_filter_type = requires()
	{
		{ T<Test>::value }-> std::convertible_to<bool>;
	};

	template<template<typename> class Filter, typename First, typename ...Rest> requires(typelist_filter_type<Filter, First>)
		struct filter<Filter, typelist<First, Rest...>> : conditional <
		Filter<First>::value
		, push_front_t < First, typename filter<Filter, typelist< Rest...>>::value_type >
		, typename filter<Filter, typelist<Rest...>>::value_type> {};


	/**
	 * @brief Alias retrieving the filtered typelist.
	 *
	 * @tparam Filter Unary type predicate.
	 * @tparam List Input typelist.
	 */
	template<template<typename> class Filter, typename List>
	using filter_t = typename filter<Filter, List>::value_type;


	/**
	 * @brief Applies a compile-time transformation to every type.
	 *
	 * Each type of the input typelist is replaced by the corresponding
	 * transformed type.
	 *
	 * @tparam Transform Unary type transformation.
	 * @tparam List Input typelist.
	 */
	template<template<typename >class Transform, typename List>
	struct transform;

	template<template<typename >class Transform>
	struct transform<Transform, typelist<>> : type_container<typelist<>> {};


	template<template<typename >class T, typename Test>
	concept typelist_transform_type = requires()
	{
		typename T<Test>::value_type;
	};

	template<template<typename> class Transform, typename First, typename ...Rest> requires(typelist_transform_type<Transform, First>)
		struct transform < Transform, typelist<First, Rest...>> : type_container<push_front_t<typename Transform<First>::value_type, typename transform<Transform, typelist<Rest...>>::value_type >> {};

	/**
	 * @brief Alias retrieving the transformed typelist.
	 *
	 * @tparam Transform Unary type transformation.
	 * @tparam List Input typelist.
	 */
	template<template<typename >class Transform, typename List>
	using transform_t = typename transform<Transform, List>::value_type;


	/**
	 * @brief Determines whether a type is a typelist.
	 *
	 * @tparam T Type to inspect.
	 */
	template<typename T>
	struct is_type_list : false_type {};
	template<typename ...Args>
	struct is_type_list<typelist<Args...>> : true_type {};
	/**
	* @brief Compile-time result indicating whether a type is a typelist.
	*
	* @tparam Type Type to inspect.
	*/
	template<typename Type>
	static constexpr bool is_type_list_v = is_type_list <Type>::value;



	/**
	 * @brief Provides runtime utilities for a typelist.
	 *
	 * Allows runtime iteration over the types stored in a typelist by
	 * invoking a templated callable for each element.
	 *
	 * @tparam List Input typelist.
	 */
	template<typename List>
	struct RunTime;

	template<typename ... Args>
	struct RunTime<typelist<Args...>>
	{
		using typelist_type = typelist<Args...>;
		/**
		* @brief Invokes a callable once for each type contained in the typelist.
		*
		* The callable must provide a templated call operator accepting the
		* forwarded runtime arguments.
		*
		* Example:
		* @code
		* struct Visitor
		* {
		*     template<typename T>
		*     void operator()();
		* };
		*
		* RunTime<MyList>::for_each(Visitor{});
		* @endcode
		*
		* @tparam fn Callable type.
		* @tparam CallArgs Types of the forwarded runtime arguments.
		*
		* @param fn_ Callable invoked for every type in the typelist.
		* @param _args Runtime arguments forwarded to each invocation.
		*/
		template<typename fn, typename ...CallArgs> requires(is_typelist_callable_RT<fn, void, CallArgs...>)
			static void for_each(fn&& fn_, CallArgs&&... _args)
		{
			(std::forward<fn>(fn_).template operator() < Args > (std::forward<CallArgs>(_args)...), ...);
		}

		/**
		 * @brief Invokes a callable once for each type while providing its index.
		 *
		 * The callable receives the zero-based index of the current type as its
		 * first runtime argument, followed by the forwarded arguments.
		 *
		 * @tparam fn Callable type.
		 * @tparam CallArgs Types of the forwarded runtime arguments.
		 *
		 * @param fn_ Callable invoked for every type in the typelist.
		 * @param _args Runtime arguments forwarded to each invocation.
		 */
		template<typename fn, typename ...CallArgs> requires(is_typelist_callable_RT<fn, void, std::uint64_t, CallArgs...>)
			static void for_each_index(fn&& fn_, CallArgs&&... _args)
		{
			std::uint64_t index = 0;

			(std::forward<fn>(fn_).template operator() < Args > (index++, std::forward<CallArgs>(_args)...), ...);
		}
	};


	/**
	 * @brief Converts a typelist into equivalent standard library types.
	 *
	 * Provides aliases exposing the typelist as a std::tuple and a
	 * std::variant.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	struct convert;

	template<typename ... Args>
	struct convert<typelist<Args...>>
	{
		using as_tuple_type = std::tuple<Args...>;
		using as_variant_type = std::variant<Args...>;
	};

	/**
	 * @brief Alias converting a typelist into a std::tuple.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	using convert_as_tuple_t = typename convert<list>::as_tuple_type;

	/**
	 * @brief Alias converting a typelist into a std::variant.
	 *
	 * @tparam list Input typelist.
	 */
	template<typename list>
	using convert_as_variant_t = typename convert<list>::as_variant_type;

}

#endif
