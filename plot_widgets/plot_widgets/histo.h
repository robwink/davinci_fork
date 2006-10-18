/*******************************************************************************
*									       *
* histo.h -- User include file for Histoscope routines			       *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
*									       *
* Created 3/9/92							       *
*									       *
*******************************************************************************/
#ifndef HISTO_H
#define HISTO_H
typedef enum _hsScaleType {HS_LINEAR, HS_LOG} hsScaleType;

typedef struct _hs1DHist {
    int id;			/* identifies histogram to users	     */
    int nBins;			/* number of bins in histogram		     */
    double min;			/* low edge of first bin		     */
    double max;			/* high edge of last bin		     */
    hsScaleType xScaleType;	/* how data is binned: linear, log, etc..    */
    float xScaleBase;		/* base for log scaling, i.e. 10, e, 2, etc  */
    char *title;		/* title for histogram window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    char *xLabel;		/* label for histogram x axis		     */
    char *yLabel;		/* label for histogram y (vertical) axis     */
    double *bins;		/* the histogram data			     */
} hs1DHist;

typedef struct _hs2DHist {
    int id;			/* identifies histogram to users	     */
    int nXBins;			/* number of bins along x axis of histogram  */
    int nYBins;			/* number of bins along y axis of histogram  */
    double xMin;		/* low edge of first x axis bin		     */
    double xMax;		/* high edge of last x axis bin		     */
    double yMin;		/* low edge of first y axis bin		     */
    double yMax;		/* high edge of last x axis bin		     */
    hsScaleType xScaleType;	/* how x data is binned: linear, log, etc..  */
    float xScaleBase;		/* base for x axis log scaling, i.e. 10, e   */
    hsScaleType yScaleType;	/* how y data is binned: linear, log, etc..  */
    float yScaleBase;		/* base for y axis log scaling, i.e. 10, e   */
    char *title;		/* title for histogram window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    char *xLabel;		/* label for histogram x axis		     */
    char *yLabel;		/* label for histogram y axis		     */
    char *zLabel;		/* label for histogram z (vertical) axis     */
    float *bins;		/* the histogram data (a 2 dim. array)	     */
} hs2DHist;

typedef struct _hsNTuple {
    int id;			/* identifies ntuple to users		     */
    int n;			/* number of elements in the ntuple	     */
    int nVariables;		/* # of variables in each element of ntuple  */
    char *title;		/* title for the ntuple window		     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    char **names;		/* array containing name of each variable    */
    double *data;		/* points to the 2 dim. array of ntuple data */
} hsNTuple;

typedef struct _hsIndicator {
    int id;			/* identifies indicator to users	     */
    char *title;		/* title for the indicator window	     */
    char *category;		/* together, category strings establish a hier-
    				   archy for displaying the hist/ntuple list */
    double value;		/* indicator value			     */
} hsIndicator;
#endif
