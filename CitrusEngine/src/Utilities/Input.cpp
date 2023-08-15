#include "Input.h"

#include "Core/CitrusClient.h"

#include "Utilities/Utilities.h"

namespace CitrusEngine {
    //Make input instance null pointer by default
    Input* Input::instance = nullptr;
    //We don't have an instance by default
    bool Input::instanceExists = false;

    Input* Input::GetInstance() {
        //Do we have a input instance yet?
        if(!instanceExists){
            //Create input instance
            instance = new Input();
            instanceExists = true;
        }

        return instance;
    }

    Input::Input() {
        //Subscribe consumers for input state change events
        cursorPosConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::CursorPosChangeHandler));
        keyUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::KeyUpHandler));
        mouseButtonUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::MouseButtonUpHandler));
        keyDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::KeyDownHandler));
        mouseButtonDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::MouseButtonDownHandler));
        CitrusClient::GetEventManager()->SubscribeConsumer("MouseMove", cursorPosConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("KeyUp", keyUpConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("MouseRelease", mouseButtonUpConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("KeyDown", keyDownConsumer);
        CitrusClient::GetEventManager()->SubscribeConsumer("MousePress", mouseButtonDownConsumer);

        //Initialize cursor pos vector
        cursorPos = glm::dvec2(0.0);
    }

    bool Input::IsKeyPressed(int key){
        if(keyStateMap.find(key) != keyStateMap.end()){
            return keyStateMap.at(key);
        }
        return false;
    }

    bool Input::IsMouseButtonPressed(int button){
        if(mouseButtonStateMap.find(button) != mouseButtonStateMap.end()){
            return mouseButtonStateMap.at(button);
        }
        return false;
    }

    glm::dvec2 Input::GetCursorPos(){
        return cursorPos;
    }

    void Input::CursorPosChangeHandler(Event& e){
        MouseMoveEvent mme = Event::EventTypeCast<MouseMoveEvent>(e);
        cursorPos = mme.position;
    }

    void Input::KeyUpHandler(Event& e){
        KeyUpEvent kue = Event::EventTypeCast<KeyUpEvent>(e);
        keyStateMap.insert_or_assign(kue.keycode, false);
    }

    void Input::KeyDownHandler(Event& e){
        KeyDownEvent kde = Event::EventTypeCast<KeyDownEvent>(e);
        keyStateMap.insert_or_assign(kde.keycode, true);
    }

    void Input::MouseButtonUpHandler(Event& e){
        MouseReleaseEvent mre = Event::EventTypeCast<MouseReleaseEvent>(e);
        mouseButtonStateMap.insert_or_assign(mre.button, false);
    }

    void Input::MouseButtonDownHandler(Event& e){
        MousePressEvent mpe = Event::EventTypeCast<MousePressEvent>(e);
        mouseButtonStateMap.insert_or_assign(mpe.button, true);
    }
}