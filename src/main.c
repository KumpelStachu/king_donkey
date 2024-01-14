#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "hotreload.h"
#include "game.h"

int main(void)
{
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SDL_Window *window = SDL_CreateWindow("King Donkey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_METAL);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  bool quit = false;

  if (!game_hotreload())
    return 1;

  game_init(window, renderer);

  while (!quit)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          quit = true;
          break;
        case SDLK_F5:
        {
          GameState *state = game_pre_reload();
          if (!game_hotreload())
            return 1;
          game_post_reload(state);
          break;
        }
        }
        break;
      }
      game_event(&event);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    game_update();

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();

  return 0;
}
