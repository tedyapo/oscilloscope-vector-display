/*
 * display_list.h
 */
#include <stdlib.h>
#include "display_list.h"

/* free pool of allocated points */
Point* free_points = NULL;

/* get a point from the free pool, or allocate a new one */
Point* NewPoint(float x, float y)
{
  if (free_points){
    Point* temp = free_points;
    free_points = free_points->next;
    temp->x = x;
    temp->y = y;
    temp->next = NULL;
    return temp;
  } else {
    Point* temp = (Point*)malloc(sizeof(Point));
    if (NULL == temp){
      ERROR("malloc() failed");
    }
    temp->x = x;
    temp->y = y;
    temp->next = NULL;
    return temp;
  }
}

/* return point to free pool */
void FreePoint(Point *p)
{
  if (!p) return;
  FreePoint(p->next);
  p->next = free_points;
  free_points = p;
}

/* free memory associated with point */
void KillPoint(Point *p)
{
  if (!p) return;
  KillPoint(p->next);
  free(p);
}

/* free pool of allocated lines */
Line* free_lines = NULL;

/* get a line from the free pool, or allocate a new one */
Line* NewLine()
{
  if (free_lines){
    Line* temp = free_lines;
    free_lines = free_lines->next;
    temp->slew = -1;
    temp->points = NULL;
    temp->next = NULL;
    return temp;
  } else {
    Line* temp = (Line*)malloc(sizeof(Line));
    if (NULL == temp){
      ERROR("malloc() failed");
    }
    temp->slew = -1;
    temp->points = NULL;
    temp->next = NULL;
    return temp;
  }
}

/* return line to free pool */
void FreeLine(Line *l)
{
  if (!l) return;
  FreeLine(l->next);
  FreePoint(l->points);
  l->next = free_lines;
  free_lines = l;
}

/* free memory associated with line */
void KillLine(Line *l)
{
  if (!l) return;
  KillLine(l->next);
  KillPoint(l->points);
  free(l);
}

/* initialize a display list */
void InitDisplayList(DisplayList *dl)
{
  dl->lines = NULL;
}

/* release a display list */
void FreeDisplayList(DisplayList *dl)
{
  FreeLine(dl->lines);
  dl->lines = NULL;
}

/* adds a single point to the *front* of the line */
void AddPoint(Line *l, Point *p)
{
  p->next = l->points;
  l->points = p;
}

/* add a line to the display list */
void AddLine(DisplayList *display_list, Line *l)
{
  l->next = display_list->lines;
  display_list->lines = l;
}

