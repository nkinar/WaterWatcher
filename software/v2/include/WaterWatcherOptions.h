#pragma once

// This is the default no sample value and it can be overriden if required.
const float NO_SAMPLE_VALUE = -127.0;

class WaterWatcherOptions
{
public:
    WaterWatcherOptions();
    // Functions to set to sample voltage inputs
    // val == true to sample
    // val == false to not sample
    void sample_a0(bool val);
    void sample_a1(bool val);
    void sample_a2(bool val);
    void sample_temp0(bool val);
    // Functions to set the raw voltage input values (need to be called before transfer functions)
    void set_a0_raw(float num);
    void set_a1_raw(float num);
    void set_a2_raw(float num);
    void set_temp0_raw(float num);
    // Functions to get the raw voltage input values (used in the transfer functions)
    float get_a0_raw();
    float get_a1_raw();
    float get_a2_raw();
    float get_temp0_raw();
    // Functions to get the outputs (override these functions)
    float get_a0_out();
    float get_a1_out();
    float get_a2_out();
    float get_temp0_out();
    // Functions to check for sampling 
    bool is_sample_a0();
    bool is_sample_a1();
    bool is_sample_a2();
    bool is_sample_temp0();

private:
float a0_raw;
float a1_raw;
float a2_raw;
float temp0_raw;

bool do_sample_a0;
bool do_sample_a1;
bool do_sample_a2;
bool do_sample_temp0;
}; // end

