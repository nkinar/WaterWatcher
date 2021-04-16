#pragma once

#ifdef ARDUINO_CODE
  #include <Arduino.h>
  #include <string.h>
#else  //  use standard library
  #include <string>
  typedef std::string String;
#endif

template <typename, typename> struct is_same { static const bool value = false;};
template <typename T> struct is_same<T,T> { static const bool value = true;};
#define BOOL_TRUE 	"true"
#define BOOL_FALSE	"false"
#ifndef ARDUINO_CODE
	// standard C++ code
	#define TO_STRING  std::to_string
	#define FIND_LAST_INDEX	find_last_of
	#define SUBSTR	substr
	#define ERASE erase
#else
	// Arduino code
	#define TO_STRING String
	#define FIND_LAST_INDEX	lastIndexOf
	#define SUBSTR	substring
	#define ERASE remove

	// provide the gcvt function for Arduino code
	void inline gcvt(float value, size_t length, char *buff)
	{
		sprintf(buff,"%.*g",length,value);
	}
#endif

