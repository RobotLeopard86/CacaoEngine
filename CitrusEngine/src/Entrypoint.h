#pragma once

#include "CitrusClientBridge.h"
#include "Core/Log.h"

//Function declared in CitrusClientBridge.h, implemented by client
extern int CitrusEngine::CreateClient();

int main(){
    //Setup loggers
    CitrusEngine::Logging::Setup();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Starting Citrus Engine...");

    //Create client
    int appId = CitrusEngine::CreateClient();

    CitrusEngine::Logging::EngineLog(CitrusEngine::LogLevel::Info, "Received client!");

    return 0;
}