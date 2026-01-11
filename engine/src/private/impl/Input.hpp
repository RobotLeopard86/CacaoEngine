#pragma once

#include "Cacao/Input.hpp"
#include "Cacao/EventConsumer.hpp"

#include <unordered_map>

namespace Cacao {
	struct Input::Impl {
		//Cursor positions
		glm::dvec2 cursorPosLive;  //Live updated
		glm::dvec2 cursorPosFrozen;//Copied from temporary by FreezeInputState

		//Keyboard & mouse data
		//Live and frozen have same connotations as above
		std::unordered_map<unsigned int, bool> keysLive, keysFrozen, mouseLive, mouseFrozen;

		//Update functions
		EventConsumer mouseMove;
		EventConsumer mouseButtonPress;
		EventConsumer mouseButtonRelease;
		EventConsumer keyUp;
		EventConsumer keyDown;
	};
}