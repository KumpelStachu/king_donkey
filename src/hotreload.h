#pragma once
#include <stdbool.h>
#include "game.h"

#define X(name, ...) extern name##_t *name;
GAME_HOTRELOAD
#undef X
bool game_hotreload(void);
