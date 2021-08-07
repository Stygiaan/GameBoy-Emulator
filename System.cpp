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

	running = true;
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
		if ((((registers.b & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.b;

		ClearBitflag(Subtract);
		registers.b == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC B
	case 0x05:
	{
		if ((((registers.b & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.b;

		SetBitflag(Subtract);
		registers.b == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD B,n
	case 0x06:
	{
		registers.b = main_memory[++pc];

		break;
	}
	// RLC A
	case 0x07:
	{
		unsigned char last_bit = (registers.a & (1 << 7)) == 0x80 ? 1 : 0;

		registers.a << 1;

		if (last_bit == 1)
		{
			registers.a |= 0b1;
			SetBitflag(Carry);
		}
		else
			ClearBitflag(Carry);

		ClearBitflag(Zero);
		ClearBitflag(Subtract);
		ClearBitflag(Half_Carry);

		break;
	}
	// LD (nn),SP
	case 0x08:
	{
		unsigned short addr = (main_memory[++pc] << 8) | main_memory[++pc];
		main_memory[addr] = sp;

		break;
	}
	// ADD HL,BC
	case 0x09:
	{
		if ((((registers.lh & 0xF00) + (registers.cb & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((registers.lh & 0xF000) + (registers.cb & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		registers.lh += registers.cb;

		ClearBitflag(Subtract);

		break;
	}
	// LD A,(BC)
	case 0x0A:
	{
		registers.a = main_memory[registers.cb];

		break;
	}
	// DEC BC
	case 0x0B:
	{
		--registers.cb;

		break;
	}
	// INC C
	case 0x0C:
	{
		if ((((registers.c & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.c;

		ClearBitflag(Subtract);
		registers.c == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC C
	case 0x0D:
	{
		if ((((registers.c & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.c;

		SetBitflag(Subtract);
		registers.c == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD C,n
	case 0x0E:
	{
		registers.c = main_memory[++pc];

		break;
	}
	// RRC A
	case 0x0F:
	{
		unsigned char rightmost_bit = (registers.a & 0x1) == 0x1 ? 1 : 0;

		registers.a >> 1;

		if (rightmost_bit == 1)
		{
			registers.a |= 1 << 7;
			SetBitflag(Carry);
		}
		else
			ClearBitflag(Carry);

		ClearBitflag(Zero);
		ClearBitflag(Subtract);
		ClearBitflag(Half_Carry);

		break;
	}
	// STOP
	case 0x10:
	{
		running = false;

		break;
	}
	// LD DE,nn
	case 0x11:
	{
		registers.ed = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}
	// LD (DE),A
	case 0x12:
	{
		main_memory[registers.ed] = registers.a;

		break;
	}
	// INC DE
	case 0x13:
	{
		++registers.ed;

		break;
	}
	// INC D
	case 0x14:
	{
		if ((((registers.d & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.d;

		ClearBitflag(Subtract);
		registers.d == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC D
	case 0x15:
	{
		if ((((registers.d & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.d;

		SetBitflag(Subtract);
		registers.d == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD D,n
	case 0x16:
	{
		registers.d = main_memory[++pc];

		break;
	}
	// RL A
	case 0x17:
	{
		unsigned char leftmost_bit = (registers.a & (0x1 << 7)) == 0x80 ? 1 : 0;

		registers.a << 1;

		registers.a |= GetBitflag(Carry);

		if (leftmost_bit == 1)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		ClearBitflag(Zero);
		ClearBitflag(Subtract);
		ClearBitflag(Half_Carry);

		break;
	}
	// JR n
	case 0x18:
	{
		pc += main_memory[pc + 1];
		--pc;

		break;
	}
	// ADD HL,DE
	case 0x19:
	{
		if ((((registers.lh & 0xF00) + (registers.ed & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((registers.lh & 0xF000) + (registers.ed & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		registers.lh += registers.ed;

		ClearBitflag(Subtract);

		break;
	}
	// LD A,(DE)
	case 0x1A:
	{
		registers.a = main_memory[registers.ed];

		break;
	}
	// DEC DE
	case 0x1B:
	{
		--registers.ed;

		break;
	}
	// INC E
	case 0x1C:
	{
		if ((((registers.e & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.e;

		ClearBitflag(Subtract);
		registers.e == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC E
	case 0x1D:
	{
		if ((((registers.e & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.e;

		SetBitflag(Subtract);
		registers.e == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD E,n
	case 0x1E:
	{
		registers.e = main_memory[++pc];

		break;
	}
	// RR A
	case 0x1F:
	{
		unsigned char right_bit = (registers.a & 0x1) == 0x1 ? 1 : 0;

		registers.a >> 1;

		// TODO: Set right bit to carry flag value
		registers.a |= GetBitflag(Carry);

		if (right_bit == 1)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		ClearBitflag(Zero);
		ClearBitflag(Subtract);
		ClearBitflag(Half_Carry);

		break;
	}
	// JR NZ,n
	case 0x20:
	{
		if (GetBitflag(Zero) == 0)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}
	// LD HL,nn
	case 0x21:
	{
		registers.lh = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}
	// LDI (HL),A
	case 0x22:
	{
		unsigned short addr = (main_memory[++pc] << 8) | main_memory[++pc];
		main_memory[addr] = registers.lh;

		break;
	}
	// INC HL
	case 0x23:
	{
		++registers.lh;

		break;
	}
	// INC H
	case 0x24:
	{
		if ((((registers.h & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.h;

		ClearBitflag(Subtract);
		registers.h == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC H
	case 0x25:
	{
		if ((((registers.h & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.h;

		SetBitflag(Subtract);
		registers.h == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD H,n
	case 0x26:
	{
		registers.h = main_memory[++pc];

		break;
	}
	// DAA (TODO)
	case 0x27:
	{
		unsigned char ls_bits = registers.a & 0x0F;
		unsigned char ms_bits = (registers.a & 0xF0) >> 4;

		if (ls_bits > 9 || (GetBitflag(Half_Carry) == 1))
			registers.a += 0x06;
		if (ms_bits > 9 || (GetBitflag(Carry) == 1))
			registers.a += 0x60;
		else
			ClearBitflag(Carry);

		break;
	}
	// JR Z,n
	case 0x28:
	{
		if (GetBitflag(Zero) == 1)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}
	// ADD HL,HL
	case 0x29:
	{
		if ((((registers.lh & 0xF00) + (registers.lh & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((registers.lh & 0xF000) + (registers.lh & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		registers.lh += registers.lh;

		ClearBitflag(Subtract);

		break;
	}
	// LDI A,(HL)
	case 0x2A:
	{
		registers.a = main_memory[registers.lh];
		++registers.lh;

		break;
	}
	// DEC HL
	case 0x2B:
	{
		--registers.lh;

		break;
	}
	// INC L
	case 0x2C:
	{
		if ((((registers.l & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.l;

		ClearBitflag(Subtract);
		registers.l == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC L
	case 0x2D:
	{
		if ((((registers.l & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.l;

		SetBitflag(Subtract);
		registers.l == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD L,n
	case 0x2E:
	{
		registers.l = main_memory[++pc];

		break;
	}
	// CPL
	case 0x2F:
	{
		registers.a = ~registers.a;

		SetBitflag(Subtract);
		SetBitflag(Half_Carry);

		break;
	}
	// JR NC,n
	case 0x30:
	{
		if (GetBitflag(Carry) == 0)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}
	// LD SP,nn
	case 0x31:
	{
		sp = (main_memory[++pc] << 8) | main_memory[++pc];

		break;
	}
	// LDD (HL),A
	case 0x32:
	{
		main_memory[registers.lh] = registers.a;
		--registers.lh;

		break;
	}
	// INC SP
	case 0x33:
	{
		++sp;

		break;
	}
	// INC (HL)
	case 0x34:
	{
		if ((((main_memory[registers.lh] & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++main_memory[registers.lh];

		ClearBitflag(Subtract);
		main_memory[registers.lh] == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC (HL)
	case 0x35:
	{
		if ((((main_memory[registers.lh] & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--main_memory[registers.lh];

		SetBitflag(Subtract);
		main_memory[registers.lh] == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD (HL),n
	case 0x36:
	{
		main_memory[registers.lh] = main_memory[++pc];

		break;
	}
	// SCF
	case 0x37:
	{
		SetBitflag(Carry);

		break;
	}
	// JR C,n
	case 0x38:
	{
		if (GetBitflag(Carry) == 1)
		{
			pc += main_memory[pc + 1];
			--pc;
		}

		break;
	}
	// ADD HL,SP
	case 0x39:
	{
		if ((((registers.lh & 0xF00) + (sp & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((registers.lh & 0xF000) + (sp & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		registers.lh += sp;

		ClearBitflag(Subtract);

		break;
	}
	// LDD A,(HL)
	case 0x3A:
	{
		registers.a = main_memory[registers.lh];
		--registers.lh;

		break;
	}
	// DEC SP
	case 0x3B:
	{
		--sp;

		break;
	}
	// INC A
	case 0x3C:
	{
		if ((((registers.a & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		++registers.a;

		ClearBitflag(Subtract);
		registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// DEC A
	case 0x3D:
	{
		if ((((registers.a & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		--registers.a;

		SetBitflag(Subtract);
		registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);

		break;
	}
	// LD A,n
	case 0x3E:
	{
		registers.a = main_memory[++pc];

		break;
	}
	// CCF
	case 0x3F:
	{
		ClearBitflag(Carry);

		break;
	}
	// LD B,B
	case 0x40:
	{
		registers.b = registers.b;

		break;
	}
	// LD B,C
	case 0x41:
	{
		registers.b = registers.c;

		break;
	}
	// LD B,D
	case 0x42:
	{
		registers.b = registers.d;

		break;
	}
	// LD B,E
	case 0x43:
	{
		registers.b = registers.e;

		break;
	}
	// LD B,H
	case 0x44:
	{
		registers.b = registers.h;

		break;
	}
	// LD B,L
	case 0x45:
	{
		registers.b = registers.l;

		break;
	}
	// LD B,(HL) 	
	case 0x46:
	{
		registers.b = main_memory[registers.lh];

		break;
	}
	// LD B,A
	case 0x47:
	{
		registers.b = registers.a;

		break;
	}
	// LD C,B
	case 0x48:
	{
		registers.c = registers.b;

		break;
	}
	// LD C,C
	case 0x49:
	{
		registers.c = registers.c;

		break;
	}
	// LD C,D
	case 0x4A:
	{
		registers.c = registers.d;

		break;
	}
	// LD C,E
	case 0x4B:
	{
		registers.c = registers.e;

		break;
	}
	// LD C,H
	case 0x4C:
	{
		registers.c = registers.h;

		break;
	}
	// LD C,L
	case 0x4D:
	{
		registers.c = registers.l;

		break;
	}
	// LD C,(HL)
	case 0x4E:
	{
		registers.c = main_memory[registers.lh];

		break;
	}
	// LD C,A
	case 0x4F:
	{
		registers.c = registers.a;

		break;
	}
	// LD D,B
	case 0x50:
	{
		registers.d = registers.b;

		break;
	}
	// LD D,C
	case 0x51:
	{
		registers.d = registers.c;

		break;
	}
	// LD D,D
	case 0x52:
	{
		registers.d = registers.d;

		break;
	}
	// LD D,E
	case 0x53:
	{
		registers.d = registers.e;

		break;
	}
	// LD D,H
	case 0x54:
	{
		registers.d = registers.h;

		break;
	}
	// LD D,L
	case 0x55:
	{
		registers.d = registers.l;

		break;
	}
	// LD D,(HL)
	case 0x56:
	{
		registers.d = main_memory[registers.lh];

		break;
	}
	// LD D,A
	case 0x57:
	{
		registers.d = registers.a;

		break;
	}
	// LD E,B
	case 0x58:
	{
		registers.e = registers.b;

		break;
	}
	// LD E,C
	case 0x59:
	{
		registers.e = registers.c;

		break;
	}
	// LD E,D
	case 0x5A:
	{
		registers.e = registers.d;

		break;
	}
	// LD E,E
	case 0x5B:
	{
		registers.e = registers.e;

		break;
	}
	// LD E,H
	case 0x5C:
	{
		registers.e = registers.h;

		break;
	}
	// LD E,L
	case 0x5D:
	{
		registers.e = registers.l;

		break;
	}
	// LD E,(HL)
	case 0x5E:
	{
		registers.e = main_memory[registers.lh];

		break;
	}
	// LD E,A
	case 0x5F:
	{
		registers.e = registers.a;

		break;
	}
	// LD H,B
	case 0x60:
	{
		registers.h = registers.b;

		break;
	}
	// LD H,C
	case 0x61:
	{
		registers.h = registers.c;

		break;
	}
	// LD H,D
	case 0x62:
	{
		registers.h = registers.d;

		break;
	}
	// LD H,E
	case 0x63:
	{
		registers.h = registers.e;

		break;
	}
	// LD H,H
	case 0x64:
	{
		registers.h = registers.h;

		break;
	}
	// LD H,L
	case 0x65:
	{
		registers.h = registers.l;

		break;
	}
	// LD H,(HL) 	
	case 0x66:
	{
		registers.h = main_memory[registers.lh];

		break;
	}
	// LD H,A
	case 0x67:
	{
		registers.h = registers.a;

		break;
	}
	// LD L,B
	case 0x68:
	{
		registers.l = registers.b;

		break;
	}
	// LD L,C
	case 0x69:
	{
		registers.l = registers.c;

		break;
	}
	// LD L,D
	case 0x6A:
	{
		registers.l = registers.d;

		break;
	}
	// LD L,E
	case 0x6B:
	{
		registers.l = registers.e;

		break;
	}
	// LD L,H
	case 0x6C:
	{
		registers.l = registers.h;

		break;
	}
	// LD L,L
	case 0x6D:
	{
		registers.l = registers.l;

		break;
	}
	// LD L,(HL)
	case 0x6E:
	{
		registers.l = main_memory[registers.lh];

		break;
	}
	// LD L,A
	case 0x6F:
	{
		registers.l = registers.a;

		break;
	}
	// LD (HL),B
	case 0x70:
	{
		main_memory[registers.lh] = registers.b;

		break;
	}
	// LD (HL),C
	case 0x71:
	{
		main_memory[registers.lh] = registers.c;

		break;
	}
	// LD (HL),D
	case 0x72:
	{
		main_memory[registers.lh] = registers.d;

		break;
	}
	// LD (HL),E
	case 0x73:
	{
		main_memory[registers.lh] = registers.e;

		break;
	}
	// LD (HL),H
	case 0x74:
	{
		main_memory[registers.lh] = registers.h;

		break;
	}
	// LD (HL),L
	case 0x75:
	{
		main_memory[registers.lh] = registers.l;

		break;
	}
	// HALT
	case 0x76:
	{
		// TODO

		break;
	}
	// LD (HL),A
	case 0x77:
	{
		main_memory[registers.lh] = registers.a;

		break;
	}
	// LD A,B
	case 0x78:
	{
		registers.a = registers.b;

		break;
	}
	// LD A,C
	case 0x79:
	{
		registers.a = registers.c;

		break;
	}
	// LD A,D
	case 0x7A:
	{
		registers.a = registers.d;

		break;
	}
	// LD A,E
	case 0x7B:
	{
		registers.a = registers.e;

		break;
	}
	// LD A,H
	case 0x7C:
	{
		registers.a = registers.h;

		break;
	}
	// LD A,L
	case 0x7D:
	{
		registers.a = registers.l;

		break;
	}
	// LD A,(HL)
	case 0x7E:
	{
		registers.a = main_memory[registers.lh];

		break;
	}
	// LD A,A
	case 0x7F:
	{
		registers.a = registers.a;

		break;
	}
	// ADD A, B
	case 0x80:
	{
		AsmADD_A(registers.b);

		break;
	}
	// ADD A,C
	case 0x81:
	{
		AsmADD_A(registers.c);

		break;
	}
	// ADD A, D
	case 0x82:
	{
		AsmADD_A(registers.d);

		break;
	}
	// ADD A,E
	case 0x83:
	{
		AsmADD_A(registers.e);

		break;
	}
	// ADD A,H
	case 0x84:
	{
		AsmADD_A(registers.h);

		break;
	}
	// ADD A,L
	case 0x85:
	{
		AsmADD_A(registers.l);

		break;
	}
	// ADD A,(HL)
	case 0x86:
	{
		AsmADD_A(main_memory[registers.lh]);

		break;
	}
	// ADD A,A
	case 0x87:
	{
		AsmADD_A(registers.a);

		break;
	}
	// ADC A,B
	case 0x88:
	{
		AsmADC_A(registers.b);

		break;
	}
	// ADC A,C
	case 0x89:
	{
		AsmADC_A(registers.c);

		break;
	}
	// ADC A,D
	case 0x8A:
	{
		AsmADC_A(registers.d);

		break;
	}
	// ADC A,E
	case 0x8B:
	{
		AsmADC_A(registers.e);

		break;
	}
	// ADC A,H
	case 0x8C:
	{
		AsmADC_A(registers.h);

		break;
	}
	// ADC A,L
	case 0x8D:
	{
		AsmADC_A(registers.l);

		break;
	}
	// ADC A,(HL)
	case 0x8E:
	{
		AsmADC_A(main_memory[registers.lh]);

		break;
	}
	// ADC A,A
	case 0x8F:
	{
		AsmADC_A(registers.a);

		break;
	}
	// SUB A,B
	case 0x90:
	{
		AsmSUB_A(registers.b);

		break;
	}
	// SUB A,C
	case 0x91:
	{
		AsmSUB_A(registers.c);

		break;
	}
	// SUB A,D
	case 0x92:
	{
		AsmSUB_A(registers.d);

		break;
	}
	// SUB A,E
	case 0x93:
	{
		AsmSUB_A(registers.e);

		break;
	}
	// SUB A,H
	case 0x94:
	{
		AsmSUB_A(registers.h);

		break;
	}
	// SUB A,L
	case 0x95:
	{
		AsmSUB_A(registers.l);

		break;
	}
	// SUB A,(HL)
	case 0x96:
	{
		AsmSUB_A(main_memory[registers.lh]);

		break;
	}
	// SUB A,A
	case 0x97:
	{
		AsmSUB_A(registers.a);

		break;
	}
	// SBC A,B
	case 0x98:
	{
		AsmSBC_A(registers.b);

		break;
	}
	// SBC A,C
	case 0x99:
	{
		AsmSBC_A(registers.c);

		break;
	}
	// SBC A,D
	case 0x9A:
	{
		AsmSBC_A(registers.d);

		break;
	}
	// SBC A,E
	case 0x9B:
	{
		AsmSBC_A(registers.e);

		break;
	}
	// SBC A,H
	case 0x9C:
	{
		AsmSBC_A(registers.h);

		break;
	}
	// SBC A,L
	case 0x9D:
	{
		AsmSBC_A(registers.l);

		break;
	}
	// SBC A,(HL)
	case 0x9E:
	{
		AsmSBC_A(main_memory[registers.lh]);

		break;
	}
	// SBC A,A
	case 0x9F:
	{
		AsmSBC_A(registers.a);

		break;
	}
	// AND B
	case 0xA0:
	{
		AsmAND_A(registers.b);

		break;
	}
	// AND C
	case 0xA1:
	{
		AsmAND_A(registers.c);

		break;
	}
	// AND D
	case 0xA2:
	{
		AsmAND_A(registers.d);

		break;
	}
	// AND E
	case 0xA3:
	{
		AsmAND_A(registers.e);

		break;
	}
	// AND H
	case 0xA4:
	{
		AsmAND_A(registers.h);

		break;
	}
	// AND L
	case 0xA5:
	{
		AsmAND_A(registers.l);

		break;
	}
	// AND (HL)
	case 0xA6:
	{
		AsmAND_A(main_memory[registers.lh]);

		break;
	}
	// AND A
	case 0xA7:
	{
		AsmAND_A(registers.a);

		break;
	}
	// XOR B
	case 0xA8:
	{
		AsmXOR_A(registers.b);

		break;
	}
	// XOR C
	case 0xA9:
	{
		AsmXOR_A(registers.c);

		break;
	}
	// XOR D
	case 0xAA:
	{
		AsmXOR_A(registers.d);

		break;
	}
	// XOR E
	case 0xAB:
	{
		AsmXOR_A(registers.e);

		break;
	}
	// XOR H
	case 0xAC:
	{
		AsmXOR_A(registers.h);

		break;
	}
	// XOR L
	case 0xAD:
	{
		AsmXOR_A(registers.l);

		break;
	}
	// XOR (HL)
	case 0xAE:
	{
		AsmXOR_A(main_memory[registers.lh]);

		break;
	}
	// XOR A
	case 0xAF:
	{
		AsmXOR_A(registers.a);

		break;
	}
	// OR B
	case 0xB0:
	{
		AsmOR_A(registers.b);

		break;
	}
	// OR C
	case 0xB1:
	{
		AsmOR_A(registers.c);

		break;
	}
	// OR D
	case 0xB2:
	{
		AsmOR_A(registers.d);

		break;
	}
	// OR E
	case 0xB3:
	{
		AsmOR_A(registers.e);

		break;
	}
	// OR H
	case 0xB4:
	{
		AsmOR_A(registers.h);

		break;
	}
	// OR L
	case 0xB5:
	{
		AsmOR_A(registers.l);

		break;
	}
	// OR (HL)
	case 0xB6:
	{
		AsmOR_A(main_memory[registers.lh]);

		break;
	}
	// OR A
	case 0xB7:
	{
		AsmOR_A(registers.a);

		break;
	}
	// CP B
	case 0xB8:
	{
		AsmCP_A(registers.b);

		break;
	}
	// CP C
	case 0xB9:
	{
		AsmCP_A(registers.c);

		break;
	}
	// CP D
	case 0xBA:
	{
		AsmCP_A(registers.d);

		break;
	}
	// CP E
	case 0xBB:
	{
		AsmCP_A(registers.e);

		break;
	}
	// CP H
	case 0xBC:
	{
		AsmCP_A(registers.h);

		break;
	}
	// CP L
	case 0xBD:
	{
		AsmCP_A(registers.l);

		break;
	}
	// CP (HL)
	case 0xBE:
	{
		AsmCP_A(main_memory[registers.lh]);

		break;
	}
	// CP A
	case 0xBF:
	{
		AsmCP_A(registers.a);

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
			AsmCALLnn();

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
		AsmADD_A(main_memory[++pc]);

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
			AsmReturn();

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
	// Extended Operations
	case 0xCB:
	{
		opcode = main_memory[++pc];

		switch (opcode)
		{
			// RLC B
		case 0x00:
		{

		}

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
			AsmCALLnn();

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
		AsmADC_A(main_memory[++pc]);

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
			AsmReturn();

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
	// Removed
	case 0xD3:
	{
		break;
	}
	// CALL NC,nn
	case 0xD4:
	{
		if (GetBitflag(Carry) == 0)
			AsmCALLnn();

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
		AsmSUB_A(main_memory[++pc]);

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
			AsmReturn();

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
	// Removed
	case 0xDB:
	{
		break;
	}
	// CALL C,nn 	
	case 0xDC:
	{
		if (GetBitflag(Carry) == 1)
			AsmCALLnn();

		break;
	}
	// Removed
	case 0xDD:
	{
		break;
	}
	// SBC A,n
	case 0xDE:
	{
		AsmSBC_A(main_memory[++pc]);

		break;
	}

	// RST 18
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
	// Removed
	case 0xE3:
	{
		break;
	}
	// Removed
	case 0xE4:
	{

	}
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
		AsmAND_A(main_memory[++pc]);

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
		// TODO: Signed value
		unsigned char n = main_memory[++pc];

		if ((((sp & 0xF00) + (n & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((sp & 0xF000) + (n & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		sp += n;

		ClearBitflag(Zero);
		ClearBitflag(Subtract);

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
	// Removed
	case 0xEB:
	{
		break;
	}
	// Removed
	case 0xEC:
	{
		break;
	}
	// Removed
	case 0xED:
	{
		break;
	}
	// XOR n
	case 0xEE:
	{
		AsmXOR_A(main_memory[++pc]);

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
	// Removed
	case 0xF2:
	{
		break;
	}
	// DI (Disable Interrupts)
	case 0xF3:
	{
		// TODO: Disable interrupts

		break;
	}
	// Removed
	case 0xF4:
	{
		break;
	}
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
		AsmOR_A(main_memory[++pc]);

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
		unsigned char n = main_memory[++pc];

		if ((((sp & 0xF00) + (n & 0xF00)) & 0x1000) == 0x1000)
			SetBitflag(Half_Carry);
		else
			ClearBitflag(Half_Carry);

		if ((((sp & 0xF000) + (n & 0xF000)) & 0x10000) == 0x10000)
			SetBitflag(Carry);
		else
			ClearBitflag(Carry);

		registers.lh = sp + n;

		ClearBitflag(Zero);
		ClearBitflag(Subtract);

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
	// Removed
	case 0xFC:
	{
		break;
	}
	// Removed
	case 0xFD:
	{
		break;
	}
	// CP n
	case 0xFE:
	{
		AsmCP_A(main_memory[++pc]);

		break;
	}
	// RST 38
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

bool System::IsRunning()
{
	return running;
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
void System::AsmADD_A(unsigned char val)
{
	if ((((registers.a & 0xF) + (val & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	if ((((registers.a & 0xF0) + (val & 0xF0)) & 0x100) == 0x100)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	registers.a += val;

	ClearBitflag(Subtract);
	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
}
void System::AsmADC_A(unsigned char val)
{
	if ((((registers.a & 0xF) + (val + GetBitflag(Carry) & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	if ((((registers.a & 0xF0) + (val + GetBitflag(Carry) & 0xF0)) & 0x100) == 0x100)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	registers.a += val + GetBitflag(Carry);

	ClearBitflag(Subtract);
	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
}
void System::AsmSUB_A(unsigned char val)
{
	if ((((registers.a & 0xF) - (val & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	if ((((registers.a & 0xF0) - (val & 0xF0)) & 0x100) == 0x100)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	registers.a -= val;

	SetBitflag(Subtract);
	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
}
void System::AsmSBC_A(unsigned char val)
{
	if ((((registers.a & 0xF) - (val + GetBitflag(Carry) & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	if ((((registers.a & 0xF0) - (val + GetBitflag(Carry) & 0xF0)) & 0x100) == 0x100)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	registers.a -= val + GetBitflag(Carry);

	SetBitflag(Subtract);
	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
}
void System::AsmAND_A(unsigned char val)
{
	registers.a &= val;

	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	SetBitflag(Half_Carry);
	ClearBitflag(Carry);
}
void System::AsmOR_A(unsigned char val)
{
	registers.a |= val;

	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
	ClearBitflag(Carry);
}
void System::AsmXOR_A(unsigned char val)
{
	registers.a ^= val;

	registers.a == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
	ClearBitflag(Carry);
}
void System::AsmCP_A(unsigned char val)
{
	short diff = registers.a - val;

	diff == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	(diff < 0) ? SetBitflag(Carry) : ClearBitflag(Carry);

	SetBitflag(Subtract);

	if ((((registers.a & 0xF) - (val & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);
}
void System::AsmRLC(unsigned char& val)
{

}