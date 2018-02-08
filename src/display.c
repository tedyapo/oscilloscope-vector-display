#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "display.h"

void* display_loop(void *p)
{
  display_params_t *dp = (display_params_t*)p;
  
  /* setup for polling pcm status */
  struct pollfd *fds;
  int count = snd_pcm_poll_descriptors_count(dp->pcm_handle);
  if (count < 0){
    ERROR("snd_pcm_poll_descriptors_count() returned %d", count);
  }
  fds = (struct pollfd *) malloc(count * sizeof(struct pollfd));
  if (NULL == fds){
    ERROR("malloc() failed");
  }
  int err = snd_pcm_poll_descriptors(dp->pcm_handle, fds, count);
  if (err < 0){
    ERROR("snd_pcm_poll_descriptors() failed: %s", snd_strerror(err));
  }

  while(1){
    fprintf(stderr, ".");

    pthread_mutex_lock(&dp->update_mutex);
    if (dp->swap_flag){
      dp->active_idx = (dp->active_idx + 1) % 2;
      dp->swap_flag = 0;
    }
    pthread_mutex_unlock(&dp->update_mutex);

    /* called frames in ALSA, but samples here to avoid confusion */
    snd_pcm_sframes_t samples;
    samples = snd_pcm_writei(dp->pcm_handle,
                             dp->buffer[dp->active_idx].buffer,
                             dp->buffer[dp->active_idx].data_len);
    if (samples < 0){
      samples = snd_pcm_recover(dp->pcm_handle, samples, 0);
      if (samples < 0){
        ERROR("snd_pcm_writei() failed");
      }
    }
    if (samples > 0 && samples < dp->buffer[dp->active_idx].data_len){
      ERROR("short write in snd_pcm_writei()");
    }

    if (snd_pcm_state(dp->pcm_handle) != SND_PCM_STATE_RUNNING){
      continue;
    }

    /* poll until ready for more data */
    while(1){
      poll(fds, count, -1);
      unsigned short revents;
      snd_pcm_poll_descriptors_revents(dp->pcm_handle, fds, count, &revents);
      if (revents & POLLERR){
        ERROR("poll error");
      }
      if (revents & POLLOUT){
        break;
      }
    }
  }
}

void UpdateDisplay(display_params_t *dp, DisplayList *list, int limit_fps)
{
  if (limit_fps){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    if (dp->next_frame_time.tv_sec > now.tv_sec ||
        dp->next_frame_time.tv_nsec > now.tv_nsec){
      struct timespec delay;
      if (now.tv_nsec > dp->next_frame_time.tv_nsec){
        delay.tv_nsec =  1000000000 + dp->next_frame_time.tv_nsec - now.tv_nsec;
        delay.tv_sec = dp->next_frame_time.tv_sec - now.tv_sec - 1;
      } else {
        delay.tv_nsec = dp->next_frame_time.tv_nsec - now.tv_nsec;
        delay.tv_sec = dp->next_frame_time.tv_sec - now.tv_sec;
      }
      nanosleep(&delay, NULL);
    }
    clock_gettime(CLOCK_REALTIME, &now);
    double period = 1./dp->frame_rate;
    time_t sec = floor(period);
    long nsec = 1.e9 * (period - sec) + now.tv_nsec;
    if (nsec > 999999999){
      nsec -= 999999999;
      sec++;
    }
    sec += now.tv_sec;
    dp->next_frame_time.tv_sec = sec;
    dp->next_frame_time.tv_nsec = nsec;
  }

  /* overwrite undisplayed frame if updating too fast */
  pthread_mutex_lock(&dp->update_mutex);
  dp->swap_flag = 0;
  int buf_idx = dp->active_idx;
  pthread_mutex_unlock(&dp->update_mutex);

  uint32_t idx = 0;
  for (Line *l = list->lines; l; l = l->next){
    Point *p = l->points;
    if (!p) continue;
    float oldx = p->x;
    float oldy = p->y;
    while (p){
      float d = sqrt((oldx - p->x)*(oldx - p->x) + (oldy - p->y)*(oldy - p->y));
      int n_pts = MAX(2, (int) dp->slew*d);
      for (int i=0; i < n_pts; ++i){
        float x = oldx + (p->x - oldx)*((float)i)/n_pts;
        float y = oldy + (p->y - oldy)*((float)i)/n_pts;
        dp->sumx += x;
        dp->sumy += y;
        dp->buffer[buf_idx].buffer[2*idx+0] = 32767. * CLAMP(x, -1., 1.);
        dp->buffer[buf_idx].buffer[2*idx+1] = 32767. * CLAMP(y, -1., 1.);
        idx++;
        if (2*idx > dp->buffer[buf_idx].buffer_len){
          dp->buffer[buf_idx].buffer_len *= 2;
          dp->buffer[buf_idx].buffer = (int16_t*)
            realloc(dp->buffer[buf_idx].buffer,
                    2*dp->buffer[buf_idx].buffer_len);
          if (!dp->buffer[buf_idx].buffer){
            ERROR("realloc() failed");
          }
        }
      }
      oldx = p->x;
      oldy = p->y;
      p = p->next;
    }
  }
  
  if (dp->ac_coupling){
    float dx, dy;
    if (fabsf(dp->sumx) > fabs(dp->sumy)){
      dx = copysign(1.f, dp->sumx);
      dy = copysign(fabs(dp->sumy/dp->sumx), dp->sumy);
    } else {
      dx = copysign(fabs(dp->sumx/dp->sumy), dp->sumx);
      dy = copysign(1.f, dp->sumy);
    }
    int n_pts = floor(MAX(fabsf(dp->sumx), fabsf(dp->sumy)));
    for (int i = 0; i < n_pts; ++i){
      dp->buffer[buf_idx].buffer[2*idx+0] = 32767. * CLAMP(-dx, -1., 1.);
      dp->buffer[buf_idx].buffer[2*idx+1] = 32767. * CLAMP(-dy, -1., 1.);
      idx++;
      if (2*idx > dp->buffer[buf_idx].buffer_len){
        dp->buffer[buf_idx].buffer_len *= 2;
        dp->buffer[buf_idx].buffer = (int16_t*)
          realloc(dp->buffer[buf_idx].buffer,
                  2*dp->buffer[buf_idx].buffer_len);
        if (!dp->buffer[buf_idx].buffer){
          ERROR("realloc() failed");
        }
      }
      dp->sumx -= dx;
      dp->sumy -= dy;
    }
  }

  dp->buffer[buf_idx].data_len = idx;

  /* flag buffer swap after current refresh */
  pthread_mutex_lock(&dp->update_mutex);
  dp->swap_flag = 1;
  pthread_mutex_unlock(&dp->update_mutex);
}

void InitDisplay(display_params_t *dp)
{
  int err;
  err = snd_pcm_open(&dp->pcm_handle, dp->pcm_device,
                     SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0){
    ERROR("snd_pcm_open() failed");
  }

  uint32_t latency_usec = (int)1000000./dp->frame_rate;
  err = snd_pcm_set_params(dp->pcm_handle,
                           SND_PCM_FORMAT_S16,
                           SND_PCM_ACCESS_RW_INTERLEAVED,
                           2,
                           dp->sample_rate,
                           0,
                           latency_usec);
  if (err < 0){
    ERROR("snd_pcm_set_params() failed");
  }

  /* first guess at buffer: will be expanded as needed */
  for (int i = 0; i < 2; ++i){
    dp->buffer[i].buffer_len = (int)(dp->sample_rate / dp->frame_rate);
    dp->buffer[i].buffer = (int16_t*)malloc(2*dp->buffer[i].buffer_len);
    if (NULL == dp->buffer[i].buffer){
      ERROR("malloc() failed");
    }
    dp->buffer[i].data_len = 0;
  }
  dp->active_idx = 0;
  dp->swap_flag = 0;

  dp->sumx = 0.f;
  dp->sumy = 0.f;

  clock_gettime(CLOCK_REALTIME, &dp->next_frame_time);

  pthread_mutex_init(&dp->update_mutex, NULL);
  int ret = pthread_create(&dp->thread, NULL, display_loop, (void*)dp);
  if (ret){
    fprintf(stderr, "Error: pthread_create() failed, returning: %d\n", ret);
    exit(EXIT_FAILURE);
  }
}

void CloseDisplay(display_params_t *dp)
{
  snd_pcm_close(dp->pcm_handle);
  pthread_cancel(dp->thread);
  pthread_join(dp->thread, NULL);
}
