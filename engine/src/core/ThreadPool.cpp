#include "Cacao/ThreadPool.hpp"
#include "SingletonGet.hpp"

#include "thread_pool/thread_pool.h"

#include <thread>
#include <set>

namespace Cacao {
	struct ThreadPool::Impl {
		using threadpool = dp::thread_pool<dp::details::default_function_type, std::jthread>;
		std::unique_ptr<threadpool> pool;
		std::set<std::thread::id> threads;
	};

	ThreadPool::ThreadPool() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ThreadPool::~ThreadPool() {
		if(IsRunning()) Stop();
	}

	CACAOST_GET(ThreadPool)

	bool ThreadPool::IsRunning() const {
		return (bool)impl->pool;
	}

	std::size_t ThreadPool::GetThreadCount() const {
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
		//Check if this is a pool thread (if so, we just run it now to avoid clogging the pool)
		if(impl->threads.contains(std::this_thread::get_id())) {
			job();
			return;
		}

		//Run on the pool
		impl->pool->enqueue_detach(job);
	}

	void ThreadPool::MarkSelfAsPoolThread() {
		impl->threads.insert(std::this_thread::get_id());
	}
}