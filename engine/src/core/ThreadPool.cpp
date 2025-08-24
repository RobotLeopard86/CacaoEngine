#include "Cacao/ThreadPool.hpp"
#include "Cacao/Engine.hpp"
#include "SingletonGet.hpp"

#include "thread_pool/thread_pool.h"

#include <thread>

namespace Cacao {
	struct ThreadPool::Impl {
		using threadpool = dp::thread_pool<dp::details::default_function_type, std::jthread>;
		std::unique_ptr<threadpool> pool;
		std::unique_ptr<threadpool> iopool;
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
		return impl->pool->size() + impl->iopool->size();
	}

	void ThreadPool::Start() {
		Check<BadInitStateException>(!IsRunning(), "The thread pool must be not running when Start is called!");
		int ioThreads = Engine::Get().GetInitConfig().ioPoolThreads;
		impl->pool.reset(new Impl::threadpool(std::thread::hardware_concurrency() - ioThreads));
		impl->iopool.reset(new Impl::threadpool(ioThreads));
	}

	void ThreadPool::Stop() {
		Check<BadInitStateException>(IsRunning(), "The thread pool must be running when Stop is called!");
		impl->pool.reset(nullptr);
		impl->iopool.reset(nullptr);
	}

	void ThreadPool::ImplSubmit(std::function<void()> job, bool io) {
		//Run on the appropriate pool
		if(io)
			impl->iopool->enqueue_detach(job);
		else
			impl->pool->enqueue_detach(job);
	}
}