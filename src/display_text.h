#ifndef DISPLAY_TEXT_H_INCLUDED_
#define  DISPLAY_TEXT_H_INCLUDED_
#include "display.h"

void render_text(DisplayList *dl, const char* text, float x, float y,
                 float height, float width, float spacing, float slew);

#endif /* #ifndef DISPLAY_TEXT_H_INCLUDED_ */
