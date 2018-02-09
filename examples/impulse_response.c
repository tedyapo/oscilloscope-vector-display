/*
 * determine impulse response of audio adapter
 */
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "display.h"

int main(int argc, char *argv[])
{
  display_params_t display_params;

  /* set the display parameters and initialize the display */
  display_params.pcm_device = "default";
  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 0;
  display_params.ac_coupling = 0;
  InitDisplay(&display_params, argc, argv);

  uint32_t update_count = 0;
  while(1){

    /* initialize a display list */
    DisplayList dl;
    InitDisplayList(&dl);

    Line *l = NewLine();
    int period = 1000;
    for (int i=0; i < period/2; ++i){
      Point *p = NewPoint(0.f, 0.f);
      AddPoint(l, p);
    }
    Point *p = NewPoint(1.f, 1.f);
    AddPoint(l, p);
    for (int i=0; i < period - period/2 - 1; ++i){
      Point *p = NewPoint(0.f, 0.f);
      AddPoint(l, p);
    }

    AddLine(&dl, l);

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

  CloseDisplay(&display_params);
  return 0;
}
