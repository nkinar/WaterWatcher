#include <Arduino.h>
#include <ArduinoSort.h>
#include "localmath.h"
#include "constants.h"


/*
 * Round to decimal point function
 */
float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
} // end


/* 
 * Function to take the square
 */
float square(float n)
{
  return n * n;
} // end


/*
 * Convert an ADC code to a voltage
 */
float convert_adc_to_v(int sensorValue)
{
  float voltage = sensorValue * (SENSOR_V / ANALOG_MAX);
  return voltage;
} // end



/*
 * Function to find the median of an array from the ADC
 * REQUIREMENTS: sortArray() from the ArduinoSort library
 */
float findMedian(int a[], int n) 
{ 
    sortArray(a, n); 
    if (n % 2 != 0) 
       return (float)a[n/2]; 
    return (float)(a[(n-1)/2] + a[n/2])/2.0; 
} // end