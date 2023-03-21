#include "Citrus.h"

class PlaygroundClient : public CitrusEngine::CitrusClient {
public:
    PlaygroundClient() { id = "playground"; }
};

CitrusEngine::CitrusClient* CitrusEngine::CreateClient() {
    CitrusEngine::Logging::ClientLog(CitrusEngine::LogLevel::Info, "Creating client...");
    return new PlaygroundClient();
}