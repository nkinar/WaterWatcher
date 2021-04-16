#include <Arduino.h>
#include "data_storage.h"
#include "localmath.h"
#include "constants.h"


/* 
 * Get the TDS data
 */
void get_tds(float &v)
{
  static int adc_tdr_samples[TDS_SAMPLES];
  v = 0.0;
  for(int k = 0; k < TDS_SAMPLES; k++)
  {
    int sensorValue = analogRead(TDS_SENSOR_PIN);
    adc_tdr_samples[k] = sensorValue;
  } // end
  int adc_median = findMedian(adc_tdr_samples, TDS_SAMPLES);
  v = convert_adc_to_v(adc_median);
} // end