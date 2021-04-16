#include <Arduino.h>
#include "WaterWatcherOptions.h"

WaterWatcherOptions::WaterWatcherOptions()
{
    // all channels are to sample initially by default
    sample_a0(true);
    sample_a1(true);
    sample_a2(true);
    sample_temp0(true);
    // set the raw values
    set_a0_raw(NO_SAMPLE_VALUE);
    set_a1_raw(NO_SAMPLE_VALUE);
    set_a2_raw(NO_SAMPLE_VALUE);
    set_temp0_raw(NO_SAMPLE_VALUE);
} // end

//-------------------------------------------------
// SET WHETHER VALUES SHOULD BE SAMPLED
//-------------------------------------------------
void WaterWatcherOptions::sample_a0(bool val)
{
     do_sample_a0 = val;
} // end

void WaterWatcherOptions::sample_a1(bool val)
{
    do_sample_a1 = val;
} // end

void WaterWatcherOptions::sample_a2(bool val)
{
    do_sample_a2 = val;
} // end

void WaterWatcherOptions::sample_temp0(bool val)
{
    do_sample_temp0 = val; 
} // end

//-------------------------------------------------
// SETTERS
//-------------------------------------------------
void WaterWatcherOptions::set_a0_raw(float num)
{
    a0_raw = num;
} // end

void WaterWatcherOptions::set_a1_raw(float num)
{
    a1_raw = num;
} // end

void WaterWatcherOptions::set_a2_raw(float num)
{
    a2_raw = num;
} // end

void WaterWatcherOptions::set_temp0_raw(float num)
{
    temp0_raw = num;
} // end


//-------------------------------------------------
// GETTERS
//-------------------------------------------------

float WaterWatcherOptions::get_a0_raw()
{
    return a0_raw;
} // end

float WaterWatcherOptions::get_a1_raw()
{
    return a1_raw;
} // end

float WaterWatcherOptions::get_a2_raw()
{
    return a2_raw;
} // end

float WaterWatcherOptions::get_temp0_raw()
{
    return temp0_raw;
} // end


// FUNCTIONS TO CHECK IF THE INPUT IS SAMPLING

bool WaterWatcherOptions::is_sample_a0()
{
    return do_sample_a0;
} // end


bool WaterWatcherOptions::is_sample_a1()
{
    return do_sample_a1;
} // end

bool WaterWatcherOptions::is_sample_a2()
{
    return do_sample_a2;
} // end

bool WaterWatcherOptions::is_sample_temp0()
{
    return do_sample_temp0;
} // end


//-------------------------------------------------
// TRANSFER FUNCTION OUTPUTS
// OVERRIDE THESE FUNCTIONS
//-------------------------------------------------
float WaterWatcherOptions::get_a0_out()
{
    return get_a0_raw(); 
} // end

float WaterWatcherOptions::get_a1_out()
{
    return get_a1_raw(); 
} // end
    
float WaterWatcherOptions::get_a2_out()
{
    return get_a2_raw();
} // end

float WaterWatcherOptions::get_temp0_out()
{
    return get_temp0_raw();
} // end

