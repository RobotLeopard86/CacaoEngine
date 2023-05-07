#pragma once

#include "Utilities/Utilities.h"

namespace CitrusEngine {
    
    //GNU/Linux implementation of Utilities (see Utilities.h for method details)
    class LinuxUtilities : public Utilities {
    public:
        LinuxUtilities() {}

        double GetElapsedTime() override;
    };
}