/*
 * draw the Hackaday Jolly Wrencher logo
 */
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "display.h"

int16_t points[] = 
{

  /* left eye */
  5321, 5022, 5431, 5038, 5542, 5114, 5578, 5229, 5542, 5327, 5391, 5416,
  5276, 5455, 5232, 5504, 5227, 5602, 5152, 5602, 5041, 5469, 5019, 5309,
  5063, 5167, 5174, 5065, 5303, 5025, 5321, 5022, 0, 0,
  
  /* right eye */
  6088, 5043, 6257, 5034, 6377, 5123, 6448, 5225, 6452, 5367, 6430, 5500,
  6355, 5566, 6284, 5606, 6257, 5509, 6159, 5438, 5999, 5358, 5928, 5273,
  5920, 5149, 6017, 5078, 6070, 5047, 6088, 5043, 0, 0,
  
  /* nose */
  5737, 5591, 5720, 5611, 5671, 5731, 5658, 5850, 5662, 5926, 5711, 5850,
  5755, 5802, 5791, 5864, 5818, 5930, 5840, 5864, 5822, 5731, 5782, 5620,
  5756, 5589, 5737, 5591, 0, 0,

  /* skull */
  4752, 5407, 4757, 5131, 4832, 4847, 4859, 4808, 4974, 4617, 5139, 4444,
  5214, 4390, 5409, 4266, 5684, 4200, 5884, 4222, 6141, 4297, 6292, 4386,
  6417, 4510, 6528, 4652, 6634, 4816, 6710, 5052, 6736, 5300, 6705, 5584,
  6656, 5735, 6550, 5948, 6479, 6050, 6369, 6151, 6301, 6210, 6341, 6285,
  6301, 6436, 6150, 6507, 6031, 6463, 5955, 6379, 5964, 6285, 5920, 6276,
  5920, 6374, 5866, 6445, 5769, 6503, 5627, 6467, 5556, 6387, 5547, 6281,
  5529, 6276, 5511, 6387, 5445, 6472, 5338, 6507, 5232, 6467, 5165, 6365,
  5161, 6237, 5181, 6203, 5171, 6195, 5118, 6139, 5121, 6143, 4961, 5979,
  4868, 5806, 4823, 5722, 0, 0, 

  /* top left wrench */
  3683, 3778, 3683, 3769, 3670, 3893, 3696, 4071, 3789, 4217, 3923, 4346,
  4136, 4408, 4357, 4395, 4791, 4761, 4797, 4754, 4859, 4648, 4926, 4532,
  5123, 4342, 5134, 4333, 5098, 4301, 4717, 3951, 4717, 3898, 4721, 3782,
  4677, 3658, 4593, 3525, 4477, 3423, 4357, 3370, 4167, 3356, 4056, 3374,
  4380, 3676, 4020, 4075, 3683, 3778, 0, 0,

  /* top right wrench */
  6701, 4737, 6607, 4612, 6536, 4488, 6368, 4346, 6346, 4336, 6763, 3960,
  6776, 3756, 6838, 3600, 6954, 3463, 7140, 3374, 7286, 3356, 7428, 3370,
  7105, 3671, 7464, 4080, 7806, 3782, 7819, 3862, 7815, 4018, 7699, 4235,
  7539, 4359, 7375, 4417, 7247, 4421, 7118, 4404, 6713, 4750, 6701, 4737,
  0, 0,

  /* bottom right wrench */
  6439, 6205, 6559, 6068, 6674, 5890, 6727, 5753, 6734, 5744, 7140, 6094,
  7286, 6072, 7460, 6090, 7615, 6188, 7739, 6316, 7819, 6503, 7815, 6698,
  7477, 6414, 7100, 6809, 7451, 7115, 7344, 7146, 7087, 7115, 6896, 6973,
  6763, 6738, 6767, 6521, 6430, 6218, 6439, 6205, 0, 0,
  
  /* bottom left wrench */
  5070, 6205, 4721, 6525, 4717, 6738, 4633, 6924, 4455, 7080, 4269, 7133,
  4100, 7133, 4038, 7115, 4384, 6809, 4025, 6410, 3674, 6711, 3661, 6560,
  3723, 6352, 3834, 6201, 3985, 6121, 4193, 6077, 4357, 6094, 4766, 5762,
  0, 0,
  

};

int main(int argc, char *argv[])
{
  display_params_t display_params;

  /* set the display parameters and initialize the display */
  display_params.pcm_device = "default";
  display_params.frame_rate = 60;
  display_params.sample_rate = 48000;
  display_params.slew = 50;
  display_params.ac_coupling = 0;
  InitDisplay(&display_params, argc, argv);

  /* scan points to determine center and scaling */
  float sumx = 0;
  float sumy = 0;
  uint32_t count = 0;
  float minx = +1e9;
  float maxx = -1e9;
  float miny = +1e9;
  float maxy = -1e9;
  for (int i=0; i<sizeof(points)/sizeof(points[0]); i+=2){
    if (points[i]){
      float x = points[i + 0];
      float y = points[i + 1];
      sumx += x;
      sumy += y;
      minx = MIN(minx, x);
      miny = MIN(miny, y);
      maxx = MAX(maxx, x);
      maxy = MAX(maxy, y);
      count++;
    }
  }
  sumx /= count;
  sumy /= count;
  float scale = 0;
  scale = MAX(scale, maxx - sumx);
  scale = MAX(scale, maxy - sumy);
  scale = MAX(scale, sumx - minx);
  scale = MAX(scale, sumy - miny);
  fprintf(stderr, "%f %f %f\n", sumx, sumy, scale);

  uint32_t update_count = 0;
  while(1){

    /* initialize a display list */
    DisplayList dl;
    InitDisplayList(&dl);

    Line *l = NewLine();
    for (int i=0; i<sizeof(points)/sizeof(points[0]); i+=2){
      if (points[i]){
        float x = points[i + 0];
        float y = points[i + 1];
        Point *p = NewPoint((x-sumx)/scale, (sumy-y)/scale);
        AddPoint(l, p);
      } else {
        AddLine(&dl, l);
        l = NewLine();
      }
    }

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
