#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "Debug.h"
#include "Input.h"
#include "FileLogger.h"
#include "System.h"
#include "Renderer.h"

int main(int argc, char** argv)
{
	FileLogger* logger = new FileLogger();
	System* system = new System(logger);

	Debug* debug = new Debug(system);
	Input* input = new Input(system, logger);
	Renderer* renderer = new Renderer(system, logger);

	logger->Log(LOG_INFO, "Test", "Test");

	system->LoadRom("./Games/tetris.gb");

	while (system->IsRunning())
	{
		// debug->Step();

		input->UpdateKeymap();

		system->EmulateCycle();

		renderer->Update();
	}

	delete logger;
	delete system;
	delete debug;
	delete input;
	delete renderer;

	return 0;
}