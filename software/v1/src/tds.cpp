#include <Arduino.h>
#include "data_storage.h"
#include "localmath.h"
#include "constants.h"


/* 
 * Get the TDS data
 */
void get_tds(float &v, float &ppm, const float T_deg_C)
{
  static int adc_tdr_samples[TDS_SAMPLES];
  v = 0.0;
  ppm = 0.0;
  for(int k = 0; k < TDS_SAMPLES; k++)
  {
    int sensorValue = analogRead(TDS_SENSOR_PIN);
    adc_tdr_samples[k] = sensorValue;
  } // end
  int adc_median = findMedian(adc_tdr_samples, TDS_SAMPLES);
  v = convert_adc_to_v(adc_median);
  v += TDR_OFFSET_V;
  if (v < 0.0) v = 0.0;
  float tdsValue = 0.0;
  if (v > 0.0)
  {
      float compensationCoefficient = 1.0 + 0.02*(T_deg_C - 25.0);
      float compensationVoltage = v / compensationCoefficient;
      tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
  }
  ppm = tdsValue;
} // end