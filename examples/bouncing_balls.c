/*
 * bouncing balls example
 */
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "display.h"

#define PI 3.141592653

/* approximate circle with n segments (also known as regular polygon) */
void draw_circle(DisplayList *dl, float cx, float cy, float r, int n)
{
  Line *l = NewLine();
  for (int i = 0; i <= n; i++){
    float theta = (2*PI*i)/n;
    float x = cx + r * cos(theta);
    float y = cy + r * sin(theta);
    Point *p = NewPoint(x, y);
    AddPoint(l, p);
  }
  AddLine(dl, l);
}

typedef struct
{
  float cx, cy;
  float r;
  float vx, vy;
} ball;

void create_random_balls(ball* balls, int n_balls)
{
  for (int i=0; i<n_balls; ++i){
    balls[i].cx = 1-2*drand48();
    balls[i].cy = 1-2*drand48();
    balls[i].r = 0.05 + 0.1*drand48();
    balls[i].vx = 0.01-0.02*drand48();
    balls[i].vy = 0.01-0.02*drand48();
  }
}

void update_positions(ball* balls, int n_balls, float dt)
{
  /* update ball positions */
  for (int i=0; i<n_balls; ++i){
    balls[i].cx += balls[i].vx * dt;
    balls[i].cy += balls[i].vy * dt;
  }

  /* bounce off walls */
  for (int i=0; i<n_balls; ++i){
    if (balls[i].cx < (-1 + balls[i].r)){
      balls[i].cx = -2 + 2*balls[i].r - balls[i].cx; 
      balls[i].vx *= -1;
    }
    if (balls[i].cx > (1 - balls[i].r)){
      balls[i].cx = 2 - 2*balls[i].r - balls[i].cx; 
      balls[i].vx *= -1;
    }
    if (balls[i].cy < (-1 + balls[i].r)){
      balls[i].cy = -2 + 2*balls[i].r - balls[i].cy; 
      balls[i].vy *= -1;
    }
    if (balls[i].cy > (1 - balls[i].r)){
      balls[i].cy = 2 - 2*balls[i].r - balls[i].cy; 
      balls[i].vy *= -1;
    }
  }

  /* bouce off other balls */
  for (int i=0; i<n_balls; ++i){
    for (int j=0; j<i; ++j){

      /* if intersection, these should have interacted */
      float d = sqrt((balls[i].cx - balls[j].cx)*(balls[i].cx - balls[j].cx) +
                     (balls[i].cy - balls[j].cy)*(balls[i].cy - balls[j].cy));
      if (d > (balls[i].r + balls[j].r)) continue;

      /* find time (in past) when collision should have happened */
      float cx = balls[i].cx - balls[j].cx;
      float cy = balls[i].cy - balls[j].cy;
      float vx = balls[i].vx - balls[j].vx;
      float vy = balls[i].vy - balls[j].vy;
      float a = vx*vx + vy*vy;
      float b = 2*(vx*cx + vy*cy);
      float c = cx*cx + cy*cy - 
        (balls[i].r + balls[j].r)*(balls[i].r + balls[j].r);
      float t0, t1;
      // use numerically stable quadratic solution
      // https://people.csail.mit.edu/bkph/articles/Quadratics.pdf
      float det = b*b-4*a*c;
      if (det < 0){
        // never intersected: corner case
        continue;
      }
      det = sqrt(det);
      if (b >= 0){
        t0 = (-b - det)/(2*a);
        t1 = 2*c/(-b - det);
      } else {
        t0 = 2*c/(-b + det);
        t1 = (-b + det)/(2*a);
      }
      float t;
      if (t0 < 0){
        t = t0;
      }
      if (t1 < t){
        t = t1;
      }
        
      /* find (previous) positions at interaction time */
      float xi = balls[i].cx + t * balls[i].vx;
      float yi = balls[i].cy + t * balls[i].vy;
      float xj = balls[j].cx + t * balls[j].vx;
      float yj = balls[j].cy + t * balls[j].vy;

      /* find unit vector between centers */
      float ax = xi - xj;
      float ay = yi - yj;
      float l = sqrt(ax*ax + ay*ay);
      ax /= l;
      ay /= l;

      /* mass */
      float mi = balls[i].r * balls[i].r;
      float mj = balls[j].r * balls[j].r;

      /* velocity components along axis before collision */
      float vib = balls[i].vx * ax + balls[i].vy * ay;
      float vjb = balls[j].vx * ax + balls[j].vy * ay;

      /* axial velocity after collison */
      float via = ((mi-mj)*vib+2*mj*vjb)/(mi+mj);
      float vja = ((mj-mi)*vjb +2*mi*vib)/(mi+mj);

      /* reverse positions to interaction time */
      balls[i].cx += t * balls[i].vx;
      balls[i].cy += t * balls[i].vy;
      balls[j].cx += t * balls[j].vx;
      balls[j].cy += t * balls[j].vy;

      /* updated velocities */
      balls[i].vx += via*ax - vib*ax;
      balls[i].vy += via*ay - vib*ay;
      balls[j].vx += vja*ax - vjb*ax;
      balls[j].vy += vja*ay - vjb*ay;

      /* update to post-interaction positions */
      balls[i].cx -= t * balls[i].vx;
      balls[i].cy -= t * balls[i].vy;
      balls[j].cx -= t * balls[j].vx;
      balls[j].cy -= t * balls[j].vy;
    }
  }
}

/* check for energy conservation */
float total_energy(ball *balls, int n_balls)
{
  float E = 0.f;
  for (int i=0; i<n_balls; ++i){
    E += (balls[i].vx * balls[i].vx + 
          balls[i].vy * balls[i].vy) * balls[i].r * balls[i].r;
  }
  return E;
}

int main()
{
  display_params_t display_params;

  /* set the display parameters and initialize the display */
  /* display_params.pcm_device = "default"; */
  display_params.pcm_device = "hw:CARD=Device,DEV=0";
  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 10;
  display_params.ac_coupling = 0;
  InitDisplay(&display_params);

  const int n_balls = 10;
  ball balls[n_balls];
  create_random_balls(balls, n_balls);

  float dt = 1.; /* simulation timestep */
  uint32_t update_count = 0;
  while(1){

    update_positions(balls, n_balls, dt);

    /* initialize a display list */
    DisplayList dl;
    InitDisplayList(&dl);

    /* render balls into display list */
    for (int i=0; i<n_balls; ++i){
      draw_circle(&dl, balls[i].cx, balls[i].cy, balls[i].r, 20);
    }

    /* update the display, and free the display list */
    int limit_fps = 1;
    UpdateDisplay(&display_params, &dl, limit_fps);
    FreeDisplayList(&dl);

    /* periodically print the display drawing rate */
    update_count++;
    if (!(update_count % 32)){
      fprintf(stderr, "%4.1f fps; E = %f\n",
              GetDisplayFPS(&display_params),
              1000000.f*total_energy(balls, n_balls));
    }
  }

  CloseDisplay(&display_params);
  return 0;
}
