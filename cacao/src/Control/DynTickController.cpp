#include "Control/DynTickController.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Utilities/MiscUtils.hpp"
#include "World/WorldManager.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "Graphics/Rendering/MeshComponent.hpp"

namespace Cacao {
	//Required static variable initialization
	DynTickController* DynTickController::instance = nullptr;
	bool DynTickController::instanceExists = false;

	//Singleton accessor
	DynTickController* DynTickController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new DynTickController();
			instanceExists = true;
			instance->isRunning = false;
		}
		return instance;
	}

	void DynTickController::Start(){
		if(isRunning) {
			Logging::EngineLog("Cannot start the already started dynamic tick controller!", LogLevel::Error);
			return;
		}
		isRunning = true;
		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(DynTickController::Run));
	}

	void DynTickController::Stop(){
		if(!isRunning) {
			Logging::EngineLog("Cannot stop the not started dynamic tick controller!", LogLevel::Error);
			return;
		}
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
		while(!stopTkn.stop_requested()){
			//Get time at tick start and calculate ideal run time
			std::chrono::steady_clock::time_point tickStart = std::chrono::steady_clock::now();
			std::chrono::steady_clock::time_point idealStopTime = tickStart + (std::chrono::milliseconds(1000)/Engine::GetInstance()->cfg.targetDynTPS);
			
			//Find all scripts that need to be run
			tickScriptList.clear();
			World& activeWorld = WorldManager::GetInstance()->GetWorld(WorldManager::GetInstance()->GetActiveWorld());
			BS::multi_future<void> fsFuture = Engine::GetInstance()->GetThreadPool().submit_loop<unsigned int>(0, activeWorld.worldTree.children.size(), [this, activeWorld](unsigned int index) {
				//Create script locator function for an entity
				auto scriptLocator = [this](TreeItem<Entity>& e) {
					//Sneaky recursive lambda trick
					auto impl = [this](TreeItem<Entity>& e, auto& implRef) mutable {
						//Stop if this component is inactive
						if(!e.val().active) return;

						//Check for script components
						for(std::shared_ptr<Component>& c : e.val().components){
							if(c->GetKind() == "SCRIPT" && c->IsActive()) {
								//Add to list
								this->tickScriptList.push_back(c);
							}
						}

						//Recurse through children
						for(TreeItem<Entity>& child : e.children){
							implRef(child, implRef);
						}
					};
					return impl(e, impl);
				};

				//Execute the script locator
				TreeItem<Entity>& ent = const_cast<TreeItem<Entity>&>(activeWorld.worldTree.children.at(index)); //We have to use const_cast because at() returns a const reference
				scriptLocator(ent);
			});
			//Wait for work to be completed
			fsFuture.wait();

			//Execute scripts
			for(std::shared_ptr<Component>& s : tickScriptList){
				Script* script = static_cast<Script*>(s.get());
				script->OnTick(timestep);
			}

			//Accumulate things to render
			tickRenderList.clear();
			BS::multi_future<void> roFuture = Engine::GetInstance()->GetThreadPool().submit_loop<unsigned int>(0, activeWorld.worldTree.children.size(), [this, activeWorld](unsigned int index) {
				//Create script locator function for an entity
				auto renderLocator = [this](TreeItem<Entity>& e) {
					//Sneaky recursive lambda trick
					auto impl = [this](TreeItem<Entity>& e, auto& implRef) mutable {
						//Stop if this component is inactive
						if(!e.val().active) return;

						//Check for mesh components
						for(std::shared_ptr<Component>& c : e.val().components){
							if(c->GetKind() == "MESH" && c->IsActive()) {
								//Add to list
								MeshComponent* mc = static_cast<MeshComponent*>(c.get());
								this->tickRenderList.push_back(RenderObject(e.val().transform.GetTransformationMatrix(), mc->mesh, mc->mat));
							}
						}

						//Recurse through children
						for(TreeItem<Entity>& child : e.children){
							implRef(child, implRef);
						}
					};
					return impl(e, impl);
				};

				//Execute the mesh locator
				TreeItem<Entity>& ent = const_cast<TreeItem<Entity>&>(activeWorld.worldTree.children.at(index)); //We have to use const_cast because at() returns a const reference
				renderLocator(ent);
			});
			//Wait for work to be completed
			roFuture.wait();

			//Create frame
			Frame f;
			f.projection = activeWorld.cam->GetProjectionMatrix();
			f.view = activeWorld.cam->GetViewMatrix();
			f.skybox = (activeWorld.skybox.has_value() ? std::make_optional<Skybox>(*(activeWorld.skybox.value())) : std::nullopt);
			f.objects = tickRenderList;

			//Send frame to render controller
			RenderController::GetInstance()->EnqueueFrame(f);

			//Check elapsed time and set timestep
			std::chrono::steady_clock::time_point tickEnd = std::chrono::steady_clock::now();
			timestep = (((double)std::chrono::duration_cast<std::chrono::milliseconds>((tickEnd - tickStart) + (tickEnd < idealStopTime ? (idealStopTime - tickEnd) : std::chrono::seconds(0))).count())/1000);

			//If we stopped before the ideal max time, wait until that point
			//Otherwise, run the next tick immediately
			if(tickEnd < idealStopTime) std::this_thread::sleep_until(idealStopTime);
		}
	}
}
