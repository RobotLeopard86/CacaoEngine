#pragma once

#include "glm/vec2.hpp"

#include "Events/EventSystem.hpp"
#include "Core/DllHelper.hpp"
#include "Flushable.hpp"

#include <map>

namespace Cacao {
	/**
	 * @brief Input handler and store
	 */
	class CACAO_API Input {
	  public:
		/**
		 * @brief Get the current cursor position
		 *
		 * @return The current cursor position
		 */
		glm::dvec2 GetCursorPos();

		/**
		 * @brief Check whether a given key is pressed
		 *
		 * @param key The key to check.
		 * @note View the page "Input Mappings" in the manual for the list of valid keys
		 *
		 * @return If the key is pressed
		 */
		bool IsKeyPressed(int key);

		/**
		 * @brief Check whether a given mouse button is pressed
		 *
		 * @param button The mouse button to check.
		 * @note View the page "Input Mappings" in the manual for the list of valid buttons
		 *
		 * @return If the mouse button is pressed
		 */
		bool IsMouseButtonPressed(int button);

		/**
		 * @brief Freeze the current input state for this tick
		 *
		 * @note For use by the engine only
		 */
		void FreezeFrameInputState();

		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Input* GetInstance();

	  private:
		//Singleton data
		static Input* instance;
		static bool instanceExists;

		//Private constructor
		Input();

		//Event consumers
		EventConsumer* cursorPosConsumer;
		EventConsumer* keyUpConsumer;
		EventConsumer* mouseButtonUpConsumer;
		EventConsumer* keyDownConsumer;
		EventConsumer* mouseButtonDownConsumer;

		//Event consumer functions
		void CursorPosChangeHandler(Event& e);
		void KeyUpHandler(Event& e);
		void KeyDownHandler(Event& e);
		void MouseButtonDownHandler(Event& e);
		void MouseButtonUpHandler(Event& e);

		//Input data storage
		glm::dvec2 _cursorPos;
		std::map<int, bool> _keyStateMap;
		std::map<int, bool> _mouseButtonStateMap;

		//Flushables for temporary holding
		Flushable<glm::dvec2> cursorPos;
		Flushable<std::map<int, bool>> keyStateMap, mouseButtonStateMap;
	};
}

//Define key and mouse button codes (these have been stolen from SDL)
#define CACAO_KEY_ENTER 0x0000000du
#define CACAO_KEY_ESCAPE 0x0000001bu
#define CACAO_KEY_BACKSPACE 0x00000008u
#define CACAO_KEY_TAB 0x00000009u
#define CACAO_KEY_SPACE 0x00000020u
#define CACAO_KEY_EXCLAMATION 0x00000021u
#define CACAO_KEY_DOUBLE_APOSTROPHE 0x00000022u
#define CACAO_KEY_HASH 0x00000023u
#define CACAO_KEY_DOLLAR 0x00000024u
#define CACAO_KEY_PERCENT 0x00000025u
#define CACAO_KEY_AMPERSAND 0x00000026u
#define CACAO_KEY_APOSTROPHE 0x00000027u
#define CACAO_KEY_LEFT_PARENTHESIS 0x00000028u
#define CACAO_KEY_RIGHT_PARENTHESIS 0x00000029u
#define CACAO_KEY_ASTERISK 0x0000002au
#define CACAO_KEY_PLUS 0x0000002bu
#define CACAO_KEY_COMMA 0x0000002cu
#define CACAO_KEY_MINUS 0x0000002du
#define CACAO_KEY_PERIOD 0x0000002eu
#define CACAO_KEY_SLASH 0x0000002fu
#define CACAO_KEY_0 0x00000030u
#define CACAO_KEY_1 0x00000031u
#define CACAO_KEY_2 0x00000032u
#define CACAO_KEY_3 0x00000033u
#define CACAO_KEY_4 0x00000034u
#define CACAO_KEY_5 0x00000035u
#define CACAO_KEY_6 0x00000036u
#define CACAO_KEY_7 0x00000037u
#define CACAO_KEY_8 0x00000038u
#define CACAO_KEY_9 0x00000039u
#define CACAO_KEY_COLON 0x0000003au
#define CACAO_KEY_SEMICOLON 0x0000003bu
#define CACAO_KEY_LESS 0x0000003cu
#define CACAO_KEY_EQUAL 0x0000003du
#define CACAO_KEY_GREATER 0x0000003eu
#define CACAO_KEY_QUESTION 0x0000003fu
#define CACAO_KEY_AT 0x00000040u
#define CACAO_KEY_LEFT_BRACKET 0x0000005bu
#define CACAO_KEY_BACKSLASH 0x0000005cu
#define CACAO_KEY_RIGHT_BRACKET 0x0000005du
#define CACAO_KEY_CARET 0x0000005eu
#define CACAO_KEY_UNDERSCORE 0x0000005fu
#define CACAO_KEY_GRAVE 0x00000060u
#define CACAO_KEY_A 0x00000061u
#define CACAO_KEY_B 0x00000062u
#define CACAO_KEY_C 0x00000063u
#define CACAO_KEY_D 0x00000064u
#define CACAO_KEY_E 0x00000065u
#define CACAO_KEY_F 0x00000066u
#define CACAO_KEY_G 0x00000067u
#define CACAO_KEY_H 0x00000068u
#define CACAO_KEY_I 0x00000069u
#define CACAO_KEY_J 0x0000006au
#define CACAO_KEY_K 0x0000006bu
#define CACAO_KEY_L 0x0000006cu
#define CACAO_KEY_M 0x0000006du
#define CACAO_KEY_N 0x0000006eu
#define CACAO_KEY_O 0x0000006fu
#define CACAO_KEY_P 0x00000070u
#define CACAO_KEY_Q 0x00000071u
#define CACAO_KEY_R 0x00000072u
#define CACAO_KEY_S 0x00000073u
#define CACAO_KEY_T 0x00000074u
#define CACAO_KEY_U 0x00000075u
#define CACAO_KEY_V 0x00000076u
#define CACAO_KEY_W 0x00000077u
#define CACAO_KEY_X 0x00000078u
#define CACAO_KEY_Y 0x00000079u
#define CACAO_KEY_Z 0x0000007au
#define CACAO_KEY_LEFT_BRACE 0x0000007bu
#define CACAO_KEY_PIPE 0x0000007cu
#define CACAO_KEY_RIGHT_BRACE 0x0000007du
#define CACAO_KEY_TILDE 0x0000007eu
#define CACAO_KEY_DELETE 0x0000007fu
#define CACAO_KEY_PLUS_MINUS 0x000000b1u
#define CACAO_KEY_CAPS_LOCK 0x40000039u
#define CACAO_KEY_F1 0x4000003au
#define CACAO_KEY_F2 0x4000003bu
#define CACAO_KEY_F3 0x4000003cu
#define CACAO_KEY_F4 0x4000003du
#define CACAO_KEY_F5 0x4000003eu
#define CACAO_KEY_F6 0x4000003fu
#define CACAO_KEY_F7 0x40000040u
#define CACAO_KEY_F8 0x40000041u
#define CACAO_KEY_F9 0x40000042u
#define CACAO_KEY_F10 0x40000043u
#define CACAO_KEY_F11 0x40000044u
#define CACAO_KEY_F12 0x40000045u
#define CACAO_KEY_PRINT_SCREEN 0x40000046u
#define CACAO_KEY_SCROLL_LOCK 0x40000047u
#define CACAO_KEY_PAUSE 0x40000048u
#define CACAO_KEY_INSERT 0x40000049u
#define CACAO_KEY_HOME 0x4000004au
#define CACAO_KEY_PAGE_UP 0x4000004bu
#define CACAO_KEY_END 0x4000004du
#define CACAO_KEY_PAGE_DOWN 0x4000004eu
#define CACAO_KEY_RIGHT 0x4000004fu
#define CACAO_KEY_LEFT 0x40000050u
#define CACAO_KEY_DOWN 0x40000051u
#define CACAO_KEY_UP 0x40000052u
#define CACAO_KEY_NUM_LOCK 0x40000053u
#define CACAO_KEY_KP_DIVIDE 0x40000054u
#define CACAO_KEY_KP_MULTIPLY 0x40000055u
#define CACAO_KEY_KP_MINUS 0x40000056u
#define CACAO_KEY_KP_PLUS 0x40000057u
#define CACAO_KEY_KP_ENTER 0x40000058u
#define CACAO_KEY_KP_1 0x40000059u
#define CACAO_KEY_KP_2 0x4000005au
#define CACAO_KEY_KP_3 0x4000005bu
#define CACAO_KEY_KP_4 0x4000005cu
#define CACAO_KEY_KP_5 0x4000005du
#define CACAO_KEY_KP_6 0x4000005eu
#define CACAO_KEY_KP_7 0x4000005fu
#define CACAO_KEY_KP_8 0x40000060u
#define CACAO_KEY_KP_9 0x40000061u
#define CACAO_KEY_KP_0 0x40000062u
#define CACAO_KEY_KP_PERIOD 0x40000063u
#define CACAO_KEY_APPLICATION 0x40000065u
#define CACAO_KEY_POWER 0x40000066u
#define CACAO_KEY_KP_EQUAL 0x40000067u
#define CACAO_KEY_F13 0x40000068u
#define CACAO_KEY_F14 0x40000069u
#define CACAO_KEY_F15 0x4000006au
#define CACAO_KEY_F16 0x4000006bu
#define CACAO_KEY_F17 0x4000006cu
#define CACAO_KEY_F18 0x4000006du
#define CACAO_KEY_F19 0x4000006eu
#define CACAO_KEY_F20 0x4000006fu
#define CACAO_KEY_F21 0x40000070u
#define CACAO_KEY_F22 0x40000071u
#define CACAO_KEY_F23 0x40000072u
#define CACAO_KEY_F24 0x40000073u
#define CACAO_KEY_EXECUTE 0x40000074u
#define CACAO_KEY_HELP 0x40000075u
#define CACAO_KEY_MENU 0x40000076u
#define CACAO_KEY_SELECT 0x40000077u
#define CACAO_KEY_STOP 0x40000078u
#define CACAO_KEY_AGAIN 0x40000079u
#define CACAO_KEY_UNDO 0x4000007au
#define CACAO_KEY_CUT 0x4000007bu
#define CACAO_KEY_COPY 0x4000007cu
#define CACAO_KEY_PASTE 0x4000007du
#define CACAO_KEY_FIND 0x4000007eu
#define CACAO_KEY_MUTE 0x4000007fu
#define CACAO_KEY_VOLUME_UP 0x40000080u
#define CACAO_KEY_VOLUME_DOWN 0x40000081u
#define CACAO_KEY_KP_COMMA 0x40000085u
#define CACAO_KEY_KP_EQUALSAS400 0x40000086u
#define CACAO_KEY_ALT_ERASE 0x40000099u
#define CACAO_KEY_SYSREQ 0x4000009au
#define CACAO_KEY_CANCEL 0x4000009bu
#define CACAO_KEY_CLEAR 0x4000009cu
#define CACAO_KEY_PRIOR 0x4000009du
#define CACAO_KEY_RETURN2 0x4000009eu
#define CACAO_KEY_SEPARATOR 0x4000009fu
#define CACAO_KEY_OUT 0x400000a0u
#define CACAO_KEY_OPER 0x400000a1u
#define CACAO_KEY_CLEAR_AGAIN 0x400000a2u
#define CACAO_KEY_CRSEL 0x400000a3u
#define CACAO_KEY_EXSEL 0x400000a4u
#define CACAO_KEY_KP_00 0x400000b0u
#define CACAO_KEY_KP_000 0x400000b1u
#define CACAO_KEY_THOUSANDS_SEPARATOR 0x400000b2u
#define CACAO_KEY_DECIMAL_SEPARATOR 0x400000b3u
#define CACAO_KEY_CURRENCY_UNIT 0x400000b4u
#define CACAO_KEY_CURRENCY_SUBUNIT 0x400000b5u
#define CACAO_KEY_KP_LEFTPARENTHESIS 0x400000b6u
#define CACAO_KEY_KP_RIGHTPARENTHESIS 0x400000b7u
#define CACAO_KEY_KP_LEFTBRACE 0x400000b8u
#define CACAO_KEY_KP_RIGHTBRACE 0x400000b9u
#define CACAO_KEY_KP_TAB 0x400000bau
#define CACAO_KEY_KP_BACKSPACE 0x400000bbu
#define CACAO_KEY_KP_A 0x400000bcu
#define CACAO_KEY_KP_B 0x400000bdu
#define CACAO_KEY_KP_C 0x400000beu
#define CACAO_KEY_KP_D 0x400000bfu
#define CACAO_KEY_KP_E 0x400000c0u
#define CACAO_KEY_KP_F 0x400000c1u
#define CACAO_KEY_KP_XOR 0x400000c2u
#define CACAO_KEY_KP_POWER 0x400000c3u
#define CACAO_KEY_KP_PERCENT 0x400000c4u
#define CACAO_KEY_KP_LESS 0x400000c5u
#define CACAO_KEY_KP_GREATER 0x400000c6u
#define CACAO_KEY_KP_AMPERSAND 0x400000c7u
#define CACAO_KEY_KP_DOUBLE_AMPERSAND 0x400000c8u
#define CACAO_KEY_KP_VERTICAL_BAR 0x400000c9u
#define CACAO_KEY_KP_DOUBLE_VERTICAL_BAR 0x400000cau
#define CACAO_KEY_KP_COLON 0x400000cbu
#define CACAO_KEY_KP_HASH 0x400000ccu
#define CACAO_KEY_KP_SPACE 0x400000cdu
#define CACAO_KEY_KP_AT 0x400000ceu
#define CACAO_KEY_KP_EXCLAM 0x400000cfu
#define CACAO_KEY_KP_MEM_STORE 0x400000d0u
#define CACAO_KEY_KP_MEM_RECALL 0x400000d1u
#define CACAO_KEY_KP_MEM_CLEAR 0x400000d2u
#define CACAO_KEY_KP_MEM_ADD 0x400000d3u
#define CACAO_KEY_KP_MEM_SUBTRACT 0x400000d4u
#define CACAO_KEY_KP_MEM_MULTIPLY 0x400000d5u
#define CACAO_KEY_KP_MEM_DIVIDE 0x400000d6u
#define CACAO_KEY_KP_PLUS_MINUS 0x400000d7u
#define CACAO_KEY_KP_CLEAR 0x400000d8u
#define CACAO_KEY_KP_CLEAR_ENTRY 0x400000d9u
#define CACAO_KEY_KP_BINARY 0x400000dau
#define CACAO_KEY_KP_OCTAL 0x400000dbu
#define CACAO_KEY_KP_DECIMAL 0x400000dcu
#define CACAO_KEY_KP_HEXADECIMAL 0x400000ddu
#define CACAO_KEY_LEFT_CONTROL 0x400000e0u
#define CACAO_KEY_LEFT_SHIFT 0x400000e1u
#define CACAO_KEY_LEFT_ALT 0x400000e2u
#define CACAO_KEY_LEFT_SUPER 0x400000e3u
#define CACAO_KEY_RIGHT_CONTROL 0x400000e4u
#define CACAO_KEY_RIGHT_SHIFT 0x400000e5u
#define CACAO_KEY_RIGHT_ALT 0x400000e6u
#define CACAO_KEY_RIGHT_SUPER 0x400000e7u
#define CACAO_MOUSE_BUTTON_LEFT 1
#define CACAO_MOUSE_BUTTON_MIDDLE 2
#define CACAO_MOUSE_BUTTON_RIGHT 3