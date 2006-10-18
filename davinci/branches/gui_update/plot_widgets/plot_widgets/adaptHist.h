#ifndef _ADAPTHIST_H
#define _ADAPTHIST_H

#define _X 0
#define _Y 1

/* structure to represent (x,y) pairs 					*/
typedef struct _t
{
    float c[2] ;
} t;
aHistStruct *Bin2DAdaptHist(t* dataArray, int nPairs,  int nLimit, int strategy);
aHistStruct *Bin2DAdaptHistInDom(t* dataArray, int nPairs, double xMin,
	double xMax, double yMin, double yMax, int nLimit, int strategy);
void Bin1DAdaptHist(float* data, int nData, int nLimit, 
  int strategy, float **bins, float **binEdges, int* nBins);
void Bin1DAdaptHistInDom(float* data, int nData,  double xMin,
	double xMax, int nLimit, int strategy, float **bins, float **binEdges, 
	int* nBins);
#endif
