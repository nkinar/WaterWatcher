#include <cstdint>
#include <ctime>
#include "GenUuid.h"
#include "../include/common_inc.h"

//------------------------------------------------------------------------------------------------
/*
 *  Generates a UUID
 * 	REFERENCE:  https://www.cryptosys.net/pki/uuid-rfc4122.html
 */
//------------------------------------------------------------------------------------------------


// Constructor
GenUuid::GenUuid()
{
	seed = 1;  // set default to something
} // end


// Set the seed from RTC information
void GenUuid::set_seed(int day, int month, int year, int hour, int minute, int second)
{
	seed = get_time_number(day, month, year, hour, minute, second);
	std::srand(static_cast<unsigned int>(seed));
} // end


// Set time seed from C time source
void GenUuid::set_seed_time()
{
	std::time_t rawtime;
	std::time( &rawtime );
	seed = rawtime;
	#ifdef ARDUINO_CODE
		randomSeed(seed); 
	#else
		std::srand(static_cast<unsigned int>(seed));
	#endif
} // end


// obtain time as a number
std::time_t GenUuid::get_time_number(int day, int month, int year, int hour, int minute, int second)
{
	std::tm current;
	current.tm_sec = second;
	current.tm_min = minute;
	current.tm_hour = hour;
	current.tm_mday = day;
	current.tm_mon = month;
	current.tm_year = year - 1900;
	current.tm_isdst = -1;
	std::time_t base = std::mktime(&current);
	return base;
} // end


// obtain a random byte as seeded by the time generator
uint8_t GenUuid::obtain_rand_byte()
{
	#ifndef ARDUINO_CODE
		return static_cast<uint8_t>(std::rand());
	#else
		return static_cast<uint8_t>(random(0, 256));  // ensure single byte number
	#endif
} // end


// obtain the UUID
String GenUuid::get_uuid(bool add_hyphen)
{
	// lookup table for converting nibbles to hex
	char lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	uint8_t b[16];		// random bytes
	for(int i = 0; i < 16; i++)
	{
		b[i] = obtain_rand_byte();
	}
	b[6] &= 0b00001111;
	b[6] |= 0b01000000;

	b[8] &= 0b00111111;
	b[8] |= 0b10000000;

	String s = "";
	for(int k = 0; k < 16; k++)
	{
		uint8_t bin = b[k];
		uint8_t high = bin >> 4u;
		uint8_t low = bin & 0x0F;
		s += String(lookup[high]);
		s += String(lookup[low]);
		if (add_hyphen)
			if ((k==3) || (k==5) || (k==7) || (k==9))
				s += '-';
	}
	return s;
} // end
























