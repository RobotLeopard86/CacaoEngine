#pragma once

#include "DllHelper.hpp"
#include "Exceptions.hpp"

#include <memory>
#include <future>
#include <concepts>
#include <type_traits>
#include <vector>
#include <functional>
#include <algorithm>

namespace Cacao {
	/**
	 * @brief Thread pool singleton
	 */
	class CACAO_API ThreadPool {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static ThreadPool& Get();

		///@cond
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
		///@endcond

		/**
		 * @brief Place a task into the queue with an expected return value
		 *
		 * @param func The task to execute
		 * @param args The arguments to the task function
		 *
		 * @return A future that will be fulfilled when the task completes
		 *
		 * @throws BadInitStateException If the pool is not running
		 */
		template<typename F, typename... Args, typename R = std::invoke_result_t<F&&, Args&&...>>
			requires std::invocable<F&&, Args&&...>
		std::shared_future<R> Exec(F func, Args... args) {
			Check<BadInitStateException>(IsRunning(), "The thread pool must be running to execute a task!");

			//Create a task and get a result future
			std::shared_ptr<std::packaged_task<R()>> task;
			if constexpr(sizeof...(args) == 0) {
				task = std::make_shared<std::packaged_task<R()>>(std::move(func));
			} else {
				//Wrap the function so it doesn't need any arguments
				auto wrapper = std::bind(std::forward<F>(func), std::forward<Args...>(args...));
				task = std::make_shared<std::packaged_task<R()>>(std::move(wrapper));
			}
			std::shared_future<R> result = task->get_future().share();

			//Use closures to capture the task so its future can be set
			ImplSubmit([task]() { (*task)(); }, false);
			return result;
		}

		/**
		 * @brief Place an IO-bound task into the queue with an expected return value
		 *
		 * @warning This function uses the dedicated IO pool, which is smaller than the general pool. Do not use this for CPU-bound tasks; only short IO-constrained tasks should be placed here.
		 *
		 * @param func The task to execute
		 * @param args The arguments to the task function
		 *
		 * @return A future that will be fulfilled when the task completes
		 *
		 * @throws BadInitStateException If the pool is not running
		 */
		template<typename F, typename... Args, typename R = std::invoke_result_t<F&&, Args&&...>>
			requires std::invocable<F&&, Args&&...> && (!std::is_same_v<R, void>)
		std::shared_future<R> ExecIOBound(F func, Args... args) {
			Check<BadInitStateException>(IsRunning(), "The thread pool must be running to execute a task!");

			//Create a task and get a result future
			std::shared_ptr<std::packaged_task<R()>> task;
			if constexpr(sizeof...(args) == 0) {
				task = std::make_shared<std::packaged_task<R()>>(std::move(func));
			} else {
				//Wrap the function so it doesn't need any arguments
				auto wrapper = std::bind(std::forward<F>(func), std::forward<Args...>(args...));
				task = std::make_shared<std::packaged_task<R()>>(std::move(wrapper));
			}
			std::shared_future<R> result = task->get_future().share();

			//Use closures to capture the task so its future can be set
			ImplSubmit([task]() { (*task)(); }, true);
			return result;
		}

		/**
		 * @brief Run a looping function on the thread pool that can be signaled to stop
		 *
		 * @note All functions will be automatically signaled to stop when the pool is
		 *
		 * @param func The function to execute
		 * @param args The arguments to the task function
		 *
		 * @return An object by which the function can be requested to stop
		 *
		 * @throws BadInitStateException If the pool is not running
		 */
		template<typename F, typename... Args>
			requires std::invocable<F&&, std::stop_token, Args&&...> && std::is_same_v<std::invoke_result_t<F&&, std::stop_token, Args&&...>, void>
		std::stop_source ExecContinuous(F func, Args... args) {
			Check<BadInitStateException>(IsRunning(), "The thread pool must be running to execute a continuous function!");

			//Create stop source
			std::stop_source& stop = stops.emplace_back();

			//Wrap the function so it doesn't need any arguments
			if constexpr(sizeof...(args) == 0) {
				auto wrapper = std::bind_front(std::forward<F>(func), stop.get_token());

				//Submit the function
				ImplSubmit([this, wrapper, &stop]() {
					//Invoke function and then remove
					wrapper();
					if(auto it = std::find(stops.begin(), stops.end(), stop); it != stops.end()) {
						stops.erase(it);
					}
				},
					false);
			} else {
				auto wrapper = std::bind_front(std::forward<F>(func), stop.get_token(), std::forward<Args...>(args...));

				//Submit the function
				ImplSubmit([this, wrapper, &stop]() {
					//Invoke function and then remove
					wrapper();
					if(auto it = std::find(stops.begin(), stops.end(), stop); it != stops.end()) {
						stops.erase(it);
					}
				},
					false);
			}

			return stop;
		}

		/**
		 * @brief Start the thread pool
		 *
		 * @throws BadInitStateException If the pool is running
		 */
		void Start();

		/**
		 * @brief Signal all continuous functions to stop and stop the pool
		 *
		 * @throws BadInitStateException If the pool is not running
		 */
		void Stop();

		/**
		 * @brief Check if the pool is running
		 *
		 * @return Whether the pool is running
		 */
		bool IsRunning() const;

		/**
		 * @brief Check how many threads are in the pool
		 *
		 * @return Number of threads in pool
		 *
		 * @throws BadInitStateException If the pool is not running
		 */
		std::size_t GetThreadCount() const;

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::vector<std::stop_source> stops;

		void ImplSubmit(std::function<void()> job, bool io);

		ThreadPool();
		~ThreadPool();
	};
}