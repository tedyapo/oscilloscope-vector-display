#include <unistd.h>
#include <math.h>
#include "display.h"

int main()
{
  display_params_t display_params;

//  display_params.pcm_device = "default";
  display_params.pcm_device = "hw:CARD=Device,DEV=0";

  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 10;
  display_params.ac_coupling = 0;

  InitDisplay(&display_params);

  float t = 0.f;
  while(1){
    DisplayList list;
    InitDisplayList(&list);
    Line *l = NewLine();
    int N = 20;
    const float pi = 3.141592653;
    float a = 0.5+0.25*cos(2*pi*t*13);
    float cx = cos(2*pi*t);
    float cy = sin(2*pi*t);
    for (int i = 0; i<N; ++i){
      float x = cx + a*cos(2.*pi*i/N);
      float y = cy + a*sin(2.*pi*i/N);
      Point *p = NewPoint(x, y);
      AddPoint(l, p);
    }
    t += 0.001;

    AddLine(&list, l);
    fprintf(stderr, "u");
    UpdateDisplay(&display_params, &list, 1);
    FreeDisplayList(&list);
  }

  CloseDisplay(&display_params);
  return 0;
}
