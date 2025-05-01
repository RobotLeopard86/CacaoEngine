#pragma once

#include "DllHelper.hpp"

#include <memory>
#include <future>
#include <concepts>

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
		static ThreadPool& Get() {
			static ThreadPool _instance;
			return _instance;
		}

		///@cond
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
		///@endcond

		/**
		 * @brief Place a task into the queue with an expected return value
		 *
		 * @param task The task to execute
		 *
		 * @return A future that will be fulfilled when the task completes
		 */
		template<typename F, typename... Args, typename R = std::invoke_result_t<F&&, Args&&...>>
			requires std::invocable<F&&, Args&&...>
		std::shared_future<R> Exec(F func, Args... args) {
			//Wrap the function so it doesn't need any arguments
			auto wrapper = std::bind(std::forward<F>(func), std::forward<Args...>(args...));

			//Create a task and get a result future
			std::shared_ptr<std::packaged_task<R()>> task = std::make_shared<std::packaged_task<R()>>(std::move(wrapper));
			std::shared_future<R> result = task->get_future().share();

			//Use closures to capture the task so its future can be set
			ImplSubmit([task]() { (*task)(); });
			return result;
		}

		/**
		 * @brief Run a looping function on the thread pool that can be signaled to stop
		 *
		 * @note All functions will be automatically signaled to stop when the pool is
		 *
		 * @param task The function to execute
		 *
		 * @return An object by which the function can be requested to stop
		 */
		template<typename F, typename... Args>
			requires std::invocable<F&&, std::stop_token, Args&&...> && std::is_same_v<std::invoke_result_t<F&&, std::stop_token, Args&&...>, void>
		std::stop_source ExecContinuous(F func, Args... args) {
			//Create stop source
			std::stop_source& stop = stops.emplace_back();

			//Wrap the function so it doesn't need any arguments
			auto wrapper = std::bind_front(std::forward<F>(func), stop.get_token(), std::forward<Args...>(args...));

			//Submit the function
			ImplSubmit([this, wrapper, &stop]() {
				//Invoke function and then remove
				wrapper();
				if(auto it = std::find(stops.begin(), stops.end(), stop); it != stops.end()) {
					stops.erase(it);
				}
			});
			return stop;
		}

		/**
		 * @brief Start the thread pool
		 *
		 * @return Whether starting the pool succeeded
		 */
		bool Start();

		/**
		 * @brief Signal all continuous functions to stop and stop the pool
		 */
		void Stop();

		/**
		 * @brief Check if the pool is running
		 *
		 * @return Whether the pool is running
		 */
		bool IsRunning();

		/**
		 * @brief Check how many threads are in the pool
		 *
		 * @return Number of threads in pool
		 */
		std::size_t GetThreadCount();

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::vector<std::stop_source> stops;

		void ImplSubmit(std::function<void()> job);

		ThreadPool();
		~ThreadPool();
	};
}