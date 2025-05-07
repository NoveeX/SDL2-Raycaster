#include "logger.h"


void _LoggerClass::initialize(int showCMD)
{
	if (showCMD) {
		AllocConsole();
		freopen_s(&consoleStream, "CONOUT$", "w", stdout);

		consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		isConsoleEnabled = true;
		
		SetConsoleTextAttribute(consoleHandle, 15);
		std::cout << "Console initialized!\n";
	}
}

void _LoggerClass::log(const char* message, ...)
{
	if (!isConsoleEnabled)
		return;
	va_list ap;
	va_start(ap, message);
	vfprintf(stdout, message, ap);
	va_end(ap);
	std::cout << "\n";
}

void _LoggerClass::warning(const char* message, ...)
{
	if (!isConsoleEnabled)
		return;
	SetConsoleTextAttribute(consoleHandle, 14);
	std::cout << "[-] ";
	va_list ap;
	va_start(ap, message);
	vfprintf(stdout, message, ap);
	va_end(ap);
	std::cout << "\n";
	SetConsoleTextAttribute(consoleHandle, 15);
}

void _LoggerClass::error(const char* message, ...)
{
	if (!isConsoleEnabled)
		return;
	SetConsoleTextAttribute(consoleHandle, 12);
	std::cout << "[X] ";
	va_list ap;
	va_start(ap, message);
	vfprintf(stdout, message, ap);
	va_end(ap);
	std::cout << "\n";
	SetConsoleTextAttribute(consoleHandle, 15);
}

