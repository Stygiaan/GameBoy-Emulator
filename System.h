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
	int EmulateCycle();
	void FetchOpcode();
	void ExecuteOpcode();

private:
	unsigned char opcode{};

	// Memory
	unsigned char main_memory[0xFFFF]{};
	// unsigned char video_memory[8192]{};
	unsigned short stack[16]{};

	// CPU Registers
	Registers registers;
	unsigned short pc{};
	unsigned short sp{};


	// Reserved memory location filled with this logo data
	unsigned char nintendo_graphic[48] = 
	{
		0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 
		0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 
		0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
	};


	FileLogger* logger;

	void Initialize();
	void SetBitflag(BitFlags flag);
	void ClearBitflag(BitFlags flag);
	void ToggleBitflag(BitFlags flag);
	unsigned char GetBitflag(BitFlags flag);

	// Assembly instruction helpers
	void AsmReturn();
	void AsmCALLnn();
	unsigned char AsmPOP();
	void AsmRST(unsigned short address);
};

