#include "Utilities/Input.hpp"

#include "Utilities/MiscUtils.hpp"

namespace Citrus {
    //Make input instance null pointer by default
    Input* Input::instance = nullptr;
    //We don't have an instance by default
    bool Input::instanceExists = false;

    Input* Input::GetInstance() {
        //Do we have a input instance yet?
        if(!instanceExists || instance == NULL){
            //Create input instance
            instance = new Input();
            instanceExists = true;
        }

        return instance;
    }

    Input::Input() 
		: cursorPos(_cursorPos), keyStateMap(_keyStateMap), mouseButtonStateMap(_mouseButtonStateMap) {
        //Subscribe consumers for input state change events
        cursorPosConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::CursorPosChangeHandler));
        keyUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::KeyUpHandler));
        mouseButtonUpConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::MouseButtonUpHandler));
        keyDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::KeyDownHandler));
        mouseButtonDownConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::MouseButtonDownHandler));
		inputFlushConsumer = new EventConsumer(BIND_MEMBER_FUNC(Input::InputFlushHandler));
        EventManager::GetInstance()->SubscribeConsumer("MouseMove", cursorPosConsumer);
        EventManager::GetInstance()->SubscribeConsumer("KeyUp", keyUpConsumer);
        EventManager::GetInstance()->SubscribeConsumer("MouseRelease", mouseButtonUpConsumer);
        EventManager::GetInstance()->SubscribeConsumer("KeyDown", keyDownConsumer);
        EventManager::GetInstance()->SubscribeConsumer("MousePress", mouseButtonDownConsumer);

        //Initialize cursor pos vector
        _cursorPos = glm::dvec2(0.0);
    }

    bool Input::IsKeyPressed(int key){
        if(_keyStateMap.find(key) != _keyStateMap.end()){
            return _keyStateMap.at(key);
        }
        return false;
    }

    bool Input::IsMouseButtonPressed(int button){
        if(_mouseButtonStateMap.find(button) != _mouseButtonStateMap.end()){
            return _mouseButtonStateMap.at(button);
        }
        return false;
    }

    glm::dvec2 Input::GetCursorPos(){
        return _cursorPos;
    }

    void Input::CursorPosChangeHandler(Event& e){
		DataEvent<glm::vec2>& mme = static_cast<DataEvent<glm::vec2>&>(e);
        cursorPos = mme.GetData();
    }

    void Input::KeyUpHandler(Event& e){
		DataEvent<int>& kue = static_cast<DataEvent<int>&>(e);
        keyStateMap->insert_or_assign(kue.GetData(), false);
    }

    void Input::KeyDownHandler(Event& e){
		DataEvent<int>& kde = static_cast<DataEvent<int>&>(e);
        keyStateMap->insert_or_assign(kde.GetData(), true);
    }

    void Input::MouseButtonUpHandler(Event& e){
		DataEvent<int>& mre = static_cast<DataEvent<int>&>(e);
        mouseButtonStateMap->insert_or_assign(mre.GetData(), false);
    }

    void Input::MouseButtonDownHandler(Event& e){
		DataEvent<int>& mpe = static_cast<DataEvent<int>&>(e);
        mouseButtonStateMap->insert_or_assign(mpe.GetData(), true);
    }

	void Input::FreezeFrameInputState(){
		cursorPos.Flush();
		keyStateMap.Flush();
		mouseButtonStateMap.Flush();
	}
}