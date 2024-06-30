#pragma once

#include <functional>
#include <future>

namespace Cacao {
	struct Task {
	  public:
		std::function<void()> func;
		std::shared_ptr<std::promise<void>> status;

		Task(const Task& other)
		  : func(other.func), status(other.status) {}
		Task(Task&& other) = delete;
		~Task() = default;

		explicit Task(std::function<void()> job)
		  : func(job) {
			status.reset(new std::promise<void>());
		}
	};
}