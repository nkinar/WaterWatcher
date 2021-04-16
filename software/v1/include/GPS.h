#pragma once
#include <TinyGPS++.h>
#include <Arduino.h>

const size_t GPS_MAX_BYTES = 2048;  // 2^12
struct last_gps_fix
{
      float lat;           // latitude
      float lng;           // longitude
      float speed;         // meters per second 
      float alt;           // altitude (m)
      float height;        // height (m)
      float lat_err; 
      float long_err;
      float alt_err;  
      int hours;
      int minutes;
      int seconds;
      int microseconds;
      int day;
      int month;
      int year;
      int fix_quality;
      int satellites;
      int total_sats;
      char line[GPS_MAX_BYTES];
      bool good;
};


class GPS
{
public:
    GPS(Stream *serial);
    void start_poll();
    void end_poll();
    void run_poll();
    void read_data();
    void obtain_gps_data(struct last_gps_fix *d);
private:
    Stream *serial;
    TinyGPSPlus gps;
    bool poll;
}; // end

