#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "vector.h"
#include "vec2.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define GRID_SIZE 30
#define SCREEN_WIDTH (GRID_SIZE * 32)
#define SCREEN_HEIGHT (GRID_SIZE * 24)
#define TIME_SCALE_MAX 10
#define PAGE_SIZE 10
#define NAME_LENGTH 16

#define GRAVITY 38
#define PLAYER_SPEED 12
#define PLAYER_JUMP 14
#define ENEMY_JUMP 13
#define BARREL_SPEED 4

#define COLLECTIBLE_SCORE 100
#define BARREL_SCORE 150
#define LEVEL_SCORE 1000

#define ENEMY_THROW_COOLDOWN 3.5
#define ENEMY_JUMP_COOLDOWN ENEMY_THROW_COOLDOWN

#define SPRITE_SIZE 30
#define SPRITES          \
  X(player_idle, 2, 150) \
  X(player_run, 2, 150)  \
  X(player_jump, 2, 150) \
  X(player_fall, 2, 150) \
  X(enemy_idle, 1, 150)  \
  X(woman, 7, 150)       \
  X(barrel, 4, 200)      \
  X(platform, 4, 1)      \
  X(collectible, 1, 1)   \
  X(ladder, 4, 1)        \
  X(heart, 2, 1)

#define debug(...) \
  if (gs->debug)   \
  __VA_ARGS__

#define dprintf(...) debug(printf(__VA_ARGS__))

#define RGB(hex) (((hex) >> 16) & 0xFF), (((hex) >> 8) & 0xFF), ((hex) & 0xFF), 0xFF

static const uint32_t Colors[] = {
    0x9a65bf,
    0xbf7465,
    0x65bf6a,
    0x6594bf,
};

typedef struct Sprite
{
  SDL_Texture *texture;
  uint8_t frames;
  uint8_t frame;
  double duration;
  double elapsed;
} Sprite;

typedef struct FloatingText
{
  Vec2 pos;
  double duration;
  double elapsed;
  SDL_Texture *texture;
} FloatingText;

typedef struct Leaderboard
{
  char name[NAME_LENGTH + 1];
  uint32_t score;
} Leaderboard;

typedef struct Entity
{
  Vec2 pos;
  Vec2 size;
  Vec2 vel;
} Entity;
typedef struct Entity Player;
typedef struct Entity Woman;
typedef struct Entity Enemy;
typedef struct Entity Platform;
typedef struct Entity Collectible;
typedef struct Entity Ladder;
typedef struct Entity Barrel;

VECTOR_DECL(Entity)
VECTOR_DECL(FloatingText)
VECTOR_DECL(Leaderboard)

typedef struct GameState
{
  bool debug;
  bool frame_limit;
  const uint8_t *keyboard;
  struct Mouse
  {
    Vec2 pos;
    uint32_t buttons;
  } mouse;
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Event *event;
  TTF_Font *font;
  bool paused;
  double time_scale;
  uint64_t last_frame;
  double delta;
  double delta_unscaled;
  double fps_timer;

  Player player;
  Woman woman;
  Enemy enemy;
  Entity_vector *platforms;
  Entity_vector *collectibles;
  Entity_vector *ladders;
  Entity_vector *barrels;
  FloatingText_vector *floating_texts;

  double play_time;
  uint8_t level;
  uint8_t lives;
  uint32_t score;
  uint64_t time;
  double enemy_jump_cooldown;
  double enemy_throw_cooldown;
  Leaderboard_vector *leaderboard;
  uint16_t leaderboard_page;

  struct Sprites
  {
#define X(n, f, d) Sprite n;
    SPRITES
#undef X
  } sprites;
} GameState;

#define GAME_HOTRELOAD                             \
  X(game_init, void, SDL_Window *, SDL_Renderer *) \
  X(game_pre_reload, GameState *, void)            \
  X(game_post_reload, void, GameState *)           \
  X(game_update, void, void)                       \
  X(game_event, void, SDL_Event *)

#define X(name, ret, ...) typedef ret(name##_t)(__VA_ARGS__);
GAME_HOTRELOAD
#undef X
