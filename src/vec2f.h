/*
 * vec2f.c : 2d floating-point vectors
 */
#ifndef VEC2F_H_INCLUDED_
#define VEC2F_H_INCLUDED_

typedef struct
{
  float x;
  float y;
} vec2f;

void vec2f_addto(vec2f* a, const vec2f b);

vec2f vec2f_mul_f(vec2f a, float s);

vec2f vec2f_sub(vec2f a, vec2f b);

void vec2f_normalize(vec2f* a);

float vec2f_dot(vec2f a, vec2f b);

float vec2f_dist(vec2f a, vec2f b);

#endif /* #ifndef VEC2F_H_INCLUDED_ */
