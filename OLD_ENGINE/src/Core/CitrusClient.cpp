#include "Core/CacaoClient.hpp"
#include "Core/Assert.hpp"
#include "Core/Backend.hpp"
#include "Core/Log.hpp"

#include "Utilities/Utilities.hpp"
#include "Utilities/Input.hpp"

#include "ImGui/ImGuiWrapper.hpp"

#include "Graphics/Skybox.hpp"

namespace CacaoEngine {

    //Required definitions of static members
    CacaoClient* CacaoClient::instance = nullptr;
    bool CacaoClient::instanceExists = false;
    Window* CacaoClient::window = nullptr;

    CacaoClient::CacaoClient(){
        //Confirm we are initializing the first client
        Asserts::EngineAssert(!instanceExists, "Client instance already exists!");

        //Set singleton instance
        instance = this;
        instanceExists = true;

        //Allow the app to run
        run = true;

        //Set up event consumers
        wceConsumer = new EventConsumer(BIND_MEMBER_FUNC(CacaoClient::Shutdown));
        fixedTickConsumer = new EventConsumer(BIND_MEMBER_FUNC(CacaoClient::FixedTickHandler));
        dynamicTickConsumer = new EventConsumer(BIND_MEMBER_FUNC(CacaoClient::DynamicTickHandler));

        //Set up event manager and subscribe consumers
        EventManager::GetInstance()->SubscribeConsumer("WindowClose", wceConsumer);
        EventManager::GetInstance()->SubscribeConsumer("FixedTick", fixedTickConsumer);
        EventManager::GetInstance()->SubscribeConsumer("DynamicTick", dynamicTickConsumer);
    }

    //Base client does not need a destructor
    CacaoClient::~CacaoClient() {}

    void CacaoClient::Run(){
        //Initialize the backend
        if(!Backend::Initialize()){
            Logging::EngineLog(LogLevel::Fatal, "Backed initialization failed! Shutting down prematurely...");
            EventManager::GetInstance()->Shutdown();
            delete wceConsumer;
            delete fixedTickConsumer;
            delete dynamicTickConsumer;
            return;
        }

        //Create window
        window = Window::Create(windowTitle, windowSize.x, windowSize.y);

        //Initialize ImGui
        ImGuiWrapper::Init();

		//Initialize skybox resources
		Skybox::CommonSetup();

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
            EventManager::GetInstance()->Dispatch(uiDrawEvent);

            //Render drawn UI into frame
            ImGuiWrapper::ComposeFrame();

            //Clear the screen
            StateManager::GetInstance()->GetActiveCamera()->Clear();

            //Dispatch tick event
            DynamicTickEvent tickEvent{elapsed - oldElapsed};
            EventManager::GetInstance()->Dispatch(tickEvent);

            //Render ImGui frame onscreen
            ImGuiWrapper::RenderFrame();

            //Update window
            window->Update();
        }

        //Prepare event manager for freeing by unsubscribing all consumers
        EventManager::GetInstance()->Shutdown();

        //Shutdown ImGui
        ImGuiWrapper::Shutdown();

		//Cleanup skybox resources
		Skybox::CommonCleanup();

        //Close window
        window->Destroy();

        //Shutdown backend
        Backend::ShutdownRenderer();
        Backend::Shutdown();
        
        //Free pointers
        delete wceConsumer;
        delete dynamicTickConsumer;
        delete fixedTickConsumer;
    }

    void CacaoClient::Shutdown(Event& e){
        //Allow client to shut down
        ClientOnShutdown();

        //Event is handled
        e.handled = true;

        //Stop app
        run = false;
    }

    void CacaoClient::FixedTickHandler(Event& e){
        ClientOnFixedTick();
        e.handled = true;
    }

    void CacaoClient::DynamicTickHandler(Event& e){
        DynamicTickEvent cdte = Event::EventTypeCast<DynamicTickEvent>(e);
        ClientOnDynamicTick(cdte.timestep);
        e.handled = true;
    }
}