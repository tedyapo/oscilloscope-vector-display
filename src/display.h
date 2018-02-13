/*
 * display.h
 */
#ifndef DISPLAY_H_INCLUDED_
#define DISPLAY_H_INCLUDED_

#include <stdint.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include "display_list.h"
#include "display_text.h"
#include "dlo.h"

typedef struct
{
  int16_t *buffer;
  uint32_t buffer_len;
  uint32_t data_len;
} display_buffer;
  
typedef struct
{
  /* user parameters */
  char *pcm_device;
  uint32_t sample_rate;
  float frame_rate;
  float slew;
  int ac_coupling;

  /* system vars */
  uint32_t total_frames;
  struct timespec start_time;
  pthread_t thread;
  pthread_mutex_t update_mutex;
  display_buffer buffer[2];
  int active_idx;
  float sumx, sumy;
  struct timespec next_frame_time;
  int swap_flag;
  snd_pcm_t *pcm_handle;
} display_params_t;

void *display_loop(void *p);
void InitDisplay(display_params_t *display_params,
                 int argc, char* const argv[]);
void UpdateDisplay(display_params_t *display_params, DisplayList *list,
                   int limit_fps);
void CloseDisplay(display_params_t *display_params);
float GetDisplayFPS(display_params_t *display_params);

#endif /* #ifndef DISPLAY_H_INCLUDED_ */
