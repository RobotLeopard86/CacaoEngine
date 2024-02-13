#include "Core/Entrypoint.hpp"
#include "Core/Log.hpp"

int main(int argc, char** argv){
    //Setup loggers
    CacaoEngine::Logging::Setup();

    //Create client
    CacaoEngine::CacaoClient* client = CreateClient();

    CacaoEngine::Logging::EngineLog(CacaoEngine::LogLevel::Info, "Welcome to Cacao Engine!");

    //Run client
    client->Run();

    //Free client pointer
    delete client;

    return 0;
}