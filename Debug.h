#pragma once

#include <initializer_list>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>

#include "System.h"

class Debug
{
public:
	Debug(System* system);
	// Debug(System* system, unsigned char* breakpoints);
	void Step();

	enum class Mode
	{
		Step,
		Wait
	};

private:
	void ProcessInput();
	std::string RegistersToString();

	System* system;

	Mode current_mode = Mode::Wait;
	unsigned char next_opcode{};
	std::set<unsigned char> breakpoints{};
};

