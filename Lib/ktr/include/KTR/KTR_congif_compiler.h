#ifndef KTR_CONFIG_COMPILER_F
#define KTR_CONFIG_COMPILER_F

namespace KTR
{
	#ifdef NDEBUG
	#define KTR_RELEASE
	static constexpr bool debugEnable = false;
	#else
	#define KTR_DEBUG
	static constexpr bool debugEnable = true;
	#endif

	#ifdef _WIN64 
		#define KTR_WIN
		#define KTR_64B 
		static constexpr bool is32Bit = false;
	#elif defined(_WIN32)
		#define  KTR_WIN
		#define KTR_32B 
		static constexpr bool is32Bit = true;
	#elif   defined(__linux__) && defined(__x86_64__)
		#define  KTR_LIN
		#define KTR_64B 
		static constexpr bool is32Bit = false;
	#elif  defined(__linux__) &&  defined(__i386__)
		#define  KTR_LIN
		#define KTR_32B 
		static constexpr bool is32Bit = true;
#else
	#error "KTR: unsupported platform (only Windows and Linux x86/x86_64 are supported)"
	#endif

}
#endif
