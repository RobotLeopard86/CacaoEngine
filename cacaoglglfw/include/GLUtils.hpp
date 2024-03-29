#pragma once

#include "Events/EventSystem.hpp"

#include <future>
#include <memory>

namespace Cacao {
	struct GLJob {
	public:
		std::function<void()> func;
		std::shared_ptr<std::promise<void>> status;

		//Need to implement a copy-constructor
		GLJob(const GLJob& other)
			: func(other.func), status(other.status) {}
		GLJob(GLJob&& other) = delete;
		~GLJob() = default;

		explicit GLJob(std::function<void()> task)
			: func(task) {
			status.reset(new std::promise<void>());
		}
	};

	inline std::shared_future<void> InvokeGL(std::function<void()> job){
		GLJob glJob(job);
		DataEvent<GLJob&> event("OpenGL", glJob);
		EventManager::GetInstance()->Dispatch(event);
		return glJob.status->get_future().share();
	}
}