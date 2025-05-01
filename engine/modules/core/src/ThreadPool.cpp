#include "Cacao/ThreadPool.hpp"

#include "thread_pool/thread_pool.h"

#include <thread>

namespace Cacao {
	struct ThreadPool::Impl {
		std::unique_ptr<dp::thread_pool<dp::details::default_function_type, std::jthread>> pool;
	};

	ThreadPool::ThreadPool() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ThreadPool::~ThreadPool() {
		if(IsRunning()) Stop();
	}

	void ThreadPool::ImplSubmit(std::function<void()> job) {
		impl->pool->enqueue_detach(job);
	}

	std::size_t ThreadPool::GetThreadCount() {
		return impl->pool->size();
	}
}