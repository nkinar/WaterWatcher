#include <Arduino.h>
#include "main_local.h"
#include "WaterWatcher.h"
#include "WaterWatcherOptions.h"

WaterWatcher::WaterWatcher()
{ }

void WaterWatcher::setup(WaterWatcherOptions *opt)
{
    setup_local(opt);
} // end

void WaterWatcher::checkState()
{
    loop_local();
} // end
