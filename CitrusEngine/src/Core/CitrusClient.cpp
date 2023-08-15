#include "CitrusClient.h"
#include "Assert.h"
#include "Log.h"
#include "Utilities/Utilities.h"
#include "Graphics/Renderer.h"
#include "Utilities/Input.h"
#include "ImGui/ImGuiWrapper.h"
#include "Backend.h"

namespace CitrusEngine {

    //Required definitions of static members
    CitrusClient* CitrusClient::instance = nullptr;
    bool CitrusClient::instanceExists = false;
    EventManager* CitrusClient::eventManager = nullptr;
    Window* CitrusClient::window = nullptr;

    CitrusClient::CitrusClient(){
        //Confirm we are initializing the first client
        Asserts::EngineAssert(!instanceExists, "Client instance already exists!");

        //Set singleton instance
        instance = this;
        instanceExists = true;

        //Allow the app to run
        run = true;

        //Set up event consumers
        wceConsumer = new EventConsumer(BIND_MEMBER_FUNC(CitrusClient::Shutdown));
        fixedTickConsumer = new EventConsumer(BIND_MEMBER_FUNC(CitrusClient::FixedTickHandler));
        dynamicTickConsumer = new EventConsumer(BIND_MEMBER_FUNC(CitrusClient::DynamicTickHandler));

        //Set up event manager and subscribe consumers
        eventManager = new EventManager();
        eventManager->SubscribeConsumer("WindowClose", wceConsumer);
        eventManager->SubscribeConsumer("FixedTick", fixedTickConsumer);
        eventManager->SubscribeConsumer("DynamicTick", dynamicTickConsumer);
    }

    //Base client does not need a destructor
    CitrusClient::~CitrusClient() {}

    void CitrusClient::Run(){
        //Initialize the backend
        if(!Backend::Initialize()){
            Logging::EngineLog(LogLevel::Fatal, "Backed initialization failed! Shutting down prematurely...");
            eventManager->Shutdown();
            delete eventManager;
            delete wceConsumer;
            delete fixedTickConsumer;
            delete dynamicTickConsumer;
            return;
        }

        //Create window
        window = Window::Create(id, windowSize.x, windowSize.y);

        //Initialize ImGui
        ImGuiWrapper::Init();

        //Allow client to set up
        ClientOnStartup();

        double elapsed = 0;

        while(run){
            //Calculate timestep since last update
            double oldElapsed = elapsed; 
            elapsed = Utilities::GetInstance()->GetElapsedTime();

            //Create ImGui frame
            ImGuiWrapper::CreateFrame();

            //Dispatch ImGui draw event
            ImGuiDrawEvent uiDrawEvent{};
            eventManager->Dispatch(uiDrawEvent);

            //Render drawn UI into frame
            ImGuiWrapper::ComposeFrame();

            //Clear the screen
            Renderer::GetInstance()->Clear();

            //Dispatch tick event
            DynamicTickEvent tickEvent{elapsed - oldElapsed};
            eventManager->Dispatch(tickEvent);

            //Render ImGui frame onscreen
            ImGuiWrapper::RenderFrame();

            //Update window
            window->Update();
        }

        //Prepare eventManager for freeing by unsubscribing all consumers
        eventManager->Shutdown();

        //Shutdown ImGui
        ImGuiWrapper::Shutdown();

        //Close window
        window->Destroy();
        
        //Free pointers
        delete eventManager;
        delete wceConsumer;
        delete dynamicTickConsumer;
        delete fixedTickConsumer;
    }

    void CitrusClient::Shutdown(Event& e){
        //Allow client to shut down
        ClientOnShutdown();

        //Event is handled
        e.handled = true;

        //Stop app
        run = false;
    }

    void CitrusClient::FixedTickHandler(Event& e){
        ClientOnFixedTick();
        e.handled = true;
    }

    void CitrusClient::DynamicTickHandler(Event& e){
        DynamicTickEvent cdte = Event::EventTypeCast<DynamicTickEvent>(e);
        ClientOnDynamicTick(cdte.timestep);
        e.handled = true;
    }
}