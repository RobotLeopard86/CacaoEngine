#include "Cacao/Input.hpp"
#include "Cacao/Event.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "SingletonGet.hpp"

#include <unordered_map>
#include <vector>

namespace Cacao {
	struct Input::Impl {
		//Frozen state
		//Updated via FreezeInputState, returned from query functions
		glm::dvec2 cursorPosFrozen;
		std::unordered_map<unsigned int, bool> keysFrozen;
		std::unordered_map<unsigned int, bool> mouseFrozen;

		//Holding state
		//Modified by events from window, read by FreezeInputState
		glm::dvec2 cursorPosTmp;
		struct InputStateDelta {
			enum class Category {
				Keyboard,
				Mouse
			} category;
			unsigned int id;
			bool newState;
		};
		std::vector<InputStateDelta> deltas;

		//Update functions
		EventConsumer mouseMove;
		EventConsumer mouseButtonPress;
		EventConsumer mouseButtonRelease;
		EventConsumer keyUp;
		EventConsumer keyDown;

		void SetupConsumerObjects() {
			//Create objects
			mouseMove = EventConsumer([this](Event& e) {
				DataEvent<glm::dvec2>& mme = static_cast<DataEvent<glm::dvec2>&>(e);
				cursorPosTmp = mme.GetData();
			});
			mouseButtonPress = EventConsumer([this](Event& e) {
				DataEvent<unsigned int>& mbpe = static_cast<DataEvent<unsigned int>&>(e);
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Mouse, .id = mbpe.GetData(), .newState = true});
			});
			mouseButtonRelease = EventConsumer([this](Event& e) {
				DataEvent<unsigned int>& mbre = static_cast<DataEvent<unsigned int>&>(e);
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Mouse, .id = mbre.GetData(), .newState = false});
			});
			keyUp = EventConsumer([this](Event& e) {
				DataEvent<unsigned int>& kue = static_cast<DataEvent<unsigned int>&>(e);
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Keyboard, .id = kue.GetData(), .newState = true});
			});
			keyDown = EventConsumer([this](Event& e) {
				DataEvent<unsigned int>& kde = static_cast<DataEvent<unsigned int>&>(e);
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Keyboard, .id = kde.GetData(), .newState = false});
			});
		}
	};

	CACAOST_GET(Input)

	Input::Input() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Setup event consumer objects
		impl->SetupConsumerObjects();

		//Register consumers
		EventManager::Get().SubscribeConsumer("MouseMove", impl->mouseMove);
		EventManager::Get().SubscribeConsumer("MousePress", impl->mouseButtonPress);
		EventManager::Get().SubscribeConsumer("MouseRelease", impl->mouseButtonRelease);
		EventManager::Get().SubscribeConsumer("KeyUp", impl->keyUp);
		EventManager::Get().SubscribeConsumer("KeyDown", impl->keyDown);
	}

	Input::~Input() {}

	void Input::FreezeInputState() {
		//Set cursor position
		impl->cursorPosFrozen = impl->cursorPosTmp;

		//Apply mouse and keyboard deltas
		for(const Impl::InputStateDelta& delta : impl->deltas) {
			//Select the target map
			std::unordered_map<unsigned int, bool>* targetMap;
			switch(delta.category) {
				case Impl::InputStateDelta::Category::Keyboard:
					targetMap = &impl->keysFrozen;
					break;
				case Impl::InputStateDelta::Category::Mouse:
					targetMap = &impl->mouseFrozen;
					break;
			}

			//Apply the state change
			targetMap->at(delta.id) = delta.newState;
		}
		impl->deltas.clear();
	}

	glm::dvec2 Input::GetCursorPos() {
		return impl->cursorPosFrozen;
	}

	bool Input::IsKeyPressed(unsigned int key) {
		Check<NonexistentValueException>(impl->keysFrozen.contains(key), "Invalid keycode provided for input state request!");
		return impl->keysFrozen[key];
	}

	bool Input::IsMouseButtonPressed(unsigned int button) {
		Check<NonexistentValueException>(impl->mouseFrozen.contains(button), "Invalid mouse button provided for input state request!");
		return impl->mouseFrozen[button];
	}
}