#pragma once
// REFERENCE: https://codereview.stackexchange.com/questions/60484/stl-vector-implementation
// With edits by nj kinar
#include <cmath>
#include "common_inc.h"

template <class T>
class  Vector {
public:

	typedef T* Iterator;

	Vector();
	Vector(unsigned int size);
	Vector(unsigned int size, const T & initial);
	Vector(const Vector<T>& v);
	~Vector();

	unsigned int capacity() const;
	unsigned int size() const;
	bool empty() const;
	Iterator begin();
	Iterator end();
	T& front();
	T& back();
	void push_back(const T& value);
	void pop_back();
	void remove(size_t index);

	void reserve(unsigned int capacity);
	void resize(unsigned int size);

	T & operator[](unsigned int index);
	Vector<T> & operator = (const Vector<T> &);
	void clear();

	String to_String();
	bool from_String(String s);

	Vector<String> split(String str, char c);

private:
	unsigned int crc32b(const char *message);
	unsigned int _size;
	unsigned int _capacity;
	unsigned int Log;
	T* buffer;
};

template<class T>
Vector<T>::Vector() {
	_capacity = 0;
	_size = 0;
	buffer = 0;
	Log = 0;
}


// NK: Remove one element function
// Added 15 May 2020 by nkinar
template<class T>
void Vector<T>::remove(size_t index)
{
	if (index >= this->size()) return;
	size_t nn = this->size() - 1;
	Vector<T> nv(nn);
	for(size_t k = 0; k < index; k++)
	{
		nv[k] = buffer[k];
	}
	for(size_t k = index+1; k < this->size(); k++)
	{
		nv[k-1] = buffer[k];
	}
	*this = nv;  // assign the new vector
} // end


template<class T>
Vector<T>::Vector(const Vector<T> & v) {
	_size = v._size;
	Log = v.Log;
	_capacity = v._capacity;
	buffer = new T[_size];
	for (unsigned int i = 0; i < _size; i++)
		buffer[i] = v.buffer[i];
}

template<class T>
Vector<T>::Vector(unsigned int size) {
	_size = size;
	Log = ceil(log((double) size) / log(2.0));
	_capacity = 1 << Log;
	buffer = new T[_capacity];
}

template <class T>
bool Vector<T>:: empty() const {
	return _size == 0;
}

template<class T>
Vector<T>::Vector(unsigned int size, const T& initial) {
	_size = size;
	Log = ceil(log((double) size) / log(2.0));
	_capacity = 1 << Log;
	buffer = new T [_capacity];
	for (unsigned int i = 0; i < size; i++)
		buffer[i] = initial;
}

template<class T>
Vector<T>& Vector<T>::operator = (const Vector<T> & v) {
	delete[] buffer;
	_size = v._size;
	Log = v.Log;
	_capacity = v._capacity;
	buffer = new T [_capacity];
	for (unsigned int i = 0; i < _size; i++)
		buffer[i] = v.buffer[i];
	return *this;
}

template<class T>
typename Vector<T>::Iterator Vector<T>::begin() {
	return buffer;
}

template<class T>
typename Vector<T>::Iterator Vector<T>::end() {
	return buffer + size();
}

template<class T>
T& Vector<T>::front() {
	return buffer[0];
}

template<class T>
T& Vector<T>::back() {
	return buffer[_size - 1];
}

template<class T>
void Vector<T>::push_back(const T & v) {
	/*
		Incidentally, one common way of regrowing an array is to double the size as needed.
		This is so that if you are inserting n items at most only O(log n) regrowths are performed
		and at most O(n) space is wasted.
	*/
	if (_size >= _capacity) {
		reserve(1 << Log);
		Log++;
	}
	buffer [_size++] = v;
}

template<class T>
void Vector<T>::pop_back() {
	_size--;
}

template<class T>
void Vector<T>::reserve(unsigned int capacity) {
	T * newBuffer = new T[capacity];

	for (unsigned int i = 0; i < _size; i++)
		newBuffer[i] = buffer[i];

	_capacity = capacity;
	delete[] buffer;
	buffer = newBuffer;
}

template<class T>
unsigned int Vector<T>::size() const {
	return _size;
}

template<class T>
void Vector<T>::resize(unsigned int size) {
	Log = ceil(log((double) size) / log(2.0));
	reserve(1 << Log);
	_size = size;
}

template<class T>
T& Vector<T>::operator[](unsigned int index) {
	return buffer[index];
}

template<class T>
unsigned int Vector<T>::capacity()const {
	return _capacity;
}

template<class T>
Vector<T>::~Vector() {
	delete[] buffer;
}

template <class T>
void Vector<T>::clear() {
	_capacity = 0;
	_size = 0;
	buffer = 0;
	Log = 0;
}

//-------------------------------------------------------------------------
const static char V_SEP_CHAR = 0x1B;  // alert character to use as string separator

// REFERENCE: https://stackoverflow.com/questions/21001659/crc32-algorithm-implementation-in-c-without-a-look-up-table-and-with-a-public-li
template <class T>
unsigned int Vector<T>::crc32b(const char *message)
{
	int i, j;
	unsigned int byte, crc, mask;

	i = 0;
	crc = 0xFFFFFFFF;
	while (message[i] != 0) {
		byte = message[i];            // Get next byte.
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--) {    // Do eight times.
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
	}
	return ~crc;
} // end


// Creates a string representation of the vector with an appended CRC32.
// [e1,e2,e3,...][crc]
// The elements in the vector are separated by V_SEP_CHAR, and this will
// never appear in normal ASCII.
//
// NOTE: The use of goto in the function below is required since the else block
// may always be evaluated if the constexpr keyword is utilized.  Can this
// be eventually fixed?
template<class T>
String Vector<T>::to_String()
{
	String s = "[";
	size_t n = size();
	for(size_t k = 0; k < n; k++)
	{
		#ifndef ARDUINO_CODE  // C++ code
			if constexpr ( is_same<T, bool>::value)
			{
				s += buffer[k] ? String(BOOL_TRUE) : String(BOOL_FALSE);
				goto add_sep;
			}
			if constexpr (is_same<T, String>::value)
			{
				s += buffer[k];
				goto add_sep;
			}
			else  // this may not be reached, hence the goto above
			{
				s += std::to_string(buffer[k]);
			}

		 #else  // Arduino
			if constexpr (is_same<T, float>::value)
			{
				s += String(buffer[k], 7);
				goto add_sep;
			}
			if constexpr (is_same<T, double>::value)
			{
				s += String(buffer[k], 15);
				goto add_sep;
			}
			if constexpr (is_same<T, bool>::value)
			{
				s += buffer[k] ? String(BOOL_TRUE) : String(BOOL_FALSE);
				goto add_sep;
			}
			else
			{
				s += String(buffer[k]);
			}
		 #endif

		add_sep:
		if (k < (n-1)) s += V_SEP_CHAR;
	}
	s += "][";
	int out = crc32b(s.c_str());
	s += TO_STRING(out);
	s += "]";
	return s;
} // end


// Function to split the string 
template <class T>
Vector<String> Vector<T>::split(String str, char c)
{
	Vector<String> out;
	size_t n = str.length();
	if(n == 0) return out;
	size_t start =  0;
	size_t end = 0;
	bool flag = false;
	for(size_t k = 0; k < n; k++)
	{
		if (str[k] == c)
		{
			flag = true;
			end = k;
			size_t diff = end-start;
			String p = str.SUBSTR(start, diff);
			if(p != "") out.push_back(p);
			start = k + 1;
		}
	}
	if (flag) end += 1;
	String last = str.SUBSTR(end);
	if (last.length() != 0)
	{
		if(last != "") out.push_back(last);
	} // end
	return out;
} // end


template<class T>
bool Vector<T>::from_String(String s)
{
	size_t n = s.length();
	if (s[0] != '[') return false;
	if (s[n-1] != ']') return false;
	size_t crc0pos = s.FIND_LAST_INDEX("[");

	if (crc0pos == static_cast<size_t>(-1)) return false;
	else if (crc0pos >= n) return false;
	else if (crc0pos < 2) return false;
	size_t f = n - crc0pos;
	if (f < 2) return false;
	size_t last = f-2;
	String crc = s.SUBSTR(crc0pos+1, last);
	String vec = s.SUBSTR(1, crc0pos-2);
	String comp = s.SUBSTR(0, crc0pos+1);

	int icrc = atoi(crc.c_str());
	int icrc_comp = crc32b(comp.c_str());
	if (icrc != icrc_comp) return false;

	Vector<String> vsplit = split(vec.c_str(), V_SEP_CHAR);

	size_t nelem = vsplit.size();
	this->resize(nelem);
	if constexpr (is_same<T, String>::value)
	{
		for(size_t k = 0; k < nelem; k++) buffer[k] = vsplit[k];
		return true;
	}
	for(size_t k = 0; k < nelem; k++)
	{
		T out;
		String v = vsplit[k];
		const char *vs = v.c_str();
		if constexpr (is_same<T, bool>::value)
		{
			out = (v == BOOL_TRUE);
		}
		if constexpr (is_same<T, float>::value)
		{
			out = strtof(vs, NULL);
		}
		if constexpr (is_same<T, double>::value)
		{
			out = atof(vs);
		}
		if constexpr (is_same<T, int>::value)
		{
			out = atoi(vs);
		}
		if constexpr (is_same<T, unsigned int>::value)
		{
			out = strtoul(vs, NULL, 10);
		}
		if constexpr (is_same<T, long double>::value)
		{
			out = strtold(vs, NULL);
		}
		if constexpr (is_same<T, char>::value)
		{
			out = static_cast<char>(vs[0]);
		}
		if constexpr (is_same<T, signed char>::value)
		{
			out = static_cast<signed char>(vs[0]);
		}
		if constexpr (is_same<T, unsigned char>::value)
		{
			out = static_cast<unsigned char>(vs[0]);
		}
		if constexpr  (is_same<T, short>::value)
		{
			out = static_cast<short>(strtol(vs, NULL, 10));
		}
		if constexpr (is_same<T, long>::value)
		{
			out = strtol(vs, NULL, 10);
		}
		if constexpr (is_same<T, unsigned long>::value)
		{
			out = strtoul(vs, NULL, 10);
		}
		if constexpr (is_same<T, long long>::value)
		{
			out = strtoll(vs, NULL, 10);
		}
		if constexpr (is_same<T, unsigned long long>::value)
		{
			out = strtoull(vs, NULL, 10);
		}
		buffer[k] = out;  // assign the output
	}
	return true;
} // end


