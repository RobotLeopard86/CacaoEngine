#pragma once

#include "Cacao/Identity.hpp"

namespace Cacao::Loader {
	//Identify your game here
	//DO NOT PERFORM INITIALIZATION TASKS HERE
	//Invoked before engine startup - no systems available
	const ClientIdentity SelfIdentify();

	//Do any final pre-gameloop setup here
	//Invoked before Engine::Run is called, so some systems may not be running quite yet
	void LaunchHook();

	//Do custom teardown here
	//Invoked after engine run loop has stopped, but before systems termination
	void TerminateHook();
}