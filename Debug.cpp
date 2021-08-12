#include "Debug.h"

Debug::Debug(System* system)
{
	this->system = system;

	this->breakpoints = { 0xc3 };
}
//Debug::Debug(System* system, unsigned char* breakpoints)
//	: breakpoints(*breakpoints, *breakpoints + sizeof(*breakpoints) / sizeof(breakpoints[0]))
//{
//	this->system = system;
//
//	for (auto i = this->breakpoints.begin(); i != this->breakpoints.end(); ++i)
//	{
//		std::cout << *i << " ";
//	}
//}

void Debug::Step()
{
	next_opcode = system->GetNextOpcode();

	switch (current_mode)
	{
	case Mode::Wait:
	{
		if (breakpoints.contains(next_opcode))
		{
			ProcessInput();
		}

		break;
	}
	case Mode::Step:
	{
		ProcessInput();

		break;
	}
	default:
		return;
	}
}

void Debug::ProcessInput()
{
	// std::cout << RegistersToString();
	std::cout << "\nOpcode to be executed: 0x" << std::setfill('0') << std::hex << std::setw(2) << std::right << (int)next_opcode << "\n";
	std::cout << "r: Print registers, s: Step one instruction, c: Continue to next breakpoint:\n";

	char input;

	std::cin >> input;

	switch (input)
	{
	case 'regs':
	{
		std::cout << RegistersToString() << "\n";

		break;
	}
	case 'step':
	{
		current_mode = Mode::Step;

		break;
	}
	case 'continue':
	{
		current_mode = Mode::Wait;

		break;
	}
	default:
		std::cout << "Invalid input.\n";

		ProcessInput();

		break;
	}
}

std::string Debug::RegistersToString()
{
	std::ostringstream ss;
	Registers regs = system->GetRegisters();

	ss << "\nAF: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << regs.fa << "\n";
	ss << "BC: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << regs.cb << "\n";
	ss << "DE: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << regs.ed << "\n";
	ss << "HL: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << regs.lh << "\n";
	ss << "PC: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << system->GetPC() << "\n";
	ss << "SP: 0x" << std::setfill('0') << std::setw(4) << std::right << std::hex << system->GetSP() << "\n";

	short z = regs.f >> 7 & 1;
	short n = regs.f >> 6 & 1;
	short h = regs.f >> 5 & 1;
	short c = regs.f >> 4 & 1;

	ss << std::hex << "Z: " << z << " N: " << z << " H: " << z << " C: " << z << "\n";

	return ss.str();
}