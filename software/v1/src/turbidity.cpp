#include <Arduino.h>
#include "localmath.h"
#include "constants.h"


/*
 * Function to obtain the turbidity
 * v = averaged voltage output
 * ntu = turbidity in NTU
 */
void get_turbidity(float &v, float &ntu)
{
   v = 0.0;
   ntu = 0.0;
   for (int k = 0; k < TURBIDITY_SAMPLES; k++)
   {
      int sensorValue = analogRead(TURBIDITY_SENSOR_PIN);
      float voltage = (17.0/11.0)*convert_adc_to_v(sensorValue);  // compensate for resistor-divider (11/17)
      v += voltage;
   }
   v /= static_cast<float>(TURBIDITY_SAMPLES);
   v += NTU_OFFSET_V;
   ntu = -1120.4*square(v)+5742.3*v-4353.8;
   ntu += NTU_OFFSET_VAL;
   if (ntu < 0.0) ntu = 0.0;
} // end
