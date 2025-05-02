#include "Cacao/ThreadPool.hpp"

#include "thread_pool/thread_pool.h"

#include <thread>

namespace Cacao {
	struct ThreadPool::Impl {
		using threadpool = dp::thread_pool<dp::details::default_function_type, std::jthread>;
		std::unique_ptr<threadpool> pool;
	};

	ThreadPool::ThreadPool() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ThreadPool::~ThreadPool() {
		if(IsRunning()) Stop();
	}

	bool ThreadPool::IsRunning() {
		return (bool)impl->pool;
	}

	std::size_t ThreadPool::GetThreadCount() {
		Check<BadInitStateException>(IsRunning(), "The thread pool must be running to check the thread count!");
		return impl->pool->size();
	}

	void ThreadPool::Start() {
		Check<BadInitStateException>(!IsRunning(), "The thread pool must be not running when Start is called!");
		impl->pool.reset(new Impl::threadpool());
	}

	void ThreadPool::Stop() {
		Check<BadInitStateException>(IsRunning(), "The thread pool must be running when Stop is called!");
		impl->pool.reset(nullptr);
	}

	void ThreadPool::ImplSubmit(std::function<void()> job) {
		impl->pool->enqueue_detach(job);
	}
}