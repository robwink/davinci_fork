#ifndef _UNILABLOG_H
#define _UNILABLOG_H
void mkLogLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
   XSegment *axisImage, double *baseLValue, double *logStep, char **format);  
void mkLogLabelsValues (double baseValue, double step, char *format,
  labeledSegment *lS);
void mkLogTicsValues (double baseValue, double step,  labeledSegment *lS);
#endif
