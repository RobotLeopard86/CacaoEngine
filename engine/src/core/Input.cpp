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
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Keyboard, .id = kue.GetData(), .newState = false});
			});
			keyDown = EventConsumer([this](Event& e) {
				DataEvent<unsigned int>& kde = static_cast<DataEvent<unsigned int>&>(e);
				deltas.push_back(InputStateDelta {.category = InputStateDelta::Category::Keyboard, .id = kde.GetData(), .newState = true});
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

		//Lists of all keys and buttons (important)
		constexpr std::array<unsigned int, 104> allKeys {{CACAO_KEY_ENTER,
			CACAO_KEY_ESCAPE,
			CACAO_KEY_BACKSPACE,
			CACAO_KEY_TAB,
			CACAO_KEY_SPACE,
			CACAO_KEY_EXCLAMATION,
			CACAO_KEY_APOSTROPHE,
			CACAO_KEY_COMMA,
			CACAO_KEY_MINUS,
			CACAO_KEY_EQUALS,
			CACAO_KEY_PERIOD,
			CACAO_KEY_SLASH,
			CACAO_KEY_0,
			CACAO_KEY_1,
			CACAO_KEY_2,
			CACAO_KEY_3,
			CACAO_KEY_4,
			CACAO_KEY_5,
			CACAO_KEY_6,
			CACAO_KEY_7,
			CACAO_KEY_8,
			CACAO_KEY_9,
			CACAO_KEY_SEMICOLON,
			CACAO_KEY_LEFT_BRACKET,
			CACAO_KEY_RIGHT_BRACKET,
			CACAO_KEY_BACKSLASH,
			CACAO_KEY_GRAVE_ACCENT,
			CACAO_KEY_A,
			CACAO_KEY_B,
			CACAO_KEY_C,
			CACAO_KEY_D,
			CACAO_KEY_E,
			CACAO_KEY_F,
			CACAO_KEY_G,
			CACAO_KEY_H,
			CACAO_KEY_I,
			CACAO_KEY_J,
			CACAO_KEY_K,
			CACAO_KEY_L,
			CACAO_KEY_M,
			CACAO_KEY_N,
			CACAO_KEY_O,
			CACAO_KEY_P,
			CACAO_KEY_Q,
			CACAO_KEY_R,
			CACAO_KEY_S,
			CACAO_KEY_T,
			CACAO_KEY_U,
			CACAO_KEY_V,
			CACAO_KEY_W,
			CACAO_KEY_X,
			CACAO_KEY_Y,
			CACAO_KEY_Z,
			CACAO_KEY_CAPS_LOCK,
			CACAO_KEY_F1,
			CACAO_KEY_F2,
			CACAO_KEY_F3,
			CACAO_KEY_F4,
			CACAO_KEY_F5,
			CACAO_KEY_F6,
			CACAO_KEY_F7,
			CACAO_KEY_F8,
			CACAO_KEY_F9,
			CACAO_KEY_F10,
			CACAO_KEY_F11,
			CACAO_KEY_F12,
			CACAO_KEY_PRINT_SCREEN,
			CACAO_KEY_SCROLL_LOCK,
			CACAO_KEY_PAUSE,
			CACAO_KEY_INSERT,
			CACAO_KEY_DELETE,
			CACAO_KEY_HOME,
			CACAO_KEY_PAGE_UP,
			CACAO_KEY_END,
			CACAO_KEY_PAGE_DOWN,
			CACAO_KEY_RIGHT,
			CACAO_KEY_LEFT,
			CACAO_KEY_DOWN,
			CACAO_KEY_UP,
			CACAO_KEY_NUM_LOCK,
			CACAO_KEY_KP_DIVIDE,
			CACAO_KEY_KP_MULTIPLY,
			CACAO_KEY_KP_MINUS,
			CACAO_KEY_KP_PLUS,
			CACAO_KEY_KP_ENTER,
			CACAO_KEY_KP_1,
			CACAO_KEY_KP_2,
			CACAO_KEY_KP_3,
			CACAO_KEY_KP_4,
			CACAO_KEY_KP_5,
			CACAO_KEY_KP_6,
			CACAO_KEY_KP_7,
			CACAO_KEY_KP_8,
			CACAO_KEY_KP_9,
			CACAO_KEY_KP_0,
			CACAO_KEY_KP_PERIOD,
			CACAO_KEY_LEFT_CONTROL,
			CACAO_KEY_LEFT_SHIFT,
			CACAO_KEY_LEFT_ALT,
			CACAO_KEY_LEFT_SUPER,
			CACAO_KEY_RIGHT_CONTROL,
			CACAO_KEY_RIGHT_SHIFT,
			CACAO_KEY_RIGHT_ALT,
			CACAO_KEY_RIGHT_SUPER}};

		constexpr std::array<unsigned int, 3> allButtons {{CACAO_BUTTON_LEFT, CACAO_BUTTON_MIDDLE, CACAO_BUTTON_RIGHT}};

		//Fill maps
		for(unsigned int key : allKeys) {
			impl->keysFrozen.insert_or_assign(key, false);
		}
		for(unsigned int button : allButtons) {
			impl->mouseFrozen.insert_or_assign(button, false);
		}
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