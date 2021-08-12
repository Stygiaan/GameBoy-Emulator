#pragma once

#include "SDL.h"

#include "System.h"
#include "FileLogger.h"

class Input
{
public:
	Input(System* system, FileLogger* logger);
	void UpdateKeymap();

private:
	unsigned char GetKey(SDL_Keycode keycode);

	System* system{};
	FileLogger* logger{};
};

