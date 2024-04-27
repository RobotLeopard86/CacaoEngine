#pragma once

#include "Events/EventSystem.hpp"

#include <future>
#include <memory>

namespace Cacao {
	struct GLJob {
	  public:
		std::function<void()> func;
		std::shared_ptr<std::promise<void>> status;

		GLJob(const GLJob& other)
		  : func(other.func), status(other.status) {}
		GLJob(GLJob&& other) = delete;
		~GLJob() = default;

		explicit GLJob(std::function<void()> task)
		  : func(task) {
			status.reset(new std::promise<void>());
		}
	};

	void EnqueueGLJob(GLJob& job);

	inline std::shared_future<void> InvokeGL(std::function<void()> job) {
		GLJob glJob(job);
		EnqueueGLJob(glJob);
		return glJob.status->get_future().share();
	}

}