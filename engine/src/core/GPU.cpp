#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/MeshRenderer.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/WorldManager.hpp"
#include "PALConfigurables.hpp"
#include "SingletonGet.hpp"
#include "exathread.hpp"
#include "impl/GPUManager.hpp"
#include "impl/PAL.hpp"
#include "impl/TickController.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <numeric>
#include <thread>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/exponential.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "crossguid/guid.hpp"
#include "libcacaoasset.hpp"

namespace Cacao {
	GPUManager::GPUManager()
	  : running(false) {
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);
		impl->numFramesInFlight.store(0);
		impl->swapchainRegen.store(false);
		impl->masterFrameGenDisable.store(false);
	}

	GPUManager::~GPUManager() {
		if(running) Stop();
	}

	CACAOST_GET(GPUManager)

	void GPUManager::Start() {
		Check<BadInitStateException>(!running, "The GPU manager must not be running when Start is called!");
		Check<BadStateException>(!running, "The graphics backend and window must be connected to start the GPU manager!");

		//Setup
		impl->vsreq.needChange.store(false);
		impl->vsreq.value = true;

		//Subscribe swapchain recreation event consumer
		impl->resizeConsumer = EventConsumer([this](Event&) {
			impl->swapchainRegen.store(true);
		});
		EventManager::Get().SubscribeConsumer("WindowResize", impl->resizeConsumer);
		EventManager::Get().SubscribeConsumer("INTERNAL-RegenSwapchain", impl->resizeConsumer);

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

	float srgbChannel2Linear(float c) {
		//This is the sRGB -> linear conversion formula
		if(c <= 0.04045f)
			return c / 12.92f;
		else {
			const float a = (c + 0.055f) / 1.055f;
			return static_cast<float>(std::pow(a, 2.4));
		}
	}

	exathread::ValueTask<std::pair<std::map<float, std::vector<MeshRenderer*>>, std::map<float, std::vector<MeshRenderer*>>>> FindMeshRenderers(ActorRef actor) {
		//Inactive actor stop
		if(!actor->IsActive()) co_return {};

		//Get all meshrenderers and add them to the list
		auto meshRenderers = actor->GetComponentsFiltered([](const std::unique_ptr<Component>& component) {
			return (dynamic_cast<MeshRenderer*>(component.get()));
		}) | std::views::transform([](const std::unordered_map<std::type_index, Component*>::value_type& item) {
			return static_cast<MeshRenderer*>(item.second);
		}) | std::views::common;
		std::map<float, std::vector<MeshRenderer*>> opaque, transparent;
		for(MeshRenderer* mr : meshRenderers) {
			float distance = glm::length2(actor.GetWorld()->cam->GetPosition() - actor->GetWorldTransform().GetPosition());
			if(mr->material->GetRenderMode() != libcacaoasset::Material::RenderMode::Transparent) {
				opaque[distance].push_back(mr);
			} else {
				transparent[distance].push_back(mr);
			}
		}

		//Handle children
		if(actor->GetAllChildren().size() > 0) {
			//Find them
			exathread::MultiFuture<std::pair<std::map<float, std::vector<MeshRenderer*>>, std::map<float, std::vector<MeshRenderer*>>>> childMeshRenderersFut = Engine::Get().GetThreadPool()->batch(actor->GetAllChildren(), FindMeshRenderers);
			co_await exathread::yieldUntilComplete(childMeshRenderersFut);

			//Merge lists and return
			std::pair<std::map<float, std::vector<MeshRenderer*>>, std::map<float, std::vector<MeshRenderer*>>> result(opaque, transparent);
			for(const auto& [childOpaque, childTransparent] : childMeshRenderersFut.results()) {
				for(const auto& [distance, value] : childOpaque) {
					std::vector<std::vector<MeshRenderer*>> toMerge {result.first[distance], value};
					auto merged = toMerge | std::views::join | std::views::common;
					result.first[distance].assign(merged.begin(), merged.end());
				}
				for(const auto& [distance, value] : childTransparent) {
					std::vector<std::vector<MeshRenderer*>> toMerge {result.second[distance], value};
					auto merged = toMerge | std::views::join | std::views::common;
					result.second[distance].assign(merged.begin(), merged.end());
				}
			}
			co_return result;
		} else {
			co_return std::pair<std::map<float, std::vector<MeshRenderer*>>, std::map<float, std::vector<MeshRenderer*>>>(opaque, transparent);
		}
	}

	void GPUManager::Impl::Runloop(std::stop_token stop) {
		//Backend init
		RunloopStart();

		//Set up variables
		counter = 0;
		lastSecond = clock::now();

		//Set up skybox data
		std::optional<exathread::Future<std::shared_ptr<Shader>>> skyShaderFut;
		std::optional<exathread::Future<std::shared_ptr<Mesh>>> skyCubeFut;
		bool skyResourcesLoading = false, skyResourcesReady = false;

		//Main loop
		while(!stop.stop_requested()) {
			//Check if we need to update V-Sync
			if(vsreq.needChange) {
				IMPL(PAL).mod->SetVSync(vsreq.value);
				vsreq.needChange.store(false);
			}

			//FPS window check
			if(clock::time_point now = clock::now(); (now - lastSecond) >= 1s) {
				lastSecond = now;
				for(unsigned int i = fpsMeasures.size() - 1; i > 0; --i) fpsMeasures[i] = fpsMeasures[i - 1];
				fpsMeasures[0] = counter;
				counter = 0;
			}

			//Check if sky resources are ready
			if(!skyResourcesReady) {
				if(!skyResourcesLoading) {
					if(IMPL(ResourceManager).builtinsReady) {
						skyShaderFut = ResourceManager::Get().Load<Shader>("a:internal_skyshader");
						skyCubeFut = ResourceManager::Get().Load<Mesh>("a:builtin_cube");
						skyResourcesLoading = true;
					}
				} else {
					if(skyShaderFut.has_value() && skyShaderFut->checkStatus() == exathread::Status::Complete) {
						skyShader = *(skyShaderFut.value());
						skyShaderFut.reset();
					}
					if(skyCubeFut.has_value() && skyCubeFut->checkStatus() == exathread::Status::Complete) {
						skyCube = *(skyCubeFut.value());
						skyCubeFut.reset();
					}
					skyResourcesReady = (skyShader && skyCube);
				}
			}

			//Handle immediate execution task state
			if(UsesImmediateExecution() && immediateExecutionTask.has_value()) {
				//If it's ready, then continue and reset state, if not, abort frame
				std::future_status status = immediateExecutionTask->wait_for(std::chrono::nanoseconds(0));
				if(status == std::future_status::ready) {
					immediateExecutionTask->get();
					if(numFramesInFlight > 0) --numFramesInFlight;
					immediateExecutionTask.reset();
				} else {
					goto abort_frame;
				}
			}

			//Frame generation
			if(!Window::Get().IsMinimized() && numFramesInFlight <= MaxFramesInFlight() && skyResourcesReady && !masterFrameGenDisable) {
				//If needed, regenerate swapchain
				if(swapchainRegen.load(std::memory_order_relaxed)) {
					if(numFramesInFlight == 0) {
						swapchainRegen.store(false);
						GenSwapchain();
					} else {
						goto abort_frame;
					}
				}

				//If tick controller is running, neogtiate snapshot
				if(TickController::Get().IsRunning()) {
					//Has the request been granted?
					TickController::Impl& tcImpl = IMPL(TickController);
					if(!tcImpl.tickControllerOwns.load(std::memory_order_relaxed)) {
						//No, set request if it wasn't
						if(!tcImpl.gpuMgrWants.load(std::memory_order_acquire)) tcImpl.gpuMgrWants.store(true, std::memory_order_release);
					} else {
						//Yes, generate a frame
						FrameGen();
					}
				} else {
					FrameGen();
				}
			}

		abort_frame:
			//Run backend iteration
			std::this_thread::yield();
			RunloopIteration();
		}

		//Spawn detached workers to reset skybox resources so that we don't get stuck waiting for ourselves
		Engine::Get().GetThreadPool()->submitDetached([this]() {
			skyMat.reset();
			skyShader.reset();
			skyCube.reset();
		});
		while(skyMat || skyShader || skyCube) {
			std::this_thread::yield();
			RunloopIteration();
		}

		//Shutdown
		RunloopStop();
	}

	void GPUManager::Impl::FrameGen() {
		//Acquire active world
		std::shared_ptr<World> world = WorldManager::Get().GetActiveWorld();
		if(!world) return;

		//Setup command buffer
		std::unique_ptr<CommandBuffer> cmd = IMPL(PAL).mod->CreateCmdBuffer();
		if(!cmd->SetupContext(true)) return;

		//Set clear color from camera
		glm::vec3 linearClearColor = glm::vec3 {srgbChannel2Linear((float)world->cam->GetClearColor().r / 0xFF),
			srgbChannel2Linear((float)world->cam->GetClearColor().g / 0xFF),
			srgbChannel2Linear((float)world->cam->GetClearColor().b / 0xFF)};

		//Update engine data
		static std::string lastWorldAddr = "";
		bool diffWorld = lastWorldAddr.compare(world->GetAddress()) != 0;
		cmd->UpdateEngineData(world->cam, diffWorld);
		if(diffWorld) lastWorldAddr = world->GetAddress();

		//Start rendering and clear screen
		cmd->StartRendering(linearClearColor);

		//Find meshes to render
		std::vector<std::map<float, std::vector<MeshRenderer*>>> opaqueMerge;
		std::vector<std::map<float, std::vector<MeshRenderer*>>> transparentMerge;
		if(world->GetToplevelActors().size() > 0) {
			//Wait for meshes to be returned
			exathread::MultiFuture<std::pair<std::map<float, std::vector<MeshRenderer*>>, std::map<float, std::vector<MeshRenderer*>>>> meshesFut = Engine::Get().GetThreadPool()->batch(world->GetToplevelActors(), FindMeshRenderers);
			meshesFut.await();

			//Join final mesh list
			for(const auto& [meshesOpaque, meshesTransparent] : meshesFut.results()) {
				opaqueMerge.push_back(meshesOpaque);
				transparentMerge.push_back(meshesTransparent);
			}
		}
		auto opaqueMerged = opaqueMerge | std::views::join | std::views::common;
		auto transparentMerged = transparentMerge | std::views::join | std::views::common;
		std::map<float, std::vector<MeshRenderer*>> opaque(opaqueMerged.begin(), opaqueMerged.end());
		std::map<float, std::vector<MeshRenderer*>> transparent(transparentMerged.begin(), transparentMerged.end());

		//Run pre-opaque callbacks
		for(const xg::Guid& guid : mappings[Phase::Opaque].first) {
			callbacks[guid](cmd);
		}

		//Render opaque/cutout meshes (order is front-to-back to maximize early depth testing)
		for(auto it = opaque.begin(); it != opaque.end(); ++it) {
			for(MeshRenderer* mr : it->second) {
				cmd->DrawMesh(mr->mesh, mr->material, mr->GetOwner()->GetWorldTransform());
			}
		}

		//Run post-opaque callbacks
		for(const xg::Guid& guid : mappings[Phase::Opaque].second) {
			callbacks[guid](cmd);
		}

		//Render skybox
		if(world->skyboxTex) {
			//Skybox material setup
			if(lastKnownSkybox.compare(world->skyboxTex->GetAddress()) != 0) {
				lastKnownSkybox = world->skyboxTex->GetAddress();
				skyMat.reset();//Explicit reset ensures the material address is free
				skyMat = Material::Create(skyShader, "a:internal_skymat");
				skyMat->SetParameter("skyTex", world->skyboxTex);
			}

			//Draw the skybox mesh
			cmd->DrawMesh(skyCube, skyMat, Transform(glm::vec3 {0.0f}, world->GetSkyboxRotation(), glm::vec3 {1.0f}));
		}

		//Run pre-transparent callbacks
		for(const xg::Guid& guid : mappings[Phase::Transparent].first) {
			callbacks[guid](cmd);
		}

		//Render transparent meshes (order is back-to-front to ensure appropriate blending)
		//This is not perfect because individual mesh faces are not sorted but it's close enough
		for(auto it = transparent.rbegin(); it != transparent.rend(); ++it) {
			for(MeshRenderer* mr : it->second) {
				cmd->DrawMesh(mr->mesh, mr->material, mr->GetOwner()->GetWorldTransform());
			}
		}

		//Run post-transparent callbacks
		for(const xg::Guid& guid : mappings[Phase::Transparent].second) {
			callbacks[guid](cmd);
		}

		//End rendering
		cmd->EndRendering();

		//Allow tick controller to resume
		if(TickController::Get().IsRunning() || IMPL(TickController).tickControllerNeedsForShutdown) {
			TickController::Impl& tcImpl = IMPL(TickController);
			tcImpl.tickControllerOwns.store(true);
			tcImpl.tickControllerOwns.notify_all();
		}

		//Execute command buffer
		try {
			++numFramesInFlight;
			++counter;
			std::shared_future<void> fut = SubmitCmdBuffer(std::move(cmd));
			if(UsesImmediateExecution()) {
				immediateExecutionTask = fut;
			}
		} catch(...) {}
	}

	std::shared_future<void> GPUManager::Submit(std::unique_ptr<CommandBuffer> cmd) {
		return impl->SubmitCmdBuffer(std::move(cmd));
	}

	void GPUManager::SetVSync(bool newState) {
		Check<BadInitStateException>(running, "Cannot set V-Sync state when the GPU manager isn't running!");
		if(impl->vsreq.value == newState) return;
		impl->vsreq.value = newState;
		impl->vsreq.needChange.store(true);
	}

	bool GPUManager::IsVSynced() {
		Check<BadInitStateException>(running, "Cannot get V-Sync state when the GPU manager isn't running!");
		return impl->vsreq.value;
	}

	unsigned int GPUManager::GetCurrentFPS() {
		return std::accumulate(impl->fpsMeasures.begin(), impl->fpsMeasures.end(), 0) / FPS_AVG_WINDOW;
	}

	xg::Guid GPUManager::RegisterRenderingCallback(std::function<void(std::unique_ptr<CommandBuffer>&)> callback, Phase phase, bool runPost) {
		xg::Guid guid = xg::newGuid();
		impl->callbacks[guid] = callback;
		impl->reverseMappings[guid] = std::pair<Phase, bool>(phase, runPost);
		if(!runPost) {
			impl->mappings[phase].first.push_back(guid);
		} else {
			impl->mappings[phase].second.push_back(guid);
		}
		return guid;
	}

	void GPUManager::UnregisterRenderingCallback(const xg::Guid& callbackGUID) {
		Check<NonexistentValueException>(impl->callbacks.contains(callbackGUID), "Cannot unregister nonexistent custom rendering callback!");
		impl->callbacks.erase(callbackGUID);
		std::pair<Phase, bool> reverse = impl->reverseMappings[callbackGUID];
		std::vector<xg::Guid>* mappingsVec;
		if(!reverse.second) {
			mappingsVec = &impl->mappings[reverse.first].first;
		} else {
			mappingsVec = &impl->mappings[reverse.first].second;
		}
		mappingsVec->erase(std::find(mappingsVec->begin(), mappingsVec->end(), callbackGUID));
		impl->reverseMappings.erase(callbackGUID);
	}

	std::unique_ptr<CommandBuffer> CommandBuffer::Create() {
		std::unique_ptr<CommandBuffer> cb = IMPL(PAL).mod->CreateCmdBuffer();
		cb->SetupContext();
		return cb;
	}
}