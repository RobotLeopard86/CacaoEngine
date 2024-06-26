#include "Core/DynTickController.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Input.hpp"
#include "World/WorldManager.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "Graphics/Rendering/MeshComponent.hpp"
#include "Utilities/MultiFuture.hpp"

#include <mutex>

namespace Cacao {
	//Required static variable initialization
	DynTickController* DynTickController::instance = nullptr;
	bool DynTickController::instanceExists = false;

	//Singleton accessor
	DynTickController* DynTickController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL) {
			//Create instance
			instance = new DynTickController();
			instanceExists = true;
			instance->isRunning = false;
		}
		return instance;
	}

	void DynTickController::Start() {
		CheckException(!isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot start the already started dynamic tick controller!")
		isRunning = true;

		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(DynTickController::Run));
	}

	void DynTickController::Stop() {
		CheckException(isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot stop the unstarted dynamic tick controller!")
		//Stop run thread
		thread->request_stop();
		thread->join();

		//Delete thread object
		delete thread;
		thread = nullptr;

		isRunning = false;
	}

	void DynTickController::Run(std::stop_token stopTkn) {
		//Run while we haven't been asked to stop
		timestep = 0.0;
		while(!stopTkn.stop_requested()) {
			//Get time at tick start and calculate ideal run time
			std::chrono::steady_clock::time_point tickStart = std::chrono::steady_clock::now();
			std::chrono::steady_clock::time_point idealStopTime = tickStart + (std::chrono::milliseconds(1000) / Engine::GetInstance()->cfg.targetDynTPS);

			//Freeze input state
			Input::GetInstance()->FreezeFrameInputState();

			//Find all scripts that need to be run
			tickScriptList.clear();
			World& activeWorld = WorldManager::GetInstance()->GetActiveWorld();
			std::mutex slMutex {};
			MultiFuture<void> slFuture;
			for(int i = 0; i < activeWorld.rootEntity->GetChildrenAsList().size(); i++) {
				std::shared_ptr<Entity> ent = activeWorld.rootEntity->GetChildrenAsList()[i];
				slFuture.push_back(Engine::GetInstance()->GetThreadPool()->enqueue([this, ent, &slMutex]() {
					//Create script locator function for an entity
					auto scriptLocator = [this, &slMutex](std::shared_ptr<Entity> e) {
						//Sneaky recursive lambda trick
						auto impl = [this, &slMutex](std::shared_ptr<Entity> e, auto& implRef) mutable {
							//Stop if this component is inactive
							if(!e->IsActive()) return;

							//Check for scripts
							for(std::shared_ptr<Component>& c : e->GetComponentsAsList()) {
								if(c->GetKind() == "SCRIPT" && c->IsActive()) {
									//Add to list (once lock is available)
									slMutex.lock();
									this->tickScriptList.push_back(c);
									slMutex.unlock();
								}
							}

							//Recurse through children
							for(std::shared_ptr<Entity> child : e->GetChildrenAsList()) {
								implRef(child, implRef);
							}
						};
						return impl(e, impl);
					};

					//Execute the script locator
					scriptLocator(ent);
				}));
			}
			//Wait for work to be completed
			slFuture.WaitAll();

			//Execute scripts
			for(std::shared_ptr<Component>& s : tickScriptList) {
				Script* script = static_cast<Script*>(s.get());
				script->OnTick(timestep);
			}

			//Accumulate things to render
			tickRenderList.clear();
			std::mutex rlMutex {};
			MultiFuture<void> roFuture;
			for(int i = 0; i < activeWorld.rootEntity->GetChildrenAsList().size(); i++) {
				std::shared_ptr<Entity> ent = activeWorld.rootEntity->GetChildrenAsList()[i];
				roFuture.push_back(Engine::GetInstance()->GetThreadPool()->enqueue([this, ent, &rlMutex]() {
					//Create mesh locator function for an entity
					auto meshLocator = [this, &rlMutex](std::shared_ptr<Entity> e) {
						//Sneaky recursive lambda trick
						auto impl = [this, &rlMutex](std::shared_ptr<Entity> e, auto& implRef) mutable {
							//Stop if this component is inactive
							if(!e->IsActive()) return;

							//Check for mesh components
							for(std::shared_ptr<Component>& c : e->GetComponentsAsList()) {
								if(c->GetKind() == "MESH" && c->IsActive()) {
									//Add to list (once lock is available)
									MeshComponent* mc = std::dynamic_pointer_cast<MeshComponent>(c).get();
									rlMutex.lock();
									this->tickRenderList.push_back(RenderObject(e->GetWorldTransformMatrix(), mc->mesh, mc->mat));
									rlMutex.unlock();
								}
							}

							//Recurse through children
							for(std::shared_ptr<Entity> child : e->GetChildrenAsList()) {
								implRef(child, implRef);
							}
						};
						return impl(e, impl);
					};

					//Execute the mesh locator
					meshLocator(ent);
				}));
			}
			//Wait for work to be completed
			roFuture.WaitAll();

			//Create frame object
			std::shared_ptr<Frame> f = std::make_shared<Frame>();
			f->projection = activeWorld.cam->GetProjectionMatrix();
			f->view = activeWorld.cam->GetViewMatrix();
			f->skybox = (activeWorld.skybox.has_value() ? std::make_optional<Skybox>(Skybox(*(activeWorld.skybox.value()))) : std::nullopt);
			f->objects = tickRenderList;

			//Send frame to render controller
			RenderController::GetInstance()->EnqueueFrame(f);

			//Check elapsed time and set timestep
			std::chrono::steady_clock::time_point tickEnd = std::chrono::steady_clock::now();
			timestep = (((double)std::chrono::duration_cast<std::chrono::milliseconds>((tickEnd - tickStart) + (tickEnd < idealStopTime ? (idealStopTime - tickEnd) : std::chrono::seconds(0))).count()) / 1000);

			//If we stopped before the ideal max time, wait until that point
			//Otherwise, run the next tick immediately
			if(tickEnd < idealStopTime) std::this_thread::sleep_until(idealStopTime);
		}
	}
}
