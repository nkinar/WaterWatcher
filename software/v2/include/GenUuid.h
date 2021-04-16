#pragma once
#include "../include/common_inc.h"
#include <ctime>

const bool GEN_UUID_DO_NOT_ADD_HYPEN = false;
const bool GEN_UUID_ADD_HYPEN = true;
class GenUuid
{
public:
	GenUuid();
	void set_seed(int day, int month, int year, int hour, int minute, int second);
	void set_seed_time();
	String get_uuid(bool add_hyphen=true);
private:
	std::time_t get_time_number(int day, int month, int year, int hour, int minute, int second);
	uint8_t obtain_rand_byte();
	std::time_t seed;
}; // end
