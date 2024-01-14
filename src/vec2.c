#include "vec2.h"
#include <math.h>

Vec2 Vec2_Copy(Vec2 a)
{
  return (Vec2){a.x, a.y};
}

Vec2 Vec2_Add(Vec2 a, Vec2 b)
{
  return (Vec2){a.x + b.x, a.y + b.y};
}

Vec2 Vec2_Sub(Vec2 a, Vec2 b)
{
  return (Vec2){a.x - b.x, a.y - b.y};
}

Vec2 Vec2_Mul(Vec2 a, double b)
{
  return (Vec2){a.x * b, a.y * b};
}

Vec2 Vec2_Div(Vec2 a, double b)
{
  return (Vec2){a.x / b, a.y / b};
}

double Vec2_Dot(Vec2 a, Vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

double Vec2_Len(Vec2 a)
{
  return sqrtf(Vec2_Dot(a, a));
}

Vec2 Vec2_Normalize(Vec2 a)
{
  double len = Vec2_Len(a);
  return len ? Vec2_Div(a, len) : (Vec2){0};
}

Vec2 Vec2_Lerp(Vec2 a, Vec2 b, double t)
{
  return Vec2_Add(a, Vec2_Mul(Vec2_Sub(b, a), t));
}

Vec2 Vec2_Rotate(Vec2 a, double angle)
{
  double c = cosf(angle);
  double s = sinf(angle);
  return (Vec2){a.x * c - a.y * s, a.x * s + a.y * c};
}

Vec2 Vec2_ClampRect(Vec2 a, SDL_Rect rect)
{
  return (Vec2){
      fminf(fmaxf(a.x, rect.x), rect.x + rect.w),
      fminf(fmaxf(a.y, rect.y), rect.y + rect.h)};
}

SDL_Point Vec2_ToPoint(Vec2 a)
{
  return (SDL_Point){a.x, a.y};
}
