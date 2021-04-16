#pragma once
#include "WaterWatcherOptions.h"

class WaterWatcher
{
    public:
        WaterWatcher();
        void setup(WaterWatcherOptions *opt);
        void checkState();
    private:
}; // end
