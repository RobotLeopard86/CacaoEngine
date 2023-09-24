#pragma once

#include "Core/Log.h"
#include "Core/Assert.h"

#include "glm/vec2.hpp"

#include <string>
#include <stdexcept>

namespace CitrusEngine {
    
    //All subclasses of Event MUST have a default constructor with zero arguments
    class Event {
    public:
        Event() : handled(false) {}

        bool handled;

        virtual std::string GetType() {
            return "None";
        }

        //Casts generic event to usable event type
        template<typename T>
        static T EventTypeCast(Event& e) {
            try {
                T casted = dynamic_cast<T&>(e);
                return casted;
            } catch(const std::bad_cast& exception){
                //Intentional auto-fail assert (fail-state already met)
                Asserts::EngineAssert(false, "Cannot convert incompatible event types!");
                //Will never be called (just there to make the compiler happy :D )
                return T{};
            }
        }
    };

    //Window events
    class WindowCloseEvent : public Event { public: std::string GetType() { return "WindowClose"; } }; //No additional parameters
    class WindowReceiveFocusEvent : public Event { public: std::string GetType() { return "WindowReceiveFocus"; } }; //No additional parameters
    class WindowLoseFocusEvent : public Event { public: std::string GetType() { return "WindowLoseFocus"; } }; //No additional parameters
    class WindowResizeEvent : public Event { //Parameters: New X and Y size of window
    public:
        WindowResizeEvent(int x, int y) : size(x, y) {}
        WindowResizeEvent() : size(0) {}

        glm::ivec2 size;

        std::string GetType() { return "WindowResize"; }
    };

    //Tick events
    class FixedTickEvent : public Event { public: std::string GetType() { return "FixedTick"; } }; //No additional parameters
    class DynamicTickEvent : public Event { //Parameters: Delta time since last update
    public:
        DynamicTickEvent(double timestep) : timestep(timestep) {}
        DynamicTickEvent() : timestep(0.0) {}

        double timestep;

        std::string GetType() { return "DynamicTick"; }
    };

    //Key events
    class KeyDownEvent : public Event { //Parameters: Keycode for key pressed
    public:
        KeyDownEvent(int keycode) : keycode(keycode) {}
        KeyDownEvent() : keycode(0) {} //0 used because Input.h has no key assigned to it

        int keycode;

        std::string GetType() { return "KeyDown"; }
    };
    class KeyUpEvent : public Event { //Parameters: Keycode for key released
    public:   
        KeyUpEvent(int keycode) : keycode(keycode) {}
        KeyUpEvent() : keycode(0) {} //0 used because Input.h has no key assigned to it

        int keycode;

        std::string GetType() { return "KeyUp"; }
    };
    class KeyTypeEvent : public Event { //Parameters: Keycode for key typed
    public:    
        KeyTypeEvent(unsigned int keycode) : keycode(keycode) {}
        KeyTypeEvent() : keycode(0) {} //0 used because Input.h has no key assigned to it

        unsigned int keycode;

        std::string GetType() { return "KeyType"; }
    };

    //Mouse events
    class MousePressEvent : public Event { //Parameters: Button code for button pressed
    public:
        MousePressEvent(int button) : button(button) {}
        MousePressEvent() : button(9) {} //9 used because Input.h has no button assigned to it

        int button;

        std::string GetType() { return "MousePress"; }
    };
    class MouseReleaseEvent : public Event { //Parameters: Button code for button released
    public:
        MouseReleaseEvent(int button) : button(button) {}
        MouseReleaseEvent() : button(9) {} //9 used because Input.h has no button assigned to it

        int button;

        std::string GetType() { return "MouseRelease"; }
    };
    class MouseScrollEvent : public Event { //Parameters: X and Y offset for scrolling
    public:
        MouseScrollEvent(double xOffset, double yOffset) : offset(xOffset, yOffset) {}
        MouseScrollEvent() : offset(0.0) {}

        glm::dvec2 offset;

        std::string GetType() { return "MouseScroll"; }
    };
    class MouseMoveEvent : public Event { //Parameters: New X and Y position of mouse
    public:
        MouseMoveEvent(double x, double y) : position(x, y) {}
        MouseMoveEvent() : position(0.0) {}

        glm::dvec2 position;

        std::string GetType() { return "MouseMove"; }
    };


    //Other events
    class ImGuiDrawEvent : public Event { public: std::string GetType() { return "ImGuiDraw"; } }; //No additional parameters
}