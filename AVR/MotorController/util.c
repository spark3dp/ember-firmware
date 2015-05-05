#include <string.h> // memcpy

#include "tinyg.h"
#include "util.h"

inline float min3(float x1, float x2, float x3)
{
  float min = x1;
  if (x2 < min) { min = x2;} 
  if (x3 < min) { return (x3);} 
  return (min);
}

inline float min4(float x1, float x2, float x3, float x4)
{
  float min = x1;
  if (x2 < min) { min = x2;} 
  if (x3 < min) { min = x3;} 
  if (x4 < min) { return (x4);}
  return (min);
}

void copy_axis_vector(float dst[], const float src[]) 
{
  memcpy(dst, src, sizeof(float)*AXES);
}
