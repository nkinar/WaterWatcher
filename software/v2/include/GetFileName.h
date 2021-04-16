//------------------------------------------------------------------------------
#pragma once
#include "common_inc.h"
#include "GenUuid.h"
#include "Vector.h"
//------------------------------------------------------------------------------

static const String MONTHS[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
								  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
//------------------------------------------------------------------------------
class GetFileName
{
public:
	GetFileName();
	void set_device_name(String name) {device_name = name;}
	bool set_month(int m);
	void set_serial_number(String sn) {device_serial_number = sn;}
	void set_file_extension(String ext) {fn_ext = ext;}

	String get_file_name(Vector<String> fn);
	void set_seed(int day, int month, int year, int hour, int minute, int second);
	void set_seed();
private:
	String get_device_name() {return device_name;}
	int get_month() {return month;}
	String get_serial_number() {return device_serial_number;}
	String get_file_extension(){return fn_ext;}

	String device_name;
	int month;
	String device_serial_number;
	String fn_ext;
	GenUuid guid;
}; // end
//------------------------------------------------------------------------------


