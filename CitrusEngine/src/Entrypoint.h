#pragma once

#include "Core/CitrusClient.h"
#include "Core/Log.h"

int main(){
    //Setup loggers
    CitrusEngine::Logging::Setup();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Starting Citrus Engine...");

    //Create client
    CitrusEngine::CitrusClient* client = CitrusEngine::CreateClient();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Running client \"" + client->GetPackageID() + "\"...");

    client->Run();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Shutting down Citrus Engine...");

    //Free client pointer
    delete client;

    return 0;
}