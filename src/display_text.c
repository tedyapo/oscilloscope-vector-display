#include "display_text.h"

int char0[11] = {11, 0, 0, 1, 2, 5, 8, 7, 6, 3, 0};
int char1[5] = {5, 1, 1, 4, 7};
int char2[10] = {10, 0, 1, 2, 5, 4, 3, 6, 7, 8};
int char3[12] = {12, 0, 1, 2, 5, 4, 3, 4, 5, 8, 7, 6};
int char4[8] = {8, 0, 3, 4, 5, 2, 5, 8};
int char5[10] = {10, 2, 1, 0, 3, 4, 5, 8, 7, 6};
int char6[11] = {11, 2, 1, 0, 3, 6, 7, 8, 5, 4, 3};
int char7[8] = {8, 0, 0, 0, 1, 2, 4, 6};
int char8[12] = {12, 5, 2, 1, 0, 3, 4, 5, 8, 7, 6, 3};
int char9[11] = {11, 8, 8, 8, 5, 2, 1, 0, 3, 4, 5};
int charF[11] = {11, 6, 6, 3, 4, 5, 4, 3, 0, 1, 2};
int charP[9] = {9, 6, 3, 0, 1, 2, 5, 4, 3};
int charS[11] = {11, 2, 2, 1, 0, 3, 4, 5, 8, 7, 6};
int charPeriod[9] = {9, 7, 7, 7, 7, 7, 7, 7, 7};

void draw_glyph(DisplayList *dl, float xc, float yc,
                float *xp, float *yp, int *pts, float slew)
{
  Line *l = NewLine();
  l->slew = slew;
  float oldx;
  float oldy;
  for (int i = 1; i < pts[0]; ++i){
    float x = xc + xp[pts[i]];
    float y = yc + yp[pts[i]];
    Point *p = NewPoint(x, y);
    AddPoint(l, p);
  }
  AddLine(dl, l);
}

void render_text(DisplayList *dl, const char* text, float x, float y,
                 float height, float width, float spacing, float slew)
{

  /* calculate point array
   *
   *   0-1-2
   *   |\|/|
   *   3-4-5
   *   |/|\|
   *   6-7-8
   */
  float px[9];
  float py[9];
  for (int i=0; i<3; ++i){
    for (int j=0; j<3; ++j){
      px[i * 3 + j] = j * width / 2;  
      py[i * 3 + j] = height - i * height / 2;  
    }
  }

  for (const char *c = text; *c; ++c){
    switch (*c){
      case '0':
        draw_glyph(dl, x, y, px, py, char0, slew);
        break;
      case '1':
        draw_glyph(dl, x, y, px, py, char1, slew);
        break;
      case '2':
        draw_glyph(dl, x, y, px, py, char2, slew);
        break;
      case '3':
        draw_glyph(dl, x, y, px, py, char3, slew);
        break;
      case '4':
        draw_glyph(dl, x, y, px, py, char4, slew);
        break;
      case '5':
        draw_glyph(dl, x, y, px, py, char5, slew);
        break;
      case '6':
        draw_glyph(dl, x, y, px, py, char6, slew);
        break;
      case '7':
        draw_glyph(dl, x, y, px, py, char7, slew);
        break;
      case '8':
        draw_glyph(dl, x, y, px, py, char8, slew);
        break;
      case '9':
        draw_glyph(dl, x, y, px, py, char9, slew);
        break;
      case 'F':
        draw_glyph(dl, x, y, px, py, charF, slew);
        break;
      case 'P':
        draw_glyph(dl, x, y, px, py, charP, slew);
        break;
      case 'S':
        draw_glyph(dl, x, y, px, py, charS, slew);
        break;
      case '.':
        draw_glyph(dl, x, y, px, py, charPeriod, slew);
        break;
      case ' ':
        x += spacing;
        break;
    }
    x += spacing;
  }
}
