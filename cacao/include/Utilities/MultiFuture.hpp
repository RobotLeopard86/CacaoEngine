#pragma once

#include <future>
#include <vector>

namespace Cacao {

	//An aggregator of futures that can be waited on all at once
	template<typename T>
	class MultiFuture : public std::vector<std::future<T>> {
	  public:
		//All of vector's constructors
		using std::vector<std::future<T>>::vector;

		//Delete copy constructor and copy-assignment
		MultiFuture(const MultiFuture&) = delete;
		MultiFuture& operator=(const MultiFuture&) = delete;

		//Explict default move constructor and move assignment
		MultiFuture(MultiFuture&&) = default;
		MultiFuture& operator=(MultiFuture&&) = default;

		//Wait on all futures
		void WaitAll() {
			for(const std::future<T>& fut : *this) {
				fut.wait();
			}
		}

		//How many futures are ready?
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