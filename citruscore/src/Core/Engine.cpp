#include "Core/Engine.hpp"
#include "Core/Log.hpp"

#include <thread>
#include <chrono>

namespace Citrus {
	//Required static variable initialization
	static Engine* instance = nullptr;
	static bool instanceExists = false;

	//Singleton accessor
	Engine* Engine::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new Engine();
			instanceExists = true;
		}

		return instance;
	}

	void Engine::FixedTickHandler(boost::asio::io_context& io){
		//Run the fixed tick code
		OnFixedTick();

		//Schedule the next fixed tick
		io.post([&io]() {
			FixedTickHandler(io);	
		})
	}

	void Engine::Run(){
		//Make sure the engine will run
		run.store(true);

		//Set up the fixed tick clock
		boost::asio::io_context io;
		boost::asio::steady_timer timer(io, std::chrono::milliseconds(fixedTickRate));
		timer.async_wait([&io](boost::system::error_code ec) {
			if(!ec) FixedTickHandler(io);
		});

		//Start the fixed tick clock
		std::thread fixedTickThread([&io]() {
			io.run();
		});

		//Engine run loop
		while(run){
			//Run dynamic tick
			OnDynamicTick();
		}

		//Join the fixed tick thread
		fixedTickThread.join();
	}

}