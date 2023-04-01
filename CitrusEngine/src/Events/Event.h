#pragma once

#include <map>
#include <utility>
#include <functional>

//Allows conversion of a method into a std::function for binding
#define BIND_FUNC(f) std::bind(&f, this, std::placeholders::_1)

namespace CitrusEngine {
    enum EventType {
        None, //Base event type
        WindowClose, WindowResize, WindowReceiveFocus, WindowLoseFocus, //Window events
        ClientFixedTick, ClientDynamicTick, //Client events (ClientFixedTick runs at a set interval, ClientDynamicTick runs whenever an update occurs)
        KeyDown, KeyUp, KeyType, //Keyboard events
        MousePress, MouseRelease, MouseScroll, MouseMove //Mouse events
    };

    class Event {
    public:
        Event();

        bool handled;

        const static void SetDispatchMethod(std::function<void(Event&)> callback){
            dispatchTarget = callback;
        }

        const void Dispatch(){
            dispatchTarget(*this);
        }

        static EventType type;
    private:
        //All events will be dispatched to this function
        static std::function<void(Event&)> dispatchTarget;
    };

    class EventHandler {
    public:
        EventHandler();

        //Registers a callback to be called if the given event type is found.
        void RegisterCallback(EventType type, std::function<void(Event&)> callback);

        //Registers a callback to be called if an incoming event's type does not match any other callback
        void RegisterFallbackCallback(std::function<void(Event&)> callback);

        //Invokes the callback for the appropriate event type
        void Handle(Event& event);
    private:
        //Map of EventTypes to callbacks
        std::map<EventType, std::function<void(Event&)>> callbacks;
        //Fallback callback
        std::function<void(Event&)> fallback;
    };

    //Window events
    class WindowCloseEvent : public Event { public: static EventType type; }; //No additional parameters
    class WindowReceiveFocusEvent : public Event { public: static EventType type; }; //No additional parameters
    class WindowLoseFocusEvent : public Event { public: static EventType type; }; //No additional parameters
    class WindowResizeEvent : public Event { //Parameters: New X and Y size of window
    public:
        WindowResizeEvent(int x, int y) : x(x), y(y) {}
        int x, y;

        static EventType type;
    };

    //Client events
    class ClientFixedTickEvent : public Event { public: static EventType type; }; //No additional parameters
    class ClientDynamicTickEvent : public Event { //Parameters: Delta time since last update
    public:
        ClientDynamicTickEvent(double timestep) : timestep(timestep) {}
        double timestep;

        static EventType type;
    };

    //Key events
    class KeyDownEvent : public Event { //Parameters: Keycode for key pressed
    public:
        KeyDownEvent(int keycode) : keycode(keycode) {}
        int keycode;

        static EventType type;
    };
    class KeyUpEvent : public Event { //Parameters: Keycode for key released
    public:   
        KeyUpEvent(int keycode) : keycode(keycode) {}
        int keycode;

        static EventType type;
    };
    class KeyTypeEvent : public Event { //Parameters: Keycode for key typed
    public:    
        KeyTypeEvent(int keycode) : keycode(keycode) {}
        int keycode;

        static EventType type;
    };

    //Mouse events
    class MousePressEvent : public Event { //Parameters: Button code for button pressed
    public:
        MousePressEvent(int button) : button(button) {}
        int button;

        static EventType type;
    };
    class MouseReleaseEvent : public Event { //Parameters: Button code for button released
    public:
        MouseReleaseEvent(int button) : button(button) {}
        int button;

        static EventType type;
    };
    class MouseScrollEvent : public Event { //Parameters: X and Y offset for scrolling
    public:
        MouseScrollEvent(int xOffset, int yOffset) : xOffset(xOffset), yOffset(yOffset) {}
        double xOffset, yOffset;

        static EventType type;
    };
    class MouseMoveEvent : public Event { //Parameters: New X and Y position of mouse
    public:
        MouseMoveEvent(int x, int y) : x(x), y(y) {}
        double x, y;

        static EventType type;
    };
}