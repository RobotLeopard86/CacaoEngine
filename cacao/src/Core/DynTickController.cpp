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

namespace Cacao {
	//Required static variable initialization
	DynTickController* DynTickController::instance = nullptr;
	bool DynTickController::instanceExists = false;

	//Singleton accessor
	DynTickController* DynTickController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new DynTickController();
			instanceExists = true;
			instance->isRunning = false;
		}
		return instance;
	}

	void DynTickController::Start() {
		CheckException(!isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot start the already started dynamic tick controller!");
		isRunning = true;

		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(DynTickController::Run));
	}

	void DynTickController::Stop() {
		CheckException(isRunning, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot stop the unstarted dynamic tick controller!");
		//Stop run thread
		thread->request_stop();
		thread->join();

		//Delete thread object
		delete thread;
		thread = nullptr;

		tickScriptList.clear();

		isRunning = false;
	}

	void DynTickController::LocateComponents(std::shared_ptr<Entity> e, std::function<void(std::shared_ptr<Component>)> maybeMatch) {
		//Stop if this component is inactive
		if(!e->IsActive()) return;

		//Check for components
		for(auto& it : e->components) {
			std::shared_ptr<Component> c = e->GetComponent<Component>(it.first);
			if(c && c->IsActive()) maybeMatch(c);
		}

		//Recurse through children
		for(std::shared_ptr<Entity> child : e->GetChildrenAsList()) {
			LocateComponents(child, maybeMatch);
		}
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
			for(std::shared_ptr<Entity> ent : activeWorld.rootEntity->GetChildrenAsList()) {
				//Execute the script locator
				LocateComponents(ent, [this](std::shared_ptr<Component> c) {
					if(c->GetKind() == "SCRIPT") {
						//Add to list
						this->tickScriptList.push_back(c);
					}
				});
			}

			//Execute scripts
			for(std::shared_ptr<Component>& s : tickScriptList) {
				Script* script = static_cast<Script*>(s.get());
				script->OnTick(timestep);
			}

			//Create frame object
			std::shared_ptr<Frame> f = std::make_shared<Frame>();
			f->projection = activeWorld.cam->GetProjectionMatrix();
			f->view = activeWorld.cam->GetViewMatrix();
			f->skybox = activeWorld.skybox;

			//Accumulate things to render
			for(std::shared_ptr<Entity> ent : activeWorld.rootEntity->GetChildrenAsList()) {
				//Execute the mesh locator
				LocateComponents(ent, [&f](std::shared_ptr<Component> c) {
					if(c->GetKind() == "MESH") {
						//Add to list
						MeshComponent* mc = std::dynamic_pointer_cast<MeshComponent>(c).get();
						f->objects.emplace_back(c->GetOwner().lock()->GetWorldTransformMatrix(), mc->mesh, *(mc->mat));
					}
				});
			}

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
