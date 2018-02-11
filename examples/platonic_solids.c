/*
 * hidden-line platonic solids
 *
 * coordinate data from: http://paulbourke.net/geometry/platonic/
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "display.h"

typedef struct
{
  float x, y, z;
} v3f;

v3f add_v3f(const v3f *a, const v3f *b)
{
  v3f c;
  c.x = a->x + b->x;
  c.y = a->y + b->y;
  c.z = a->z + b->z;
  return c;
}

v3f sub_v3f(const v3f *a, const v3f *b)
{
  v3f c;
  c.x = a->x - b->x;
  c.y = a->y - b->y;
  c.z = a->z - b->z;
  return c;
}

float dot_v3f(const v3f *a, const v3f *b)
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

v3f cross_v3f(const v3f *a, const v3f *b)
{
  v3f c;
  c.x = a->y*b->z - b->y*a->z;
  c.y = b->x*a->z - a->x*b->z;
  c.z = a->x*b->y - b->x*a->y;
  return c;
}

void scale_v3f(v3f *v, float s)
{
  v->x *= s;
  v->y *= s;
  v->z *= s;
}

void normalize_v3f(v3f *v)
{
  float l = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
  v->x /= l;
  v->y /= l;
  v->z /= l;
}

typedef struct
{
  int faces;
  int points_per_face;
  v3f *points;
  v3f *normals;
} polyhedron;

v3f tetrahedron_points[] = {
  {1, 1, 1}, {-1, 1, -1}, {1, -1, -1},
  {-1, 1, -1}, {-1, -1, 1}, {1, -1, -1},
  {1, 1, 1}, {1, -1, -1}, {-1, -1, 1},
  {1, 1, 1}, {-1, -1, 1}, {-1, 1, -1}};

#define a (1./(2.*sqrt(2.)))
#define b (1./2.)
v3f octahedron_points[] = {
  {-a, 0, a},      {-a, 0, -a},    {0, b, 0},
  {-a,  0, -a},    {a,  0, -a},    {0,  b,  0},
  {a,  0, -a},    {a,  0,  a},    {0,  b,  0},
  {a,  0,  a},   {-a,  0,  a},    {0,  b,  0},
  {a,  0, -a},   {-a,  0, -a},    {0, -b,  0},
  {-a,  0, -a},   {-a,  0,  a},    {0, -b,  0},
  {a,  0,  a},    {a,  0, -a},    {0, -b,  0},
  {-a,  0,  a},    {a,  0,  a},    {0, -b,  0}};
#undef a
#undef b

v3f hexahedron_points[] = {
  {-1, -1, -1},    {1, -1, -1},    {1, -1,  1},   {-1, -1,  1},
  {-1, -1, -1},   {-1, -1,  1},   {-1,  1,  1},   {-1,  1, -1},
  {-1, -1,  1},    {1, -1,  1},    {1,  1,  1},   {-1,  1,  1},
  {-1,  1, -1},   {-1,  1,  1},    {1,  1,  1},    {1,  1, -1},
  {1, -1, -1},    {1,  1, -1},    {1,  1,  1},    {1, -1,  1},
  {-1, -1, -1},   {-1,  1, -1},    {1,  1, -1},    {1, -1, -1}};

#define phi ((1. + sqrt(5.))/2.)
#define b (1./phi)
#define c (2.-phi)
v3f dodecahedron_points[] = {
  {c,  0,  1},  {-c,  0,  1},   {-b,  b,  b},    {0,  1,  c},    {b,  b,  b},
  {-c,  0,  1},   {c,  0,  1},    {b, -b,  b},    {0, -1,  c},   {-b, -b,  b},
  {c,  0, -1},   {-c,  0, -1},   {-b, -b, -b},    {0, -1, -c},    {b, -b, -b},
  {-c,  0, -1},   {c,  0, -1},    {b,  b, -b},    {0,  1, -c},   {-b,  b, -b},
  {0,  1, -c},    {0,  1,  c},    {b,  b,  b},    {1,  c,  0},   {b,  b, -b},
  {0,  1,  c},    {0,  1, -c},   {-b,  b, -b},   {-1,  c,  0},   {-b,  b,  b},
  {0, -1, -c},    {0, -1,  c},   {-b, -b,  b},   {-1, -c,  0},   {-b, -b, -b},
  {0, -1,  c},    {0, -1, -c},    {b, -b, -b},    {1, -c,  0},   {b, -b,  b},
  {1,  c,  0},    {1, -c,  0},    {b, -b,  b},    {c,  0,  1},    {b,  b,  b},
  {1, -c,  0},    {1,  c,  0},    {b, b, -b},    {c,  0, -1},    {b, -b, -b},
  {-1,  c,  0},   {-1, -c,  0},   {-b, -b, -b},   {-c,  0, -1},   {-b,  b, -b},
  {-1, -c,  0},   {-1,  c,  0},   {-b,  b,  b},   {-c,  0,  1},   {-b, -b,  b}};
#undef phi
#undef b
#undef c

#define a  (1./2.)
#define phi ((1. + sqrt(5.))/2.)
#define b (1./(2. + phi))
v3f icosahedron_points[] = {
  {0,  b, -a},    {b,  a,  0},   {-b,  a,  0},
  {0,  b,  a},   {-b,  a,  0},    {b,  a,  0},
  {0,  b,  a},   {0, -b,  a},   {-a,  0,  b},
  {0,  b,  a},   {a,  0,  b},    {0, -b,  a},
  {0,  b, -a},   {0, -b, -a},    {a,  0, -b},
  {0,  b, -a},   {-a,  0, -b},    {0, -b, -a},
  {0, -b,  a},   {b, -a,  0},   {-b, -a,  0},
  {0, -b, -a},   {-b, -a,  0},   {b, -a,  0},
  {-b,  a,  0},   {-a,  0,  b},   {-a, 0, -b},
  {-b, -a,  0},   {-a,  0, -b},   {-a,  0,  b},
  {b,  a,  0},    {a,  0, -b},    {a,  0,  b},
  {b, -a,  0},    {a,  0,  b},    {a,  0, -b},
  {0,  b,  a},   {-a,  0,  b},   {-b,  a,  0},
  {0,  b,  a},    {b,  a,  0},    {a,  0,  b},
  {0,  b, -a},   {-b, a,  0},   {-a,  0, -b},
  {0,  b, -a},    {a,  0, -b},    {b,  a,  0},
  {0, -b, -a},   {-a,  0, -b},   {-b, -a,  0},
  {0, -b, -a},    {b, -a,  0},   {a, 0, -b},
  {0, -b,  a},   {-b, -a,  0},   {-a,  0,  b},
  {0, -b,  a},    {a,  0,  b},    {b, -a,  0}};
#undef a
#undef phi
#undef b

void normalize_size(polyhedron *p)
{
  for (int i=0; i<p->faces; ++i){
    Line *l = NewLine();
    for (int j=0; j<p->points_per_face; ++j){
      normalize_v3f(&p->points[i * p->points_per_face + j]);
    }
  }
}

void calculate_normals(polyhedron *p)
{
  p->normals = (v3f*)malloc(p->faces * sizeof(v3f));
  if (!p->normals){
    ERROR("malloc() failed");
  }
  for (int i = 0; i < p->faces; ++i){
    v3f p0, p1, p2;
    p0 = p->points[i * p->points_per_face + 0];
    p1 = p->points[i * p->points_per_face + 1];
    p2 = p->points[i * p->points_per_face + 2];
    v3f a = sub_v3f(&p1, &p0);
    v3f b = sub_v3f(&p2, &p0);
    v3f n = cross_v3f(&a, &b);
    normalize_v3f(&n);
    /* vertices aren't in consistent winding order
     *   so flip normal if required */
    if (dot_v3f(&p0, &n) < 0){
      scale_v3f(&n, -1.f);
    }
    p->normals[i] = n;
  }
}

void init_polyhedra(polyhedron *p)
{
  /* p[0]: tetrahedron */
  p[0].faces = 4;
  p[0].points_per_face = 3;
  p[0].points = tetrahedron_points;
  calculate_normals(&p[0]);
  normalize_size(&p[0]);

  /* p[1]:hexahedron */
  p[1].faces = 6;
  p[1].points_per_face = 4;
  p[1].points = hexahedron_points;
  calculate_normals(&p[1]);
  normalize_size(&p[1]);

  /* p[2]: octahedron */
  p[2].faces = 8;
  p[2].points_per_face = 3;
  p[2].points = octahedron_points;
  calculate_normals(&p[2]);
  normalize_size(&p[2]);

  /* p[3]: dodecahedron */
  p[3].faces = 12;
  p[3].points_per_face = 5;
  p[3].points = dodecahedron_points;
  for (int i = 0; i < p[3].faces*p[3].points_per_face; ++i){
    scale_v3f(&p[3].points[i], 0.5);
  }
  calculate_normals(&p[3]);
  normalize_size(&p[3]);

  /* p[4]: icosahedron */
  p[4].faces = 20;
  p[4].points_per_face = 3;
  p[4].points = icosahedron_points;
  calculate_normals(&p[4]);
  normalize_size(&p[4]);
}

v3f rotate(const v3f* p, const v3f* a)
{
  v3f p1, p2, p3;
  p1.x = p->x;
  p1.y = p->y * cos(a->x) - p->z * sin(a->x);
  p1.z = p->y * sin(a->x) + p->z * cos(a->x);
  p2.x = p1.x * cos(a->y) - p1.z * sin(a->y);
  p2.y = p1.y;
  p2.z = p1.x * sin(a->y) + p1.z * cos(a->y);
  p3.x = p2.x * cos(a->z) - p2.y * sin(a->z);
  p3.y = p2.x * sin(a->z) + p2.y * cos(a->z);
  p3.z = p2.z;
  return p3;
}

void render_polyhedron(DisplayList *dl, polyhedron *p,
                       const v3f* center, float scale,
                       const v3f* angle, const v3f* vp)
{
  float z0 = -1.f; /* proj plane */

  for (int i=0; i<p->faces; ++i){
    
    /* cull back-faces */
    for (int j=0; j<=p->points_per_face; ++j){
      v3f vv = sub_v3f(vp, &p->points[i * p->points_per_face + j]);
      vv = sub_v3f(&vv, center);
      v3f n = rotate(&p->normals[i], angle);
      if (dot_v3f(&vv, &n) < 0){
        goto BACKFACE_CULL;
      }
    }
    
    /* draw the visible lines */
    Line *l = NewLine();
    for (int j=0; j<=p->points_per_face; ++j){
      v3f p3d = rotate(&p->points[i * p->points_per_face
                                  + (j % p->points_per_face)], angle);
      scale_v3f(&p3d, scale);
      p3d = add_v3f(center, &p3d);
      float d = (z0 - vp->z)/(p3d.z - vp->z);
      float x = vp->x + d*(p3d.x - vp->x);
      float y = vp->y + d*(p3d.y - vp->y);
      Point *p2d = NewPoint(x, y);
      AddPoint(l, p2d);
    }
    AddLine(dl, l);

    BACKFACE_CULL:;
  }
}

int main(int argc, char *argv[])
{
  display_params_t display_params;

  /* set the display parameters and initialize the display */
  display_params.pcm_device = "default";
  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 100;
  display_params.ac_coupling = 0;
  InitDisplay(&display_params, argc, argv);

  const int n_polyhedra = 5;
  polyhedron polyhedra[n_polyhedra];
  init_polyhedra(polyhedra);

  v3f pos = {0.f, 0.f, 0.f};
  v3f vel = {0.01f, 0.0029f, 0.017f};
  //v3f vel = {0.f, 0.f, 0.f};
  v3f angle = {0.f, 0.f, 0.f};
  v3f dangle = {0.01f, 0.03f, 0.0001f};
  v3f viewpoint = {0.f, 0.f, -3.f};
  float r = 0.3f;
  int type_idx = 0;

  uint32_t update_count = 0;
  uint32_t bounce_count = 0;
  float fps = 0.f;
  while(1){

    /* initialize a display list */
    DisplayList dl;
    InitDisplayList(&dl);

    /* update position */
    pos = add_v3f(&pos, &vel);
    angle = add_v3f(&angle, &dangle);

    /* bounce off walls */
    if (pos.x < (-1 + r)){
      pos.x = -2 + 2*r - pos.x;
      vel.x *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }
    if (pos.x > (1 - r)){
      pos.x = 2 - 2*r - pos.x;
      vel.x *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }
    if (pos.y < (-1 + r)){
      pos.y = -2 + 2*r - pos.y;
      vel.y *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }
    if (pos.y > (1 - r)){
      pos.y = 2 - 2*r - pos.y;
      vel.y *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }
    if (pos.z < (-1 + r)){
      pos.z = -2 + 2*r - pos.z;
      vel.z *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }
    if (pos.z > (1 - r)){
      pos.z = 2 - 2*r - pos.z;
      vel.z *= -1;
      bounce_count++;
      if (!(bounce_count % 5)){
        type_idx = (type_idx + 1) % n_polyhedra;
      }
    }

    render_polyhedron(&dl, &polyhedra[type_idx], &pos, r, &angle, &viewpoint);

    char message[20];
    snprintf(message, 20, "%4.1f FPS", fps);
    render_text(&dl, message, -1.f, -1.f, 0.3, 0.1, 0.2);

    /* update the display, and free the display list */
    int limit_fps = 1;
    UpdateDisplay(&display_params, &dl, limit_fps);
    FreeDisplayList(&dl);

    /* periodically print the display drawing rate */
    update_count++;
    if (!(update_count % 64)){
      fps = GetDisplayFPS(&display_params);
      fprintf(stderr, "%4.1f fps\n", fps);
              
    }
  }

  CloseDisplay(&display_params);
  return 0;
}
