/*
 * vec2f.c : 2d floating-point vectors
 */
#include <math.h>

typedef struct
{
  float x;
  float y;
} vec2f;

void vec2f_addto(vec2f* a, const vec2f b)
{
  a->x += b.x;
  a->y += b.y;
}

vec2f vec2f_mul_f(vec2f a, float s)
{
  vec2f c;
  c.x = a.x * s;
  c.y = a.y * s;
  return c;
}

vec2f vec2f_sub(vec2f a, vec2f b)
{
  vec2f c;
  c.x = a.x - b.x;
  c.y = a.y - b.y;
  return c;
}

void vec2f_normalize(vec2f* a)
{
  float l = sqrtf(a->x * a->x + a->y * a->y);
  a->x /= l;
  a->y /= l;
}

float vec2f_dot(vec2f a, vec2f b)
{
  return a.x * b.x + a.y * b.y;
}

float vec2f_dist(vec2f a, vec2f b)
{
  return sqrtf((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}
