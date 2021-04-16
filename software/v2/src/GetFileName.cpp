#include "GetFileName.h"
#include "Vector.h"

GetFileName::GetFileName()
{
	device_name = "NONAME";
	month = 1;
	device_serial_number = "SN";
	fn_ext = ".txt";
} // end


// set the seed using manual time
void GetFileName::set_seed(int day, int month, int year, int hour, int minute, int second)
{
	guid.set_seed(day, month, year, hour, minute, second);
	this->month = month;
} // end


// set the seed using the time
void GetFileName::set_seed()
{
	guid.set_seed_time();
} // end


// Set the month
bool GetFileName::set_month(int m)
{
	if (m >= 1 && m <= 12)
	{
		month = m;
		return true;
	}
	return false;
} // end


// Obtain the file name with UUID
// Before this function is called, the seed must be set using the set_seed() function
// fn = a vector with a list of file names in the directory
String GetFileName::get_file_name(Vector<String> fn)
{
	const String dot = ".";
	const String dash = "-";
	String out = device_name + dash + device_serial_number + dash + MONTHS[month-1] + dash;
	// Check to see if part of the string is present in the file list passed in to the function.
	// If the string is already present in the file list, then we return a stripped version of the string.
	int n = fn.size();
	for(int k = 0; k < n; k++)
	{
		#ifndef ARDUINO_CODE
			int found = fn[k].find(out);
		#else  // on Arduino
			int found  = fn[k].indexOf(out);
		#endif
			if (found != -1)
			{
				String uuid;
				// extract the UUID and send this back along with the rest of the string and the extension
				#ifndef ARDUINO_CODE
					int first = fn[k].find_last_of(dash);
					int second = fn[k].find_last_of(dot);
				#else  // Arduino
					int first = fn[k].lastIndexOf(dash);
					int second = fn[k].lastIndexOf(dot);
				#endif
					first += 1;
					if((first != -1) && (second != -1) && (first < second))
					{
						int diff = second-first;
						#ifndef ARDUINO_CODE
							uuid = fn[k].substr(first, diff);
						#else // Arduino
							uuid = fn[k].substring(first, diff);
						#endif
					}
				return out + uuid + fn_ext;
			}
	}
	// If the string is not present, then we add on the UUID and the text file extension
	out += guid.get_uuid(GEN_UUID_DO_NOT_ADD_HYPEN) + fn_ext;
	return out;
} // end


