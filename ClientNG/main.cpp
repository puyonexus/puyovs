#include "Game.h"

#include <SDL.h>
#include <SDL_ttf.h>

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	if (TTF_Init() < 0) {
		SDL_Log("Unable to initialize SDL TTF: %s\n", SDL_GetError());
		SDL_Quit();
		return 2;
	}

    PuyoVS::ClientNG::Game game;
	game.run();
	SDL_Quit();
	return 0;
}
