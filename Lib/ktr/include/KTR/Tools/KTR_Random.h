#ifndef KTR_RANDOM_F
#define KTR_RANDOM_F
#include "KTR/KTR_congif_compiler.h"
#include <cstdint>
#include <random>
#include "KTR/KTR_assert.h"

namespace KTR::rd_priv
{
	[[nodiscard]] static constexpr uint64_t multiply64to128(uint64_t lhs, uint64_t rhs, uint64_t* high)
	{
		// this function multiplies two 64-bit unsigned integers and returns the lower 64 bits of the result, while storing the upper 64 bits in the variable pointed to by `high`.
		// found this implementation on Stack Overflow, which provides a portable way to perform 64-bit multiplication and obtain a 128-bit result.
		// https://stackoverflow.com/questions/25095741/how-can-i-multiply-64-bit-operands-and-get-128-bit-result-portably
		uint64_t lo_lo = (lhs & 0xFFFFFFFF) * (rhs & 0xFFFFFFFF);
		uint64_t hi_lo = (lhs >> 32) * (rhs & 0xFFFFFFFF);
		uint64_t lo_hi = (lhs & 0xFFFFFFFF) * (rhs >> 32);
		uint64_t hi_hi = (lhs >> 32) * (rhs >> 32);

		/* Now add the products together. These will never overflow. */
		uint64_t cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
		*high = (hi_lo >> 32) + (cross >> 32) + hi_hi;
		return (cross << 32) | (lo_lo & 0xFFFFFFFF);
	}
	template<typename T, typename U>
	struct UpperTypeCrtp
	{
		using value_type = T;
		using upper_type = U;
		static constexpr std::size_t bits = sizeof(value_type) * 8;
		static constexpr std::size_t upperBits = sizeof(value_type) * 8;
	};

	template<typename T>
	struct UpperType;

	template<>
	struct UpperType<std::uint8_t> : UpperTypeCrtp<std::uint8_t, std::uint16_t> {};
	template<>
	struct UpperType<std::uint16_t> : UpperTypeCrtp<std::uint16_t, std::uint32_t> {};
	template<>
	struct UpperType<std::uint32_t> : UpperTypeCrtp<std::uint32_t, std::uint64_t> {};
	template<>
	struct UpperType<std::uint64_t> : UpperTypeCrtp<std::uint64_t, void> {};

	template<typename T>
	concept ValidUpperType = requires()
	{
		typename UpperType<T>::value_type;
		typename UpperType<T>::upper_type;
		{ UpperType<T>::bits };
	};

	template<typename T>
	using upper_type_t = UpperType<T>;

	template<ValidUpperType T>
	[[nodiscard]] static constexpr T multiplyTto2xT(T lhs, T rhs, T* high)
	{
		if constexpr (std::is_same_v<std::uint64_t, typename upper_type_t<T>::value_type> && std::is_void_v<typename upper_type_t<T>::upper_type>)
		{
			return rd_priv::multiply64to128(lhs, rhs, high);
		}
		else
		{
			typename UpperType<T>::upper_type cross = static_cast<typename upper_type_t<T>::upper_type>(lhs) * static_cast<typename upper_type_t<T>::upper_type>(rhs);
			*high = static_cast<T>(cross >> upper_type_t<T>::bits);
			return static_cast<T>(cross);
		}

	}

	[[nodiscard]] static constexpr std::uint64_t mix64(std::uint64_t& value)
	{
		std::uint64_t result = (value += 0x9E3779B97F4A7C15);
		result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
		result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
		return result ^ (result >> 31);
	}
	[[nodiscard]] static constexpr std::uint32_t mix32(std::uint32_t& value)
	{
		std::uint32_t result = (value += 0x9e3779b9);
		result = (result ^ (result >> 15)) * 0x85ebca6b;
		result = (result ^ (result >> 13)) * 0xc2b2ae35;
		return result ^ (result >> 16);
	}


}

namespace KTR::random
{
	template<typename T>
	struct CRTPTypeByBits
	{
		using value_type = T;
		static constexpr std::size_t bits = sizeof(value_type) * 8;
	};

	template<std::size_t  bits>
	struct TypeByBits;

	template<std::size_t bits> requires(bits > 0 && bits <= 8)
		struct TypeByBits<bits> : CRTPTypeByBits<std::uint8_t> {};

	template<std::size_t  bits> requires(bits > 8 && bits <= 16)
		struct TypeByBits<bits> : CRTPTypeByBits<std::uint16_t> {};

	template<std::size_t  bits> requires(bits > 16 && bits <= 32)
		struct TypeByBits<bits> : CRTPTypeByBits<std::uint32_t> {};

	template<std::size_t  bits> requires(bits > 32)
		struct TypeByBits<bits> : CRTPTypeByBits<std::uint64_t> {};

	template<std::size_t  bits>
	using type_by_bits_t = TypeByBits<bits>;

	template<typename T>
	concept IsValidBitGen = requires(T t)
	{
		typename T::result_type;
		{ t() } -> std::convertible_to<typename T::result_type>;
	}&& rd_priv::ValidUpperType<typename T::result_type>;


	template<std::size_t  bits, IsValidBitGen Gen> requires(sizeof(typename Gen::result_type) * 8 >= bits)
		[[nodiscard]] static type_by_bits_t<bits>::value_type fixedWidthRandom(Gen& gen)
	{
		using gen_result_type = typename Gen::result_type;
		using bit_type = type_by_bits_t<bits>::value_type;

		constexpr std::size_t genResultTypeBits = sizeof(gen_result_type) * 8;
		// keep only the highest bits of the generated number to fit into the desired bit width
		return static_cast<bit_type>(gen() >> (genResultTypeBits - bits));
	}


	template<std::size_t bits, IsValidBitGen Gen> requires(sizeof(typename Gen::result_type) * 8 < bits)
		[[nodiscard]] static type_by_bits_t<bits>::value_type fixedWidthRandom(Gen& gen)
	{
		using gen_result_type = typename Gen::result_type;
		using bit_type = type_by_bits_t<bits>::value_type;

		constexpr  std::size_t genResultTypeBits = sizeof(gen_result_type) * 8;
		std::size_t count = 0;
		bit_type result = 0;

		while (genResultTypeBits * count < type_by_bits_t<bits>::bits)
		{
			gen_result_type random = gen();
			result |= static_cast<bit_type>(random) << (genResultTypeBits * count);
			++count;
		}
		return  result;
	}

}
namespace KTR::distrib
{
	template<typename T, random::IsValidBitGen Gen> requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
		[[nodiscard]] static T lemire_nearlydivisionless(Gen& gen, T _ceil)
	{

		using wanted_type = T;
		static constexpr std::size_t bits = sizeof(wanted_type) * 8;
		using gen_result_type = random::type_by_bits_t<bits>::value_type;
		gen_result_type ceil = static_cast<gen_result_type>(_ceil);
		gen_result_type rd = random::fixedWidthRandom< bits, Gen >(gen);
		gen_result_type high;
		gen_result_type lowBit = rd_priv::multiplyTto2xT<gen_result_type>(rd, ceil, &high);
		if (lowBit < ceil) {
			gen_result_type t = -ceil % ceil;
			while (lowBit < t) {
				rd = random::fixedWidthRandom< bits, Gen >(gen);
				lowBit = rd_priv::multiplyTto2xT<gen_result_type>(rd, ceil, &high);
			}
		}
		return static_cast<wanted_type>(high);
	}

	template<typename T, random::IsValidBitGen Gen> requires(std::is_floating_point_v<T>)
		[[nodiscard]] static T cross_platform_real_range(Gen& gen, T ceil)
	{

		using wanted_type = T;
		static constexpr std::size_t bits = sizeof(wanted_type) * 8;
		using gen_result_type = random::type_by_bits_t<bits>::value_type;
		static constexpr std::size_t digits = std::numeric_limits<T>::digits;
		static constexpr std::size_t genBits = sizeof(gen_result_type) * 8;

		gen_result_type high = random::fixedWidthRandom< bits, Gen >(gen);
		// 2^-digits
		static wanted_type ldexp = std::ldexp(static_cast<wanted_type>(1), -static_cast<int>(digits));

		if constexpr (digits <= genBits)
		{
			gen_result_type rand = high >> (genBits - digits);
			return static_cast<wanted_type>(rand) * ldexp * ceil;
		}
		else
		{
			static wanted_type ldexpGen = std::ldexp(static_cast<wanted_type>(1), -static_cast<int>(genBits));

			static constexpr gen_result_type extra = digits - genBits;

			gen_result_type low = random::fixedWidthRandom< bits, Gen >(gen);

			wanted_type x =
				static_cast<wanted_type>(high) * ldexpGen
				+ static_cast<wanted_type>(low >> (genBits - extra))
				* ldexp;

			return static_cast<wanted_type>(x * ceil);
		}
	}

	template<typename T>
	class Distribution;

	template<typename T>requires((std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_floating_point_v<T>)
		class Distribution<T>
	{
	public:
		using value_type = T;
		struct Parameters
		{
			value_type min;
			value_type max;
		};
	public:
		Distribution() : m_param{ static_cast<T>(0), std::numeric_limits<T>::max() }
		{
			
		}
		Distribution(T min_, T max_) : m_param{ min_, max_ }
		{
			KTR_ASSERT((m_param.min < m_param.max), "min must be less than max");
		}
		Distribution(const Parameters& param) : m_param(param)
		{
			KTR_ASSERT((m_param.min < m_param.max), "min must be less than max");
		}
		Distribution(Parameters&& param) : m_param(std::move(param))
		{
			KTR_ASSERT((m_param.min < m_param.max), "min must be less than max");
		}
		Distribution(const Distribution&) = default;
		Distribution(Distribution&&) noexcept = default;
		~Distribution() = default;
		Distribution& operator=(const Distribution&) = default;
		Distribution& operator=(Distribution&&) noexcept = default;
		bool operator==(const Distribution&) const = default;
		bool operator!=(const Distribution&) const = default;
	public:
		template<random::IsValidBitGen Gen>
		[[nodiscard]] T operator()(Gen& gen) const
		{
			value_type range = m_param.max - m_param.min;
			return m_param.min + Distribution::_validRandom(gen, range);
		}
		[[nodiscard]] Parameters GetParam() const
		{
			return m_param;
		}
		void Param(const Parameters& param)
		{
			m_param = param;
			KTR_ASSERT(m_param.min < m_param.max, "min must be less than max");
		}
		[[nodiscard]] value_type Min() const
		{
			return m_param.min;
		}
		[[nodiscard]] value_type Max() const
		{
			return m_param.max;
		}
	private:
		template<random::IsValidBitGen Gen> requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
			[[nodiscard]] static value_type _validRandom(Gen& gen, value_type range)
		{
			return lemire_nearlydivisionless<T>(gen, range);
		}
		template<random::IsValidBitGen Gen> requires(std::is_floating_point_v<T>)
			[[nodiscard]] static value_type _validRandom(Gen& gen, value_type range)
		{
			return cross_platform_real_range<T>(gen, range);
		}
		Parameters m_param;
	};
}

namespace KTR::Random
{
	template<typename T>
	concept ValidSeqType = requires(T & seq, void* first, void* last)
	{
		seq.generate(first, last);
	};

	template<typename T>
	struct SplitMix;

	template<>
	struct SplitMix<std::uint64_t>
	{
	public:
		using result_type = std::uint64_t;
		static constexpr  result_type default_seed = 0;
	public:
		SplitMix() = default;
		SplitMix(result_type seed) : m_value(seed) {}
		template<ValidSeqType Seq>
		SplitMix(Seq& seq) : m_value(0) { seq.generate(&m_value, &m_value + 1); }
		SplitMix(const SplitMix&) = default;
		SplitMix(SplitMix&&) noexcept = default;
		~SplitMix() = default;
		SplitMix& operator=(const SplitMix&) = default;
		SplitMix& operator=(SplitMix&&) noexcept = default;
	public:
		[[nodiscard]] result_type operator()()
		{
			return rd_priv::mix64(m_value);
		}
		[[nodiscard]] static constexpr result_type(min)() noexcept {
			return std::numeric_limits<result_type>::min();
		}

		[[nodiscard]] static constexpr result_type(max)() noexcept {
			return std::numeric_limits<result_type>::max();
		}
		void seed(result_type value)
		{
			m_value = value;
		}
		template<ValidSeqType Seq>
		void sequence(Seq& seq)
		{
			seq.generate(&m_value, &m_value + 1);
		}
		void discard(result_type n)
		{
			for (result_type i = 0; i < n; ++i)
				(void)this->operator()();
		}
	private:
		result_type m_value = default_seed;
	};

	template<>
	struct SplitMix<std::uint32_t>
	{
	public:
		using result_type = std::uint32_t;
		static constexpr  result_type default_seed = 0;
	public:
		SplitMix() = default;
		SplitMix(result_type seed) : m_value(seed) {}
		template<ValidSeqType Seq>
		SplitMix(Seq& seq) : m_value(0) { seq.generate(&m_value, &m_value + 1); }
		SplitMix(const SplitMix&) = default;
		SplitMix(SplitMix&&) noexcept = default;
		~SplitMix() = default;
		SplitMix& operator=(const SplitMix&) = default;
		SplitMix& operator=(SplitMix&&) noexcept = default;
	public:
		[[nodiscard]] result_type operator()()
		{
			return rd_priv::mix32(m_value);
		}
		[[nodiscard]] static constexpr result_type(min)() noexcept {
			return std::numeric_limits<result_type>::min();
		}

		[[nodiscard]] static constexpr result_type(max)() noexcept {
			return std::numeric_limits<result_type>::max();
		}
		void seed(result_type value)
		{
			m_value = value;
		}
		template<ValidSeqType Seq>
		void sequence(Seq& seq)
		{
			seq.generate(&m_value, &m_value + 1);
		}
		void discard(result_type n)
		{
			for (result_type i = 0; i < n; ++i)
				(void)this->operator()();
		}
	private:
		result_type m_value = default_seed;
	};



	template<typename T>
	struct Xoshiro_pp;

	template<>
	struct Xoshiro_pp<std::uint64_t>
	{
	public:
		using result_type = std::uint64_t;
		static constexpr  result_type default_seed = 0;
		static constexpr std::size_t bits = sizeof(result_type) * 8;
	public:
		struct Xoshiro_pp_data
		{
			result_type s[4];
		};
	public:
		Xoshiro_pp() : m_value()
		{
			_seed(default_seed);
		}
		Xoshiro_pp(result_type seed) : m_value()
		{
			_seed(seed);
		}
		template<ValidSeqType Seq>
		Xoshiro_pp(Seq& seq) : m_value()
		{
			this->_seed<Seq>(seq);
		}
		Xoshiro_pp(const Xoshiro_pp&) = default;
		Xoshiro_pp(Xoshiro_pp&&) noexcept = default;
		~Xoshiro_pp() = default;
		Xoshiro_pp& operator=(const Xoshiro_pp&) = default;
		Xoshiro_pp& operator=(Xoshiro_pp&&) noexcept = default;
	public:
		[[nodiscard]] result_type operator()()
		{
			result_type* s = m_value.s;
			const result_type result = Xoshiro_pp::rotl(s[0] + s[3], 23) + s[0];
			const result_type t = s[1] << 17;

			s[2] ^= s[0];
			s[3] ^= s[1];
			s[1] ^= s[2];
			s[0] ^= s[3];

			s[2] ^= t;
			s[3] = Xoshiro_pp::rotl(s[3], 45);

			return result;
		}
		[[nodiscard]] static constexpr result_type(min)() noexcept {
			return std::numeric_limits<result_type>::min();
		}

		[[nodiscard]] static constexpr result_type(max)() noexcept {
			return std::numeric_limits<result_type>::max();
		}
		void seed(result_type value)
		{
			_seed(value);
		}
		template<ValidSeqType Seq>
		void sequence(Seq& seq)
		{
			this->_seed<Seq>(seq);
		}
		void discard(result_type n)
		{
			for (result_type i = 0; i < n; ++i)
				(void)this->operator()();
		}
	private:
		[[nodiscard]] static constexpr result_type rotl(const result_type x, std::int32_t k) {
			return (x << k) | (x >> (bits - k));
		}
		void _seed(result_type seed)
		{
			result_type seed_c = seed;
			m_value.s[0] = rd_priv::mix64(seed_c);
			m_value.s[1] = rd_priv::mix64(seed_c);
			m_value.s[2] = rd_priv::mix64(seed_c);
			m_value.s[3] = rd_priv::mix64(seed_c);
		}
		template<ValidSeqType Seq>
		void _seed(Seq& seq)
		{
			result_type vec[4];
			seq.generate(vec, vec + 4);
			m_value.s[0] = vec[0];
			m_value.s[1] = vec[1];
			m_value.s[2] = vec[2];
			m_value.s[3] = vec[3];
			KTR_ASSERT(((m_value.s[0] |m_value.s[1] |m_value.s[2] |m_value.s[3]) != 0), "Xoshiro state cannot be zero");
		}
		Xoshiro_pp_data m_value;
	};


	template<>
	struct Xoshiro_pp<std::uint32_t>
	{
	public:
		using result_type = std::uint32_t;
		static constexpr  result_type default_seed = 0;
		static constexpr std::size_t bits = sizeof(result_type) * 8;
	public:
		struct Xoshiro_pp_data
		{
			result_type s[4];
		};
	public:
		Xoshiro_pp() : m_value()
		{
			_seed(default_seed);
		}
		Xoshiro_pp(result_type seed) : m_value()
		{
			_seed(seed);
		}
		template<ValidSeqType Seq>
		Xoshiro_pp(Seq& seq) : m_value()
		{
			this->_seed<Seq>(seq);
		}
		Xoshiro_pp(const Xoshiro_pp&) = default;
		Xoshiro_pp(Xoshiro_pp&&) noexcept = default;
		~Xoshiro_pp() = default;
		Xoshiro_pp& operator=(const Xoshiro_pp&) = default;
		Xoshiro_pp& operator=(Xoshiro_pp&&) noexcept = default;
	public:
		[[nodiscard]] result_type operator()()
		{
			result_type* s = m_value.s;
			const result_type result = Xoshiro_pp::rotl(s[0] + s[3], 7) + s[0];
			const result_type t = s[1] << 9;

			s[2] ^= s[0];
			s[3] ^= s[1];
			s[1] ^= s[2];
			s[0] ^= s[3];

			s[2] ^= t;
			s[3] = Xoshiro_pp::rotl(s[3], 11);

			return result;
		}
		[[nodiscard]] static constexpr result_type(min)() noexcept {
			return std::numeric_limits<result_type>::min();
		}

		[[nodiscard]] static constexpr result_type(max)() noexcept {
			return std::numeric_limits<result_type>::max();
		}
		void seed(result_type value)
		{
			_seed(value);
		}
		template<ValidSeqType Seq>
		void sequence(Seq& seq)
		{
			this->_seed<Seq>(seq);
		}
		void discard(result_type n)
		{
			for (result_type i = 0; i < n; ++i)
				(void)this->operator()();
		}
	private:
		[[nodiscard]] static constexpr result_type rotl(const result_type x, std::int32_t k) {
			return (x << k) | (x >> (bits - k));
		}
		void _seed(result_type seed)
		{
			result_type seed_c = seed;
			m_value.s[0] = rd_priv::mix32(seed_c);
			m_value.s[1] = rd_priv::mix32(seed_c);
			m_value.s[2] = rd_priv::mix32(seed_c);
			m_value.s[3] = rd_priv::mix32(seed_c);
		}
		template<ValidSeqType Seq>
		void _seed(Seq& seq)
		{
			result_type vec[4];
			seq.generate(vec, vec + 4);
			m_value.s[0] = vec[0];
			m_value.s[1] = vec[1];
			m_value.s[2] = vec[2];
			m_value.s[3] = vec[3];
			KTR_ASSERT((m_value.s[0] |
				m_value.s[1] |
				m_value.s[2] |
				m_value.s[3]) != 0, "Xoshiro state cannot be zero");
		}
		Xoshiro_pp_data m_value;
	};

	template<typename T>
	struct SeedGenerator;

	template<>
	struct SeedGenerator<std::uint32_t>
	{
		using result_type = std::uint32_t;
		using internal_result_type = std::random_device::result_type;
		[[nodiscard]] static constexpr  result_type(min)()
		{
			return std::numeric_limits<result_type>::min();
		}
		[[nodiscard]] static constexpr  result_type(max)()
		{
			return std::numeric_limits<result_type>::max();
		}
		[[nodiscard]] static result_type operator()()
		{
			return static_cast<result_type>(SeedGenerator::_gen());
		}
	private:
		[[nodiscard]] static  internal_result_type _gen()
		{
			static std::random_device glob_seed_gen = std::random_device();
			return glob_seed_gen();
		}
	};

	template<>
	struct SeedGenerator<std::uint64_t>
	{
		using result_type = std::uint64_t;
		using internal_result_type = std::random_device::result_type;
		[[nodiscard]] static constexpr  result_type(min)()
		{
			return std::numeric_limits<result_type>::min();
		}
		[[nodiscard]] static constexpr  result_type(max)()
		{
			return std::numeric_limits<result_type>::max();
		}
		[[nodiscard]] static result_type operator()()
		{
			result_type result = 0;
			result = (result + static_cast<result_type>(SeedGenerator::_gen())) << sizeof(internal_result_type) * 8;
			result += static_cast<result_type>(SeedGenerator::_gen());
			return result;
		}
	private:
		[[nodiscard]] static  internal_result_type _gen()
		{
			static std::random_device glob_seed_gen = std::random_device();
			return glob_seed_gen();
		}
	};

#ifdef KTR_64B
	using default_value_type = std::uint64_t;
	using default_seed_gen_type = SeedGenerator<default_value_type>;
	using default_bit_gen_type = Xoshiro_pp<default_value_type>;
	template<typename T>
	using default_bit_dist_type = distrib::Distribution<T>;
#elifdef KTR_32B
	using default_value_type = std::uint32_t;
	using default_seed_gen_type = SeedGenerator<default_value_type>;
	using default_bit_gen_type = Xoshiro_pp<default_value_type>;
	template<typename T>
	using default_bit_dist_type = distrib::Distribution<T>;
#endif

}

#endif
