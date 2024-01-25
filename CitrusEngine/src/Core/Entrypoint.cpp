#include "Core/Entrypoint.hpp"
#include "Core/Log.hpp"

int main(){
    //Setup loggers
    CitrusEngine::Logging::Setup();

    //Create client
    CitrusEngine::CitrusClient* client = CreateClient();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Welcome to Citrus Engine!");

    //Run client
    client->Run();

    //Free client pointer
    delete client;

    return 0;
}