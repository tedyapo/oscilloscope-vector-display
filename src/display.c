#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "display.h"

void* display_loop(void *p)
{
  display_params_t *display_params = (display_params_t*)p;
  
  /* setup for polling pcm status */
  struct pollfd *fds;
  int count = snd_pcm_poll_descriptors_count(display_params->pcm_handle);
  if (count < 0){
    ERROR("snd_pcm_poll_descriptors_count() returned %d", count);
  }
  fds = (struct pollfd *) malloc(count * sizeof(struct pollfd));
  if (NULL == fds){
    ERROR("malloc() failed");
  }
  int err = snd_pcm_poll_descriptors(display_params->pcm_handle, fds, count);
  if (err < 0){
    ERROR("snd_pcm_poll_descriptors() failed: %s", snd_strerror(err));
  }

  while(1){
    fprintf(stderr, ".");

    pthread_mutex_lock(&display_params->update_mutex);
    if (display_params->swap_flag){
      display_params->active_idx = (display_params->active_idx + 1) % 2;
      display_params->swap_flag = 0;
    }
    pthread_mutex_unlock(&display_params->update_mutex);

    snd_pcm_sframes_t samples; /* called frames in ALSA, but samples here to avoid confusion */
    samples = snd_pcm_writei(display_params->pcm_handle,
                             display_params->buffer[display_params->active_idx].buffer,
                             display_params->buffer[display_params->active_idx].data_len);
    if (samples < 0){
      samples = snd_pcm_recover(display_params->pcm_handle, samples, 0);
      if (samples < 0){
        ERROR("snd_pcm_writei() failed");
      }
    }
    if (samples > 0 && samples < display_params->buffer[display_params->active_idx].data_len){
      ERROR("short write in snd_pcm_writei()");
    }

    if (snd_pcm_state(display_params->pcm_handle) != SND_PCM_STATE_RUNNING){
      continue;
    }

    /* poll until ready for more data */
    while(1){
      poll(fds, count, -1);
      unsigned short revents;
      snd_pcm_poll_descriptors_revents(display_params->pcm_handle, fds, count, &revents);
      if (revents & POLLERR){
        ERROR("poll error");
      }
      if (revents & POLLOUT){
        break;
      }
    }
  }
}

void UpdateDisplay(display_params_t *display_params, DisplayList *list, int limit_fps)
{
  if (limit_fps){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    if (display_params->next_frame_time.tv_sec > now.tv_sec ||
        display_params->next_frame_time.tv_nsec > now.tv_nsec){
      struct timespec delay;
      if (now.tv_nsec > display_params->next_frame_time.tv_nsec){
        delay.tv_nsec =  1000000000 + display_params->next_frame_time.tv_nsec - now.tv_nsec;
        delay.tv_sec = display_params->next_frame_time.tv_sec - now.tv_sec - 1;
      } else {
        delay.tv_nsec = display_params->next_frame_time.tv_nsec - now.tv_nsec;
        delay.tv_sec = display_params->next_frame_time.tv_sec - now.tv_sec;
      }
      nanosleep(&delay, NULL);
    }
    clock_gettime(CLOCK_REALTIME, &now);
    double period = 1./display_params->frame_rate;
    time_t sec = floor(period);
    long nsec = 1.e9 * (period - sec) + now.tv_nsec;
    if (nsec > 999999999){
      nsec -= 999999999;
      sec++;
    }
    sec += now.tv_sec;
    display_params->next_frame_time.tv_sec = sec;
    display_params->next_frame_time.tv_nsec = nsec;
  }

  /* overwrite undisplayed frame if updating too fast */
  pthread_mutex_lock(&display_params->update_mutex);
  display_params->swap_flag = 0;
  int buf_idx = display_params->active_idx;
  pthread_mutex_unlock(&display_params->update_mutex);

  uint32_t idx = 0;
  for (Line *l = list->lines; l; l = l->next){
    Point *p = l->points;
    if (!p) continue;
    float oldx = p->x;
    float oldy = p->y;
    while (p){
      float d = sqrt((oldx - p->x)*(oldx - p->x) + (oldy - p->y)*(oldy - p->y));
      int n_pts = MAX(2, (int) display_params->slew*d);
      for (int i=0; i < n_pts; ++i){
        float x = oldx + (p->x - oldx)*((float)i)/n_pts;
        float y = oldy + (p->y - oldy)*((float)i)/n_pts;
        display_params->buffer[buf_idx].buffer[2*idx+0] = 32767. * CLAMP(x, -1., 1.);
        display_params->buffer[buf_idx].buffer[2*idx+1] = 32767. * CLAMP(y, -1., 1.);
        idx++;
        if (2*idx > display_params->buffer[buf_idx].buffer_len){
          display_params->buffer[buf_idx].buffer_len *= 2;
          display_params->buffer[buf_idx].buffer = (int16_t*) realloc(display_params->buffer[buf_idx].buffer,
                                                                      2*display_params->buffer[buf_idx].buffer_len);
          if (!display_params->buffer[buf_idx].buffer){
            ERROR("realloc() failed");
          }
        }
      }
      oldx = p->x;
      oldy = p->y;
      p = p->next;
    }
  }

  display_params->buffer[buf_idx].data_len = idx;

  /* flag buffer swap after current refresh */
  pthread_mutex_lock(&display_params->update_mutex);
  display_params->swap_flag = 1;
  pthread_mutex_unlock(&display_params->update_mutex);
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

  /* first guess at buffer: will be expanded as needed */
  for (int i = 0; i < 2; ++i){
    display_params->buffer[i].buffer_len = (int)(display_params->sample_rate / display_params->frame_rate);
    display_params->buffer[i].buffer = (int16_t*)malloc(2*display_params->buffer[i].buffer_len);
    if (NULL == display_params->buffer[i].buffer){
      ERROR("malloc() failed");
    }
    display_params->buffer[i].data_len = 0;
  }
  display_params->active_idx = 0;
  display_params->swap_flag = 0;

  clock_gettime(CLOCK_REALTIME, &display_params->next_frame_time);

  pthread_mutex_init(&display_params->update_mutex, NULL);
  int ret = pthread_create(&display_params->thread, NULL, display_loop, (void*)display_params);
  if (ret){
    fprintf(stderr, "Error: pthread_create() failed, returning: %d\n", ret);
    exit(EXIT_FAILURE);
  }
}

void CloseDisplay(display_params_t *display_params)
{
  snd_pcm_close(display_params->pcm_handle);
  pthread_cancel(display_params->thread);
  pthread_join(display_params->thread, NULL);
}
