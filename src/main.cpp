#include <stdio.h>

#include <SDL3\SDL.h>

#include "core.h";

int main(int argc, char* argv[])
{
	platform_init();

	SDL_Window* window{};

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"huge game",					 
		1800,                               
		1200,                              
		SDL_WINDOW_OPENGL
	);
	ASSERT(window != nullptr, "window good");

	if (window == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Event e{};
	while (true)
	{
		SDL_PollEvent(&e);
		if (e.type == SDL_EVENT_QUIT)
			break;
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	platform_destroy();
	return 0;
}