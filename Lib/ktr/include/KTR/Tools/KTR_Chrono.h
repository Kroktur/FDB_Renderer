#ifndef KTR_CHRONO_F
#define KTR_CHRONO_F

#include <chrono>

namespace KTR
{
	namespace ratio
	{
		/**
		* @brief Checks whether a type can be used as a ratio type.
		*
		* A valid ratio type exposes the members required by the ratio utilities:
		* a type, a numerator and a denominator.
		*
		* @tparam T Type to validate.
		*/
		template<typename T>
		concept ValidRatioType = requires()
		{
			typename T::type;
			{ T::num };
			{ T::den };
		};
		/**
		* @brief Creates a compile-time ratio from a numerator and denominator.
		*
		* @tparam LhsT Numerator.
		* @tparam RhsT Denominator.
		*/
		template<long long LhsT, long long RhsT = 1>
		using ratio_type = std::ratio<LhsT, RhsT>;

		using seconds_type = ratio_type<1>;
		using milli_seconds_type = std::milli;
		using micro_seconds_type = std::micro;
		using nano_seconds_type = std::nano;
		using minutes_type = ratio_type<60>;
		using hours_type = ratio_type<3600>;
		using days_type = std::ratio_multiply<ratio_type<24>, hours_type>;
		using weeks_type = std::ratio_multiply<ratio_type<7>, days_type>;
		using years_type = std::ratio_multiply<ratio_type<146097, 400>, days_type>;
		using month_type = std::ratio_divide<years_type, ratio_type<12>>;
	}

	namespace duration
	{
		/**
		* @brief Creates a chrono duration with the specified representation and ratio.
		*
		* @tparam T Duration representation type.
		* @tparam Ratio Duration period.
		*/
		template<typename T, ratio::ValidRatioType Ratio> requires(std::is_arithmetic_v<T>)
			using duration_type = std::chrono::duration<T, Ratio>;

		/**
		* @brief Checks whether a type exposes the components required by a duration.
		*
		* @tparam T Type to validate.
		*/
		template<typename T>
		concept ValidDurationType = requires()
		{
			typename T::rep;
			typename T::period;
		};

		

		// Convenient aliases for the most commonly used duration units.
		template<typename T = long long> requires(std::is_arithmetic_v<T>)
			using seconds_type = duration_type<T, ratio::seconds_type>;
		template<typename T = long long> requires(std::is_arithmetic_v<T>)
			using milli_seconds_type = duration_type<T, ratio::milli_seconds_type>;
		template<typename T = long long> requires(std::is_arithmetic_v<T>)
			using micro_seconds_type = duration_type<T, ratio::micro_seconds_type>;
		template<typename T = long long> requires(std::is_arithmetic_v<T>)
			using nano_seconds_type = duration_type<T, ratio::nano_seconds_type>;

		template<typename T = int> requires(std::is_arithmetic_v<T>)
			using minutes_type = duration_type<T, ratio::minutes_type>;
		template<typename T = int> requires(std::is_arithmetic_v<T>)
			using hours_type = duration_type<T, ratio::hours_type>;
		template<typename T = int> requires(std::is_arithmetic_v<T>)
			using days_type = duration_type<T, ratio::days_type>;
		template<typename T = int> requires(std::is_arithmetic_v<T>)
			using weeks_type = duration_type<T, ratio::weeks_type>;
		template<typename T = int> requires(std::is_arithmetic_v<T>)
			using years_type = duration_type<T, ratio::years_type>;
		template<typename T = int > requires(std::is_arithmetic_v<T>)
			using month_type = duration_type<T, ratio::month_type>;

	}

	namespace time_point
	{
		template<typename Clock, duration::ValidDurationType Duration = Clock::duration>
		using time_point_type = std::chrono::time_point<Clock, Duration>;
	}

	namespace chrono
	{
		/**
		* @brief Provides common operations for chrono durations.
		*/
		struct Function
		{
			/**
			* @brief Converts a duration to another duration type.
			*
			* @tparam TargetDuration Target duration type.
			* @tparam IncomingDuration Input duration type.
			*
			* @param dur Duration to convert.
			* @return Converted duration.
			*/
			template<duration::ValidDurationType TargetDuration, duration::ValidDurationType IncomingDuration>
			[[nodiscard]] static constexpr TargetDuration DurationCast(const IncomingDuration& dur)
			{
				return std::chrono::duration_cast<TargetDuration>(dur);
			}
			/**
			 * @brief Converts a duration while explicitly specifying its representation and period.
			*
			* @tparam TargetT Target representation type.
			* @tparam TargetRatio Target duration period.
			* @tparam IncomingDuration Input duration type.
			*
			* @param dur Duration to convert.
			* @return Converted duration.
			*/
			template<typename TargetT, ratio::ValidRatioType TargetRatio, duration::ValidDurationType IncomingDuration> requires(std::is_arithmetic_v<TargetT>)
			[[nodiscard]] static constexpr duration::duration_type<TargetT,TargetRatio> DurationCast(const IncomingDuration& dur)
			{
				return std::chrono::duration_cast<duration::duration_type<TargetT, TargetRatio>>(dur);
			}
			/**
			* @brief Converts a duration while changing only its representation type.
			*
			* The period of the incoming duration is preserved.
			*
			* @tparam TargetT Target representation type.
			* @tparam IncomingDuration Input duration type.
			 *
			* @param dur Duration to convert.
			* @return Converted duration.
			*/
			template<typename TargetT, duration::ValidDurationType IncomingDuration> requires(std::is_arithmetic_v<TargetT>)
				[[nodiscard]] static constexpr duration::duration_type<TargetT, typename IncomingDuration::period> DurationCast(const IncomingDuration& dur)
			{
				return std::chrono::duration_cast<duration::duration_type<TargetT,typename IncomingDuration::period>>(dur);
			}
			/**
			* @brief Converts a duration while changing only its period.
			*
			* The representation type of the incoming duration is preserved.
			*
			* @tparam TargetRatio Target duration period.
			* @tparam IncomingDuration Input duration type.
			*
			* @param dur Duration to convert.
			* @return Converted duration.
			*/
			template<ratio::ValidRatioType TargetRatio, duration::ValidDurationType IncomingDuration>
				[[nodiscard]] static constexpr duration::duration_type<typename IncomingDuration::rep, TargetRatio> DurationCast(const IncomingDuration& dur)
			{
				return std::chrono::duration_cast<duration::duration_type<typename IncomingDuration::rep, TargetRatio>>(dur);
			}

			/**
			* @brief Retrieves the stored numeric value of a duration.
			*
			* @tparam IncomingDuration Input duration type.
			*
			* @param dur Duration whose value is retrieved.
			* @return Numeric duration value.
			*/
			template<duration::ValidDurationType IncomingDuration>
			[[nodiscard]] static constexpr IncomingDuration::rep AsValue(const IncomingDuration& dur)
			{
				return dur.count();
			}
		};
		/**
		 * @brief Identifies the native clock types supported by KTR's chrono utilities.
		*/
		enum class native_clock_type : std::uint8_t {
			system,
			steady,
			high_resolution,
			utc,
			tai,
			gps,
			file
		};

		/**
		* @brief Maps a KTR native clock identifier to its clock implementation.
		*
		* @tparam ClockT KTR clock identifier.
		*/
		template<native_clock_type ClockT>
		struct  Clock;


		/**
		* @brief Timer associated with a KTR native clock.
		*
		* @tparam ClockT Native clock identifier.
		* @tparam OutputDuration Duration returned by the timer operations.
		*/
		template<native_clock_type ClockT, duration::ValidDurationType OutputDuration>
		struct Timer;

		namespace Priv
		{
			/**
			* @brief Provides the common implementation shared by KTR clock wrappers.
			*
			* This CRTP helper connects a KTR clock identifier with its corresponding
			* std::chrono clock type.
			*
			* @tparam StdClockType Underlying std::chrono clock type.
			* @tparam ClockType KTR native clock identifier.
			*/
			template<typename StdClockType, native_clock_type ClockType>
			struct ClockCrtp
			{
				static constexpr native_clock_type clock_type_v = ClockType;
				using clock_type = StdClockType;
				using time_point_type = clock_type::time_point;
				using duration_type = clock_type::duration;
				using ratio_type = duration_type::period;
				using timer_type = Timer<clock_type_v, duration_type>;
				/**
				* @brief Retrieves the current time from the underlying clock.
				*
				* @return Current clock time point.
				*/
				[[nodiscard]] static time_point_type Now()
				{
					return clock_type::now();
				}
			};
		}



		template<>
		struct Clock<native_clock_type::system> : Priv::ClockCrtp<std::chrono::system_clock, native_clock_type::system> {};
		template<>
		struct Clock<native_clock_type::steady> : Priv::ClockCrtp<std::chrono::steady_clock, native_clock_type::steady> {};
		template<>
		struct Clock<native_clock_type::high_resolution> : Priv::ClockCrtp<std::chrono::high_resolution_clock, native_clock_type::high_resolution> {};
		template<>
		struct Clock<native_clock_type::utc> : Priv::ClockCrtp<std::chrono::utc_clock, native_clock_type::utc> {};
		template<>
		struct Clock<native_clock_type::tai> : Priv::ClockCrtp<std::chrono::tai_clock, native_clock_type::tai> {};
		template<>
		struct Clock<native_clock_type::gps> : Priv::ClockCrtp<std::chrono::gps_clock, native_clock_type::gps> {};
		template<>
		struct Clock<native_clock_type::file> : Priv::ClockCrtp<std::chrono::file_clock, native_clock_type::file> {};

		/**
		* @brief Represents the current state of a timer.
		*/
		enum class timer_state : std::uint8_t
		{
			idle,
			running,
			paused
		};
		/**
		* @brief Measures elapsed time using one of the supported native clocks.
		*
		* The timer can be started, paused, resumed, reset and restarted while
		* exposing the elapsed time in a configurable output duration.
		*
		* @tparam ClockT Clock used to measure elapsed time.
		* @tparam OutputDuration Duration type returned by the timer operations.
		*/
		template<native_clock_type ClockT, duration::ValidDurationType OutputDuration = Clock<ClockT>::duration_type>
		struct Timer
		{
		public:
			using clock_type = Clock<ClockT>;
			using time_point_type = clock_type::time_point_type;
			using duration_type = clock_type::duration_type;
			using output_duration_type = OutputDuration;

		public:
			/**
			 * @brief Creates a timer in the idle state.
			*/
			Timer() = default;
			~Timer() = default;
			Timer(const Timer&) = default;
			Timer(Timer&&) noexcept = default;
			Timer& operator=(const Timer&) = default;
			Timer& operator=(Timer&&) noexcept = default;
		public:
			/**
			* @brief Pauses the timer.
			*
			* If the timer is currently running, the elapsed time since the last
			* start or resume is accumulated and the timer enters the paused state.
			*
			* @return Elapsed time at the moment of the pause.
			*/
			output_duration_type Pause()
			{
				time_point_type now = clock_type::Now();
				if (m_state == timer_state::running)
				{
					m_acumulated += (now - m_start);
					m_state = timer_state::paused;
				}
				return Function::DurationCast<output_duration_type>(m_acumulated);
			}
			/**
			* @brief Resumes the timer.
			*
			* If the timer is not currently running, a new starting point is stored
			* and the timer enters the running state.
			*
			* @return Current elapsed time after resuming.
			*/
			output_duration_type Resume()
			{
				time_point_type now = clock_type::Now();
				if (m_state != timer_state::running)
				{
					m_start = now;
					m_state = timer_state::running;
				}
				return Function::DurationCast<output_duration_type>(ComputeNow(now));
			}
			/**
			* @brief Resets the timer to the idle state.
			*
			* The elapsed time accumulated before the reset is returned.
			*
			* @return Elapsed time immediately before the reset.
			*/
			output_duration_type Reset()
			{
				time_point_type now = clock_type::Now();
				duration_type current = this->ComputeNow(now);


				m_state = timer_state::idle;
				m_acumulated = duration_type::zero();
				return Function::DurationCast<output_duration_type>(current);
			}
			/**
			* @brief Restarts the timer from zero.
			*
			* The current elapsed time is returned before the timer is reset
			* and immediately placed into the running state.
			*
			* @return Elapsed time immediately before the restart.
			*/
			output_duration_type Restart()
			{
				time_point_type now = clock_type::Now();
				duration_type current = this->ComputeNow(now);

				m_state = timer_state::running;
				m_start = now;
				m_acumulated = duration_type::zero();

				return Function::DurationCast<output_duration_type>(current);
			}
			/**
			* @brief Retrieves the current elapsed time.
			*
			* @return Current elapsed time in the configured output duration.
			*/
			[[nodiscard]] output_duration_type GetTime() const
			{
				return Function::DurationCast<output_duration_type>(this->ComputeNow(clock_type::Now()));
			}
			/**
			* @brief Retrieves the current state of the timer.
			*
			* @return Current timer state.
			*/
			[[nodiscard]] timer_state GetState() const
			{
				return m_state;
			}
		private:
			/**
			* @brief Computes the elapsed duration at a given time point.
			*
			* The accumulated duration is always included. When the timer is
			* running, the time elapsed since the current start point is added.
			*
			* @param now Time point used as the current reference.
			* @return Elapsed duration in the clock's native duration type.
			*/
			[[nodiscard]] duration_type ComputeNow(time_point_type now) const
			{
				duration_type current = m_acumulated;
				if (m_state == timer_state::running)
					current += (now - m_start);
				return current;
			}
			time_point_type m_start = clock_type::Now();
			duration_type m_acumulated = duration_type::zero();
			timer_state m_state = timer_state::idle;

		};

	}


}


#endif
