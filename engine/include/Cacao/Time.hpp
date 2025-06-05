#pragma once

#include <chrono>
using namespace std::chrono_literals;

namespace Cacao {
	namespace time {
		///@brief Single-precision floating-point hours duration
		using fhours = std::chrono::duration<float, std::ratio<3600>>;

		///@brief Single-precision floating-point minutes duration
		using fminutes = std::chrono::duration<float, std::ratio<60>>;

		///@brief Single-precision floating-point seconds duration
		using fseconds = std::chrono::duration<float>;

		///@brief Single-precision floating-point milliseconds duration
		using fmilliseconds = std::chrono::duration<float, std::milli>;

		///@brief Single-precision floating-point microseconds duration
		using fmicroseconds = std::chrono::duration<float, std::micro>;

		///@brief Single-precision floating-point nanoseconds duration
		using fnanoseconds = std::chrono::duration<float, std::nano>;

		///@brief Double-precision floating-point hours duration
		using dhours = std::chrono::duration<long double, std::ratio<3600>>;

		///@brief Double-precision floating-point minutes duration
		using dminutes = std::chrono::duration<long double, std::ratio<60>>;

		///@brief Double-precision floating-point seconds duration
		using dseconds = std::chrono::duration<long double>;

		///@brief Double-precision floating-point milliseconds duration
		using dmilliseconds = std::chrono::duration<long double, std::milli>;

		///@brief Double-precision floating-point microseconds duration
		using dmicroseconds = std::chrono::duration<long double, std::micro>;

		///@brief Double-precision floating-point nanoseconds duration
		using dnanoseconds = std::chrono::duration<long double, std::nano>;

		///@brief Time point using single-precision floating-point times and std::chrono::steady_clock
		using ftime_point = std::chrono::time_point<std::chrono::steady_clock, fnanoseconds>;

		///@brief Time point using double-precision floating-point times and std::chrono::steady_clock
		using dtime_point = std::chrono::time_point<std::chrono::steady_clock, dnanoseconds>;
	}

	///@cond
	constexpr time::fhours operator""_fh(long double h) {
		return time::fhours(h);
	}

	constexpr time::fminutes operator""_fmi(long double m) {
		return time::fminutes(m);
	}

	constexpr time::fseconds operator""_fs(long double s) {
		return time::fseconds(s);
	}

	constexpr time::fmilliseconds operator""_fms(long double ms) {
		return time::fmilliseconds(ms);
	}

	constexpr time::fmicroseconds operator""_fus(long double us) {
		return time::fmicroseconds(us);
	}

	constexpr time::fnanoseconds operator""_fns(long double ns) {
		return time::fnanoseconds(ns);
	}

	constexpr time::dhours operator""_dh(long double h) {
		return time::dhours(h);
	}

	constexpr time::dminutes operator""_dmi(long double m) {
		return time::dminutes(m);
	}

	constexpr time::dseconds operator""_ds(long double s) {
		return time::dseconds(s);
	}

	constexpr time::dmilliseconds operator""_dms(long double ms) {
		return time::dmilliseconds(ms);
	}

	constexpr time::dmicroseconds operator""_dus(long double us) {
		return time::dmicroseconds(us);
	}

	constexpr time::dnanoseconds operator""_dns(long double ns) {
		return time::dnanoseconds(ns);
	}

	constexpr time::fhours operator""_fh(unsigned long long h) {
		return time::fhours(h);
	}

	constexpr time::fminutes operator""_fmi(unsigned long long m) {
		return time::fminutes(m);
	}

	constexpr time::fseconds operator""_fs(unsigned long long s) {
		return time::fseconds(s);
	}

	constexpr time::fmilliseconds operator""_fms(unsigned long long ms) {
		return time::fmilliseconds(ms);
	}

	constexpr time::fmicroseconds operator""_fus(unsigned long long us) {
		return time::fmicroseconds(us);
	}

	constexpr time::fnanoseconds operator""_fns(unsigned long long ns) {
		return time::fnanoseconds(ns);
	}

	constexpr time::dhours operator""_dh(unsigned long long h) {
		return time::dhours(h);
	}

	constexpr time::dminutes operator""_dmi(unsigned long long m) {
		return time::dminutes(m);
	}

	constexpr time::dseconds operator""_ds(unsigned long long s) {
		return time::dseconds(s);
	}

	constexpr time::dmilliseconds operator""_dms(unsigned long long ms) {
		return time::dmilliseconds(ms);
	}

	constexpr time::dmicroseconds operator""_dus(unsigned long long us) {
		return time::dmicroseconds(us);
	}

	constexpr time::dnanoseconds operator""_dns(unsigned long long ns) {
		return time::dnanoseconds(ns);
	}
	///@endcond
}