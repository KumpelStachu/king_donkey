#include <SDL2/SDL.h>
#include <stdio.h>
#include <dlfcn.h>
#include "hotreload.h"

static const char *libplug_file_name = "libgame.so";
static void *libgame = NULL;

#define X(name, ...) name##_t *name = NULL;
GAME_HOTRELOAD
#undef X

bool game_hotreload(void)
{
  SDL_Log("HOTRELOAD: reloading %s", libplug_file_name);

  if (libgame != NULL)
    dlclose(libgame);

  libgame = dlopen(libplug_file_name, RTLD_NOW);
  if (libgame == NULL)
  {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "HOTRELOAD: could not load %s: %s", libplug_file_name, dlerror());
    return false;
  }

#define X(name, ...)                                                                                                            \
  name = dlsym(libgame, #name);                                                                                                 \
  if (name == NULL)                                                                                                             \
  {                                                                                                                             \
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "HOTRELOAD: could not find %s symbol in %s: %s", #name, libplug_file_name, dlerror()); \
    return false;                                                                                                               \
  }
  GAME_HOTRELOAD
#undef X

  return true;
}
