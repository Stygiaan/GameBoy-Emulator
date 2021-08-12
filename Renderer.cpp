#include "Renderer.h"

Renderer::Renderer(System* system, FileLogger* logger)
{
	this->system = system;
	this->logger = logger;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		logger->Log(LOG_ERROR, "Unable to initialize SDL - Video.");

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("GBE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 400, SDL_WINDOW_OPENGL);
	gl_context = SDL_GL_CreateContext(window);

	GLenum err = glewInit();

	if (err != GLEW_OK)
		logger->Log(LOG_ERROR, glewGetErrorString(err));

	 // logger->Log("Initialized OpenGL version ", glGetString(GL_VERSION));
}

void Renderer::Update()
{
	HandleWindowEvents();

	Clear();

	SDL_GL_SwapWindow(window);
}

void Renderer::HandleWindowEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
		{
			SDL_Quit();
			system->SetRunning(false);

			break;
		}
		}
	}
}

void Renderer::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}