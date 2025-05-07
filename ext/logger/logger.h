#pragma once

#include <iostream>
#include <windows.h>
#include <string>
#include <stdarg.h>

class _LoggerClass {
public:
	void initialize(int showCMD);
	void log(const char* message, ...);
	void warning(const char* message, ...);
	void error(const char* message, ...);
private:
	bool isConsoleEnabled = false;
	HANDLE consoleHandle = nullptr;
	FILE* consoleStream = nullptr;
};

inline _LoggerClass logger;