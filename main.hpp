#pragma once

#include <cstdlib>
#include <algorithm>
#include <map>
#include <string>
#include <thread>
#include <vector>
//#include <set>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>
#include <random>
#include <chrono>
#include <functional>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <condition_variable>
#include <atomic>
#include <wchar.h>
#include <filesystem>
#include "WaitGroup.hpp"

using std::cout;
using std::endl;
using std::string;
using std::wstring;
using std::map;
using std::vector;
using std::thread;
//using std::set;
using std::unordered_set;
using std::stringstream;
using std::wstringstream;
using std::shared_ptr;
using std::unique_ptr;
using std::recursive_mutex;
using std::lock_guard;
using std::hash;
using std::exception;
using std::condition_variable;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using CppWaitGroup::WaitGroup;
using std::atomic;




//	#define	USE_ATOMIC

#include "logging.hpp"

template<typename ... Args>
std::string sprintf(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + (1 * sizeof(char));
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1);
}

template<typename ... Args>
std::wstring sprintf(const std::wstring& format, Args ... args)
{
	wstring	rvalue;
	size_t size = _snwprintf(nullptr, 0, format.c_str(), args ...) + (1 * sizeof(wchar_t));
	std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
	_snwprintf(buf.get(), size, format.c_str(), args ...);

	rvalue = buf.get();

	return rvalue;
}

template<typename ... Args>
std::string xsprintf( string format,  Args ... args )
{
	return sprintf(format, args...);
}

template<typename ... Args>
std::string xsprintfw(string format, Args ... args)
{
	return sprintf(format, args...);
}







extern	vector<uint64_t>    		PossibleValues;
extern	unordered_set<uint64_t>		UnsortedValues;

