/**
 * @file 	high_resolution_sleep.hpp
 * @brief 	high_resolution_sleep.hpp defines a number of cross-platform utility functions for sleeping for
 * 			millisecond and microsecond durations.
 * @details	The file provides cross platform implementations for sleeping for millisecond
 * 			and microsecond durations, as well as querying the current time in microseconds.
 * 			On Windows platforms the sleep functions have been optimized to provide maximum
 * 			resolution in sleeping: the sleep_ms function uses waitable timers to attain
 * 			slightly better resolution than sleep at little performance cost. The sleep_us
 * 			function uses a combination of sleeping and busy waiting to attain the minimal
 * 			error possible, at the cost of performance, thus it is advisable to use sleep_ms
 * 			where possible on Windows platforms
 * @author 	James Horner (James.Horner@nrc-cnrc.gc.ca or jwehorner@gmail.com)
 * @date 	2023-06-13
 */

#ifndef SLEEP_HPP
#define SLEEP_HPP

//C++ Standard Library Headers
#include <cstdint>

//Platform Dependant System Libraries
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#else /* UNIX */
#include <time.h>
#include <errno.h>

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif /* __APPLE__ */
#endif /* _WIN32 */


namespace high_resolution_sleep {
/**************************************************************************************************/
/* UNIX Implementations			 																  */
/**************************************************************************************************/
#ifndef _WIN32
	/**
	 * @brief	Function sleep_ms sleeps for the specified number of milliseconds.
	 * @param	ms	uint32_t number of milliseconds to sleep for.
	 */
	const void sleep_ms(const uint32_t ms) {
		struct timespec ts;
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = ms % 1000 * 1000000;

		while(nanosleep(&ts, &ts) == -1 && errno == EINTR);
	}

	/**
	 * @brief	Function sleep_us sleeps for the specified number of microseconds.
	 * @param	us	uint32_t number of microseconds to sleep for.
	 */
	const void sleep_us(const uint32_t us) {
		struct timespec ts;
		ts.tv_sec = us / 1000000;
		ts.tv_nsec = us % 1000000 * 1000;

		while(nanosleep(&ts, &ts) == -1 && errno == EINTR);
	}

	/**
	 * 	@brief		Function sleep_ms_corrected sleeps for the specified number of milliseconds minus
	 * 				the provided schedule slip, so that the mean sleep time converges onto the desired period.
	 *	@param		ms			uint32_t number of milliseconds to sleep for.
	 *	@param		error_us 	int64_t number of microseconds of error accumulated by sleeping.
	 *	@details	This function is intended to be used in the included kind of error tracking pattern,
	 *				where the schedule slip is tracked with each iteration, so when the slip gets too large
	 *				a sleep is skipped and the schedule catches back up.
	 *	@code 		{.cpp}
	 *				int64_t start_time_us, error_us = 0;
	 *				uint32_t duration_ms = 1;
	 *				while(CONDITION) {
	 *					start_time_us = sleep::now_us();
	 *					sleep::sleep_ms_corrected(duration_ms, error_us);
	 *					error_us += ((int64_t)sleep::now_us() - start_time_us) - (duration_ms * 1'000);
	 *				}
	 * 	@endcode
	 */
	const void sleep_ms_corrected(const uint32_t ms, const int64_t error_us) {
		//If the error is greater than or equal to the requested sleep duration, skip the sleep.
		int32_t adjusted_sleep_ms = ms - (error_us / 1'000);
		if(adjusted_sleep_ms > 0) {
			sleep_ms(adjusted_sleep_ms);
		}
		return;
	}

/**************************************************************************************************/
/* Regular UNIX Implementations		 															  */
/**************************************************************************************************/
#ifndef __APPLE__
	/**
	 * @brief	Function now_us gets the current system time in microseconds.
	 * @return	uint64_t current system time in microseconds.
	 */
	const uint64_t now_us() {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return static_cast<uint64_t>(now.tv_sec) * 1000000 + now.tv_nsec / 1000;
	}

	/**
	 * @brief	Function now_ns gets the current system time in nanoseconds.
	 * @return	uint64_t current system time in nanoseconds.
	 */
	const uint64_t now_ns() {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return static_cast<uint64_t>(now.tv_sec) * 1'000'000'000 + now.tv_nsec;
	}

#else  /* APPLE */
	/**
	 * @brief	Function now_us gets the current system time in microseconds.
	 * @return	uint64_t current system time in microseconds.
	 */
	const uint64_t now_us() {
		clock_serv_t cs;
		mach_timespec_t ts;

		host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cs);
		clock_get_time(cs, &ts);
		mach_port_deallocate(mach_task_self(), cs);

		return static_cast<uint64_t>(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
	}

	/**
	 * @brief	Function now_ns gets the current system time in nanoseconds.
	 * @return	uint64_t current system time in nanoseconds.
	 */
	const uint64_t now_ns() {
		clock_serv_t cs;
		mach_timespec_t ts;

		host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cs);
		clock_get_time(cs, &ts);
		mach_port_deallocate(mach_task_self(), cs);

		return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000 + ts.tv_nsec;
	}
#endif /* __APPLE__ */
#endif /* _WIN32 */

/**************************************************************************************************/
/* Windows Implementations		 																  */
/**************************************************************************************************/
#ifdef _WIN32


	/**************************************************************************************************/
	/* Static Variables and Functions 																  */
	/**************************************************************************************************/

	///Estimated minimum number of microseconds that a process can accurately sleep for using Windows timer objects.
	const static uint64_t min_sleep_time_us = 10'000;
	///Portion of the remaining count that should be slept for if the minimum sleep threshold is met.
	const static float remaining_count_sleep_percent = 0.75f;

	/**
	 * @brief	Function GetPerfFrequency gets the frequency of the Windows performance counter.
	 * @return	uint64_t frequency of the performance counter.
	 */
	const inline uint64_t GetPerfFrequency() {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return freq.QuadPart;
	}

	/**
	 * @brief	Function GetPerfCounter gets the current value of the Windows performance counter.
	 * @return	uint64_t current value of the performance counter.
	 */
	const inline uint64_t GetPerfCounter() {
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}


	/**************************************************************************************************/
	/* Non-Static Variables and Functions															  */
	/**************************************************************************************************/
	///Flag for if the Windows static variables have been initialised.
	static bool windows_timers_initialised = false;
	///Result of the timeBeginPeriod call to set the minimum Windows timer object resolution.
	static MMRESULT begin_period_result;
	///Number of cycles per second of the Windows performance counter.
	static uint64_t cycles_per_s;
	///Number of cycles per millisecond of the Windows performance counter.
	static uint64_t cycles_per_ms;
	///Number of cycles per microsecond of the Windows performance counter.
	static uint64_t cycles_per_us;
	///Number of cycles of the Windows performance counter that a Windows timer objects can reliably sleep for.
	static uint64_t min_sleep_time_cycles;

	/**
	 * @brief	Function initialise_windows_timers initialises the static variables for using Windows
	 * 			timer objects for sleeping.
	 */
	const void initialise_windows_timers() {
		//Try to set the minimum Windows timer object resolution and store the result.
		begin_period_result = timeBeginPeriod(1);

		//Get the frequency of the Windows performance counter.
		cycles_per_s = GetPerfFrequency();
		//Calculate the number of cycles of the Windows performance counter per millisecond.
		cycles_per_ms = (uint64_t)cycles_per_s / 1'000.0;
		//Calculate the number of cycles of the Windows performance counter per microsecond.
		cycles_per_us = (uint64_t)cycles_per_ms / 1'000.0;

		//Calculate the minimum sleep time in number of cycles of the Windows performance counter.
		min_sleep_time_cycles = min_sleep_time_us * cycles_per_us;

		//Confirm that the timer variables have been initialised if there was no error in setting the timer resolution.
		windows_timers_initialised = (begin_period_result == TIMERR_NOERROR);
	}

	/**
	 * @brief	Function now_us gets the current system time in microseconds.
	 * @return	uint64_t current system time in microseconds.
	 */
	const uint64_t now_us() {
		//Initialise the Windows timer object variables if they haven't been already.
		if(!windows_timers_initialised) initialise_windows_timers();

		//Return the current Windows performance counter value converted to microseconds.
		return static_cast<uint64_t>(static_cast<double>(GetPerfCounter()) * 1'000'000 / cycles_per_s);
	}

	/**
	 * @brief	Function now_ns gets the current system time in nanoseconds.
	 * @return	uint64_t current system time in nanoseconds.
	 */
	const uint64_t now_ns() {
		//Initialise the Windows timer object variables if they haven't been already.
		if(!windows_timers_initialised) initialise_windows_timers();

		//Return the current Windows performance counter value converted to nanoseconds.
		return static_cast<uint64_t>(static_cast<double>(GetPerfCounter()) * 1'000'000'000 / cycles_per_s);
	}

	/**
	 * @brief	Function sleep_ms sleeps for the specified number of milliseconds.
	 * @param	ms	uint32_t number of milliseconds to sleep for.
	 */
	const void sleep_ms(const uint32_t ms) {
		//Initialise the Windows timer object variables if they haven't been already.
		if(!windows_timers_initialised) initialise_windows_timers();

		//Convert the number of milliseconds to 100s of nanoseconds for SetWaitableTimer.
		LARGE_INTEGER ft;
		ft.QuadPart = -static_cast<int64_t>(ms * 10'000);

		//Create a Windows timer object to wait on.
		HANDLE timer = CreateWaitableTimerA(NULL, TRUE, NULL);
		//Set the timeout on the timer to the number of nanoseconds calculated earlier.
		SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
		//Wait for the timer to expire.
		WaitForSingleObject(timer, INFINITE);
		//Close the timer once we're done with it.
		CloseHandle(timer);
	}

	/**
	 * 	@brief		Function sleep_ms_corrected sleeps for the specified number of milliseconds minus
	 * 				the provided schedule slip, so that the mean sleep time converges onto the desired period.
	 *	@param		ms			uint32_t number of milliseconds to sleep for.
	 *	@param		error_us 	int64_t number of microseconds of error accumulated by sleeping.
	 *	@details	This function is intended to be used in the included kind of error tracking pattern,
	 *				where the schedule slip is tracked with each iteration, so when the slip gets too large
	 *				a sleep is skipped and the schedule catches back up.
	 *	@code 		{.cpp}
	 *				int64_t start_time_us, error_us = 0;
	 *				uint32_t duration_ms = 1;
	 *				while(CONDITION) {
	 *					start_time_us = sleep::now_us();
	 *					sleep::sleep_ms_corrected(duration_ms, error_us);
	 *					error_us += ((int64_t)sleep::now_us() - start_time_us) - (duration_ms * 1'000);
	 *				}
	 * 	@endcode
	 */
	const void sleep_ms_corrected(const uint32_t ms, const int64_t error_us) {
		//If the error is greater than or equal to the requested sleep duration, skip the sleep.
		int32_t adjusted_sleep_ms = ms - (error_us / 1'000);
		if(adjusted_sleep_ms > 0) {
			sleep_ms(adjusted_sleep_ms);
		}
		return;
	}

	/**
	 * @brief	Function sleep_us sleeps for the specified number of microseconds.
	 * @param	us	uint32_t number of microseconds to sleep for.
	 */
	const void sleep_us(const uint32_t us) {
		//Initialise the Windows timer object variables if they haven't been already.
		if(!windows_timers_initialised) initialise_windows_timers();

		//Calculate at what performance counter value we should stop sleeping.
		uint64_t end_counter = GetPerfCounter() + us * cycles_per_us;

		//If the end counter is greater than the value of the performance counter.
		int64_t remaining_count;
		if(end_counter > GetPerfCounter()) {
			//Calculate how many counts are left.
			remaining_count = end_counter - GetPerfCounter();
		} else {
			//Else we have reached the end already.
			return;
		}

		//While there are still cycles, we should wait.
		while(remaining_count > 0) {
			//If the remaining counter is greater than the sleep threshold,
			if(remaining_count > min_sleep_time_cycles) {
				//Try to sleep for a portion of the remaining count.
				sleep_ms((uint32_t)((remaining_count * remaining_count_sleep_percent) / cycles_per_ms));
			}
			//Decrement the number of counts left.
			remaining_count = end_counter - GetPerfCounter();
		}
	}
#endif//_WIN32
}

#endif /* SLEEP_HPP */