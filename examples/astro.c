/*
 * determine impulse response of audio adapter
 */
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "dlo.h"

static const float PI = 3.141592653f;

#define SHOT_ACTIVE 0x1

void create_rock(DLO *d, DLO *ship)
{
  float r_max = 0.05 + 0.09 * drand48();
  DLO_init(d, DLO_OUTLINE_RANDOM, 6, r_max/3, r_max);
  do {
    d->pos.x = 1-2*drand48();
    d->pos.y = 1-2*drand48();
    d->pvel.x = 0.005*(1-2*drand48());
    d->pvel.y = 0.005*(1-2*drand48());
    d->ang = 0;
    d->avel = 0.1*(1-2*drand48());
  } while (vec2f_dist(d->pos, ship->pos) < 0.5);
}

void create_shot(DLO *shot, DLO *ship)
{
  DLO_init(shot, DLO_OUTLINE_REGULAR, 3, 0.05, 0.05);
  shot->pos = ship->pos;
  shot->pvel.x = 0.1 * cosf(ship->ang);
  shot->pvel.y = 0.1 * sinf(ship->ang);
  shot->ang = ship->ang;
  shot->avel = 0;
  shot->flags = SHOT_ACTIVE;
}

int main(int argc, char* argv[])
{
  display_params_t display_params;

  /* seed RNG based on time */
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  srand48(now.tv_nsec);

  /* set the display parameters and initialize the display */
  display_params.pcm_device = "default";
  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 20;
  display_params.ac_coupling = 0;
  InitDisplay(&display_params, argc, argv);

  /* initialize SDL */
  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    fprintf(stderr, "%s", SDL_GetError());
  atexit(SDL_Quit);
  if (0 == SDL_NumJoysticks()){
    ERROR("no joystick found");
  }
  SDL_Joystick *joy = SDL_JoystickOpen(0);
  SDL_JoystickEventState(SDL_ENABLE);

  DLO ship;
  DLO_init(&ship, DLO_OUTLINE_REGULAR, 3, 0.1, 0.1); 
  ship.pos.x = 0;
  ship.pos.y = 0;
  ship.pvel.x = 0;
  ship.pvel.y = 0;
  ship.ang = 0;
  ship.avel = 0;

  const int max_shots = 10;
  DLO shots[max_shots];
  for (int i=0; i<max_shots; i++){
    shots[i].flags = 0;
  }

  const int max_rocks = 10;
  DLO rocks[max_rocks];
  int n_rocks = 5;

  for (int i=0; i<n_rocks; i++){
    create_rock(&rocks[i], &ship);
  }

  SDL_Event event;
  int quit_flag = 0;
  uint32_t update_count = 0;
  float x = 0;
  float y = 0;
  int score = 0;
  while (!quit_flag){

    /* process joystick events */
    while(SDL_PollEvent(&event)){
      switch(event.type){
      case SDL_JOYAXISMOTION:
        /* horizontal */
        if (event.jaxis.axis == 0){
          if (event.jaxis.value < 0){
            ship.ang = fmodf(ship.ang + PI/12, 2*PI);
          }
          if (event.jaxis.value > 0){
            ship.ang = fmodf(ship.ang - PI/12, 2*PI);
          }
        }
        /* vertical */
        if (event.jaxis.axis == 1){
          if (event.jaxis.value < 0){
            ship.pvel.x += 0.001*cosf(ship.ang);
            ship.pvel.y += 0.001*sinf(ship.ang);
          }
          if (event.jaxis.value > 0){
            ship.pvel.x -= 0.001*cosf(ship.ang);
            ship.pvel.y -= 0.001*sinf(ship.ang);
          }
        }
        break;
      case SDL_JOYBUTTONDOWN:
        fprintf(stderr, "button %d\n", event.jbutton.button);
        switch (event.jbutton.button){
        case 0:
          /* fire */
        {
          int i;
          for (i=0; i<max_shots; ++i){
            if (!shots[i].flags & SHOT_ACTIVE){
              break;
            }
          }
          if (i < max_shots){
            create_shot(&shots[i], &ship);
          }
          break;
        }
        case 1:
          /* hyperspace */
          ship.pos.x = 0.8*(1 - 2 * drand48());
          ship.pos.y = 0.8*(1 - 2 * drand48());
          break;
        case 7:
          quit_flag = 1;
          break;
        default:;
        }
        break;
      }
    }

    /* initialize a display list */
    DisplayList dl;
    InitDisplayList(&dl);

    /* draw the frame */
    Line *l = NewLine();
    Point *p = NewPoint(-1, -1);
    AddPoint(l, p);
    p = NewPoint(-1, +1);
    AddPoint(l, p);
    p = NewPoint(+1, +1);
    AddPoint(l, p);
    p = NewPoint(+1, -1);
    AddPoint(l, p);
    p = NewPoint(-1, -1);
    AddPoint(l, p);
    AddLine(&dl, l);

    /* update the rocks positions */
    for (int i=0; i<n_rocks; i++){
      DLO_update(&rocks[i]);
      if (rocks[i].pos.x > +1) rocks[i].pos.x -= 2;
      if (rocks[i].pos.x < -1) rocks[i].pos.x += 2;
      if (rocks[i].pos.y > +1) rocks[i].pos.y -= 2;
      if (rocks[i].pos.y < -1) rocks[i].pos.y += 2;
    }

    /* bounce rocks off rocks */
    for (int i=0; i<n_rocks; ++i){
      for (int j=0; j<i; ++j){
        DLO_collision coll = DLO_intersect_DLO(&rocks[i], &rocks[j]);
        DLO_interact_DLO(&rocks[i], &rocks[j], &coll);
      }
    }

    /* check for collision with ship */
    for (int i=0; i<n_rocks; ++i){
      DLO_collision coll = DLO_intersect_DLO(&rocks[i], &ship);
      if (coll.detected){
        //quit_flag = 1;
      }
    }

    /* update ship position */
    DLO_update(&ship);
    if (ship.pos.x > +1) ship.pos.x -= 2;
    if (ship.pos.x < -1) ship.pos.x += 2;
    if (ship.pos.y > +1) ship.pos.y -= 2;
    if (ship.pos.y < -1) ship.pos.y += 2;
    ship.pvel = vec2f_mul_f(ship.pvel, 0.995);

    /* update the shots positions */
    for (int i=0; i<max_shots; i++){
      if (shots[i].flags & SHOT_ACTIVE){
        DLO_update(&shots[i]);
        if (shots[i].pos.x > +1 ||
            shots[i].pos.x < -1 ||
            shots[i].pos.y > +1 ||
            shots[i].pos.y < -1){
          shots[i].flags = 0;
          DLO_free(&shots[i]);
        }
      }
    }

    /* check for shot hitting rocks */
    for (int i=0; i<n_rocks; ++i){
      for (int j=0; j<max_shots; ++j){
        if (shots[j].flags & SHOT_ACTIVE){
          DLO_collision coll = DLO_intersect_DLO(&rocks[i], &shots[j]);
          if (coll.detected){
            DLO_free(&shots[j]);
            shots[j].flags = 0;
            create_rock(&rocks[i], &ship);
            score++;
          }
        }
      }
    }

    /* draw ship */
    DLO_render(&dl, &ship, 50);

    /* draw rocks */
    for (int i=0; i<n_rocks; i++){
      DLO_render(&dl, &rocks[i], 50);
    }

    /* draw shots */
    for (int i=0; i<max_shots; i++){
      if (shots[i].flags & SHOT_ACTIVE){
        DLO_render(&dl, &shots[i], 50);
      }
    }

    /* display the score */
    char scoretext[20];
    snprintf(scoretext, 20, "%05d", score);
    render_text(&dl, scoretext, -0.5, -0.95, 0.1, 0.15, 0.2, 50);

    /* update the display, and free the display list */
    int limit_fps = 1;
    UpdateDisplay(&display_params, &dl, limit_fps);
    FreeDisplayList(&dl);

    /* periodically print the display drawing rate */
    update_count++;
    if (!(update_count % 32)){
      fprintf(stderr, "%4.1f fps\n",
              GetDisplayFPS(&display_params));
    }

  }
  
  if (SDL_JoystickGetAttached(joy)){
    SDL_JoystickClose(joy);
  }

  CloseDisplay(&display_params);  
  return 0;
}
