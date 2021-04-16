#include <Arduino.h>
#include "minmea.h"
#include "GPS.h"
#include "gpio.h"
#include "main_local.h"
#include "string_helper.h"
#include "constants.h"

struct last_gps_fix gdata;

GPS::GPS(Stream *serial)
{
      this->serial = serial;
      poll = false;
} // end


// For debugging only
void GPS::start_poll()
{
      poll = true;
} // end


// For debugging only
void GPS::end_poll()
{
      poll = false;
} // end

// For debugging only 
void GPS::run_poll()
{
      if (poll==false) return;
      if(serial->available())
      {
            char c = (char)serial->read();
            Serial.print(c);
      }
} // end


// Call this function to read the data from the GPS
// CLI: sample-gps
void GPS::read_data()
{
      gdata.good = false;
      memset(gdata.line,0,GPS_MAX_BYTES);
      serial->readBytes(gdata.line, GPS_MAX_BYTES-1);
      const char *tok = "\n\r";
      char *line_out = strtok(gdata.line, tok);
      while(line_out != NULL)
      {     
            char *line = trimwhitespace(line_out);

            #ifdef DEBUG_GPS
                  print_debug(String(line));
            #endif

            switch(minmea_sentence_id(line, false))
            {
                  case MINMEA_SENTENCE_RMC:
                  {
                         struct minmea_sentence_rmc frame;
                         if (minmea_parse_rmc(&frame, line))
                         {
                              gdata.lat = minmea_tocoord(&frame.latitude);
                              gdata.lng = minmea_tocoord(&frame.longitude);
                              gdata.speed = minmea_tofloat(&frame.speed);
                              gdata.hours = frame.time.hours;
                              gdata.minutes = frame.time.minutes;
                              gdata.seconds = frame.time.seconds;
                              gdata.microseconds = frame.time.microseconds;
                              gdata.day = frame.date.day;
                              gdata.month = frame.date.month;
                              gdata.year = frame.date.year;
                         }
                  } break;
                  case MINMEA_SENTENCE_GGA:
                  {
                        struct minmea_sentence_gga frame;
                        if (minmea_parse_gga(&frame, line))
                        {
                              gdata.lat = minmea_tocoord(&frame.latitude);
                              gdata.lng = minmea_tocoord(&frame.longitude);
                              gdata.alt = minmea_tofloat(&frame.altitude);
                              gdata.hours = frame.time.hours;
                              gdata.minutes = frame.time.minutes;
                              gdata.seconds = frame.time.seconds;
                              gdata.fix_quality = frame.fix_quality;
                              gdata.satellites = frame.satellites_tracked;
                              gdata.height = minmea_tofloat(&frame.height);
                        }
                  } break;
                  case MINMEA_SENTENCE_GSA:
                  {  
                  }
                  case MINMEA_SENTENCE_GLL:
                  {
                        struct minmea_sentence_gll frame;
                        if (minmea_parse_gll(&frame, line))
                        {
                              gdata.lat = minmea_tocoord(&frame.latitude);
                              gdata.lng = minmea_tocoord(&frame.longitude);
                              gdata.hours = frame.time.hours;
                              gdata.minutes = frame.time.minutes;
                              gdata.seconds = frame.time.seconds;
                        }
                  } break;
                  case  MINMEA_SENTENCE_GST:
                  {
                        struct minmea_sentence_gst frame;
                        if (minmea_parse_gst(&frame, line))
                        {
                              gdata.hours = frame.time.hours;
                              gdata.minutes = frame.time.minutes;
                              gdata.seconds = frame.time.seconds;
                              gdata.lat_err = minmea_tocoord(&frame.latitude_error_deviation); 
                              gdata.long_err = minmea_tocoord(&frame.longitude_error_deviation);
                              gdata.alt_err = minmea_tofloat(&frame.altitude_error_deviation);
                        }
                  }
                  case MINMEA_SENTENCE_GSV:
                  {
                      struct minmea_sentence_gsv frame;  
                      if (minmea_parse_gsv(&frame, line))
                      {
                        gdata.total_sats = frame.total_sats;
                      }
                  } break;
                  case MINMEA_SENTENCE_VTG:
                  {    
                  } break;
                  case MINMEA_SENTENCE_ZDA:
                  {
                        struct minmea_sentence_zda frame;
                        if(minmea_parse_zda(&frame, line))
                        {
                              gdata.day = frame.date.day;
                              gdata.month = frame.date.month;
                              gdata.year = frame.date.year;
                              gdata.hours = frame.time.hours;
                              gdata.minutes = frame.time.minutes;
                              gdata.seconds = frame.time.seconds;     
                        }
                  } break;
                  case MINMEA_INVALID:  // incomplete string, so do nothing
                  {
                  } break;
                  default:
                  {
                  } break;
            } // end case
            line_out = strtok(NULL, tok);  // grab the next line
      } // end while

      // check to see if the data is good
      if (gdata.satellites != 0 && !isnan(gdata.lat) && !isnan(gdata.lng)) gdata.good = true;


#ifdef DEBUG_GPS
      print_debug("GPS data:");
      print_debug("lat:");
      print_debug(String(gdata.lat));
      print_debug("lng:");
      print_debug(String(gdata.lng));
      print_debug("speed:");
      print_debug(String(gdata.speed));
      print_debug("alt:");
      print_debug(String(gdata.alt));
      print_debug("height:");
      print_debug(String(gdata.height));
      print_debug("lat_err:");
      print_debug(String(gdata.lat_err));
      print_debug("long_err:");
      print_debug(String(gdata.long_err));
      print_debug("alt_err:");
      print_debug(String(gdata.alt_err));
      print_debug("hours:");
      print_debug(String(gdata.hours));
      print_debug("minutes:");
      print_debug(String(gdata.minutes));
      print_debug("seconds:");
      print_debug(String(gdata.seconds));
      print_debug("microseconds:");
      print_debug(String(gdata.microseconds));
      print_debug("day:");
      print_debug(String(gdata.day));
      print_debug("month:");
      print_debug(String(gdata.month));
      print_debug("year:");
      print_debug(String(gdata.year));
      print_debug("fix_quality:");
      print_debug(String(gdata.fix_quality));
      print_debug("satellites:");
      print_debug(String(gdata.satellites));
      print_debug("total_sats:"); 
      print_debug(String(gdata.total_sats));
      print_debug("good:");
      print_debug(String(gdata.good));
#endif
} // end


/*
Obtain the last GPS data for use elsewhere in the program
*/
void GPS::obtain_gps_data(struct last_gps_fix *d)
{
      *d = gdata;
} // end
