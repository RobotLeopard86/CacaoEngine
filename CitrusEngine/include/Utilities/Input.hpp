#pragma once

#include "glm/vec2.hpp"

#include "Events/EventSystem.hpp"

#include <map>

namespace CitrusEngine {
    //Input singleton
    class Input {
    public:
        virtual ~Input() {}

        //Returns a two-component vector of doubles representing the current cursor position
        glm::dvec2 GetCursorPos();
        //Returns a boolean representing whether the given key is pressed
        bool IsKeyPressed(int key);
        //Returns a boolean representing whether the given mouse button is pressed
        bool IsMouseButtonPressed(int button);

        //Get the current instance or create one if it doesn't exist
        static Input* GetInstance(); 

        //Event handlers
        void CursorPosChangeHandler(Event& e);
        void KeyUpHandler(Event& e);
        void MouseButtonUpHandler(Event& e);
        void KeyDownHandler(Event& e);
        void MouseButtonDownHandler(Event& e);
    private:
        static Input* instance;
        static bool instanceExists;

        //Private constructor so only the singleton initializer can call it
        Input();

        //Event consumers
        EventConsumer* cursorPosConsumer;
        EventConsumer* keyUpConsumer;
        EventConsumer* mouseButtonUpConsumer;
        EventConsumer* keyDownConsumer;
        EventConsumer* mouseButtonDownConsumer;

        //Input data storage
        glm::dvec2 cursorPos;
        std::map<int, bool> keyStateMap;
        std::map<int, bool> mouseButtonStateMap;
    };
}

//Define key and mouse button codes (taken from GLFW)
//A LUT will be added to convert these codes to usable codes for other APIs (e.g. Win32) later

#define CITRUS_KEY_SPACE              32
#define CITRUS_KEY_APOSTROPHE         39
#define CITRUS_KEY_COMMA              44
#define CITRUS_KEY_MINUS              45
#define CITRUS_KEY_PERIOD             46
#define CITRUS_KEY_SLASH              47
#define CITRUS_KEY_0                  48
#define CITRUS_KEY_1                  49
#define CITRUS_KEY_2                  50
#define CITRUS_KEY_3                  51
#define CITRUS_KEY_4                  52
#define CITRUS_KEY_5                  53
#define CITRUS_KEY_6                  54
#define CITRUS_KEY_7                  55
#define CITRUS_KEY_8                  56
#define CITRUS_KEY_9                  57
#define CITRUS_KEY_SEMICOLON          59
#define CITRUS_KEY_EQUAL              61
#define CITRUS_KEY_A                  65
#define CITRUS_KEY_B                  66
#define CITRUS_KEY_C                  67
#define CITRUS_KEY_D                  68
#define CITRUS_KEY_E                  69
#define CITRUS_KEY_F                  70
#define CITRUS_KEY_G                  71
#define CITRUS_KEY_H                  72
#define CITRUS_KEY_I                  73
#define CITRUS_KEY_J                  74
#define CITRUS_KEY_K                  75
#define CITRUS_KEY_L                  76
#define CITRUS_KEY_M                  77
#define CITRUS_KEY_N                  78
#define CITRUS_KEY_O                  79
#define CITRUS_KEY_P                  80
#define CITRUS_KEY_Q                  81
#define CITRUS_KEY_R                  82
#define CITRUS_KEY_S                  83
#define CITRUS_KEY_T                  84
#define CITRUS_KEY_U                  85
#define CITRUS_KEY_V                  86
#define CITRUS_KEY_W                  87
#define CITRUS_KEY_X                  88
#define CITRUS_KEY_Y                  89
#define CITRUS_KEY_Z                  90
#define CITRUS_KEY_LEFT_BRACKET       91
#define CITRUS_KEY_BACKSLASH          92
#define CITRUS_KEY_RIGHT_BRACKET      93
#define CITRUS_KEY_GRAVE_ACCENT       96
#define CITRUS_KEY_WORLD_1            161
#define CITRUS_KEY_WORLD_2            162
#define CITRUS_KEY_ESCAPE             256
#define CITRUS_KEY_ENTER              257
#define CITRUS_KEY_TAB                258
#define CITRUS_KEY_BACKSPACE          259
#define CITRUS_KEY_INSERT             260
#define CITRUS_KEY_DELETE             261
#define CITRUS_KEY_RIGHT              262
#define CITRUS_KEY_LEFT               263
#define CITRUS_KEY_DOWN               264
#define CITRUS_KEY_UP                 265
#define CITRUS_KEY_PAGE_UP            266
#define CITRUS_KEY_PAGE_DOWN          267
#define CITRUS_KEY_HOME               268
#define CITRUS_KEY_END                269
#define CITRUS_KEY_CAPS_LOCK          280
#define CITRUS_KEY_SCROLL_LOCK        281
#define CITRUS_KEY_NUM_LOCK           282
#define CITRUS_KEY_PRINT_SCREEN       283
#define CITRUS_KEY_PAUSE              284
#define CITRUS_KEY_F1                 290
#define CITRUS_KEY_F2                 291
#define CITRUS_KEY_F3                 292
#define CITRUS_KEY_F4                 293
#define CITRUS_KEY_F5                 294
#define CITRUS_KEY_F6                 295
#define CITRUS_KEY_F7                 296
#define CITRUS_KEY_F8                 297
#define CITRUS_KEY_F9                 298
#define CITRUS_KEY_F10                299
#define CITRUS_KEY_F11                300
#define CITRUS_KEY_F12                301
#define CITRUS_KEY_F13                302
#define CITRUS_KEY_F14                303
#define CITRUS_KEY_F15                304
#define CITRUS_KEY_F16                305
#define CITRUS_KEY_F17                306
#define CITRUS_KEY_F18                307
#define CITRUS_KEY_F19                308
#define CITRUS_KEY_F20                309
#define CITRUS_KEY_F21                310
#define CITRUS_KEY_F22                311
#define CITRUS_KEY_F23                312
#define CITRUS_KEY_F24                313
#define CITRUS_KEY_F25                314
#define CITRUS_KEY_KP_0               320
#define CITRUS_KEY_KP_1               321
#define CITRUS_KEY_KP_2               322
#define CITRUS_KEY_KP_3               323
#define CITRUS_KEY_KP_4               324
#define CITRUS_KEY_KP_5               325
#define CITRUS_KEY_KP_6               326
#define CITRUS_KEY_KP_7               327
#define CITRUS_KEY_KP_8               328
#define CITRUS_KEY_KP_9               329
#define CITRUS_KEY_KP_DECIMAL         330
#define CITRUS_KEY_KP_DIVIDE          331
#define CITRUS_KEY_KP_MULTIPLY        332
#define CITRUS_KEY_KP_SUBTRACT        333
#define CITRUS_KEY_KP_ADD             334
#define CITRUS_KEY_KP_ENTER           335
#define CITRUS_KEY_KP_EQUAL           336
#define CITRUS_KEY_LEFT_SHIFT         340
#define CITRUS_KEY_LEFT_CONTROL       341
#define CITRUS_KEY_LEFT_ALT           342
#define CITRUS_KEY_LEFT_SUPER         343
#define CITRUS_KEY_RIGHT_SHIFT        344
#define CITRUS_KEY_RIGHT_CONTROL      345
#define CITRUS_KEY_RIGHT_ALT          346
#define CITRUS_KEY_RIGHT_SUPER        347
#define CITRUS_KEY_MENU               348
#define CITRUS_KEY_LAST               CITRUS_KEY_MENU
#define CITRUS_MOUSE_BUTTON_1         0
#define CITRUS_MOUSE_BUTTON_2         1
#define CITRUS_MOUSE_BUTTON_3         2
#define CITRUS_MOUSE_BUTTON_4         3
#define CITRUS_MOUSE_BUTTON_5         4
#define CITRUS_MOUSE_BUTTON_6         5
#define CITRUS_MOUSE_BUTTON_7         6
#define CITRUS_MOUSE_BUTTON_8         7
#define CITRUS_MOUSE_BUTTON_LAST      CITRUS_MOUSE_BUTTON_8
#define CITRUS_MOUSE_BUTTON_LEFT      CITRUS_MOUSE_BUTTON_1
#define CITRUS_MOUSE_BUTTON_RIGHT     CITRUS_MOUSE_BUTTON_2
#define CITRUS_MOUSE_BUTTON_MIDDLE    CITRUS_MOUSE_BUTTON_3