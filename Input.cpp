#include "Input.h"

Input::Input(System* system, FileLogger* logger)
{
	this->system = system;
	this->logger = logger;

	SDL_Init(SDL_INIT_EVENTS);
	if (SDL_Init(SDL_INIT_EVENTS) < 0)
		logger->Log(LOG_ERROR, "Unable to initialize SDL - Events.");
}

void Input::UpdateKeymap()
{
	SDL_PumpEvents();

	unsigned char joypad = system->GetInputRegister();

	if ((joypad & (1 << 5)) == 0)
	{
		joypad &= 0xFF & GetKey(SDLK_d) << 0;
		joypad &= 0xFF & GetKey(SDLK_a) << 1;
		joypad &= 0xFF & GetKey(SDLK_w) << 2;
		joypad &= 0xFF & GetKey(SDLK_s) << 3;
	}
	else if ((joypad & (1 << 4)) == 0)
	{
		joypad &= 0xFF & GetKey(SDLK_k) << 0;
		joypad &= 0xFF & GetKey(SDLK_j) << 1;
		joypad &= 0xFF & GetKey(SDLK_n) << 2;
		joypad &= 0xFF & GetKey(SDLK_m) << 3;
	}

	system->SetInputRegister(joypad);
}

unsigned char Input::GetKey(SDL_Keycode keycode)
{
	SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);

	const Uint8* boyboard_state = SDL_GetKeyboardState(NULL);

	return boyboard_state[scancode] ^ 1;
}