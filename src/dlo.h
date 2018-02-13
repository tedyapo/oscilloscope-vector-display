/*
 * dlo.h - Disk-Like Objects: vector game sprites that behave as hard disks
 *         (geometrical disks, not the data-storage type)
 */
#ifndef DLO_H_INCLUDED_
#define DLO_H_INCLUDED_

#include "vec2f.h"

/* line defined by dot(n, p) - d = 0 */
typedef struct
{
  vec2f n;
  float d;
} Wall;

typedef struct
{
  vec2f pos;  /* position */
  vec2f pvel; /* velocity */
  float ang;  /* rotation angle */
  float avel; /* angular velociy */
  float r;    /* bounding radius */
  float m;    /* mass */
  int n_pts;  /* number of outline points */
  vec2f *pts; /* outline points */
  int flags;  /* for general-purpose use */
} DLO;

typedef struct
{
  uint8_t detected;
  float t; /* time of collison; negative is past */
} DLO_collision;

typedef enum
{
  DLO_OUTLINE_REGULAR,
  DLO_OUTLINE_RANDOM
} dlo_outline_t;

void DLO_init(DLO *d, dlo_outline_t ot, 
              int n_pts, float r_min, float r_max);

void DLO_free(DLO *d);

void DLO_update(DLO *d);

/* detect if a collision happened in the last time step */
DLO_collision DLO_intersect_DLO(const DLO* a, const DLO* b);

DLO_collision DLO_intersect_WALL(const DLO* d, const Wall* w);

/* bounce two DLOs off each other */
void DLO_interact_DLO(DLO *a, DLO *b, DLO_collision *coll);

void DLO_interact_WALL();

void DLO_render(DisplayList *dl, DLO *d, float slew);

#endif /* #ifndef DLO_H_INCLUDED_ */
