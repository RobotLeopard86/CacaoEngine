#include "Cacao/Engine.hpp"
#include "Cacao/FrameProcessor.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/AudioManager.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/TickController.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "Freetype.hpp"
#include "SingletonGet.hpp"
#include "ImplAccessor.hpp"
#include "SafeGetenv.hpp"
#include "exathread.hpp"
#include "impl/PAL.hpp"
#include <memory>
#include <thread>

#ifndef CACAO_VER
#define CACAO_VER "unknown"
#endif

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#endif

#include <vector>
#include <filesystem>

namespace Cacao {
	Engine::Engine()
	  : state(State::Dead) {}

	Engine::~Engine() {
		if(state == State::Running) Quit();
		if(state == State::Ready) GfxShutdown();
		if(state == State::Alive) CoreShutdown();
	}

	CACAOST_GET(Engine)

	void Engine::CoreInit(const Engine::InitConfig& initCfg) {
		Check<BadStateException>(state == State::Dead, "Engine must be in dead state to run core initialization!");
		Check<BadValueException>(!initCfg.clientID.id.empty() && !initCfg.clientID.displayName.empty(), "Invalid client identitification given to engine core initialization!");

		//Store config
		icfg = initCfg;

		//Say hello (this will also trigger logging initialization)
		Logger::Engine(Logger::Level::Info) << "Welcome to Cacao Engine v" << CACAO_VER << "!";

		//Start thread pool
		Logger::Engine(Logger::Level::Trace) << "Starting thread pool...";
		constexpr unsigned int coreServiceCount = 3;//Tick controller, GPU manager, frame processor
		pool = exathread::Pool::Create(std::clamp<std::size_t>(std::thread::hardware_concurrency() - coreServiceCount, 2, SIZE_MAX));

		//Store thread ID
		mainThread = std::this_thread::get_id();

		/* ------------------------------------- *\
		|*      PLACEHOLDER: BUNDLE LOADING      *|
		\* ------------------------------------- */

		//Initialize audio
		Logger::Engine(Logger::Level::Trace) << "Initializing audio system...";
		AudioManager::Get().Initialize();

		//Initialize FreeType
		Logger::Engine(Logger::Level::Trace) << "Initializing FreeType instance...";
		Check<ExternalException>(FT_Init_FreeType(&freeType) == FT_Err_Ok, "Failed to initialize FreeType instance!");

		//Done with stage
		Logger::Engine(Logger::Level::Info) << "Reached target Core Initialization.";

		std::lock_guard lkg(stateMtx);
		state = State::Alive;
	}

	void Engine::GfxInit() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run graphics initialization!");

		//This will call the Window constructor which will decide what windowing provider is to be used
		//We don't care about the window yet, so we just discard the return value
		{
			Window::Get();
		}

		//In descending order of priority
		std::vector<std::string> backends;
#ifndef __APPLE__
#ifdef HAS_VK
		backends.push_back("vulkan");
#endif
#ifdef HAS_GL
		backends.push_back("opengl");
#endif
#else
#ifdef HAS_GL
		backends.push_back("opengl");
#endif
#endif
		if(!icfg.initialRequestedBackend.empty()) {
			auto it = std::find(backends.begin(), backends.end(), icfg.initialRequestedBackend);
			if(it != backends.end()) {
				backends.erase(it);
				backends.insert(backends.begin(), icfg.initialRequestedBackend);
			}
		}

		//Try to intialize backend
		bool found = false;
		std::string chosen;
		for(const std::string& backend : backends) {
			//Set module in PAL
			Logger::Engine(Logger::Level::Trace) << "Trying backend \"" << backend << "\"...";
			try {
				PAL::Get().SetModule(backend);
				if(PAL::Get().InitializeModule()) {
					found = true;
					chosen = backend;
					break;
				}
			} catch(...) {}
		}
		if(!found) {
			Logger::Engine(Logger::Level::Fatal) << "No graphics backends are available!";
			CoreShutdown();
			return;
		}
		Logger::Engine(Logger::Level::Info) << "Selected backend \"" << chosen << "\".";

		//Open window
		Logger::Engine(Logger::Level::Trace) << "Creating window...";
		Window::Get().Open(icfg.clientID.displayName, {1280, 720}, true, Window::Mode::Windowed);

		//Start the GPU manager
		Logger::Engine(Logger::Level::Trace) << "Starting GPU manager...";
		GPUManager::Get().Start();

		//Enable V-Sync by default
		GPUManager::Get().SetVSync(true);

		//Start the frame processor
		Logger::Engine(Logger::Level::Trace) << "Starting frame processor...";
		FrameProcessor::Get().Start();

		//Done with stage
		Logger::Engine(Logger::Level::Info) << "Reached target Graphics Initialization.";
		std::lock_guard lkg(stateMtx);
		state = State::Ready;
	}

	void Engine::Run() {
		Check<BadStateException>(state == State::Ready, "Engine must be in ready state to start!");
		{
			std::lock_guard lkg(stateMtx);
			state = State::Running;
		}

		Logger::Engine(Logger::Level::Info) << "Performing final initialization tasks...";

		//Start the tick controller
		Logger::Engine(Logger::Level::Trace) << "Starting tick controller...";
		TickController::Get().Start();

		Logger::Engine(Logger::Level::Info) << "Reached target Game Launch.";

		while(state == State::Running) {
			//Handle OS events
			Window::Get().HandleOSEvents();

			//Process events from the main thread tasks queue
			std::vector<std::function<void()>> tasks;
			{
				std::lock_guard lk(mttQueueMtx);
				while(!mainThreadTasksQueue.empty()) {
					tasks.push_back(mainThreadTasksQueue.front());
					mainThreadTasksQueue.pop();
				}
			}
			for(const auto& task : tasks) {
				task();
			}
		}
	}

	void Engine::Quit() {
		Check<BadStateException>(state == State::Running, "Engine must be in running state to quit!");

		//Set state
		std::lock_guard lkg(stateMtx);
		state = State::Ready;
		Logger::Engine(Logger::Level::Info) << "Engine shutdown requested!";

		//Stop the tick controller
		Logger::Engine(Logger::Level::Trace) << "Stopping tick controller...";
		TickController::Get().Stop();

		//Fire shutdown event (this (for now) will block until all consumers have responded)
		Event e("EngineShutdown");
		EventManager::Get().Dispatch(e);
	}

	void Engine::GfxShutdown() {
		Check<BadStateException>(state == State::Ready, "Engine must be in ready state to run graphics shutdown!");

		//Stop the frame processor
		Logger::Engine(Logger::Level::Trace) << "Stopping frame processor...";
		FrameProcessor::Get().Stop();

		//Stop the GPU manager
		Logger::Engine(Logger::Level::Trace) << "Stopping GPU manager...";
		GPUManager::Get().Stop();

		//Close window
		Logger::Engine(Logger::Level::Trace) << "Destroying window...";
		Window::Get().Close();

		//Unload backend
		Logger::Engine(Logger::Level::Trace) << "Terminating graphics backend...";
		PAL::Get().TerminateModule();
		IMPL(PAL).mod->Destroy();
		IMPL(PAL).mod.reset();

		std::lock_guard lkg(stateMtx);
		state = State::Alive;
	}

	void Engine::CoreShutdown() {
		Check<BadStateException>(state == State::Alive, "Engine must be in alive state to run core shutdown!");
		Logger::Engine(Logger::Level::Info) << "Shutting down engine core...";

		//Shutdown FreeType
		Logger::Engine(Logger::Level::Trace) << "Destroying FreeType instance...";
		Check<ExternalException>(FT_Done_FreeType(freeType) == FT_Err_Ok, "Failed to destroy FreeType instance!");

		//Terminate audio
		Logger::Engine(Logger::Level::Trace) << "Terminating audio system...";
		AudioManager::Get().Terminate();

		//Stop thread pool
		Logger::Engine(Logger::Level::Trace) << "Stopping thread pool...";
		pool->waitIdle();
		pool.reset();

		//Final goodbye message
		std::lock_guard lkg(stateMtx);
		state = State::Dead;
		Logger::Engine(Logger::Level::Info) << "Engine shutdown complete. Goodbye.";
	}

	const std::filesystem::path Engine::GetDataDirectory() {
		//Start with a fallback
		std::filesystem::path p = std::filesystem::current_path();

		//Platform-specific detection
#ifdef _WIN32
		PWSTR path = nullptr;
		if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
			p = std::filesystem::path(path);
			CoTaskMemFree(path);
		}
#elif defined(__APPLE__)
		std::string home = safe_getenv("HOME");
		if(!home.empty()) p = std::filesystem::path(home) / "Library" / "Application Support";
#elif defined(__linux__)
		std::string home = safe_getenv("HOME");
		if(!home.empty()) p = std::filesystem::path(home) / ".local" / "share";
#endif

		//Append client ID
		p /= icfg.clientID.id;

		//Make directory if nonexistent
		if(!std::filesystem::exists(p)) std::filesystem::create_directories(p);

		return p;
	}

	std::shared_ptr<exathread::Pool> Engine::GetThreadPool() {
		Check<exathread::Pool, BadStateException>(pool, "Engine must be in the Running state to use the thread pool!");

		return pool;
	}
}