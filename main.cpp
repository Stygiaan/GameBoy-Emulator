#include "FileLogger.h"
#include "System.h"

int main()
{
	FileLogger logger;
	System system(&logger);

	/*std::stringstream ss;
	ss << "Unknown Opcode: " << std::hex << 0xDD;

	logger.Log(ss.str());*/

	system.LoadRom("./Games/tetris.gb");

	while (true)
	{
		if (system.EmulateCycle() != 0)
			return 0;
	}
}