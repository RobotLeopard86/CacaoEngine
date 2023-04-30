#pragma once

#include "Utilities/Input.h"
#include "Events/EventSystem.h"

#include <map>

namespace CitrusEngine {
    
    //GNU/Linux implementation of Input (see Input.h for method details)
    class LinuxInput : public Input {
    public:
        LinuxInput();

        glm::dvec2 GetCursorPos_Impl() override;
        bool IsKeyPressed_Impl(int key) override;
        bool IsMouseButtonPressed_Impl(int button) override;

        void CursorPosChangeHandler(Event& e);
        void KeyUpHandler(Event& e);
        void MouseButtonUpHandler(Event& e);
        void KeyDownHandler(Event& e);
        void MouseButtonDownHandler(Event& e);
    private:
        EventConsumer* cursorPosConsumer;
        EventConsumer* keyUpConsumer;
        EventConsumer* mouseButtonUpConsumer;
        EventConsumer* keyDownConsumer;
        EventConsumer* mouseButtonDownConsumer;

        glm::dvec2 cursorPos;
        std::map<int, bool> keyStateMap;
        std::map<int, bool> mouseButtonStateMap;
    };
}