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

  InitDisplay(&display_params);

  float a = 1.;
  while(1){
    usleep(100000);
    DisplayList list;
    InitDisplayList(&list);
    Line *l = NewLine();
    int N = 100;
    float pi = 3.141592653;
    for (int i = 0; i<N; ++i){
      float x = a*cos(2.*pi*i/N);
      float y = a*sin(2.*pi*i/N);
      Point *p = NewPoint(x, y);
      AddPoint(l, p);
    }
    a *= 0.99;
/*
    Point *p0 = NewPoint(-1, -1);
    Point *p1 = NewPoint(-1,  1);
    AddPoint(l, p0);
    AddPoint(l, p1);
*/
    AddLine(&list, l);
    UpdateDisplay(&display_params, &list);
    FreeDisplayList(&list);
  }

  CloseDisplay(&display_params);
  return 0;
}
