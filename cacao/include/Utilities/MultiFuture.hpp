#pragma once

#include <future>
#include <vector>

namespace Cacao {
	/**
	 * @brief Aggregates a list of futures into one future that can be waited on all at once
	 */
	template<typename T>
	class MultiFuture : public std::vector<std::future<T>> {
	  public:
		//All of std::vector's constructors
		using std::vector<std::future<T>>::vector;

		///@brief Copy-construction is banned
		MultiFuture(const MultiFuture&) = delete;

		///@brief Copy-assignment is banned
		MultiFuture& operator=(const MultiFuture&) = delete;

		///@cond
		MultiFuture(MultiFuture&&) = default;
		MultiFuture& operator=(MultiFuture&&) = default;
		///@endcond

		/**
		 * @brief Wait on all futures
		 *
		 * @note Blocks until all futures are completed
		 */
		void WaitAll() {
			for(const std::future<T>& fut : *this) {
				fut.wait();
			}
		}

		/**
		 * @brief Get the number of futures that are ready
		 *
		 * @return The number of ready futures
		 */
		int NumReadyFutures() {
			int retval;
			for(const std::future<T>& fut : *this) {
				//Wait for no time to get status
				if(fut.wait_for(std::chrono::duration<double>::zero()) == std::future_status::ready) retval++;
			}
			return retval;
		}
	};
}