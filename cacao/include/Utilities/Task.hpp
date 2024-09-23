#pragma once

#include <functional>
#include <future>

namespace Cacao {
	/**
	 * @brief Internal encapsulation of a std::promise and an associated promise
	 */
	struct Task {
	  public:
		std::function<void()> func;				   ///<The function to run
		std::shared_ptr<std::promise<void>> status;///<The status promise

		/**
		 * @brief Copy-construct a task
		 * @details The new task shares the function and status of the copied-from task
		 *
		 * @param other The task to copy from
		 */
		Task(const Task& other)
		  : func(other.func), status(other.status) {}

		///@brief Move-assignment is banned
		Task(Task&& other) = delete;

		///@brief Default destructor
		~Task() = default;

		/**
		 * @brief Create a task and a new status promise
		 */
		explicit Task(std::function<void()> job)
		  : func(job) {
			status.reset(new std::promise<void>());
		}
	};
}