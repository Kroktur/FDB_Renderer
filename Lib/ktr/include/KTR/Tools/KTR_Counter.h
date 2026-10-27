#ifndef KTR_COUNTER_F	
#define KTR_COUNTER_F
#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include "KTR/KTR_assert.h"
namespace KTR
{
	/**
	* @brief Defines the behavior of a Counter when reaching its limits.
	*
	* Lock prevents the counter from moving outside its configured range,
	* while Loop, LoopMax and LoopMin define the behavior when crossing
	* the corresponding boundaries.
	*/
	enum class counter_state : std::uint8_t
	{
		Lock,
		Loop,
		LoopMax,
		LoopMin
	};

	/**
	* @brief A bounded arithmetic counter with configurable boundary behavior.
	*
	* The counter stores a value between a configurable minimum and maximum
	* and provides arithmetic, comparison and state-management operations.
	*
	* @tparam T Arithmetic type used to represent the counter value.
	*/
	template<typename T> requires(std::is_integral_v<T>)
	class Counter
	{
	public:
		using value_type = T;
		/**
		* @brief Defines the valid range of the counter.
		*/
		struct Parameters
		{
			value_type min;
			value_type max;
		};
		/**
		* @brief Constructs a counter using its default configuration.
		*/
		Counter();
		/**
		* @brief Constructs a counter with a custom range, value and state.
		*
		* @param param Counter boundaries.
		* @param val Initial counter value.
		* @param state Initial counter state.
		*/
		Counter(const Parameters& param,value_type val = static_cast<value_type>(0), counter_state state = counter_state::Lock);
		Counter(const Counter&) = default;
		Counter(Counter&&) noexcept= default;
		~Counter() = default;
		Counter& operator=(const Counter&) = default;
		Counter& operator=(Counter&&) noexcept = default;
	public: 
		/**
		* @brief Increments the counter.
		*
		* The resulting value is affected by the current counter state
		* and configured boundaries.
		*
		* @return Reference to this counter.
		*/
		Counter& operator++();
		/**
		* @brief Increments the counter and returns its previous value.
		*
		* @return Copy of the counter before the increment.
		*/
		[[nodiscard]] Counter operator++(int);
		/**
		* @brief Decrements the counter.
		*
		* The resulting value is affected by the current counter state
		* and configured boundaries.
		*
		* @return Reference to this counter.
		*/
		Counter& operator--();
		/**
		* @brief Decrements the counter and returns its previous value.
		*
		* @return Copy of the counter before the decrement.
		*/
		[[nodiscard]] Counter operator--(int);
		/**
		* @brief Returns the result of adding a value without modifying the counter.
		*
		* @param value Value to add.
		* @return Counter containing the resulting value.
		*/
		[[nodiscard]] Counter operator+(value_type value ) const;
		/**
		* @brief Adds a value to the counter.
		*
		* @param value Value to add.
		* @return Reference to this counter.
		*/
		Counter& operator+=(value_type value);
		/**
		* @brief Returns the result of subtracting a value without modifying the counter.
		*
		* @param value Value to subtract.
		* @return Counter containing the resulting value.
		*/
		[[nodiscard]] Counter operator-(value_type value) const;
		/**
		* @brief Subtracts a value from the counter.
		*
		* @param value Value to subtract.
		* @return Reference to this counter.
		*/
		Counter& operator-=(value_type value);
		/**
		* @brief Checks whether the current value equals the given value.
		*
		* @param value Value to compare against.
		* @return true when the counter value is equal to the given value.
		*/
		[[nodiscard]] bool operator==(value_type value) const;
		/**
		* @brief Checks whether the current value differs from the given value.
		*
		* @param value Value to compare against.
		* @return true when the counter value differs from the given value.
		*/
		[[nodiscard]] bool operator!=(value_type value) const;
		/**
		* @brief Checks whether the current value is less than the given value.
		*
		* @param value Value to compare against.
		* @return true when the counter value is less than the given value.
		*/
		[[nodiscard]] bool operator<(value_type value) const;
		/**
		* @brief Checks whether the current value is less than or equal to the given value.
		*
		* @param value Value to compare against.
		* @return true when the counter value is less than or equal to the given value.
		*/
		[[nodiscard]] bool operator<=(value_type value) const;
		/**
		 * @brief Checks whether the current value is greater than the given value.
		 *
		 * @param value Value to compare against.
		 * @return true when the counter value is greater than the given value.
		 */
		[[nodiscard]] bool operator>(value_type value) const;
		/**
		 * @brief Checks whether the current value is greater than or equal to the given value.
		 *
		 * @param value Value to compare against.
		 * @return true when the counter value is greater than or equal to the given value.
		 */
		[[nodiscard]] bool operator>=(value_type value) const;
		/**
		 * @brief Retrieves the current counter state.
		 *
		 * @return Current counter state.
		 */
		[[nodiscard]] counter_state GetState() const;
		/**
		 * @brief Changes the counter state.
		 *
		 * @param state New counter state.
		 */
		void SetState(counter_state state);
		/**
		 * @brief Assigns a new value to the counter.
		 *
		 * @param value New counter value.
		 * @return Reference to this counter.
		 */
		Counter& operator=(value_type value);
		/**
		 * @brief Sets the current counter value.
		 *
		 * @param value New counter value.
		 */
		void Set(value_type value);
		/**
		 * @brief Updates the counter boundaries.
		 *
		 * @param param New counter parameters.
		 */
		void SetParam(const Parameters& param);
		/**
		 * @brief Retrieves the current counter value.
		 *
		 * @return Current counter value.
		 */
		[[nodiscard]] value_type Get() const;

		/**
		 * @brief Retrieves the current counter boundaries.
		 *
		 * @return Current counter parameters.
		 */
		[[nodiscard]] Parameters GetParam() const;
	private:
		/**
		* @brief Adds a value while applying the counter's boundary rules.
		*
		* @param value Value to add.
		*/
		void Plus(value_type value = 1);
		/**
		 * @brief Subtracts a value while applying the counter's boundary rules.
		 *
		 * @param value Value to subtract.
		 */
		void Minus(value_type value = 1);
	
		value_type m_val;
		Parameters m_param;
		counter_state m_state;
	};


	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>::Counter(): m_val(0), m_param(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max()),m_state(counter_state::Lock)
	{
		
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>::Counter(const Parameters& param, value_type val, counter_state state) : m_param(param), m_val(val), m_state(state)
	{
		KTR_ASSERT((m_param.min < m_param.max), "min must be less than max");
	}


	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>& Counter<T>::operator++()
	{
		Plus();
		return *this;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T> Counter<T>::operator++(int)
	{
		Counter tmp = *this;
		Plus();
		return tmp;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>& Counter<T>::operator--()
	{
		Minus();
		return *this;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T> Counter<T>::operator--(int)
	{
		Counter tmp = *this;
		Minus();
		return tmp;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T> Counter<T>::operator+(value_type value) const
	{
		Counter tmp = *this;
		tmp.Plus(value);
		return tmp;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>& Counter<T>::operator+=(value_type value)
	{
		this->Plus(value);
		return *this;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T> Counter<T>::operator-(value_type value) const
	{
		Counter tmp = *this;
		tmp.Minus(value);
		return tmp;
	}

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>& Counter<T>::operator-=(value_type value)
	{
		this->Minus(value);
		return *this;
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator==(value_type value) const
	{
		return m_val == value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator!=(value_type value) const
	{
		return !(*this == value);
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator<(value_type value) const
	{
		return m_val < value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator<=(value_type value) const
	{
		return m_val <= value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator>(value_type value) const
	{
		return m_val > value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	bool Counter<T>::operator>=(value_type value) const
	{
		return m_val >= value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	counter_state Counter<T>::GetState() const
	{
		return m_state;
	}

	

	template <typename T> requires (std::is_integral_v<T>)
	void Counter<T>::SetState(counter_state state)
	{
		m_state = state;
	}

	

	template <typename T> requires (std::is_integral_v<T>)
	Counter<T>& Counter<T>::operator=(value_type value)
	{
		if (value == m_val)
			return *this;

		if (value > m_val)
			this->Plus(value - m_val);
		else
			this->Minus(m_val - value);
		return *this;
	}

	template <typename T> requires (std::is_integral_v<T>)
	void Counter<T>::Set(value_type value)
	{
		*this = value;
	}

	template <typename T> requires (std::is_integral_v<T>)
	void Counter<T>::SetParam(const Parameters& param)
	{
		m_param = param;
		KTR_ASSERT((m_param.min < m_param.max), "min must be less than max");
		m_val = std::clamp(m_val, m_param.min, m_param.max);
	}


	template <typename T> requires (std::is_integral_v<T>)
	 Counter<T>::value_type Counter<T>::Get() const
	{
		return m_val;
	}

	template <typename T> requires (std::is_integral_v<T>)
	 Counter<T>::Parameters Counter<T>::GetParam() const
	{
		return m_param;
	}

	template <typename T> requires (std::is_integral_v<T>)
	void Counter<T>::Plus(value_type size)
	{
		KTR_ASSERT(size > 0, "shoule be over 0");
		if (size <= m_param.max - m_val)
		{
			m_val += size;
			return;
		}
		if (m_state != counter_state::Loop && m_state != counter_state::LoopMax)
		{
			m_val = m_param.max;
			return;
		}
		const value_type range = m_param.max - m_param.min + 1;
		if (size <= range)
			m_val = m_param.min + size - (m_param.max - m_val) - 1;
		else
			m_val = m_param.min + ((m_val - m_param.min + size) % range);
		
	}


	template <typename T> requires (std::is_integral_v<T>)
	void Counter<T>::Minus(value_type size)
	{
		KTR_ASSERT(size > 0, "shoule be over 0");
		if (size <= m_val - m_param.min)
		{
			m_val -= size;
			return;
		}
		if (m_state != counter_state::Loop && m_state != counter_state::LoopMin)
		{
			m_val = m_param.min;
			return;
		}

		const value_type range = m_param.max - m_param.min + 1;
		if (size <= range)
			m_val = m_param.max - size + (m_val - m_param.min) + 1;
		else
			m_val = m_param.max - ((size - (m_val - m_param.min) - 1) % range);
	}

}

#endif


