/*
 * dlo.c - Disk-Like Objects: vector game sprites that behave as hard disks
 *         (geometrical disks, not the data-storage type)
 */

#include <stdlib.h>
#include <math.h>
#include "vec2f.h"
#include "display.h"
#include "dlo.h"

const float PI = 3.141592653f;

void DLO_init(DLO *d, dlo_outline_t ot, 
              int n_pts, float r_min, float r_max)
{
  d->n_pts = n_pts;
  d->pts = (vec2f *) malloc(n_pts * sizeof(vec2f));
  if (NULL == d->pts){
    ERROR("malloc() failed");
  }

  d->r = r_min;
  vec2f zero = {0.f, 0.f};
  d->pos = zero;
  d->pvel = zero;
  d->ang = 0;
  d->avel = 0;

  d->m = 0;
  float dtheta = 2.f*PI/n_pts;
  for (int i=0; i<n_pts; ++i){
    float theta = 2.f*PI*i/n_pts;
    switch(ot){
    case DLO_OUTLINE_REGULAR:
      d->pts[i].x = r_min * cosf(theta);
      d->pts[i].y = r_min * sinf(theta);
      d->m += 2 * r_min*cosf(dtheta/2) * r_min*sinf(dtheta/2);
      break;
    case DLO_OUTLINE_RANDOM:
    {
      float r = r_min + (r_max - r_min) * drand48();
      if (r > d->r){
        d->r = r;
      }
      d->pts[i].x = r * cosf(theta);
      d->pts[i].y = r * sinf(theta);
      d->m += 2 * r*cosf(dtheta/2) * r*sinf(dtheta/2); /* approx */
      break;
    }
    default:
      ERROR("unknown DLO outline type");
    }
  }
}

void DLO_free(DLO *d)
{
  if (d->pts){
    free(d->pts);
  }
}

void DLO_update(DLO *d)
{
  vec2f_addto(&d->pos, d->pvel);
  d->ang += d->avel;
}

/* detect if a collision happened in the last time step */
DLO_collision DLO_intersect_DLO(const DLO* d, const DLO* e)
{
  DLO_collision collision;

  if (vec2f_dist(d->pos, e->pos) > (d->r + e->r)){
    collision.detected = 0;
    return collision;
  }
  
  /* find time (in past) when collision should have happened */
  vec2f dp = vec2f_sub(d->pos, e->pos);
  vec2f v = vec2f_sub(d->pvel, e->pvel);
  float a = v.x*v.x + v.y*v.y;
  float b = 2.f*(v.x*dp.x + v.y*dp.y);
  float c = dp.x*dp.x + dp.y*dp.y - (d->r + e->r)*(d->r + e->r);
  float t0, t1;
  // use numerically stable quadratic solution
  // https://people.csail.mit.edu/bkph/articles/Quadratics.pdf
  float det = b*b-4.f*a*c;
  if (det < 0.f){
    // never intersected: corner case
    collision.detected = 0;
    return collision;
  }
  det = sqrtf(det);
  if (b >= 0){
    t0 = (-b - det)/(2.f*a);
    t1 = 2.f*c/(-b - det);
  } else {
    t0 = 2.f*c/(-b + det);
    t1 = (-b + det)/(2.f*a);
  }
  float t;
  if (t0 < 0.f){
    t = t0;
  }
  if (t1 < t){
    t = t1;
  }

  collision.detected = 1;
  collision.t = t;
  return collision;
}

DLO_collision DLO_intersect_WALL(const DLO* d, const Wall* w)
{
  
}

/* bounce two DLOs off each other */
void DLO_interact_DLO(DLO *a, DLO *b, DLO_collision *coll)
{
  if (!coll->detected){
    return;
  }
  
  /* reverse positions to interaction time */
  vec2f_addto(&a->pos, vec2f_mul_f(a->pvel, coll->t));
  vec2f_addto(&b->pos, vec2f_mul_f(b->pvel, coll->t));

  /* find unit vector between centers */
  vec2f l = vec2f_sub(a->pos, b->pos);
  vec2f_normalize(&l);
  
  /* velocity components along axis before collision */
  float va = vec2f_dot(a->pvel, l);
  float vb = vec2f_dot(b->pvel, l);

  /* axial velocity post collison */
  float va_p = ((a->m - b->m)*va + 2*b->m*vb)/(a->m + b->m);
  float vb_p = ((b->m - a->m)*vb + 2*a->m*va)/(a->m + b->m);

  /* update velocities */
  vec2f_addto(&a->pvel, vec2f_mul_f(l, -va)); 
  vec2f_addto(&a->pvel, vec2f_mul_f(l, va_p)); 
  vec2f_addto(&b->pvel, vec2f_mul_f(l, -vb)); 
  vec2f_addto(&b->pvel, vec2f_mul_f(l, vb_p)); 

  /* update to post-interaction positions */
  vec2f_addto(&a->pos, vec2f_mul_f(a->pvel, -coll->t));
  vec2f_addto(&b->pos, vec2f_mul_f(b->pvel, -coll->t));
}

void DLO_interact_WALL()
{
}

void DLO_render(DisplayList *dl, DLO *d, float slew)
{
  Line *l = NewLine();
  l->slew = slew;
  for (int i = 0; i <= d->n_pts; ++i){
    float x = d->pts[i % d->n_pts].x;
    float y = d->pts[i % d->n_pts].y;
    float xp = x * cosf(d->ang) - y * sinf(d->ang);
    float yp = x * sinf(d->ang) + y * cosf(d->ang);
    Point *p = NewPoint(d->pos.x + xp, d->pos.y + yp);
    AddPoint(l, p);
  }
  AddLine(dl, l);
}
