#include "System.h"

System::System(FileLogger* logger)
{
	this->logger = logger;
}

void System::Initialize()
{
	opcode = 0;

	std::memset(main_memory, 0, sizeof(main_memory));
	// std::memset(video_memory, 0, sizeof(video_memory));
	std::memset(stack, 0, sizeof(stack));

	// registers = {};
	pc = 0x0100;
	sp = 0xFFFE;
}

void System::LoadRom(std::string path)
{
	Initialize();

	std::ifstream input(path, std::ios::in | std::ios::binary);

	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

	for (int i = 0; i < 0x8000; ++i)
	{
		main_memory[i] = buffer[i];
	}

	logger->Log("Loaded Rom of size: ", buffer.size());
}

int System::EmulateCycle()
{
	if (pc > sizeof(main_memory))
		return 1;

	FetchOpcode();
	ExecuteOpcode();

	return 0;
}

void System::FetchOpcode()
{
	opcode = main_memory[pc];
}

void System::ExecuteOpcode()
{
	switch (opcode)
	{
		// NOP
	case 0x00:
	{
		break;
	}

	// LD BC,nn
	case 0x01:
	{
		registers.cb = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}

	// LD (BC),A
	case 0x02:
	{
		main_memory[registers.cb] = registers.a;

		break;
	}

	// INC BC
	case 0x03:
	{
		++registers.cb;

		break;
	}

	// INC B
	case 0x04:
	{
		if (++registers.b > std::numeric_limits<unsigned char>::max())
			SetBitflag(BitFlags::Carry);
		else
			ClearBitflag(BitFlags::Carry);

		ClearBitflag(Subtract);

		break;
	}

	case 0x05:
	{
		--registers.b;

		break;
	}

	case 0x06:
	{
		registers.b = main_memory[++pc];

		break;
	}

	case 0x07:
	{
		unsigned char left_bit = (registers.a & (1 << 7)) == 0x80 ? 1 : 0;

		registers.a << 1;

		if (left_bit == 1)
		{
			registers.a |= 0b1;
			SetBitflag(BitFlags::Carry);
		}
		else
			ClearBitflag(BitFlags::Carry);

		ClearBitflag(BitFlags::Subtract);
		ClearBitflag(BitFlags::Half_Carry);

		break;
	}

	case 0x08:
	{
		unsigned short addr = (main_memory[++pc] << 8) | main_memory[++pc];
		main_memory[addr] = sp;

		break;
	}

	case 0x09:
	{
		registers.lh += registers.cb;

		break;
	}

	case 0x0A:
	{
		registers.a = main_memory[registers.cb];

		break;
	}

	case 0x0B:
	{
		--registers.cb;

		break;
	}

	case 0x0C:
	{
		++registers.c;

		break;
	}

	case 0x0D:
	{
		--registers.c;

		break;
	}

	case 0x0E:
	{
		registers.c = main_memory[++pc];

		break;
	}

	case 0x0F:
	{
		unsigned char right_bit = (registers.a & 0x1) == 0x1 ? 1 : 0;

		registers.a >> 1;

		if (right_bit == 1)
		{
			registers.a |= 1 << 7;
			SetBitflag(BitFlags::Carry);
		}
		else
			ClearBitflag(BitFlags::Carry);

		ClearBitflag(BitFlags::Subtract);
		ClearBitflag(BitFlags::Half_Carry);

		break;
	}

	case 0x10:
	{
		--pc;

		break;
	}

	case 0x11:
	{
		registers.ed = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}

	case 0x12:
	{
		main_memory[registers.ed] = registers.a;

		break;
	}

	case 0x13:
	{
		++registers.ed;

		break;
	}

	case 0x14:
	{
		++registers.d;

		break;
	}

	case 0x15:
	{
		--registers.d;

		break;
	}

	case 0x16:
	{
		registers.d = main_memory[++pc];

		break;
	}

	case 0x17:
	{
		unsigned char left_bit = (registers.a & (0x1 << 7)) == 0x80 ? 1 : 0;

		registers.a << 1;

		registers.a |= GetBitflag(BitFlags::Carry);

		if (left_bit == 1)
			SetBitflag(BitFlags::Carry);
		else
			ClearBitflag(BitFlags::Carry);

		ClearBitflag(BitFlags::Subtract);
		ClearBitflag(BitFlags::Half_Carry);

		break;
	}

	case 0x18:
	{
		pc += main_memory[pc + 1];
		--pc;

		break;
	}

	case 0x19:
	{
		registers.lh += registers.ed;

		break;
	}

	case 0x1A:
	{
		registers.a = main_memory[registers.ed];

		break;
	}

	case 0x1B:
	{
		--registers.ed;

		break;
	}

	case 0x1C:
	{
		++registers.e;

		break;
	}

	case 0x1D:
	{
		--registers.e;

		break;
	}

	case 0x1E:
	{
		registers.e = main_memory[++pc];

		break;
	}

	case 0x1F:
	{
		unsigned char right_bit = (registers.a & 0x1) == 0x1 ? 1 : 0;

		registers.a >> 1;

		// TODO: Set right bit to carry flag value
		registers.a |= GetBitflag(BitFlags::Carry);

		if (right_bit == 1)
			SetBitflag(BitFlags::Carry);
		else
			ClearBitflag(BitFlags::Carry);

		ClearBitflag(BitFlags::Subtract);
		ClearBitflag(BitFlags::Half_Carry);

		break;
	}

	case 0x20:
	{
		if (GetBitflag(Zero) == 0)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}

	case 0x21:
	{
		registers.lh = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}

	case 0x22:
	{
		unsigned short addr = (main_memory[++pc] << 8) | main_memory[++pc];
		main_memory[addr] = registers.lh;

		break;
	}

	case 0x23:
	{
		++registers.lh;

		break;
	}

	case 0x24:
	{
		++registers.h;

		break;
	}

	case 0x25:
	{
		--registers.h;

		break;
	}

	case 0x26:
	{
		registers.h = main_memory[++pc];

		break;
	}

	case 0x27:
	{
		unsigned char ls_bits = registers.a & 0x0F;
		unsigned char ms_bits = (registers.a & 0xF0) >> 4;

		if (ls_bits > 9 || (GetBitflag(Half_Carry) == 1))
			registers.a += 0x06;
		if (ms_bits > 9 || (GetBitflag(Carry) == 1))
			registers.a += 0x60;

		break;
	}

	case 0x28:
	{
		if (GetBitflag(Zero) == 1)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}

	case 0x29:
	{
		registers.lh += registers.lh;

		break;
	}

	case 0x2A:
	{
		registers.a = main_memory[registers.lh];
		++registers.lh;

		break;
	}

	case 0x2B:
	{
		--registers.lh;

		break;
	}

	case 0x2C:
	{
		++registers.l;

		break;
	}

	case 0x2D:
	{
		--registers.l;

		break;
	}

	case 0x2E:
	{
		registers.l = main_memory[++pc];

		break;
	}

	case 0x2F:
	{
		registers.a = ~registers.a;

		break;
	}

	case 0x30:
	{
		if (GetBitflag(Carry) == 0)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}

	case 0x31:
	{
		sp = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}

	case 0x32:
	{
		main_memory[registers.lh] = registers.a;
		--registers.lh;

		break;
	}

	case 0x33:
	{
		++sp;

		break;
	}

	case 0x34:
	{
		++main_memory[registers.lh];

		break;
	}

	case 0x35:
	{
		--main_memory[registers.lh];

		break;
	}

	case 0x36:
	{
		main_memory[registers.lh] = main_memory[++pc];

		break;
	}

	case 0x37:
	{
		SetBitflag(Carry);

		break;
	}

	case 0x38:
	{
		if (GetBitflag(Carry) == 1)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}

	case 0x39:
	{
		registers.lh += sp;

		break;
	}

	case 0x3A:
	{
		registers.a = main_memory[registers.lh];

		break;
	}

	case 0x3B:
	{
		--sp;

		break;
	}

	case 0x3C:
	{
		++registers.a;

		break;
	}

	case 0x3D:
	{
		--registers.a;

		break;
	}

	case 0x3E:
	{
		registers.a = main_memory[++pc];

		break;
	}

	case 0x3F:
	{
		ClearBitflag(Carry);

		break;
	}

	case 0x40:
	{
		registers.b = registers.b;

		break;
	}

	case 0x41:
	{
		registers.b = registers.c;

		break;
	}

	case 0x42:
	{
		registers.b = registers.d;

		break;
	}

	case 0x43:
	{
		registers.b = registers.e;

		break;
	}

	case 0x44:
	{
		registers.b = registers.h;

		break;
	}

	case 0x45:
	{
		registers.b = registers.l;

		break;
	}

	case 0x46:
	{
		registers.b = main_memory[registers.lh];

		break;
	}

	case 0x47:
	{
		registers.b = registers.a;

		break;
	}

	case 0x48:
	{
		registers.c = registers.b;

		break;
	}

	case 0x49:
	{
		registers.c = registers.c;

		break;
	}

	case 0x4A:
	{
		registers.c = registers.d;

		break;
	}

	case 0x4B:
	{
		registers.c = registers.e;

		break;
	}

	case 0x4C:
	{
		registers.c = registers.h;

		break;
	}

	case 0x4D:
	{
		registers.c = registers.l;

		break;
	}

	case 0x4E:
	{
		registers.c = main_memory[registers.lh];

		break;
	}

	case 0x4F:
	{
		registers.c = registers.a;

		break;
	}

	case 0x50:
	{
		registers.d = registers.b;

		break;
	}

	case 0x51:
	{
		registers.d = registers.c;

		break;
	}

	case 0x52:
	{
		registers.d = registers.d;

		break;
	}

	case 0x53:
	{
		registers.d = registers.e;

		break;
	}

	case 0x54:
	{
		registers.d = registers.h;

		break;
	}

	case 0x55:
	{
		registers.d = registers.l;

		break;
	}

	case 0x56:
	{
		registers.d = main_memory[registers.lh];

		break;
	}

	case 0x57:
	{
		registers.d = registers.a;

		break;
	}

	case 0x58:
	{
		registers.e = registers.b;

		break;
	}

	case 0x59:
	{
		registers.e = registers.c;

		break;
	}

	case 0x5A:
	{
		registers.e = registers.d;

		break;
	}

	case 0x5B:
	{
		registers.e = registers.e;

		break;
	}

	case 0x5C:
	{
		registers.e = registers.h;

		break;
	}

	case 0x5D:
	{
		registers.e = registers.l;

		break;
	}

	case 0x5E:
	{
		registers.e = main_memory[registers.lh];

		break;
	}

	case 0x5F:
	{
		registers.e = registers.a;

		break;
	}

	case 0x60:
	{
		registers.h = registers.b;

		break;
	}

	case 0x61:
	{
		registers.h = registers.c;

		break;
	}

	case 0x62:
	{
		registers.h = registers.d;

		break;
	}

	case 0x63:
	{
		registers.h = registers.e;

		break;
	}

	case 0x64:
	{
		registers.h = registers.h;

		break;
	}

	case 0x65:
	{
		registers.h = registers.l;

		break;
	}

	case 0x66:
	{
		registers.h = main_memory[registers.lh];

		break;
	}

	case 0x67:
	{
		registers.h = registers.a;

		break;
	}

	case 0x68:
	{
		registers.l = registers.b;

		break;
	}

	case 0x69:
	{
		registers.l = registers.c;

		break;
	}

	case 0x6A:
	{
		registers.l = registers.d;

		break;
	}

	case 0x6B:
	{
		registers.l = registers.e;

		break;
	}

	case 0x6C:
	{
		registers.l = registers.h;

		break;
	}

	case 0x6D:
	{
		registers.l = registers.l;

		break;
	}

	case 0x6E:
	{
		registers.l = main_memory[registers.lh];

		break;
	}

	case 0x6F:
	{
		registers.l = registers.a;

		break;
	}

	case 0x70:
	{
		main_memory[registers.lh] = registers.b;

		break;
	}

	case 0x71:
	{
		main_memory[registers.lh] = registers.c;

		break;
	}

	case 0x72:
	{
		main_memory[registers.lh] = registers.d;

		break;
	}

	case 0x73:
	{
		main_memory[registers.lh] = registers.e;

		break;
	}

	case 0x74:
	{
		main_memory[registers.lh] = registers.h;

		break;
	}

	case 0x75:
	{
		main_memory[registers.lh] = registers.l;

		break;
	}

	case 0x76:
	{
		// HALT

		break;
	}

	case 0x77:
	{
		main_memory[registers.lh] = registers.a;

		break;
	}

	case 0x78:
	{
		registers.a = registers.b;

		break;
	}

	case 0x79:
	{
		registers.a = registers.c;

		break;
	}

	case 0x7A:
	{
		registers.a = registers.d;

		break;
	}

	case 0x7B:
	{
		registers.a = registers.e;

		break;
	}

	case 0x7C:
	{
		registers.a = registers.h;

		break;
	}

	case 0x7D:
	{
		registers.a = registers.l;

		break;
	}

	case 0x7E:
	{
		registers.a = main_memory[registers.lh];

		break;
	}

	case 0x7F:
	{
		registers.a = registers.a;

		break;
	}

	case 0x80:
	{
		registers.a += registers.b;

		break;
	}

	case 0x81:
	{
		registers.a += registers.c;

		break;
	}

	case 0x82:
	{
		registers.a += registers.d;

		break;
	}

	case 0x83:
	{
		registers.a += registers.e;

		break;
	}

	case 0x84:
	{
		registers.a += registers.h;

		break;
	}

	case 0x85:
	{
		registers.a += registers.l;

		break;
	}

	case 0x86:
	{
		registers.a += main_memory[registers.lh];

		break;
	}

	case 0x87:
	{
		registers.a += registers.a;

		break;
	}

	case 0x88:
	{
		registers.a += registers.b + GetBitflag(Carry);

		break;
	}

	case 0x89:
	{
		registers.a += registers.c + GetBitflag(Carry);

		break;
	}

	case 0x8A:
	{
		registers.a += registers.d + GetBitflag(Carry);

		break;
	}

	case 0x8B:
	{
		registers.a += registers.e + GetBitflag(Carry);

		break;
	}

	case 0x8C:
	{
		registers.a += registers.h + GetBitflag(Carry);

		break;
	}

	case 0x8D:
	{
		registers.a += registers.l + GetBitflag(Carry);

		break;
	}

	case 0x8E:
	{
		registers.a += main_memory[registers.lh] + GetBitflag(Carry);

		break;
	}

	case 0x8F:
	{
		registers.a += registers.a + GetBitflag(Carry);

		break;
	}

	case 0x90:
	{
		registers.a -= registers.b;

		break;
	}

	case 0x91:
	{
		registers.a -= registers.c;

		break;
	}

	case 0x92:
	{
		registers.a -= registers.d;

		break;
	}

	case 0x93:
	{
		registers.a -= registers.e;

		break;
	}

	case 0x94:
	{
		registers.a -= registers.h;

		break;
	}

	case 0x95:
	{
		registers.a -= registers.l;

		break;
	}

	case 0x96:
	{
		registers.a -= main_memory[registers.lh];

		break;
	}

	case 0x97:
	{
		registers.a -= registers.a;

		break;
	}

	case 0x98:
	{
		registers.a -= registers.b + GetBitflag(Carry);

		break;
	}

	case 0x99:
	{
		registers.a -= registers.c + GetBitflag(Carry);

		break;
	}

	case 0x9A:
	{
		registers.a -= registers.d + GetBitflag(Carry);

		break;
	}

	case 0x9B:
	{
		registers.a -= registers.e + GetBitflag(Carry);

		break;
	}

	case 0x9C:
	{
		registers.a -= registers.h + GetBitflag(Carry);

		break;
	}

	case 0x9D:
	{
		registers.a -= registers.l + GetBitflag(Carry);

		break;
	}

	case 0x9E:
	{
		registers.a -= main_memory[registers.lh] + GetBitflag(Carry);

		break;
	}

	case 0x9F:
	{
		registers.a -= registers.a + GetBitflag(Carry);

		break;
	}

	case 0xA0:
	{
		registers.a &= registers.b;

		break;
	}

	case 0xA1:
	{
		registers.a &= registers.c;

		break;
	}

	case 0xA2:
	{
		registers.a &= registers.d;

		break;
	}

	case 0xA3:
	{
		registers.a &= registers.e;

		break;
	}

	case 0xA4:
	{
		registers.a &= registers.h;

		break;
	}

	case 0xA5:
	{
		registers.a &= registers.l;

		break;
	}

	case 0xA6:
	{
		registers.a &= main_memory[registers.lh];

		break;
	}

	case 0xA7:
	{
		registers.a &= registers.a;

		break;
	}

	case 0xA8:
	{
		registers.a ^= registers.b;

		break;
	}

	case 0xA9:
	{
		registers.a ^= registers.c;

		break;
	}

	case 0xAA:
	{
		registers.a ^= registers.d;

		break;
	}

	case 0xAB:
	{
		registers.a ^= registers.e;

		break;
	}

	case 0xAC:
	{
		registers.a ^= registers.h;

		break;
	}

	case 0xAD:
	{
		registers.a ^= registers.l;

		break;
	}

	case 0xAE:
	{
		registers.a ^= main_memory[registers.lh];

		break;
	}

	case 0xAF:
	{
		registers.a ^= registers.a;

		break;
	}

	case 0xB0:
	{
		registers.a |= registers.b;

		break;
	}

	case 0xB1:
	{
		registers.a |= registers.c;

		break;
	}

	case 0xB2:
	{
		registers.a |= registers.d;

		break;
	}

	case 0xB3:
	{
		registers.a |= registers.e;

		break;
	}

	case 0xB4:
	{
		registers.a |= registers.h;

		break;
	}

	case 0xB5:
	{
		registers.a |= registers.l;

		break;
	}

	case 0xB6:
	{
		registers.a |= main_memory[registers.lh];

		break;
	}

	case 0xB7:
	{
		registers.a |= registers.a;

		break;
	}

	// CP
	case 0xB8:
	{
		if (registers.a - registers.b == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xB9:
	{
		if (registers.a - registers.c == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBA:
	{
		if (registers.a - registers.d == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBB:
	{
		if (registers.a - registers.e == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBC:
	{
		if (registers.a - registers.h == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBD:
	{
		if (registers.a - registers.l == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBE:
	{
		if (registers.a - main_memory[registers.lh] == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	case 0xBF:
	{
		if (registers.a - registers.a == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		break;
	}

	// RET NZ
	case 0xC0:
	{
		if (GetBitflag(Zero) == 0)
		{
			AsmReturn();
		}

		break;
	}

	// POP BC
	case 0xC1:
	{
		registers.c = AsmPOP();
		registers.b = AsmPOP();

		break;
	}

	// JP NZ,nn
	case 0xC2:
	{
		if (GetBitflag(Zero) == 0)
		{
			pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
			--pc;
		}

		break;
	}

	// JP nn
	case 0xC3:
	{
		pc = (main_memory[++pc] << 8) | main_memory[++pc];
		--pc;

		break;
	}

	// CALL NZ, nn
	case 0xC4:
	{
		if (GetBitflag(Zero) == 0)
		{
			AsmCALLnn();
		}

		break;
	}

	// PUSH BC
	case 0xC5:
	{
		main_memory[--sp] = registers.b;
		main_memory[--sp] = registers.c;

		break;
	}

	// ADD A, n
	case 0xC6:
	{
		registers.a += main_memory[++pc];

		break;
	}

	// RST 00h
	case 0xC7:
	{
		AsmRST(0x0000);

		break;
	}

	// RET Z
	case 0xC8:
	{
		if (GetBitflag(Zero) == 1)
		{
			AsmReturn();
		}

		break;
	}

	// RET
	case 0xC9:
	{
		AsmReturn();

		break;
	}

	// JP Z, nn
	case 0xCA:
	{
		if (GetBitflag(Zero) == 1)
		{
			pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
			--pc;
		}

		break;
	}

	// Extended Operation
	case 0xCB:
	{
		opcode = main_memory[++pc];

		switch (opcode)
		{
		default:
		{
			std::stringstream ss;
			ss << "Unknown Extended(BC) Opcode: " << std::hex << static_cast<int>(opcode);

			logger->Log(ss.str());
		}
		}

		break;
	}

	// CALL Z, nn
	case 0xCC:
	{
		if (GetBitflag(Zero) == 1)
		{
			AsmCALLnn();
		}

		break;
	}

	// CALL nn
	case 0xCD:
	{
		AsmCALLnn();

		break;
	}

	// ADC A, n
	case 0xCE:
	{
		registers.a += main_memory[++pc] + GetBitflag(Carry);

		break;
	}

	// RST 8
	case 0xCF:
	{
		AsmRST(0x0008);

		break;
	}

	// RET NC
	case 0xD0:
	{
		if (GetBitflag(Carry) == 0)
		{
			AsmReturn();
		}

		break;
	}

	// POP DE
	case 0xD1:
	{
		registers.e = AsmPOP();
		registers.d = AsmPOP();

		break;
	}

	// JP NC,nn
	case 0xD2:
	{
		if (GetBitflag(Carry) == 0)
		{
			pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
			--pc;
		}

		break;
	}

	// 0xD3 Removed

	// CALL NC,nn
	case 0xD4:
	{
		if (GetBitflag(Carry) == 0)
		{
			AsmCALLnn();
		}

		break;
	}

	// PUSH DE
	case 0xD5:
	{
		main_memory[--sp] = registers.d;
		main_memory[--sp] = registers.e;

		break;
	}

	// SUB A,n
	case 0xD6:
	{
		registers.a -= main_memory[++pc];

		break;
	}

	// RST 10 	
	case 0xD7:
	{
		AsmRST(0x0010);

		break;
	}

	// RET C
	case 0xD8:
	{
		if (GetBitflag(Carry) == 1)
		{
			AsmReturn();
		}

		break;
	}

	// RETI
	case 0xD9:
	{
		AsmReturn();

		// TODO: Enable Interrupts

		break;
	}

	// JP C,nn
	case 0xDA:
	{
		if (GetBitflag(Carry) == 1)
		{
			pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
			--pc;
		}

		break;
	}

	// 0xDB Removed

	// CALL C,nn 	
	case 0xDC:
	{
		if (GetBitflag(Carry) == 1)
		{
			AsmCALLnn();
		}

		break;
	}

	// 0xDD Removed

	// SBC A,n
	case 0xDE:
	{
		registers.a -= main_memory[++pc] + GetBitflag(Carry);

		break;
	}

	// SBC A,n
	case 0xDF:
	{
		++pc;

		main_memory[--sp] = static_cast<unsigned char>((pc & 0xFF00) >> 8);
		main_memory[--sp] = static_cast<unsigned char>(pc & 0x00FF);

		pc = 0x0018;

		break;
	}

	// LDH (n),A
	case 0xE0:
	{
		unsigned char n = main_memory[++pc];

		main_memory[0xFF00 + n] = registers.a;

		break;
	}

	// POP HL
	case 0xE1:
	{
		registers.l = AsmPOP();
		registers.h = AsmPOP();

		break;
	}

	// LDH (C),A
	case 0xE2:
	{
		main_memory[0xFF00 + registers.c] = registers.a;

		break;
	}

	// 0xE3 Removed

	// 0xE4 Removed

	// PUSH HL
	case 0xE5:
	{
		main_memory[--sp] = registers.h;
		main_memory[--sp] = registers.l;

		break;
	}

	// AND n
	case 0xE6:
	{
		registers.a &= main_memory[++pc];

		break;
	}

	// RST 20
	case 0xE7:
	{
		AsmRST(0x0020);

		break;
	}

	// ADD SP,n
	case 0xE8:
	{
		sp += main_memory[++pc];

		break;
	}

	// JP (HL)
	case 0xE9:
	{
		pc = main_memory[registers.lh];
		--pc;

		break;
	}

	// LD (nn),A
	case 0xEA:
	{
		unsigned short addr = (main_memory[++pc] << 8) | main_memory[++pc];

		main_memory[addr] = registers.a;

		break;
	}

	// 0xEB Removed

	// 0xEC Removed

	// 0xED Removed

	// XOR n
	case 0xEE:
	{
		registers.a ^= main_memory[++pc];

		break;
	}

	// RST 28
	case 0xEF:
	{
		AsmRST(0x0028);

		break;
	}

	// LDH A,(n)
	case 0xF0:
	{
		unsigned char n = main_memory[++pc];

		registers.a = main_memory[0xFF00 + n];

		break;
	}

	// POP AF
	case 0xF1:
	{
		registers.f = AsmPOP();
		registers.a = AsmPOP();

		break;
	}

	// 0xF2 Removed

	// DI (Disable Interrupts)
	case 0xF3:
	{
		// TODO: Disable interrupts

		break;
	}

	// 0xF4 Removed

	// PUSH AF
	case 0xF5:
	{
		main_memory[--sp] = registers.a;
		main_memory[--sp] = registers.f;

		break;
	}

	// OR n
	case 0xF6:
	{
		registers.a |= main_memory[++pc];

		break;
	}

	// RST 30
	case 0xF7:
	{
		AsmRST(0x0030);

		break;
	}

	// LDHL SP,d
	case 0xF8:
	{
		registers.lh = sp += main_memory[++pc];

		break;
	}

	// LD SP,HL
	case 0xF9:
	{
		sp = registers.lh;

		break;
	}

	// LD A,(nn)
	case 0xFA:
	{
		registers.a = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}

	// EI (Enable Interrupts)
	case 0xFB:
	{
		// TODO: Enable Interrupts

		break;
	}

	// 0xFC Removed

	// 0xFD Removed

	// CP n
	case 0xFE:
	{
		signed char result = registers.a - main_memory[++pc];

		SetBitflag(Subtract);

		if (result == 0)
			SetBitflag(Zero);
		else
			ClearBitflag(Zero);

		if (result < 0)
			SetBitflag(Carry);

		break;
	}

	// CP n
	case 0xFF:
	{
		AsmRST(0x0038);

		break;
	}

	default:
	{
		std::stringstream ss;
		ss << "Unknown Opcode: " << std::hex << static_cast<int>(opcode);

		logger->Log(ss.str());

		break;
	}
	}

	++pc;
}

void System::SetBitflag(BitFlags flag)
{
	registers.f |= 1 << flag;
}
void System::ClearBitflag(BitFlags flag)
{
	registers.f &= ~(1 << flag);
}
void System::ToggleBitflag(BitFlags flag)
{
	registers.f ^= 1 << flag;
}
unsigned char System::GetBitflag(BitFlags flag)
{
	if ((registers.f & (1 << flag)) == (1 << flag))
		return 1;

	return 0;
}

void System::AsmReturn()
{
	unsigned short addr = main_memory[sp] << 8;
	main_memory[sp] = 0;
	addr |= main_memory[++sp];
	main_memory[sp] = 0;
	++sp;

	pc = addr;
	--pc;
}
void System::AsmCALLnn()
{
	main_memory[--sp] = static_cast<unsigned char>((pc + 3) & 0x00FF);
	main_memory[--sp] = static_cast<unsigned char>(((pc + 3) & 0xFF00) >> 8);

	pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
	--pc;
}
unsigned char System::AsmPOP()
{
	unsigned char val = main_memory[sp];
	main_memory[sp] = 0;
	++sp;

	return val;
}
void System::AsmRST(unsigned short addr)
{
	++pc;

	main_memory[--sp] = static_cast<unsigned char>((pc & 0xFF00) >> 8);
	main_memory[--sp] = static_cast<unsigned char>(pc & 0x00FF);

	pc = addr;
}