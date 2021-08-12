#pragma once

#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "FileLogger.h"

struct Registers
{
	union
	{
		struct
		{
			unsigned char f;
			unsigned char a;
		};
		unsigned short fa{};
	};
	union
	{
		struct
		{
			unsigned char c;
			unsigned char b;
		};
		unsigned short cb{};
	};
	union
	{
		struct
		{
			unsigned char e;
			unsigned char d;
		};
		unsigned short ed{};
	};
	union
	{
		struct
		{
			unsigned char l;
			unsigned char h;
		};
		unsigned short lh{};
	};
};

enum BitFlags
{
	Carry = 4,
	Half_Carry = 5,
	Subtract = 6,
	Zero = 7
};

class System
{
public:
	System(FileLogger* logger);
	void LoadRom(std::string path);
	void EmulateCycle();
	void FetchOpcode();
	void ExecuteOpcode();
	void ProcessInterrupts();

	bool IsRunning();
	void SetRunning(bool running);
	unsigned char GetLastOpcode();
	unsigned char GetNextOpcode();
	Registers GetRegisters();
	unsigned short GetPC();
	unsigned short GetSP();

	unsigned char GetInputRegister();
	void SetInputRegister(unsigned char joypad);

private:
	bool running = false;
	bool halted = false;
	unsigned char opcode{};
	short interrupt_addresses[5] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

	// Interrupt Master Enable Flag
	bool IME = false;

	// Memory
	unsigned char main_memory[0xFFFF + 1]{};

	// CPU Registers
	Registers registers;
	unsigned short pc{};
	unsigned short sp{};

	FileLogger* logger;

	void Initialize();
	void SetBitflag(BitFlags flag);
	void ClearBitflag(BitFlags flag);
	void ToggleBitflag(BitFlags flag);
	unsigned char GetBitflag(BitFlags flag);

	// Assembly instruction helpers
	void AsmINC_s(unsigned char* value);
	void AsmDEC_s(unsigned char* value);
	void AsmReturn();
	void AsmCALLInterrupt(short address);
	void AsmCALLnn();
	unsigned char AsmPOP();
	void AsmRST(unsigned short address);
	void AsmADD_A(unsigned char value);
	void AsmADC_A(unsigned char value);
	void AsmSUB_A(unsigned char value);
	void AsmSBC_A(unsigned char value);
	void AsmAND_A(unsigned char value);
	void AsmOR_A(unsigned char value);
	void AsmXOR_A(unsigned char value);
	void AsmCP_A(unsigned char value);
	void AsmRLC(unsigned char* value);
	void AsmRRC(unsigned char* value);
	void AsmRL(unsigned char* value);
	void AsmRR(unsigned char* value);
	void AsmSLA(unsigned char* value);
	void AsmSRA(unsigned char* value);
	void AsmSWAP(unsigned char* value);
	void AsmSRL(unsigned char* value);
	void AsmBIT(unsigned char value, short bit);
	void AsmRES(unsigned char* value, short bit);
	void AsmSET(unsigned char* value, short bit);
};

