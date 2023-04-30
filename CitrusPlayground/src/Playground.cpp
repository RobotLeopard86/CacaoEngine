#include "Citrus.h"

class PlaygroundClient : public CitrusEngine::CitrusClient {
public:
    PlaygroundClient() { id = "citrus-playground"; }

    void ClientOnStartup() override {}
    void ClientOnShutdown() override {}
    void ClientOnDynamicTick(double timestep) override {}
    void ClientOnFixedTick() override {}
};

CitrusEngine::CitrusClient* CreateClient() {
    return new PlaygroundClient();
}