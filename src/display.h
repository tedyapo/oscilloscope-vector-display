/*
 * display.h
 */
#ifndef DISPLAY_H_INCLUDED_
#define DISPLAY_H_INCLUDED_

#include <stdint.h>
#include <alsa/asoundlib.h>
#include "display_list.h"

typedef struct
{
  pthread_t thread;
  pthread_mutex_t update_mutex;
  int16_t *frame_data;
  uint32_t frame_len;
  float frame_rate;
  uint32_t sample_rate;
  char *pcm_device;
  snd_pcm_t *pcm_handle;
} display_params_t;

void *display_loop(void *p);
void InitDisplay(display_params_t *display_params);
void UpdateDisplay(display_params_t *display_params, DisplayList *list);
void CloseDisplay(display_params_t *display_params);

#endif /* #ifndef DISPLAY_H_INCLUDED_ */
