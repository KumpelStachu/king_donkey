#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"
#include <stdbool.h>

#define CC "clang"
#define CFLAGS "-Wall",                   \
               "-Wextra",                 \
               "-Werror",                 \
               "-Wno-unused-parameter",   \
               "-Wno-unused-variable",    \
               "-Wno-format",             \
               "-fsanitize=address",      \
               "-std=c99",                \
               "-O3",                     \
               "-I./src",                 \
               "-I/opt/homebrew/include", \
               "-L/opt/homebrew/lib",     \
               "-D_THREAD_SAFE",          \
               "-lsdl2",                  \
               "-lsdl2_ttf",              \
               "-lm",                     \
               "-ldl",                    \
               "-lpthread"

#define MAIN_FLAGS ""
#define MAIN_INPUT "./src/main.c", "./src/hotreload.c"

#define LIB_FLAGS "-shared", "-fPIC"
#define LIB_INPUT "./src/game.c", "./src/vec2.c"

void build_game(void)
{
  CMD(CC, CFLAGS, LIB_FLAGS, LIB_INPUT, "-o", "./build/libgame.so");
}

void build_main(void)
{
  CMD(CC, CFLAGS, MAIN_FLAGS, MAIN_INPUT, "-o", "./build/king_donkey");
}

void build(void)
{
  MKDIRS("./build");

  build_game();
  build_main();
}

void print_usage(char *name)
{
  INFO("Usage: %s <command>", name);
  INFO("  build");
  INFO("  watch");
  INFO("  run");
  INFO("  clean");
  INFO("  update");
  INFO("  help");
}

int main(int argc, char **argv)
{
  GO_REBUILD_URSELF(argc, argv);

  if (argc < 2)
  {
    print_usage(argv[0]);
    return 1;
  }
  else
  {
    if (strcmp(argv[1], "build") == 0)
    {
      build();
    }
    else if (strcmp(argv[1], "run") == 0)
    {
      build();
      CMD("./build/king_donkey");
    }
    else if (strcmp(argv[1], "watch") == 0)
    {
      build();

      Cmd cmd = {.line = cstr_array_make("./build/king_donkey", NULL)};
      cmd_run_async(cmd, (Fd *)stdin, (Fd *)stdout);

      while (true)
      {
        CMD("fswatch", "-r", "-1", "./src");
        build_game();
      }
    }
    else if (strcmp(argv[1], "clean") == 0)
    {
      CMD("rm", "-rf", "./build");
    }
    else if (strcmp(argv[1], "update") == 0)
    {
      CMD("wget", "-O", "nobuild.h", "https://raw.githubusercontent.com/tsoding/nobuild/master/nobuild.h");
    }
    else if (strcmp(argv[1], "help") == 0)
    {
      print_usage(argv[0]);
    }
    else
    {
      ERRO("Unknown command: %s", argv[1]);
      print_usage(argv[0]);
      return 1;
    }
  }

  return 0;
}