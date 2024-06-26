#pragma once

#include "Events/EventSystem.hpp"
#include "Utilities/Task.hpp"

#include <future>
#include <memory>

namespace Cacao {
	void EnqueueGLJob(Task& task);

	inline std::shared_future<void> InvokeGL(std::function<void()> job) {
		Task glJob(job);
		EnqueueGLJob(glJob);
		return glJob.status->get_future().share();
	}

}