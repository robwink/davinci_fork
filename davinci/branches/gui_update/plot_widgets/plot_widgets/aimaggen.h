#ifndef _AIMAGGEN_H
#define _AIMAGGEN_H
/* ratio of bin margin in a bin cell to the size of bin cell */
#define MARG_FACTOR 0.125

void makeAHistImage(Hist2DWidget w);
int levelsNumber(aHistNode *aHist);
#endif
