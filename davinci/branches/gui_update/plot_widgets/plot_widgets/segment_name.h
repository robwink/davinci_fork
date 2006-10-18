#ifndef SEGMENT_NAME_H
#define SEGMENT_NAME_H

void placeName (labeledSegment *lS, XFontStruct *fs, char* name, 
    XSegment *axisImage, int clipX0, int clipX1,
    XSegment *tics, int nTics, labelsToDraw2 *ticMarks );

#endif
