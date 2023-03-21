#pragma once

namespace CitrusEngine {
    enum EventType {
        WindowClose, WindowResize, WindowReceiveFocus, WindowLoseFocus, //Window events
        ClientFixedTick, ClientDynamicTick, //Client events (ClientFixedTick runs at a set interval, ClientUpdate runs whenever an update occurs)
        KeyDown, KeyUp, KeyType, //Keyboard events
        MousePress, MouseRelease, MouseScroll, MouseMove //Mouse events
    }

    class Event {
    public:
        EventType type;
    }
}