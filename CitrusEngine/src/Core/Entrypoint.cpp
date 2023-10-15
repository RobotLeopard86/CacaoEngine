#include "Core/Entrypoint.hpp"
#include "Core/Log.hpp"

int main(){
    //Setup loggers
    CitrusEngine::Logging::Setup();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Starting Citrus Engine...");

    //Create client
    CitrusEngine::CitrusClient* client = CreateClient();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Running client \"" + client->GetPackageID() + "\"...");

    //Run client
    client->Run();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Shutting down Citrus Engine...");

    //Free client pointer
    delete client;

    return 0;
}