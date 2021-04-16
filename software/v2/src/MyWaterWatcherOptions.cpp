#include "WaterWatcherOptions.h"

class MyWaterWatcherOptions : public WaterWatcherOptions
{
    public:

        MyWaterWatcherOptions()
        {
            sample_a0(true);        // sample a0
            sample_a1(true);        // sample a1
            sample_a2(false);       // do not sample a2
            sample_temp0(true);     // sample temp0
        } // end

        //-------------------------------------------------
        // TRANSFER FUNCTION OUTPUTS
        // OVERRIDE THESE FUNCTIONS
        //-------------------------------------------------
        float get_a0_out()
        {
            // Calibration calculation can be placed in here as a return value
            float out = get_a0_raw(); 
            return out;
        } // end

        float get_a1_out()
        {
            // Calibration calculation can be placed in here as a return value
            float out = get_a1_raw(); 
            return out;
        } // end
            
        float get_a2_out()
        {
            // Calibration calculation can be placed in here as a return value
            float out = get_a2_raw(); 
            return out;
        } // end
        
        float get_temp0_out()
        {
            // Calibration calculation can be placed in here as a return value
            float out = get_temp0_raw(); 
            return out;
        } // end

}; // end
