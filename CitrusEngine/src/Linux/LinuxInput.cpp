#include "LinuxInput.h"

#include "Core/CitrusClient.h"
#include "Utilities/Utilities.h"

namespace CitrusEngine {

    Input* Input::CreateNativeInput(){
        return new LinuxInput();
    }

    LinuxInput::LinuxInput() {
        //Subscribe consumers for input state change events
        cursorPosConsumer = new EventConsumer(BIND_MEMBER_FUNC(LinuxInput::CursorPosChangeHandler));
        keyUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(LinuxInput::KeyUpHandler));
        mouseButtonUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(LinuxInput::MouseButtonUpHandler));
        keyDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(LinuxInput::KeyDownHandler));
        mouseButtonDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(LinuxInput::MouseButtonDownHandler));
        CitrusClient::GetEventManager()->SubscribeConsumer("MouseMove", cursorPosConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("KeyUp", keyUpConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("MouseRelease", mouseButtonUpConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("KeyDown", keyDownConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("MousePress", mouseButtonDownConsumer);

        //Initialize cursor pos vector
        cursorPos = glm::dvec2(0.0);
    }

    bool LinuxInput::IsKeyPressed_Impl(int key){
        if(keyStateMap.find(key) != keyStateMap.end()){
            return keyStateMap.at(key);
        }
        return false;
    }

    bool LinuxInput::IsMouseButtonPressed_Impl(int button){
        if(mouseButtonStateMap.find(button) != mouseButtonStateMap.end()){
            return mouseButtonStateMap.at(button);
        }
        return false;
    }

    glm::dvec2 LinuxInput::GetCursorPos_Impl(){
        return cursorPos;
    }

    void LinuxInput::CursorPosChangeHandler(Event& e){
        MouseMoveEvent mme = Event::EventTypeCast<MouseMoveEvent>(e);
        cursorPos = mme.position;
    }

    void LinuxInput::KeyUpHandler(Event& e){
        KeyUpEvent kue = Event::EventTypeCast<KeyUpEvent>(e);
        keyStateMap.insert_or_assign(kue.keycode, false);
    }

    void LinuxInput::KeyDownHandler(Event& e){
        KeyDownEvent kde = Event::EventTypeCast<KeyDownEvent>(e);
        keyStateMap.insert_or_assign(kde.keycode, true);
    }

    void LinuxInput::MouseButtonUpHandler(Event& e){
        MouseReleaseEvent mre = Event::EventTypeCast<MouseReleaseEvent>(e);
        mouseButtonStateMap.insert_or_assign(mre.button, false);
    }

    void LinuxInput::MouseButtonDownHandler(Event& e){
        MousePressEvent mpe = Event::EventTypeCast<MousePressEvent>(e);
        mouseButtonStateMap.insert_or_assign(mpe.button, true);
    }
}