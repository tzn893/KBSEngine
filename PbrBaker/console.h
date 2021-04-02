#pragma once

#include <Windows.h>
#include <iostream>

class Console {
public:

	Console();
	
	template<typename T,typename ...Args>
	Console& Log(T arg1,Args... arg2) {
		std::cout << arg1;
		Log(arg2...);
		return *this;
	}

	
	template<typename T>
	Console& Log(T arg1) {
		std::cout << arg1;
		return *this;
	}


	template<typename T, typename ...Args>
	Console& WLog(T arg1, Args... arg2) {
		std::wcout << arg1;
		WLog(arg2...);
		return *this;
	}


	template<typename T>
	Console& WLog(T arg1) {
		std::wcout << arg1;
		return *this;
	}

	Console& FlushLog();

	Console& ClearLog();

	Console& LogProcess(const char* desc1,float process,const char* desc2);

	Console& LineSwitch(size_t num = 1);

private:
	COORD cursor;
};

inline Console gConsole;