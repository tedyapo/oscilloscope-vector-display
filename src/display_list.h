/*
 * display_list.h
 */
#ifndef DISPLAY_LIST_H_INCLUDED_
#define DISPLAY_LIST_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>

#define WARNING(...) {fprintf(stderr, "Error (line %d, file %s): ",\
                              __LINE__, __FILE__);                 \
    fprintf(stderr, __VA_ARGS__);                                  \
    fprintf(stderr, "\n");};
    
#define ERROR(...) {fprintf(stderr, "Error (line %d, file %s): ",\
                            __LINE__, __FILE__);                 \
    fprintf(stderr, __VA_ARGS__);                                \
    fprintf(stderr, "\n");                                       \
    exit(EXIT_FAILURE);}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(a, b, c) (MIN((c), MAX((a), (b))))

typedef struct Point_
{
  float x;
  float y;
  struct Point_ *next;
} Point;

Point* NewPoint(float x, float y);
void FreePoint(Point *p);
void KillPoint(Point *p);

typedef struct Line_
{
  float slew;
  Point *points;
  struct Line_ *next;
} Line;

Line* NewLine();
void FreeLine(Line *l);
void KillLine(Line *l);

typedef struct
{
  Line *lines;
} DisplayList;

void InitDisplayList(DisplayList *dl);
void FreeDisplayList(DisplayList *dl);

void AddPoint(Line *l, Point *p);
void AddLine(DisplayList *display_list, Line *l);

#endif /* ifdef DISPLAY_LIST_H_INCLUDED_ */
