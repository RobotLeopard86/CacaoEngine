#pragma once

#include "glm/vec2.hpp"

#include "Events/EventSystem.hpp"
#include "Flushable.hpp"

#include <map>

namespace Cacao {
	//Input utility singleton
	class Input {
	public:
		//Returns a two-component vector of doubles representing the current cursor position
        glm::dvec2 GetCursorPos();
        //Returns a boolean representing whether the given key is pressed
        bool IsKeyPressed(int key);
        //Returns a boolean representing whether the given mouse button is pressed
        bool IsMouseButtonPressed(int button);

		//Flush the input state
		void FreezeFrameInputState();

        //Get the current instance or create one if it doesn't exist
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

//Define key and mouse button codes (taken from GLFW)
//A LUT will be added to convert these codes to usable codes for other APIs (e.g. Win32) later

#define CACAO_KEY_SPACE              32
#define CACAO_KEY_APOSTROPHE         39
#define CACAO_KEY_COMMA              44
#define CACAO_KEY_MINUS              45
#define CACAO_KEY_PERIOD             46
#define CACAO_KEY_SLASH              47
#define CACAO_KEY_0                  48
#define CACAO_KEY_1                  49
#define CACAO_KEY_2                  50
#define CACAO_KEY_3                  51
#define CACAO_KEY_4                  52
#define CACAO_KEY_5                  53
#define CACAO_KEY_6                  54
#define CACAO_KEY_7                  55
#define CACAO_KEY_8                  56
#define CACAO_KEY_9                  57
#define CACAO_KEY_SEMICOLON          59
#define CACAO_KEY_EQUAL              61
#define CACAO_KEY_A                  65
#define CACAO_KEY_B                  66
#define CACAO_KEY_C                  67
#define CACAO_KEY_D                  68
#define CACAO_KEY_E                  69
#define CACAO_KEY_F                  70
#define CACAO_KEY_G                  71
#define CACAO_KEY_H                  72
#define CACAO_KEY_I                  73
#define CACAO_KEY_J                  74
#define CACAO_KEY_K                  75
#define CACAO_KEY_L                  76
#define CACAO_KEY_M                  77
#define CACAO_KEY_N                  78
#define CACAO_KEY_O                  79
#define CACAO_KEY_P                  80
#define CACAO_KEY_Q                  81
#define CACAO_KEY_R                  82
#define CACAO_KEY_S                  83
#define CACAO_KEY_T                  84
#define CACAO_KEY_U                  85
#define CACAO_KEY_V                  86
#define CACAO_KEY_W                  87
#define CACAO_KEY_X                  88
#define CACAO_KEY_Y                  89
#define CACAO_KEY_Z                  90
#define CACAO_KEY_LEFT_BRACKET       91
#define CACAO_KEY_BACKSLASH          92
#define CACAO_KEY_RIGHT_BRACKET      93
#define CACAO_KEY_GRAVE_ACCENT       96
#define CACAO_KEY_WORLD_1            161
#define CACAO_KEY_WORLD_2            162
#define CACAO_KEY_ESCAPE             256
#define CACAO_KEY_ENTER              257
#define CACAO_KEY_TAB                258
#define CACAO_KEY_BACKSPACE          259
#define CACAO_KEY_INSERT             260
#define CACAO_KEY_DELETE             261
#define CACAO_KEY_RIGHT              262
#define CACAO_KEY_LEFT               263
#define CACAO_KEY_DOWN               264
#define CACAO_KEY_UP                 265
#define CACAO_KEY_PAGE_UP            266
#define CACAO_KEY_PAGE_DOWN          267
#define CACAO_KEY_HOME               268
#define CACAO_KEY_END                269
#define CACAO_KEY_CAPS_LOCK          280
#define CACAO_KEY_SCROLL_LOCK        281
#define CACAO_KEY_NUM_LOCK           282
#define CACAO_KEY_PRINT_SCREEN       283
#define CACAO_KEY_PAUSE              284
#define CACAO_KEY_F1                 290
#define CACAO_KEY_F2                 291
#define CACAO_KEY_F3                 292
#define CACAO_KEY_F4                 293
#define CACAO_KEY_F5                 294
#define CACAO_KEY_F6                 295
#define CACAO_KEY_F7                 296
#define CACAO_KEY_F8                 297
#define CACAO_KEY_F9                 298
#define CACAO_KEY_F10                299
#define CACAO_KEY_F11                300
#define CACAO_KEY_F12                301
#define CACAO_KEY_F13                302
#define CACAO_KEY_F14                303
#define CACAO_KEY_F15                304
#define CACAO_KEY_F16                305
#define CACAO_KEY_F17                306
#define CACAO_KEY_F18                307
#define CACAO_KEY_F19                308
#define CACAO_KEY_F20                309
#define CACAO_KEY_F21                310
#define CACAO_KEY_F22                311
#define CACAO_KEY_F23                312
#define CACAO_KEY_F24                313
#define CACAO_KEY_F25                314
#define CACAO_KEY_KP_0               320
#define CACAO_KEY_KP_1               321
#define CACAO_KEY_KP_2               322
#define CACAO_KEY_KP_3               323
#define CACAO_KEY_KP_4               324
#define CACAO_KEY_KP_5               325
#define CACAO_KEY_KP_6               326
#define CACAO_KEY_KP_7               327
#define CACAO_KEY_KP_8               328
#define CACAO_KEY_KP_9               329
#define CACAO_KEY_KP_DECIMAL         330
#define CACAO_KEY_KP_DIVIDE          331
#define CACAO_KEY_KP_MULTIPLY        332
#define CACAO_KEY_KP_SUBTRACT        333
#define CACAO_KEY_KP_ADD             334
#define CACAO_KEY_KP_ENTER           335
#define CACAO_KEY_KP_EQUAL           336
#define CACAO_KEY_LEFT_SHIFT         340
#define CACAO_KEY_LEFT_CONTROL       341
#define CACAO_KEY_LEFT_ALT           342
#define CACAO_KEY_LEFT_SUPER         343
#define CACAO_KEY_RIGHT_SHIFT        344
#define CACAO_KEY_RIGHT_CONTROL      345
#define CACAO_KEY_RIGHT_ALT          346
#define CACAO_KEY_RIGHT_SUPER        347
#define CACAO_KEY_MENU               348
#define CACAO_KEY_LAST               CACAO_KEY_MENU
#define CACAO_MOUSE_BUTTON_1         0
#define CACAO_MOUSE_BUTTON_2         1
#define CACAO_MOUSE_BUTTON_3         2
#define CACAO_MOUSE_BUTTON_4         3
#define CACAO_MOUSE_BUTTON_5         4
#define CACAO_MOUSE_BUTTON_6         5
#define CACAO_MOUSE_BUTTON_7         6
#define CACAO_MOUSE_BUTTON_8         7
#define CACAO_MOUSE_BUTTON_LAST      CACAO_MOUSE_BUTTON_8
#define CACAO_MOUSE_BUTTON_LEFT      CACAO_MOUSE_BUTTON_1
#define CACAO_MOUSE_BUTTON_RIGHT     CACAO_MOUSE_BUTTON_2
#define CACAO_MOUSE_BUTTON_MIDDLE    CACAO_MOUSE_BUTTON_3