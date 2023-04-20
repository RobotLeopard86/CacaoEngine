#include "Citrus.h"

class PlaygroundClient : public CitrusEngine::CitrusClient {
public:
    PlaygroundClient() { id = "citrus-playground"; }
};

CitrusEngine::CitrusClient* CreateClient() {
    return new PlaygroundClient();
}