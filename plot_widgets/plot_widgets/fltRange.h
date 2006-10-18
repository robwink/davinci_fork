#ifndef FLTRANGE_H
#define FLTRANGE_H
void setFRangeLimits(float *min, float *max);
Boolean setFRangeLogScale (float *visibleStart , float *visibleEnd, 
  float min, float max);
void adjustFRange(float *start, float *end,float min, float max, 
    Boolean LogScale);
void pullFRangeStart(float *start, float *end, double d, float min, float max, 
    Boolean LogScale);
void pullFRangeEnd(float *start, float *end, double d, float min, float max, 
    Boolean LogScale);
void moveFRange(float *start, float *end, double d, float min, float max, 
    Boolean LogScale);
#endif
