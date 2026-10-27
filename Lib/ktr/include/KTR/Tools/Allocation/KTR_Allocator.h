#ifndef KTR_ALLOCATOR_F
#define KTR_ALLOCATOR_F

#include <concepts>
#include <memory>

namespace KTR::allocation
{
	template<typename Allocator>
	concept ValidBaseAllocator = requires(Allocator allocator, typename Allocator::value_type * data, std::size_t n)
	{
		typename Allocator::value_type;

		{ allocator.allocate(n) } -> std::same_as<typename Allocator::value_type*>;
		{ allocator.deallocate(data, n) };
	};

	template<typename Allocator>
	concept ValidRebindAllocator = requires(Allocator alloc)
	{
		typename std::allocator_traits<Allocator>::template rebind_alloc<int>;
		{ typename std::allocator_traits<Allocator>::template rebind_alloc<int>{alloc} };
		{ Allocator{} };
	}&& ValidBaseAllocator<Allocator>;

	template<typename T>
	using default_allocator_type = std::allocator<T>;
}
#endif
