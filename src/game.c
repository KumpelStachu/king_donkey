#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "game.h"

VECTOR_IMPL(Entity)
VECTOR_IMPL(FloatingText)
VECTOR_IMPL(Leaderboard)

static GameState *gs;

#define ERect(entity) ((SDL_Rect){(entity).pos.x, (entity).pos.y, (entity).size.x, (entity).size.y})
#define REAL_LEVEL (gs->level > 0 && gs->level < 4)
#define LEVEL_COLOR RGB(Colors[gs->level % (sizeof(Colors) / sizeof(Colors[0]))])
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void reset_animations(void)
{
  for (uint8_t i = 0; i < sizeof(gs->sprites) / sizeof(Sprite); i++)
  {
    Sprite *sprite = &((Sprite *)&gs->sprites)[i];
    sprite->frame = 0;
    sprite->elapsed = 0;
  }

  gs->enemy_jump_cooldown = ENEMY_JUMP_COOLDOWN;
  gs->enemy_throw_cooldown = ENEMY_THROW_COOLDOWN;
}

SDL_Texture *render_text(const char *text, SDL_Color color)
{
  SDL_Surface *surface = TTF_RenderText_Solid(gs->font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surface);
  SDL_FreeSurface(surface);

  return texture;
}

void show_floating_text(const char *text, Vec2 pos, double duration)
{
  SDL_Texture *texture = render_text(text, (SDL_Color){255, 255, 255, 255});

  FloatingText_vector_push(gs->floating_texts, &(FloatingText){.pos = pos, .duration = duration, .elapsed = 0, .texture = texture});
}

SDL_Texture *load_texture(const char *name)
{
  size_t filename_len = strlen(name) + 12;
  char *filename = malloc(filename_len);
  snprintf(filename, filename_len, "assets/%s.bmp", name);

  SDL_Surface *surface = SDL_LoadBMP(filename);
  Uint32 key = SDL_MapRGB(surface->format, 0, 0, 0);
  SDL_SetColorKey(surface, SDL_TRUE, key);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surface);
  assert(texture != NULL);

  free(filename);
  SDL_FreeSurface(surface);

  return texture;
}

int leaderboard_comparator(const Leaderboard *a, const Leaderboard *b)
{
  return b->score - a->score;
}

void load_leaderboard(void)
{
  FILE *file = fopen("assets/leaderboard.kd", "r");
  if (file == NULL)
    return;

  Leaderboard_vector_clear(gs->leaderboard);

  while (!feof(file))
  {
    Leaderboard *entry = Leaderboard_vector_push(gs->leaderboard, &(Leaderboard){0});
    fscanf(file, "%d %[^\n]s\n", &entry->score, &entry->name);
  }

  Leaderboard_vector_sort(gs->leaderboard, &leaderboard_comparator);

  fclose(file);
}

void unload_level()
{
  reset_animations();

  Entity_vector_clear(gs->platforms);
  Entity_vector_clear(gs->collectibles);
  Entity_vector_clear(gs->ladders);
  Entity_vector_clear(gs->barrels);
  FloatingText_vector_clear(gs->floating_texts);

  Entity_vector_push(gs->platforms, &(Platform){.pos = {0, SCREEN_HEIGHT - GRID_SIZE}, .size = {SCREEN_WIDTH, GRID_SIZE}});
}

void load_level(uint8_t level)
{
  char filename[] = "assets/level0.kd";
  filename[12] += level;

  dprintf("Loading %s\n", filename);

  FILE *file = fopen(filename, "r");
  if (file == NULL)
    goto error;

  unload_level();

  char type[16];
  while (!feof(file))
  {
    fscanf(file, "%s\n", type);

    if (!strcmp(type, "Player"))
    {
      fscanf(file, "pos(%lf %lf)\n", &gs->player.pos.x, &gs->player.pos.y);
      gs->player.pos = Vec2_Mul(gs->player.pos, GRID_SIZE);

      gs->player.vel = (Vec2){0};
      gs->player.size = (Vec2){GRID_SIZE, GRID_SIZE * 2};
    }
    else if (!strcmp(type, "Woman"))
    {
      fscanf(file, "pos(%lf %lf)\n", &gs->woman.pos.x, &gs->woman.pos.y);
      gs->woman.pos = Vec2_Mul(gs->woman.pos, GRID_SIZE);

      gs->woman.vel = (Vec2){0};
      gs->woman.size = (Vec2){GRID_SIZE * 2, GRID_SIZE * 3};
    }
    else if (!strcmp(type, "Enemy"))
    {
      fscanf(file, "pos(%lf %lf)\n", &gs->enemy.pos.x, &gs->enemy.pos.y);
      gs->enemy.pos = Vec2_Mul(gs->enemy.pos, GRID_SIZE);

      gs->enemy.vel = (Vec2){0};
      gs->enemy.size = (Vec2){GRID_SIZE * 3, GRID_SIZE * 5};
    }
    else if (!strcmp(type, "Platform"))
    {
      Platform *platform = Entity_vector_push(gs->platforms, &(Platform){0});

      fscanf(file, "pos(%lf %lf)\n", &platform->pos.x, &platform->pos.y);
      platform->pos = Vec2_Mul(platform->pos, GRID_SIZE);

      fscanf(file, "size(%lf %lf)\n", &platform->size.x, &platform->size.y);
      platform->size = Vec2_Mul(platform->size, GRID_SIZE);
    }
    else if (!strcmp(type, "Collectible"))
    {
      Collectible *collectible = Entity_vector_push(gs->collectibles, &(Collectible){0});

      fscanf(file, "pos(%lf %lf)\n", &collectible->pos.x, &collectible->pos.y);
      collectible->pos = Vec2_Mul(collectible->pos, GRID_SIZE);

      collectible->size = (Vec2){GRID_SIZE, GRID_SIZE};
    }
    else if (!strcmp(type, "Ladder"))
    {
      Ladder *ladder = Entity_vector_push(gs->ladders, &(Ladder){0});

      fscanf(file, "pos(%lf %lf)\n", &ladder->pos.x, &ladder->pos.y);
      ladder->pos = Vec2_Mul(ladder->pos, GRID_SIZE);

      fscanf(file, "size(%lf %lf)\n", &ladder->size.x, &ladder->size.y);
      ladder->size = Vec2_Mul(ladder->size, GRID_SIZE);
    }
    else
      goto error;
  }

  if (level == 0)
  {
    assert(gs->ladders->size > 0);
    load_leaderboard();
  }
  else if (level == 4)
  {
    Leaderboard_vector_push(gs->leaderboard, &(Leaderboard){.score = gs->score, .name = {0}});
  }

  gs->level = level;

  goto cleanup;

error:
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load %s", filename);
cleanup:
  if (file != NULL)
    fclose(file);
}

Platform *intersect_platform(Entity *entity)
{
  for (size_t i = 0; i < gs->platforms->size; i++)
  {
    Platform *platform = &gs->platforms->data[i];
    if (SDL_HasIntersection(&ERect(*entity), &ERect(*platform)))
      return platform;
  }
  return NULL;
}

Ladder *intersect_ladder(Entity *entity)
{
  for (size_t i = 0; i < gs->ladders->size; i++)
  {
    Ladder *ladder = &gs->ladders->data[i];

    SDL_Rect intersection;
    if (SDL_IntersectRect(&ERect(gs->player), &ERect(*ladder), &intersection))
    {
      if (intersection.h >= gs->player.size.y / 2 && intersection.w >= gs->player.size.x / 2)
        return ladder;
    }
  }

  return NULL;
}

#define INTER_LEFT(v) (v.x <= 0)
#define INTER_RIGHT(v) (v.x >= 0)
#define INTER_TOP(v) (v.y <= 0)
#define INTER_BOTTOM(v) (v.y >= 0)
Vec2 where_intersection(Entity *a, Entity *b)
{
  if (a->pos.y + a->size.y <= b->pos.y + b->size.y)
    return (Vec2){0, 1};
  else if (a->pos.y >= b->pos.y)
    return (Vec2){0, -1};
  else if (a->pos.x + a->size.x <= b->pos.x + b->size.x)
    return (Vec2){1, 0};
  else if (a->pos.x >= b->pos.x)
    return (Vec2){-1, 0};
  else
    return (Vec2){0};
}

void update_physic(Entity *entity)
{
  entity->vel.x *= 0.8f;
  entity->pos = Vec2_Add(entity->pos, Vec2_Mul(entity->vel, gs->delta));

  Platform *platform = intersect_platform(entity);
  if (platform != NULL)
  {
    Vec2 where = where_intersection(entity, platform);
    if (INTER_BOTTOM(where))
    {
      dprintf("entity bottom platform\n");
      entity->pos.y = platform->pos.y - entity->size.y;
      entity->vel.y = 0; // fmin(entity->vel.y, 0);
    }
    else if (INTER_TOP(where))
    {
      dprintf("entity top platform\n");
      entity->pos.y = platform->pos.y + platform->size.y;
      entity->vel.y = fmax(entity->vel.y, 0);
    }
    else if (INTER_RIGHT(where))
    {
      dprintf("entity right platform\n");
      entity->pos.x = platform->pos.x - entity->size.x;
      entity->vel.x = 0;
    }
    else if (INTER_LEFT(where))
    {
      dprintf("entity left platform\n");
      entity->pos.x = platform->pos.x + platform->size.x;
      entity->vel.x = 0;
    }
  }
  else
  {
    entity->vel.y += GRAVITY * GRID_SIZE * gs->delta;
  }

  if (entity->pos.x > SCREEN_WIDTH - entity->size.x)
  {
    entity->pos.x = SCREEN_WIDTH - entity->size.x;
    entity->vel.x = 0;
  }
  else if (entity->pos.x < 0)
  {
    entity->pos.x = 0;
    entity->vel.x = 0;
  }

  if (entity->pos.y > SCREEN_HEIGHT - entity->size.y)
  {
    entity->pos.y = SCREEN_HEIGHT - entity->size.y;
    entity->vel.y = 0;
  }
  else if (entity->pos.y < 0)
  {
    entity->pos.y = 0;
    entity->vel.y = 0;
  }
}

bool can_jump(void)
{
  Player *player = malloc(sizeof(*player));
  memcpy(player, &gs->player, sizeof(*player));
  player->pos.y += 1;

  Platform *platform = intersect_platform(player);
  if (platform == NULL)
    return false;

  return INTER_BOTTOM(where_intersection(player, platform));
}

bool can_ladder(void)
{
  return intersect_ladder(&gs->player) != NULL;
}

void render_sprite_flip(Sprite *sprite, SDL_Rect *rect, SDL_RendererFlip flip)
{
  static const double scale = 1.f * GRID_SIZE / SPRITE_SIZE;
  SDL_RenderCopyEx(gs->renderer, sprite->texture, &(SDL_Rect){sprite->frame * rect->w / scale, 0, rect->w / scale, rect->h / scale}, rect, 0, NULL, flip);
}

void render_sprite(Sprite *sprite, SDL_Rect *rect)
{
  render_sprite_flip(sprite, rect, SDL_FLIP_NONE);
}

void render_player(void)
{
  Vec2 dir = Vec2_Copy(gs->player.vel);
  if (can_jump())
    dir.y = 0;
  dir = Vec2_Normalize(dir);

  Sprite *sprite = &gs->sprites.player_idle;
  if (!can_jump())
  {
    if (dir.y > 0)
      sprite = &gs->sprites.player_fall;
    else if (dir.y < 0)
      sprite = &gs->sprites.player_jump;
  }
  else if (dir.x != 0)
    sprite = &gs->sprites.player_run;

  render_sprite_flip(sprite, &ERect(gs->player), dir.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void render_enemy(void)
{
  Sprite *sprite = &gs->sprites.enemy_idle;
  // sprite = &gs->sprites.enemy_throw;

  render_sprite(sprite, &ERect(gs->enemy));
}

void render_woman(void)
{
  SDL_RendererFlip flip = SDL_FLIP_NONE;

  if (gs->player.pos.x + gs->player.size.x / 2 < gs->woman.pos.x + gs->woman.size.x / 2)
    flip = SDL_FLIP_HORIZONTAL;

  render_sprite_flip(&gs->sprites.woman, &ERect(gs->woman), flip);
}

void render_platforms(void)
{
  for (size_t i = 0; i < gs->platforms->size; i++)
  {
    Platform platform = *Entity_vector_at(gs->platforms, i);
    for (uint8_t x = 0; x < platform.size.x / GRID_SIZE; x++)
    {
      SDL_Rect rect = ERect(platform);
      rect.x += x * GRID_SIZE;
      rect.w = GRID_SIZE;

      Sprite *sprite = &gs->sprites.platform;
      sprite->frame = gs->level % sprite->frames;

      render_sprite(sprite, &rect);
    }
  }
}

void render_ladders(void)
{
  for (size_t i = 0; i < gs->ladders->size; i++)
  {
    Ladder ladder = *Entity_vector_at(gs->ladders, i);
    for (uint8_t i = 0; i < ladder.size.y / GRID_SIZE; i++)
    {
      SDL_Rect rect = ERect(ladder);
      rect.y += i * GRID_SIZE;
      rect.h = GRID_SIZE;

      Sprite *sprite = &gs->sprites.ladder;
      sprite->frame = gs->level % sprite->frames;

      render_sprite(sprite, &rect);
    }
  }
}

void render_collectibles(void)
{
  for (size_t i = 0; i < gs->collectibles->size; i++)
  {
    Collectible collectible = *Entity_vector_at(gs->collectibles, i);
    render_sprite(&gs->sprites.collectible, &ERect(collectible));
  }
}

void render_barrels(void)
{
  for (size_t i = 0; i < gs->barrels->size; i++)
  {
    Barrel barrel = gs->barrels->data[i];
    render_sprite_flip(&gs->sprites.barrel, &ERect(barrel), barrel.vel.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  }
}

void render_floating_texts(void)
{
  for (size_t i = 0; i < gs->floating_texts->size; i++)
  {
    FloatingText *text = FloatingText_vector_at(gs->floating_texts, i);

    text->elapsed += gs->delta;
    if (text->elapsed >= text->duration)
    {
      SDL_DestroyTexture(text->texture);
      FloatingText_vector_erase(gs->floating_texts, i);
      i--;
      continue;
    }

    SDL_Rect rect = {text->pos.x, text->pos.y, 0, 0};
    SDL_QueryTexture(text->texture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(gs->renderer, text->texture, NULL, &rect);
  }
}

void render_leaderboard(void)
{
  SDL_Rect rect = {.x = GRID_SIZE * 14, .y = GRID_SIZE * 4};
  SDL_Color color = {LEVEL_COLOR};
  SDL_Texture *texture;
  char text[NAME_LENGTH + 16];

  uint16_t offset = gs->leaderboard_page * PAGE_SIZE;
  uint8_t page_size = MIN(PAGE_SIZE, gs->leaderboard->size - offset);

  for (size_t i = 0; i < page_size; i++)
  {
    Leaderboard *entry = Leaderboard_vector_at(gs->leaderboard, i + offset);
    snprintf(text, sizeof(text), "%2d. %s - %d", i + offset + 1, entry->name, entry->score);

    texture = render_text(text, color);
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    rect.w *= 1.75;
    rect.h *= 1.75;
    rect.y += rect.h;

    SDL_RenderCopy(gs->renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
  }
}

void render_text_input(void)
{
  SDL_Rect rect = {.x = GRID_SIZE * 2, .y = GRID_SIZE * 4};
  SDL_Color color = {LEVEL_COLOR};
  SDL_Texture *texture;
  char text[48];

  Leaderboard *entry = Leaderboard_vector_back(gs->leaderboard);
  snprintf(text, 44, "Enter your name: %s", entry->name);

  texture = render_text(text, color);
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
  rect.w *= 2;
  rect.h *= 2;
  rect.y += rect.h;

  SDL_RenderCopy(gs->renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

void render_ui(void)
{
  const uint8_t border = 2;

  if (gs->level == 0)
    return render_leaderboard();

  if (gs->level == 4)
    return render_text_input();

  gs->play_time += gs->delta;

  SDL_Rect rect = {SCREEN_WIDTH - border * 2, border * 2, GRID_SIZE * 4, GRID_SIZE * 2};
  rect.x -= rect.w;

  SDL_SetRenderDrawColor(gs->renderer, RGB(Colors[gs->level % (sizeof(Colors) / sizeof(Colors[0]))]));
  SDL_RenderFillRect(gs->renderer, &rect);

  rect.x += border;
  rect.y += border;
  rect.w -= border * 2;
  rect.h -= border * 2;

  SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(gs->renderer, &rect);

  rect.x += border;
  rect.y += border;

  SDL_Color color = {LEVEL_COLOR};
  SDL_Texture *texture;
  char text[16];

  snprintf(text, 16, "Score %d", gs->score);
  texture = render_text(text, color);
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
  SDL_RenderCopy(gs->renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);

  rect.y += rect.h + border;

  snprintf(text, 16, "Lives %d", gs->lives);
  texture = render_text(text, color);
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
  SDL_RenderCopy(gs->renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);

  rect.y += rect.h + border;

  snprintf(text, 16, "Time %d:%02d", (int)gs->play_time / 60, (int)gs->play_time % 60);
  texture = render_text(text, color);
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
  SDL_RenderCopy(gs->renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

void game_render(void)
{
  assert(gs->renderer != NULL);
  assert(gs->keyboard != NULL);

  debug({
    for (uint8_t y = 0; y < SCREEN_HEIGHT / GRID_SIZE; y++)
    {
      for (uint8_t x = 0; x < SCREEN_WIDTH / GRID_SIZE; x++)
      {
        SDL_Rect rect = {x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE};
        SDL_Point point = Vec2_ToPoint(gs->mouse.pos);

        SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
        if (gs->mouse.buttons & SDL_BUTTON(SDL_BUTTON_LEFT) && SDL_PointInRect(&point, &rect))
        {
          SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 255);
          dprintf("pos(%d %d)\n", x, y);
        }

        SDL_RenderDrawRect(gs->renderer, &rect);
      }
    }
  });

  render_platforms();
  render_ladders();
  render_collectibles();
  render_barrels();
  render_ui();
  render_woman();
  render_enemy();
  render_player();
  render_floating_texts();
}

void new_game(void)
{
  gs->lives = 3;
  gs->score = 0;
  gs->play_time = 0;
  load_level(1);
}

void update_menu(void)
{
  if (gs->level > 0)
    return;

  if (gs->player.pos.y < Entity_vector_at(gs->ladders, 0)->pos.y)
    new_game();
}

void update_sprites(void)
{
  for (uint8_t i = 0; i < sizeof(gs->sprites) / sizeof(Sprite); i++)
  {
    Sprite *sprite = &((Sprite *)&gs->sprites)[i];
    sprite->elapsed += gs->delta * 1000;
    if (sprite->elapsed >= sprite->duration)
    {
      sprite->elapsed = 0;
      sprite->frame = (sprite->frame + 1) % sprite->frames;
    }
  }
}

void update_enemy(void)
{
  update_physic(&gs->enemy);

  if (gs->enemy_jump_cooldown > 0)
    gs->enemy_jump_cooldown -= gs->delta;
  else
  {
    gs->enemy_jump_cooldown = ENEMY_JUMP_COOLDOWN;
    gs->enemy.vel.y = -ENEMY_JUMP * GRID_SIZE;
  }

  if (!REAL_LEVEL)
    return;

  if (gs->enemy_throw_cooldown > 0)
    gs->enemy_throw_cooldown -= gs->delta;
  else
  {
    gs->enemy_throw_cooldown = ENEMY_THROW_COOLDOWN;
    Entity_vector_push(gs->barrels, &(Barrel){.pos = Vec2_Add(gs->enemy.pos, (Vec2){GRID_SIZE, GRID_SIZE}), .size = {GRID_SIZE, GRID_SIZE}});
  }
}

void update_barrels(void)
{
  for (size_t i = 0; i < gs->barrels->size; i++)
  {
    Barrel *barrel = &gs->barrels->data[i];
    SDL_Rect rect = ERect(*barrel);

    if (SDL_HasIntersection(&rect, &ERect(gs->player)))
    {
      Entity_vector_erase(gs->barrels, i);
      gs->lives--;
      continue;
    }
    else if (barrel->size.x == GRID_SIZE && intersect_ladder(&gs->player) == NULL)
    {
      rect.y -= barrel->size.y * 2;
      rect.h += barrel->size.y;
      if (SDL_HasIntersection(&rect, &ERect(gs->player)))
      {
        gs->score += BARREL_SCORE;
        barrel->size.x += 0.00001;
        show_floating_text(STR(BARREL_SCORE), Vec2_Add(barrel->pos, (Vec2){0, -GRID_SIZE / 2}), 1);
      }
    }

    double prev = barrel->vel.x = BARREL_SPEED * GRID_SIZE * (barrel->vel.x < 0 ? -1 : 1);

    update_physic(barrel);

    if (barrel->vel.x == 0)
    {
      barrel->vel.x = -prev;

      if (barrel->pos.y >= Entity_vector_at(gs->platforms, 1)->pos.y)
        Entity_vector_erase(gs->barrels, i);
    }
  }
}

void update_collectibles(void)
{
  SDL_Rect player = ERect(gs->player);

  for (size_t i = 0; i < gs->collectibles->size; i++)
  {
    Collectible *collectible = &gs->collectibles->data[i];

    if (SDL_HasIntersection(&ERect(*collectible), &player))
    {
      show_floating_text(STR(COLLECTIBLE_SCORE), collectible->pos, 1);
      Entity_vector_erase(gs->collectibles, i);
      gs->score += COLLECTIBLE_SCORE;
      i--;
      break;
    }
  }
}

void update_player(void)
{
  if (gs->level != 4)
  {
    Vec2 dir = {
        .x = gs->keyboard[SDL_SCANCODE_D] - gs->keyboard[SDL_SCANCODE_A],
        .y = gs->keyboard[SDL_SCANCODE_S] - gs->keyboard[SDL_SCANCODE_W]};

    gs->player.vel.x = dir.x * PLAYER_SPEED * GRID_SIZE;

    if (gs->keyboard[SDL_SCANCODE_SPACE] && can_jump())
      gs->player.vel.y = -PLAYER_JUMP * GRID_SIZE;

    if (can_ladder())
      gs->player.vel.y = dir.y * PLAYER_SPEED * GRID_SIZE / 1.5;
  }

  update_physic(&gs->player);

  if (REAL_LEVEL && SDL_HasIntersection(&ERect(gs->player), &ERect(gs->woman)))
  {
    load_level(++gs->level);
    gs->score += LEVEL_SCORE;
  }
}

void handle_text_input(SDL_Keycode key)
{
  Leaderboard *entry = Leaderboard_vector_back(gs->leaderboard);
  size_t len = strlen(entry->name);

  if (key == SDLK_BACKSPACE)
  {
    if (len > 0)
      entry->name[len - 1] = '\0';
  }
  else if (key == SDLK_RETURN)
  {
    FILE *file = fopen("assets/leaderboard.kd", "a");
    fprintf(file, "\n%d %s", entry->score, entry->name);
    fclose(file);

    load_level(0);
  }
  else if (len < NAME_LENGTH && isalnum(key))
  {
    entry->name[len] = key;
  }
}

GameState *game_pre_reload(void)
{
  SDL_Log("Pre reload");

  TTF_CloseFont(gs->font);

  for (uint8_t i = 0; i < sizeof(gs->sprites) / sizeof(Sprite); i++)
    if (((Sprite *)&gs->sprites)[i].texture != NULL)
      SDL_DestroyTexture(((Sprite *)&gs->sprites)[i].texture);

  return gs;
}

void game_post_reload(GameState *pgs)
{
  gs = pgs;

  gs->font = TTF_OpenFont("assets/slkscr.ttf", GRID_SIZE / 2);
  assert(gs->font != NULL);

#define X(n, f, d)                          \
  gs->sprites.n.texture = load_texture(#n); \
  gs->sprites.n.frames = f;                 \
  gs->sprites.n.duration = d;
  SPRITES
#undef X

  reset_animations();

  SDL_Log("Post reload");
}

void game_init(SDL_Window *window, SDL_Renderer *renderer, SDL_Event *event)
{
  if (gs != NULL)
    free(gs);
  gs = malloc(sizeof(*gs));
  assert(gs != NULL);
  memset(gs, 0, sizeof(*gs));

  gs->keyboard = SDL_GetKeyboardState(NULL);
  gs->renderer = renderer;
  gs->window = window;
  gs->event = event;
  gs->last_frame = SDL_GetPerformanceCounter();
  gs->time_scale = 1.0f;
  gs->platforms = Entity_vector_new();
  gs->collectibles = Entity_vector_new();
  gs->ladders = Entity_vector_new();
  gs->barrels = Entity_vector_new();
  gs->floating_texts = FloatingText_vector_new();
  gs->leaderboard = Leaderboard_vector_new();

  load_level(0);

  game_post_reload(gs);
}

void game_update(void)
{
  int mouseX, mouseY;
  gs->mouse.buttons = SDL_GetMouseState(&mouseX, &mouseY);
  gs->mouse.pos.x = mouseX;
  gs->mouse.pos.y = mouseY;

  uint64_t now = SDL_GetPerformanceCounter();
  gs->delta_unscaled = (now - gs->last_frame) / (double)SDL_GetPerformanceFrequency();
  gs->delta = gs->delta_unscaled * gs->time_scale * !gs->paused;
  gs->last_frame = now;

  update_sprites();
  update_menu();

  if (!REAL_LEVEL)
    update_physic(&gs->woman);

  update_collectibles();
  update_barrels();
  update_enemy();
  update_player();

  game_render();

  if (REAL_LEVEL && gs->lives == 0)
    load_level(4);

  gs->fps_timer += gs->delta_unscaled;
  if (gs->fps_timer >= 1)
  {
    gs->fps_timer = 0;

    char title[32];
    snprintf(title, 32, "King Donkey (%.1f fps)", 1 / gs->delta_unscaled);
    SDL_SetWindowTitle(gs->window, title);
  }

  if (gs->frame_limit)
  {
    uint64_t elapsed = SDL_GetPerformanceCounter() - now;
    uint64_t target = SDL_GetPerformanceFrequency() / 120;
    if (elapsed < target)
      SDL_Delay((target - elapsed) * 1000 / SDL_GetPerformanceFrequency());
  }
}

void game_event(SDL_Event *event)
{
  switch (event->type)
  {
  case SDL_KEYDOWN:
    if (gs->level == 4)
      return handle_text_input(event->key.keysym.sym);

    switch (event->key.keysym.sym)
    {
    case SDLK_F1:
      gs->debug = !gs->debug;
      SDL_Log("Debug mode %s", gs->debug ? "on" : "off");
      break;
    case SDLK_F2:
      gs->frame_limit = !gs->frame_limit;
      SDL_Log("Frame limit %s", gs->frame_limit ? "on" : "off");
      break;
    case SDLK_PLUS:
    case SDLK_EQUALS:
      gs->time_scale = fmin(gs->time_scale + (gs->time_scale < 1.0f ? 0.1f : 0.5f), TIME_SCALE_MAX);
      SDL_Log("Time scale: %lf", gs->time_scale);
      break;
    case SDLK_MINUS:
      gs->time_scale = fmaxf(gs->time_scale - (gs->time_scale <= 1.0f ? 0.1f : 0.5f), 0.0f);
      SDL_Log("Time scale: %lf", gs->time_scale);
      break;
    case SDLK_0:
      gs->time_scale = 1.0f;
      SDL_Log("Time scale: %lf (Default)", gs->time_scale);
      break;
    case SDLK_LEFTBRACKET:
      gs->score -= 100;
      break;
    case SDLK_RIGHTBRACKET:
      gs->score += 100;
      break;
    case SDLK_BACKSLASH:
      gs->lives = 3;
      break;
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
      load_level(event->key.keysym.sym - SDLK_0);
      break;
    case SDLK_n:
      new_game();
      break;
    case SDLK_m:
      load_level(0);
      break;
    case SDLK_r:
      load_level(gs->level);
      break;
    case SDLK_p:
      gs->paused = !gs->paused;
      break;
    case SDLK_f:
      show_floating_text("Floating text", gs->player.pos, 2);
      break;
    case SDLK_LEFT:
      gs->leaderboard_page = MAX(gs->leaderboard_page - 1, 0);
      break;
    case SDLK_RIGHT:
      gs->leaderboard_page = MIN(gs->leaderboard_page + 1, gs->leaderboard->size / PAGE_SIZE);
      break;
    }
  }
}
