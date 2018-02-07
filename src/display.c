#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "display.h"

void *display_loop(void *p)
{
  display_params_t *display_params = (display_params_t*)p;
  while(1){
    fprintf(stderr, ".");
    pthread_mutex_lock(&display_params->update_mutex);

    snd_pcm_sframes_t samples; /* called frames in ALSA, but samples here to avoid confusion */
    samples = snd_pcm_writei(display_params->pcm_handle, display_params->frame_data, display_params->frame_len);
    if (samples < 0){
      samples = snd_pcm_recover(display_params->pcm_handle, samples, 0);
      if (samples < 0){
        ERROR("snd_pcm_writei() failed");
      }
    }
    if (samples > 0 && samples < display_params->frame_len){
      ERROR("short write in snd_pcm_writei()");
    }
   
    pthread_mutex_unlock(&display_params->update_mutex);
  }
}

void InitDisplay(display_params_t *display_params)
{
  int err;
  err = snd_pcm_open(&display_params->pcm_handle, display_params->pcm_device, SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0){
    ERROR("snd_pcm_open() failed");
  }

  uint32_t latency_usec = (int)1000000./display_params->frame_rate;
  err = snd_pcm_set_params(display_params->pcm_handle,
                           SND_PCM_FORMAT_S16,
                           SND_PCM_ACCESS_RW_INTERLEAVED,
                           2,
                           display_params->sample_rate,
                           0,
                           latency_usec);
  if (err < 0){
    ERROR("snd_pcm_set_params() failed");
  }

  display_params->frame_data = NULL;
  display_params->frame_len = 0;

  pthread_mutex_init(&display_params->update_mutex, NULL);
  int ret = pthread_create(&display_params->thread, NULL, display_loop, (void*)display_params);
  if (ret){
    fprintf(stderr, "Error: pthread_create() failed, returning: %d\n", ret);
    exit(EXIT_FAILURE);
  }
}

void UpdateDisplay(display_params_t *display_params, DisplayList *list)
{
  /* find frame length */
  uint32_t len = 0;
  Line *l = list->lines;
  while (l){
    Point *p = l->points;
    while (p){
      len++;
      p = p->next;
    }
    l = l->next;
  }
  fprintf(stderr, "len = %d\n", len);

  pthread_mutex_lock(&display_params->update_mutex);

  display_params->frame_len = len;

  if (display_params->frame_data){
    free(display_params->frame_data);
  }
  display_params->frame_data = (int16_t*) malloc(2*len*sizeof(int16_t));
  if (!display_params->frame_data){
    ERROR("malloc() failed");
  }

  int16_t *d = display_params->frame_data;
  l = list->lines;
  while (l){
    Point *p = l->points;
    while (p){
      *d++ = 32767. * CLAMP(p->x, -1., 1.);
      *d++ = 32767. * CLAMP(p->y, -1., 1.);
      //printf("%f %f\n", p->x, p->y);
      p = p->next;
    }
    l = l->next;
  }

  pthread_mutex_unlock(&display_params->update_mutex);
}

void CloseDisplay(display_params_t *display_params)
{
  snd_pcm_close(display_params->pcm_handle);
  pthread_cancel(display_params->thread);
  pthread_join(display_params->thread, NULL);
}
