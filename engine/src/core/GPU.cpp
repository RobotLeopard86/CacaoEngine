#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/PAL.hpp"
#include "impl/GPUManager.hpp"
#include "PALConfigurables.hpp"
#include "SingletonGet.hpp"
#include "impl/PAL.hpp"
#include "ImplAccessor.hpp"
#include <thread>

namespace Cacao {
	GPUManager::GPUManager()
	  : running(false) {
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);
	}

	GPUManager::~GPUManager() {
		if(running) Stop();
	}

	CACAOST_GET(GPUManager)

	void GPUManager::Start() {
		Check<BadInitStateException>(!running, "The GPU manager must not be running when Start is called!");
		Check<BadStateException>(!running, "The graphics backend and window must be connected to start the GPU manager!");

		//Setup
		impl->vsreq.needChange = false;
		impl->vsreq.value = true;

		//Start runloop on background thread
		auto runloop = [this](std::stop_token stop) { impl->Runloop(stop); };
		impl->thread = std::make_unique<std::jthread>(runloop);

		running = true;
	}

	void GPUManager::Stop() {
		Check<BadInitStateException>(running, "The GPU manager must be running when Stop is called!");

		running = false;

		//Signal run loop stop
		impl->thread->request_stop();
		impl->thread->join();
	}

	//This just handles looping and lifecycle to avoid code duplication in the backend
	void GPUManager::Impl::Runloop(std::stop_token stop) {
		RunloopStart();
		while(!stop.stop_requested()) {
			//Check if we need to update V-Sync
			{
				std::lock_guard lk(vsreq.mtx);
				if(vsreq.needChange) {
					IMPL(PAL).mod->SetVSync(vsreq.value);
					vsreq.needChange = false;
				}
			}

			RunloopIteration();
		}
		RunloopStop();
	}

	std::shared_future<void> GPUManager::Submit(std::unique_ptr<CommandBuffer> cmd) {
		return impl->SubmitCmdBuffer(std::move(cmd));
	}

	void GPUManager::SetVSync(bool newState) {
		Check<BadInitStateException>(running, "Cannot set V-Sync state when the GPU manager isn't running!");
		std::lock_guard lk(impl->vsreq.mtx);
		if(impl->vsreq.value == newState) return;
		impl->vsreq.value = newState;
		impl->vsreq.needChange = true;
	}

	std::unique_ptr<CommandBuffer> CommandBuffer::Create() {
		std::unique_ptr<CommandBuffer> cb = IMPL(PAL).mod->CreateCmdBuffer();
		cb->SetupContext();
		return cb;
	}
}