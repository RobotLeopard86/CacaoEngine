#include "Event.h"
#include "Core/Log.h"

namespace CitrusEngine {

    std::function<void(Event&)> Event::dispatchTarget = nullptr;
    std::function<void(Event&)> EventHandler::fallback = nullptr;

    EventType Event::type = EventType::None;
    EventType WindowCloseEvent::type = EventType::WindowClose;
    EventType WindowResizeEvent::type = EventType::WindowResize;
    EventType WindowRecieveFocusEvent::type = EventType::WindowRecieveFocus;
    EventType WindowLoseFocusEvent::type = EventType::WindowLoseFocus;
    EventType ClientFixedTickEvent::type = EventType::ClientFixedTick;
    EventType ClientDynamicTickEvent::type = EventType::ClientDynamicTick;
    EventType KeyDownEvent::type = EventType::KeyDown;
    EventType KeyUpEvent::type = EventType::KeyUp;
    EventType KeyTypeEvent::type = EventType::KeyType;
    EventType MousePressEvent::type = EventType::MousePress;
    EventType MouseReleaseEvent::type = EventType::MouseRelease;
    EventType MouseScrollEvent::type = EventType::MouseScroll;
    EventType MouseMoveEvent::type = EventType::MouseMove;

    Event::Event(){
        handled = false;
    }

    void EventHandler::RegisterCallback(EventType type, std::function<void(Event&)> callback){
        //Returns std::pair<std::pair<EventType, std::function<void(Event&)>>, bool> (too long to write out)
        const auto mapOperationResult = callbacks.insert_or_assign(type, callback);
        //Check if second component is true (value overriden rather than inserted)
        if(mapOperationResult.second){
            Logging::EngineLog(LogLevel::Warn, "Overriding event handler callback...");
        }
    }

    void EventHandler::RegisterFallbackCallback(std::function<void(Event&)> callback){
        if(fallback != nullptr){
            Logging::EngineLog(LogLevel::Warn, "Overriding event handler fallback callback...");
        }
        fallback = callback;
    }

    void EventHandler::Handle(Event& event){
        if(callbacks.contains(event.type)){
            if (auto search = callbacks.find(event.type); search != callbacks.end()){
                search(event);
            }
        } else {
            fallback(event);   
        }
    }
}