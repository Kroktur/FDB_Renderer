#ifndef FDB_CORE_RECTF
#define FDB_CORE_RECTF

#include <type_traits>
namespace FDB
{
	template<typename T> requires(std::is_arithmetic_v<T>)
		struct Rect
	{
		using value_type = T;
		value_type x = 0;
		value_type y = 0;
		value_type width = 0;
		value_type height = 0;

		bool operator==(const Rect& other) const
		{
			return x == other.x && y == other.y && width == other.width && height == other.height;
		}
	};
}

#endif
