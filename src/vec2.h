#pragma once
#include <SDL2/SDL.h>

typedef struct Vec2
{
  double x;
  double y;
} Vec2;

Vec2 Vec2_Copy(Vec2 a);
Vec2 Vec2_Add(Vec2 a, Vec2 b);
Vec2 Vec2_Sub(Vec2 a, Vec2 b);
Vec2 Vec2_Mul(Vec2 a, double b);
Vec2 Vec2_Div(Vec2 a, double b);
double Vec2_Dot(Vec2 a, Vec2 b);
double Vec2_Len(Vec2 a);
Vec2 Vec2_Normalize(Vec2 a);
Vec2 Vec2_Lerp(Vec2 a, Vec2 b, double t);
Vec2 Vec2_Rotate(Vec2 a, double angle);
Vec2 Vec2_ClampRect(Vec2 a, SDL_Rect rect);
SDL_Point Vec2_ToPoint(Vec2 a);
