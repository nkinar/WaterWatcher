#include <Arduino.h>
#include "localmath.h"
#include "constants.h"
#include "turbidity.h"


/*
 * Function to obtain the turbidity
 * v = averaged voltage output
 * ntu = turbidity in NTU
 */
void get_turbidity(float &v)
{
   v = 0.0;
   for (int k = 0; k < TURBIDITY_SAMPLES; k++)
   {
      int sensorValue = analogRead(TURBIDITY_SENSOR_PIN);
      float voltage = (17.0/11.0)*convert_adc_to_v(sensorValue);  // compensate for resistor-divider (11/17)
      v += voltage;
   }
   v /= static_cast<float>(TURBIDITY_SAMPLES);
} // end
