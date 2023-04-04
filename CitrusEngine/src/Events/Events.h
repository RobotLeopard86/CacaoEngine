#pragma once

#include "Core/Log.h"

#include <string>

namespace CitrusEngine {

    class Event {
    public:
        Event() : handled(false) {}

        bool handled;

        virtual std::string GetType() {
            return "None";
        }
    };

    //Window events
    class WindowCloseEvent : public Event { public: std::string GetType() { return "WindowClose"; } }; //No additional parameters
    class WindowReceiveFocusEvent : public Event { public: std::string GetType() { return "WindowReceiveFocus"; } }; //No additional parameters
    class WindowLoseFocusEvent : public Event { public: std::string GetType() { return "WindowLoseFocus"; } }; //No additional parameters
    class WindowResizeEvent : public Event { //Parameters: New X and Y size of window
    public:
        WindowResizeEvent(int x, int y) : x(x), y(y) {}
        int x, y;

        std::string GetType() { return "WindowResize"; }
    };

    //Client events
    class ClientFixedTickEvent : public Event { public: std::string GetType() { return "ClientFixedTick"; } }; //No additional parameters
    class ClientDynamicTickEvent : public Event { //Parameters: Delta time since last update
    public:
        ClientDynamicTickEvent(double timestep) : timestep(timestep) {}
        double timestep;

        std::string GetType() { return "ClientDynamicTick"; }
    };

    //Key events
    class KeyDownEvent : public Event { //Parameters: Keycode for key pressed
    public:
        KeyDownEvent(int keycode) : keycode(keycode) {}
        int keycode;

        std::string GetType() { return "KeyDown"; }
    };
    class KeyUpEvent : public Event { //Parameters: Keycode for key released
    public:   
        KeyUpEvent(int keycode) : keycode(keycode) {}
        int keycode;

        std::string GetType() { return "KeyUp"; }
    };
    class KeyTypeEvent : public Event { //Parameters: Keycode for key typed
    public:    
        KeyTypeEvent(int keycode) : keycode(keycode) {}
        int keycode;

        std::string GetType() { return "KeyType"; }
    };

    //Mouse events
    class MousePressEvent : public Event { //Parameters: Button code for button pressed
    public:
        MousePressEvent(int button) : button(button) {}
        int button;

        std::string GetType() { return "MousePress"; }
    };
    class MouseReleaseEvent : public Event { //Parameters: Button code for button released
    public:
        MouseReleaseEvent(int button) : button(button) {}
        int button;

        std::string GetType() { return "MouseRelease"; }
    };
    class MouseScrollEvent : public Event { //Parameters: X and Y offset for scrolling
    public:
        MouseScrollEvent(int xOffset, int yOffset) : xOffset(xOffset), yOffset(yOffset) {}
        double xOffset, yOffset;

        std::string GetType() { return "MouseScroll"; }
    };
    class MouseMoveEvent : public Event { //Parameters: New X and Y position of mouse
    public:
        MouseMoveEvent(int x, int y) : x(x), y(y) {}
        double x, y;

        std::string GetType() { return "MouseMove"; }
    };
}