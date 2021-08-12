#pragma once

#include "SDL.h"
#define GLEW_STATIC
#include <GL/glew.h>

#include "System.h"
#include "FileLogger.h"

class Renderer
{
public:
	Renderer(System* system, FileLogger* logger);
	void Update();

private:
	void HandleWindowEvents();
	void Clear();

	SDL_Window* window;
	SDL_GLContext gl_context;

	System* system;
	FileLogger* logger;
};