#include "System.h"

System::System(FileLogger* logger)
{
	this->logger = logger;
}

void System::Initialize()
{
	std::memset(main_memory, 0, sizeof(main_memory));

	// registers = {0};
	pc = 0x0000;
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

	logger->Log(LOG_INFO, "Loaded Rom of size: ", buffer.size());
}

unsigned char System::GetInputRegister()
{
	return main_memory[0xFF00];
}
void System::SetInputRegister(unsigned char keypad)
{
	main_memory[0xFF00] = keypad;
}

bool System::IsRunning()
{
	return running;
}
void System::SetRunning(bool running)
{
	this->running = running;
}
unsigned char System::GetLastOpcode()
{
	return opcode;
}
unsigned char System::GetNextOpcode()
{
	return main_memory[pc];
}
Registers System::GetRegisters()
{
	return registers;
}
unsigned short System::GetPC()
{
	return pc;
}
unsigned short System::GetSP()
{
	return sp;
}

void System::EmulateCycle()
{
	if (!halted)
	{
		if (pc > sizeof(main_memory))
		{
			logger->Log(LOG_ERROR, "PC points out of memory.");
			return;
		}

		FetchOpcode();
		ExecuteOpcode();
	}

	if (IME)
		ProcessInterrupts();
}

void System::ProcessInterrupts()
{
	unsigned char IF = main_memory[0xFF0F];
	unsigned char IE = main_memory[0xFFFF];
	unsigned char mask = 1;

	for (int i = 0; i <= 4; ++i)
	{
		if ((IF & mask) == mask)
		{
			if ((IE & mask) == mask)
			{
				// main_memory[0xFF0F] = 0;
				main_memory[0xFF0F] ^= mask;

				if (halted)
				{
					++pc;
					halted = false;
				}

				IME = false;

				AsmCALLInterrupt(interrupt_addresses[1]);

				break;
			}
		}

		mask <<= 1;
	}
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
		AsmINC_s(&registers.b);

		break;
	}
	// DEC B
	case 0x05:
	{
		AsmDEC_s(&registers.b);

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
		unsigned char leftmost_bit = (registers.a & (1 << 7)) == 0x80 ? 1 : 0;

		registers.a <<= 1;
		registers.a |= leftmost_bit << 7;

		if (leftmost_bit == 1)
			SetBitflag(Carry);
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
		main_memory[addr] = static_cast<unsigned char>(sp >> 8);
		main_memory[addr + 1] = static_cast<unsigned char>(sp & 0xFF);

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
		AsmINC_s(&registers.c);

		break;
	}
	// DEC C
	case 0x0D:
	{
		AsmDEC_s(&registers.c);

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

		registers.a >>= 1;
		registers.a |= rightmost_bit << 7;

		if (rightmost_bit == 1)
			SetBitflag(Carry);
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
		AsmINC_s(&registers.d);

		break;
	}
	// DEC D
	case 0x15:
	{
		AsmDEC_s(&registers.d);

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

		registers.a <<= 1;

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
		AsmINC_s(&registers.e);

		break;
	}
	// DEC E
	case 0x1D:
	{
		AsmDEC_s(&registers.e);

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
		unsigned char rightmost_bit = (registers.a & 0x1) == 0x1 ? 1 : 0;

		registers.a >>= 1;

		registers.a |= GetBitflag(Carry) << 7;

		if (rightmost_bit == 1)
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
		main_memory[registers.lh] = registers.a;
		++registers.lh;

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
		AsmINC_s(&registers.h);

		break;
	}
	// DEC H
	case 0x25:
	{
		AsmDEC_s(&registers.h);

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
		AsmINC_s(&registers.l);

		break;
	}
	// DEC L
	case 0x2D:
	{
		AsmDEC_s(&registers.l);

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
		AsmINC_s(&main_memory[registers.lh]);

		break;
	}
	// DEC (HL)
	case 0x35:
	{
		AsmDEC_s(&main_memory[registers.lh]);

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
		AsmINC_s(&registers.a);

		break;
	}
	// DEC A
	case 0x3D:
	{
		AsmDEC_s(&registers.a);

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
		halted = true;

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
		pc = (main_memory[pc + 1] << 8) | main_memory[pc + 2];
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
			AsmRLC(&registers.b);

			break;
		}
		// RLC C
		case 0x01:
		{
			AsmRLC(&registers.c);

			break;
		}
		// RLC D
		case 0x02:
		{
			AsmRLC(&registers.d);

			break;
		}
		// RLC E
		case 0x03:
		{
			AsmRLC(&registers.e);

			break;
		}
		// RLC H
		case 0x04:
		{
			AsmRLC(&registers.h);

			break;
		}
		// RLC L
		case 0x05:
		{
			AsmRLC(&registers.l);

			break;
		}
		// RLC (HL)
		case 0x06:
		{
			AsmRLC(&main_memory[registers.lh]);

			break;
		}
		// RLC A
		case 0x07:
		{
			AsmRLC(&registers.a);

			break;
		}
		// RRC B
		case 0x08:
		{
			AsmRRC(&registers.b);

			break;
		}
		// RRC C
		case 0x09:
		{
			AsmRRC(&registers.c);

			break;
		}
		// RRC D
		case 0x0A:
		{
			AsmRRC(&registers.d);

			break;
		}
		// RRC E
		case 0x0B:
		{
			AsmRRC(&registers.e);

			break;
		}
		// RRC H
		case 0x0C:
		{
			AsmRRC(&registers.h);

			break;
		}
		// RRC L
		case 0x0D:
		{
			AsmRRC(&registers.l);

			break;
		}
		// RRC (HL)
		case 0x0E:
		{
			AsmRRC(&main_memory[registers.lh]);

			break;
		}
		// RRC A
		case 0x0F:
		{
			AsmRRC(&registers.a);

			break;
		}
		// RL B
		case 0x10:
		{
			AsmRL(&registers.b);

			break;
		}
		// RL C
		case 0x11:
		{
			AsmRL(&registers.c);

			break;
		}
		// RL D
		case 0x12:
		{
			AsmRL(&registers.d);

			break;
		}
		// RL E
		case 0x13:
		{
			AsmRL(&registers.e);

			break;
		}
		// RL H
		case 0x14:
		{
			AsmRL(&registers.h);

			break;
		}
		// RL L
		case 0x15:
		{
			AsmRL(&registers.l);

			break;
		}
		// RL (HL)
		case 0x16:
		{
			AsmRL(&main_memory[registers.lh]);

			break;
		}
		// RL A
		case 0x17:
		{
			AsmRL(&registers.a);

			break;
		}
		// RR B
		case 0x18:
		{
			AsmRR(&registers.b);

			break;
		}
		// RR C
		case 0x19:
		{
			AsmRR(&registers.c);

			break;
		}
		// RR D
		case 0x1A:
		{
			AsmRR(&registers.d);

			break;
		}
		// RR E
		case 0x1B:
		{
			AsmRR(&registers.e);

			break;
		}
		// RR H
		case 0x1C:
		{
			AsmRR(&registers.h);

			break;
		}
		// RR L
		case 0x1D:
		{
			AsmRR(&registers.l);

			break;
		}
		// RR (HL)
		case 0x1E:
		{
			AsmRR(&main_memory[registers.lh]);

			break;
		}
		// RR A
		case 0x1F:
		{
			AsmRR(&registers.a);

			break;
		}
		// SLA B
		case 0x20:
		{
			AsmSLA(&registers.b);

			break;
		}
		// SLA C
		case 0x21:
		{
			AsmSLA(&registers.c);

			break;
		}
		// SLA D
		case 0x22:
		{
			AsmSLA(&registers.d);

			break;
		}
		// SLA E
		case 0x23:
		{
			AsmSLA(&registers.e);

			break;
		}
		// SLA H
		case 0x24:
		{
			AsmSLA(&registers.h);

			break;
		}
		// SLA L
		case 0x25:
		{
			AsmSLA(&registers.l);

			break;
		}
		// SLA (HL)
		case 0x26:
		{
			AsmSLA(&main_memory[registers.lh]);

			break;
		}
		// SLA A
		case 0x27:
		{
			AsmSLA(&registers.a);

			break;
		}
		// SRA B
		case 0x28:
		{
			AsmSRA(&registers.b);

			break;
		}
		// SRA C
		case 0x29:
		{
			AsmSRA(&registers.c);

			break;
		}
		// SRA D
		case 0x2A:
		{
			AsmSRA(&registers.d);

			break;
		}
		// SRA E
		case 0x2B:
		{
			AsmSRA(&registers.e);

			break;
		}
		// SRA H
		case 0x2C:
		{
			AsmSRA(&registers.h);

			break;
		}
		// SRA L
		case 0x2D:
		{
			AsmSRA(&registers.l);

			break;
		}
		// SRA (HL)
		case 0x2E:
		{
			AsmSRA(&main_memory[registers.lh]);

			break;
		}
		// SRA A
		case 0x2F:
		{
			AsmSRA(&registers.a);

			break;
		}
		// SWAP B
		case 0x30:
		{
			AsmSWAP(&registers.b);

			break;
		}
		// SWAP C
		case 0x31:
		{
			AsmSWAP(&registers.c);

			break;
		}
		// SWAP D
		case 0x32:
		{
			AsmSWAP(&registers.d);

			break;
		}
		// SWAP E
		case 0x33:
		{
			AsmSWAP(&registers.e);

			break;
		}
		// SWAP H
		case 0x34:
		{
			AsmSWAP(&registers.l);

			break;
		}
		// SWAP L
		case 0x35:
		{
			AsmSWAP(&registers.l);

			break;
		}
		// SWAP (HL)
		case 0x36:
		{
			AsmSWAP(&main_memory[registers.lh]);

			break;
		}
		// SWAP A
		case 0x37:
		{
			AsmSWAP(&registers.a);

			break;
		}
		// SRL B
		case 0x38:
		{
			AsmSRL(&registers.b);

			break;
		}
		// SRL C
		case 0x39:
		{
			AsmSRL(&registers.c);

			break;
		}
		// SRL D
		case 0x3A:
		{
			AsmSRL(&registers.d);

			break;
		}
		// SRL E
		case 0x3B:
		{
			AsmSRL(&registers.e);

			break;
		}
		// SRL H
		case 0x3C:
		{
			AsmSRL(&registers.h);

			break;
		}
		// SRL L
		case 0x3D:
		{
			AsmSRL(&registers.l);

			break;
		}
		// SRL (HL)
		case 0x3E:
		{
			AsmSRL(&main_memory[registers.b]);

			break;
		}
		// SRL A
		case 0x3F:
		{
			AsmSRL(&registers.a);

			break;
		}
		// BIT 0, B
		case 0x40:
		{
			AsmBIT(registers.b, 0);

			break;
		}
		// BIT 0, C
		case 0x41:
		{
			AsmBIT(registers.c, 0);

			break;
		}
		// BIT 0, D
		case 0x42:
		{
			AsmBIT(registers.d, 0);

			break;
		}
		// BIT 0, E
		case 0x43:
		{
			AsmBIT(registers.e, 0);

			break;
		}
		// BIT 0, H
		case 0x44:
		{
			AsmBIT(registers.h, 0);

			break;
		}
		// BIT 0, L
		case 0x45:
		{
			AsmBIT(registers.l, 0);

			break;
		}
		// BIT 0, (HL)
		case 0x46:
		{
			AsmBIT(main_memory[registers.lh], 0);

			break;
		}
		// BIT 0, A
		case 0x47:
		{
			AsmBIT(registers.a, 0);

			break;
		}
		// BIT 1, B
		case 0x48:
		{
			AsmBIT(registers.b, 1);

			break;
		}
		// BIT 1, C
		case 0x49:
		{
			AsmBIT(registers.c, 1);

			break;
		}
		// BIT 1, D
		case 0x4A:
		{
			AsmBIT(registers.d, 1);

			break;
		}
		// BIT 1, E
		case 0x4B:
		{
			AsmBIT(registers.e, 1);

			break;
		}
		// BIT 1, H
		case 0x4C:
		{
			AsmBIT(registers.h, 1);

			break;
		}
		// BIT 1, L
		case 0x4D:
		{
			AsmBIT(registers.l, 1);

			break;
		}
		// BIT 1, (HL)
		case 0x4E:
		{
			AsmBIT(main_memory[registers.lh], 1);

			break;
		}
		// BIT 1, A
		case 0x4F:
		{
			AsmBIT(registers.a, 1);

			break;
		}
		// BIT 2, B
		case 0x50:
		{
			AsmBIT(registers.b, 2);

			break;
		}
		// BIT 2, C
		case 0x51:
		{
			AsmBIT(registers.c, 2);

			break;
		}
		// BIT 2, D
		case 0x52:
		{
			AsmBIT(registers.d, 2);

			break;
		}
		// BIT 2, E
		case 0x53:
		{
			AsmBIT(registers.e, 2);

			break;
		}
		// BIT 2, H
		case 0x54:
		{
			AsmBIT(registers.h, 2);

			break;
		}
		// BIT 2, L
		case 0x55:
		{
			AsmBIT(registers.l, 2);

			break;
		}
		// BIT 2, (HL)
		case 0x56:
		{
			AsmBIT(main_memory[registers.lh], 2);

			break;
		}
		// BIT 2, A
		case 0x57:
		{
			AsmBIT(registers.a, 2);

			break;
		}
		// BIT 3, B
		case 0x58:
		{
			AsmBIT(registers.b, 3);

			break;
		}
		// BIT 3, C
		case 0x59:
		{
			AsmBIT(registers.c, 3);

			break;
		}
		// BIT 3, D
		case 0x5A:
		{
			AsmBIT(registers.d, 3);

			break;
		}
		// BIT 3, E
		case 0x5B:
		{
			AsmBIT(registers.e, 3);

			break;
		}
		// BIT 3, H
		case 0x5C:
		{
			AsmBIT(registers.h, 3);

			break;
		}
		// BIT 3, L
		case 0x5D:
		{
			AsmBIT(registers.l, 3);

			break;
		}
		// BIT 3, (HL)
		case 0x5E:
		{
			AsmBIT(main_memory[registers.lh], 3);

			break;
		}
		// BIT 3, A
		case 0x5F:
		{
			AsmBIT(registers.a, 3);

			break;
		}
		// BIT 4, B
		case 0x60:
		{
			AsmBIT(registers.b, 4);

			break;
		}
		// BIT 4, C
		case 0x61:
		{
			AsmBIT(registers.c, 4);

			break;
		}
		// BIT 4, D
		case 0x62:
		{
			AsmBIT(registers.d, 4);

			break;
		}
		// BIT 4, E
		case 0x63:
		{
			AsmBIT(registers.e, 4);

			break;
		}
		// BIT 4, H
		case 0x64:
		{
			AsmBIT(registers.h, 4);

			break;
		}
		// BIT 4, L
		case 0x65:
		{
			AsmBIT(registers.l, 4);

			break;
		}
		// BIT 4, (HL)
		case 0x66:
		{
			AsmBIT(main_memory[registers.lh], 4);

			break;
		}
		// BIT 4, A
		case 0x67:
		{
			AsmBIT(registers.a, 4);

			break;
		}
		// BIT 5, B
		case 0x68:
		{
			AsmBIT(registers.b, 5);

			break;
		}
		// BIT 5, C
		case 0x69:
		{
			AsmBIT(registers.c, 5);

			break;
		}
		// BIT 5, D
		case 0x6A:
		{
			AsmBIT(registers.d, 5);

			break;
		}
		// BIT 5, E
		case 0x6B:
		{
			AsmBIT(registers.e, 5);

			break;
		}
		// BIT 5, H
		case 0x6C:
		{
			AsmBIT(registers.h, 5);

			break;
		}
		// BIT 5, L
		case 0x6D:
		{
			AsmBIT(registers.l, 5);

			break;
		}
		// BIT 5, (HL)
		case 0x6E:
		{
			AsmBIT(main_memory[registers.lh], 5);

			break;
		}
		// BIT 5, A
		case 0x6F:
		{
			AsmBIT(registers.a, 5);

			break;
		}
		// BIT 6, B
		case 0x70:
		{
			AsmBIT(registers.b, 6);

			break;
		}
		// BIT 6, C
		case 0x71:
		{
			AsmBIT(registers.c, 6);

			break;
		}
		// BIT 6, D
		case 0x72:
		{
			AsmBIT(registers.d, 6);

			break;
		}
		// BIT 6, E
		case 0x73:
		{
			AsmBIT(registers.e, 6);

			break;
		}
		// BIT 6, H
		case 0x74:
		{
			AsmBIT(registers.h, 6);

			break;
		}
		// BIT 6, L
		case 0x75:
		{
			AsmBIT(registers.l, 6);

			break;
		}
		// BIT 6, (HL)
		case 0x76:
		{
			AsmBIT(main_memory[registers.lh], 6);

			break;
		}
		// BIT 6, A
		case 0x77:
		{
			AsmBIT(registers.a, 6);

			break;
		}
		// BIT 7, B
		case 0x78:
		{
			AsmBIT(registers.b, 7);

			break;
		}
		// BIT 7, C
		case 0x79:
		{
			AsmBIT(registers.c, 7);

			break;
		}
		// BIT 7, D
		case 0x7A:
		{
			AsmBIT(registers.d, 7);

			break;
		}
		// BIT 7, E
		case 0x7B:
		{
			AsmBIT(registers.e, 7);

			break;
		}
		// BIT 7, H
		case 0x7C:
		{
			AsmBIT(registers.h, 7);

			break;
		}
		// BIT 7, L
		case 0x7D:
		{
			AsmBIT(registers.l, 7);

			break;
		}
		// BIT 7, (HL)
		case 0x7E:
		{
			AsmBIT(main_memory[registers.lh], 7);

			break;
		}
		// BIT 7, A
		case 0x7F:
		{
			AsmBIT(registers.a, 7);

			break;
		}
		// RES 0, B
		case 0x80:
		{
			AsmRES(&registers.b, 0);

			break;
		}
		// RES 0, C
		case 0x81:
		{
			AsmRES(&registers.c, 0);

			break;
		}
		// RES 0, D
		case 0x82:
		{
			AsmRES(&registers.d, 0);

			break;
		}
		// RES 0, E
		case 0x83:
		{
			AsmRES(&registers.e, 0);

			break;
		}
		// RES 0, H
		case 0x84:
		{
			AsmRES(&registers.h, 0);

			break;
		}
		// RES 0, L
		case 0x85:
		{
			AsmRES(&registers.l, 0);

			break;
		}
		// RES 0, (HL)
		case 0x86:
		{
			AsmRES(&main_memory[registers.lh], 0);

			break;
		}
		// RES 0, A
		case 0x87:
		{
			AsmRES(&registers.a, 0);

			break;
		}
		// RES 1, B
		case 0x88:
		{
			AsmRES(&registers.b, 1);

			break;
		}
		// RES 1, C
		case 0x89:
		{
			AsmRES(&registers.c, 1);

			break;
		}
		// RES 1, D
		case 0x8A:
		{
			AsmRES(&registers.d, 1);

			break;
		}
		// RES 1, E
		case 0x8B:
		{
			AsmRES(&registers.e, 1);

			break;
		}
		// RES 1, H
		case 0x8C:
		{
			AsmRES(&registers.h, 1);

			break;
		}
		// RES 1, L
		case 0x8D:
		{
			AsmRES(&registers.l, 1);

			break;
		}
		// RES 1, (HL)
		case 0x8E:
		{
			AsmRES(&main_memory[registers.lh], 1);

			break;
		}
		// RES 1, A
		case 0x8F:
		{
			AsmRES(&registers.a, 1);

			break;
		}
		// RES 2, B
		case 0x90:
		{
			AsmRES(&registers.b, 2);

			break;
		}
		// RES 2, C
		case 0x91:
		{
			AsmRES(&registers.c, 2);

			break;
		}
		// RES 2, D
		case 0x92:
		{
			AsmRES(&registers.d, 2);

			break;
		}
		// RES 2, E
		case 0x93:
		{
			AsmRES(&registers.e, 2);

			break;
		}
		// RES 2, H
		case 0x94:
		{
			AsmRES(&registers.h, 2);

			break;
		}
		// RES 2, L
		case 0x95:
		{
			AsmRES(&registers.l, 2);

			break;
		}
		// RES 2, (HL)
		case 0x96:
		{
			AsmRES(&main_memory[registers.lh], 2);

			break;
		}
		// RES 2, A
		case 0x97:
		{
			AsmRES(&registers.a, 2);

			break;
		}
		// RES 3, B
		case 0x98:
		{
			AsmRES(&registers.b, 3);

			break;
		}
		// RES 3, C
		case 0x99:
		{
			AsmRES(&registers.c, 3);

			break;
		}
		// RES 3, D
		case 0x9A:
		{
			AsmRES(&registers.d, 3);

			break;
		}
		// RES 3, E
		case 0x9B:
		{
			AsmRES(&registers.e, 3);

			break;
		}
		// RES 3, H
		case 0x9C:
		{
			AsmRES(&registers.h, 3);

			break;
		}
		// RES 3, L
		case 0x9D:
		{
			AsmRES(&registers.l, 3);

			break;
		}
		// RES 3, (HL)
		case 0x9E:
		{
			AsmRES(&main_memory[registers.lh], 3);

			break;
		}
		// RES 3, A
		case 0x9F:
		{
			AsmRES(&registers.a, 3);

			break;
		}
		// RES 4, B
		case 0xA0:
		{
			AsmRES(&registers.b, 4);

			break;
		}
		// RES 4, C
		case 0xA1:
		{
			AsmRES(&registers.c, 4);

			break;
		}
		// RES 4, D
		case 0xA2:
		{
			AsmRES(&registers.d, 4);

			break;
		}
		// RES 4, E
		case 0xA3:
		{
			AsmRES(&registers.e, 4);

			break;
		}
		// RES 4, H
		case 0xA4:
		{
			AsmRES(&registers.h, 4);

			break;
		}
		// RES 4, L
		case 0xA5:
		{
			AsmRES(&registers.l, 4);

			break;
		}
		// RES 4, (HL)
		case 0xA6:
		{
			AsmRES(&main_memory[registers.lh], 4);

			break;
		}
		// RES 4, A
		case 0xA7:
		{
			AsmRES(&registers.a, 4);

			break;
		}
		// RES 5, B
		case 0xA8:
		{
			AsmRES(&registers.b, 5);

			break;
		}
		// RES 5, C
		case 0xA9:
		{
			AsmRES(&registers.c, 5);

			break;
		}
		// RES 5, D
		case 0xAA:
		{
			AsmRES(&registers.d, 5);

			break;
		}
		// RES 5, E
		case 0xAB:
		{
			AsmRES(&registers.e, 5);

			break;
		}
		// RES 5, H
		case 0xAC:
		{
			AsmRES(&registers.h, 5);

			break;
		}
		// RES 5, L
		case 0xAD:
		{
			AsmRES(&registers.l, 5);

			break;
		}
		// RES 5, (HL)
		case 0xAE:
		{
			AsmRES(&main_memory[registers.lh], 5);

			break;
		}
		// RES 5, A
		case 0xAF:
		{
			AsmRES(&registers.a, 5);

			break;
		}
		// RES 6, B
		case 0xB0:
		{
			AsmRES(&registers.b, 6);

			break;
		}
		// RES 6, C
		case 0xB1:
		{
			AsmRES(&registers.c, 6);

			break;
		}
		// RES 6, D
		case 0xB2:
		{
			AsmRES(&registers.d, 6);

			break;
		}
		// RES 6, E
		case 0xB3:
		{
			AsmRES(&registers.e, 6);

			break;
		}
		// RES 6, H
		case 0xB4:
		{
			AsmRES(&registers.h, 6);

			break;
		}
		// RES 6, L
		case 0xB5:
		{
			AsmRES(&registers.l, 6);

			break;
		}
		// RES 6, (HL)
		case 0xB6:
		{
			AsmRES(&main_memory[registers.lh], 6);

			break;
		}
		// RES 6, A
		case 0xB7:
		{
			AsmRES(&registers.a, 6);

			break;
		}
		// RES 7, B
		case 0xB8:
		{
			AsmRES(&registers.b, 7);

			break;
		}
		// RES 7, C
		case 0xB9:
		{
			AsmRES(&registers.c, 7);

			break;
		}
		// RES 7, D
		case 0xBA:
		{
			AsmRES(&registers.d, 7);

			break;
		}
		// RES 7, E
		case 0xBB:
		{
			AsmRES(&registers.e, 7);

			break;
		}
		// RES 7, H
		case 0xBC:
		{
			AsmRES(&registers.h, 7);

			break;
		}
		// RES 7, L
		case 0xBD:
		{
			AsmRES(&registers.l, 7);

			break;
		}
		// RES 7, (HL)
		case 0xBE:
		{
			AsmRES(&main_memory[registers.lh], 7);

			break;
		}
		// RES 7, A
		case 0xBF:
		{
			AsmRES(&registers.a, 7);

			break;
		}
		// SET 0, B
		case 0xC0:
		{
			AsmSET(&registers.b, 0);

			break;
		}
		// SET 0, C
		case 0xC1:
		{
			AsmSET(&registers.c, 0);

			break;
		}
		// SET 0, D
		case 0xC2:
		{
			AsmSET(&registers.d, 0);

			break;
		}
		// SET 0, E
		case 0xC3:
		{
			AsmSET(&registers.e, 0);

			break;
		}
		// SET 0, H
		case 0xC4:
		{
			AsmSET(&registers.h, 0);

			break;
		}
		// SET 0, L
		case 0xC5:
		{
			AsmSET(&registers.l, 0);

			break;
		}
		// SET 0, (HL)
		case 0xC6:
		{
			AsmSET(&main_memory[registers.lh], 0);

			break;
		}
		// SET 0, A
		case 0xC7:
		{
			AsmSET(&registers.a, 0);

			break;
		}
		// SET 1, B
		case 0xC8:
		{
			AsmSET(&registers.b, 1);

			break;
		}
		// SET 1, C
		case 0xC9:
		{
			AsmSET(&registers.c, 1);

			break;
		}
		// SET 1, D
		case 0xCA:
		{
			AsmSET(&registers.d, 1);

			break;
		}
		// SET 1, E
		case 0xCB:
		{
			AsmSET(&registers.e, 1);

			break;
		}
		// SET 1, H
		case 0xCC:
		{
			AsmSET(&registers.h, 1);

			break;
		}
		// SET 1, L
		case 0xCD:
		{
			AsmSET(&registers.l, 1);

			break;
		}
		// SET 1, (HL)
		case 0xCE:
		{
			AsmSET(&main_memory[registers.lh], 1);

			break;
		}
		// SET 1, A
		case 0xCF:
		{
			AsmSET(&registers.a, 1);

			break;
		}
		// SET 2, B
		case 0xD0:
		{
			AsmSET(&registers.b, 2);

			break;
		}
		// SET 2, C
		case 0xD1:
		{
			AsmSET(&registers.c, 2);

			break;
		}
		// SET 2, D
		case 0xD2:
		{
			AsmSET(&registers.d, 2);

			break;
		}
		// SET 2, E
		case 0xD3:
		{
			AsmSET(&registers.e, 2);

			break;
		}
		// SET 2, H
		case 0xD4:
		{
			AsmSET(&registers.h, 2);

			break;
		}
		// SET 2, L
		case 0xD5:
		{
			AsmSET(&registers.l, 2);

			break;
		}
		// SET 2, (HL)
		case 0xD6:
		{
			AsmSET(&main_memory[registers.lh], 2);

			break;
		}
		// SET 2, A
		case 0xD7:
		{
			AsmSET(&registers.a, 2);

			break;
		}
		// SET 3, B
		case 0xD8:
		{
			AsmSET(&registers.b, 3);

			break;
		}
		// SET 3, C
		case 0xD9:
		{
			AsmSET(&registers.c, 3);

			break;
		}
		// SET 3, D
		case 0xDA:
		{
			AsmSET(&registers.d, 3);

			break;
		}
		// SET 3, E
		case 0xDB:
		{
			AsmSET(&registers.e, 3);

			break;
		}
		// SET 3, H
		case 0xDC:
		{
			AsmSET(&registers.h, 3);

			break;
		}
		// SET 3, L
		case 0xDD:
		{
			AsmSET(&registers.l, 3);

			break;
		}
		// SET 3, (HL)
		case 0xDE:
		{
			AsmSET(&main_memory[registers.lh], 3);

			break;
		}
		// SET 3, A
		case 0xDF:
		{
			AsmSET(&registers.a, 3);

			break;
		}
		// SET 4, B
		case 0xE0:
		{
			AsmSET(&registers.b, 4);

			break;
		}
		// SET 4, C
		case 0xE1:
		{
			AsmSET(&registers.c, 4);

			break;
		}
		// SET 4, D
		case 0xE2:
		{
			AsmSET(&registers.d, 4);

			break;
		}
		// SET 4, E
		case 0xE3:
		{
			AsmSET(&registers.e, 4);

			break;
		}
		// SET 4, H
		case 0xE4:
		{
			AsmSET(&registers.h, 4);

			break;
		}
		// SET 4, L
		case 0xE5:
		{
			AsmSET(&registers.l, 4);

			break;
		}
		// SET 4, (HL)
		case 0xE6:
		{
			AsmSET(&main_memory[registers.lh], 4);

			break;
		}
		// SET 4, A
		case 0xE7:
		{
			AsmSET(&registers.a, 4);

			break;
		}
		// SET 5, B
		case 0xE8:
		{
			AsmSET(&registers.b, 5);

			break;
		}
		// SET 5, C
		case 0xE9:
		{
			AsmSET(&registers.c, 5);

			break;
		}
		// SET 5, D
		case 0xEA:
		{
			AsmSET(&registers.d, 5);

			break;
		}
		// SET 5, E
		case 0xEB:
		{
			AsmSET(&registers.e, 5);

			break;
		}
		// SET 5, H
		case 0xEC:
		{
			AsmSET(&registers.h, 5);

			break;
		}
		// SET 5, L
		case 0xED:
		{
			AsmSET(&registers.l, 5);

			break;
		}
		// SET 5, (HL)
		case 0xEE:
		{
			AsmSET(&main_memory[registers.lh], 5);

			break;
		}
		// SET 5, A
		case 0xEF:
		{
			AsmSET(&registers.a, 5);

			break;
		}
		// SET 6, B
		case 0xF0:
		{
			AsmSET(&registers.b, 6);

			break;
		}
		// SET 6, C
		case 0xF1:
		{
			AsmSET(&registers.c, 6);

			break;
		}
		// SET 6, D
		case 0xF2:
		{
			AsmSET(&registers.d, 6);

			break;
		}
		// SET 6, E
		case 0xF3:
		{
			AsmSET(&registers.e, 6);

			break;
		}
		// SET 6, H
		case 0xF4:
		{
			AsmSET(&registers.h, 6);

			break;
		}
		// SET 6, L
		case 0xF5:
		{
			AsmSET(&registers.l, 6);

			break;
		}
		// SET 6, (HL)
		case 0xF6:
		{
			AsmSET(&main_memory[registers.lh], 6);

			break;
		}
		// SET 6, A
		case 0xF7:
		{
			AsmSET(&registers.a, 6);

			break;
		}
		// SET 7, B
		case 0xF8:
		{
			AsmSET(&registers.b, 7);

			break;
		}
		// SET 7, C
		case 0xF9:
		{
			AsmSET(&registers.c, 7);

			break;
		}
		// SET 7, D
		case 0xFA:
		{
			AsmSET(&registers.d, 7);

			break;
		}
		// SET 7, E
		case 0xFB:
		{
			AsmSET(&registers.e, 7);

			break;
		}
		// SET 7, H
		case 0xFC:
		{
			AsmSET(&registers.h, 7);

			break;
		}
		// SET 7, L
		case 0xFD:
		{
			AsmSET(&registers.l, 7);

			break;
		}
		// SET 7, (HL)
		case 0xFE:
		{
			AsmSET(&main_memory[registers.lh], 7);

			break;
		}
		// SET 7, A
		case 0xFF:
		{
			AsmSET(&registers.a, 7);

			break;
		}

		default:
		{
			std::stringstream ss;
			ss << "Unknown Extended(BC) Opcode: " << std::hex << static_cast<int>(opcode);

			logger->Log(LOG_WARNING, ss.str());
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

		IME = true;

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
		main_memory[0xFF0F] = 0;
		main_memory[0xFFFF] = 0;
		IME = false;

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
		IME = true;

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

		logger->Log(LOG_WARNING, ss.str());

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

void System::AsmINC_s(unsigned char* val)
{
	if ((((*val & 0xF) + (1 & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	++*val;

	ClearBitflag(Subtract);
	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
}
void System::AsmDEC_s(unsigned char* val)
{
	if ((((*val & 0xF) - (1 & 0xF)) & 0x10) == 0x10)
		SetBitflag(Half_Carry);
	else
		ClearBitflag(Half_Carry);

	--*val;

	SetBitflag(Subtract);
	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
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
void System::AsmCALLInterrupt(short addr)
{
	main_memory[--sp] = static_cast<unsigned char>((pc) & 0x00FF);
	main_memory[--sp] = static_cast<unsigned char>(((pc) & 0xFF00) >> 8);

	pc = addr;
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
void System::AsmRLC(unsigned char* val)
{
	unsigned char leftmost_bit = (*val & (0x1 << 7)) == 0x80 ? 1 : 0;

	*val <<= 1;
	*val |= leftmost_bit;

	if (leftmost_bit == 1)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmRRC(unsigned char* val)
{
	unsigned char rightmost_bit = (*val & 0x1) == 0x1 ? 1 : 0;

	*val >>= 1;
	*val |= rightmost_bit << 7;

	if (rightmost_bit == 1)
		SetBitflag(Carry);
	else
		ClearBitflag(Carry);

	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmRL(unsigned char* val)
{
	unsigned char leftmost_bit = (*val & (0x1 << 7)) == 0x80 ? 1 : 0;

	*val <<= 1;
	*val |= GetBitflag(Carry);

	leftmost_bit == 1 ? SetBitflag(Carry) : ClearBitflag(Carry);

	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmRR(unsigned char* val)
{
	unsigned char rightmost_bit = (*val & 0x1) == 0x1 ? 1 : 0;

	*val >>= 1;
	*val |= GetBitflag(Carry) << 7;

	rightmost_bit == 1 ? SetBitflag(Carry) : ClearBitflag(Carry);

	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmSLA(unsigned char* val)
{
	unsigned char leftmost_bit = (*val & (0x1 << 7)) == 0x80 ? 1 : 0;

	*val <<= 1;

	leftmost_bit == 1 ? SetBitflag(Carry) : ClearBitflag(Carry);
	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmSRA(unsigned char* val)
{
	unsigned char rightmost_bit = (*val & 0x1) == 0x1 ? 1 : 0;

	*val <<= 1;
	*val |= rightmost_bit << 7;

	rightmost_bit == 1 ? SetBitflag(Carry) : ClearBitflag(Carry);
	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmSWAP(unsigned char* val)
{
	unsigned char low_nibble = *val & 0x0F;

	*val >>= 4;
	*val &= low_nibble << 4;

	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
	ClearBitflag(Carry);
}
void System::AsmSRL(unsigned char* val)
{
	unsigned char rightmost_bit = (*val & 0x1) == 0x1 ? 1 : 0;

	*val >>= 1;

	rightmost_bit == 1 ? SetBitflag(Carry) : ClearBitflag(Carry);
	*val == 0 ? SetBitflag(Zero) : ClearBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmBIT(unsigned char val, short bit)
{
	(val & (1 << bit)) == (1 << bit) ? ClearBitflag(Zero) : SetBitflag(Zero);
	ClearBitflag(Subtract);
	ClearBitflag(Half_Carry);
}
void System::AsmRES(unsigned char* val, short bit)
{
	*val &= 0xFF ^ (1 << bit);
}
void System::AsmSET(unsigned char* val, short bit)
{
	*val |= 1 << bit;
}