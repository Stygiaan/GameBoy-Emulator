#include "Debug.h"
#include "FileLogger.h"
#include "System.h"

int main()
{
	FileLogger* logger = new FileLogger();
	System* system = new System(logger);

	unsigned char breakpoints[] = { 0x00, 0x15 };
	Debug* debug = new Debug(system);

	/*std::stringstream ss;
	ss << "Unknown Opcode: " << std::hex << 0xDD;

	logger.Log(ss.str());*/

	system->LoadRom("./Games/tetris.gb");

	while (system->IsRunning())
	{
		system->EmulateCycle();

		debug->Step();
	}

	delete logger;
	delete system;
	delete debug;
}