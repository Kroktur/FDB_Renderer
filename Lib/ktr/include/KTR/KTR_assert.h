#ifndef KTR_ASSERT_F
#define KTR_ASSERT_F
#include <cassert>


namespace KTR
{
	#define KTR_STATIC_ASSERT(cond,msg) static_assert(cond,msg)
	#define KTR_ASSERT(cond,msg) assert(cond)
}

#endif
