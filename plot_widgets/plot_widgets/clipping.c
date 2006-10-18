static void clipRectangle(float cX1, float cY1, float cX2, float cY2,
	float *x1, float *y1, float *x2, float *y2)
{
    clipAbove(cY1, x1, y1, x2, y2);
    clipBelow(cY2, x1, y1, x2, y2);
    clipLeft(cX1, x1, y1, x2, y2);
    clipRight(cX1, x1, y1, x2, y2);
}

static void clipAbove(float clipY, float *x1, float *y1, float *x2, float *y2)
{
    if (*y1 < clipY && *y2 < clipY) {
    	return;
    else if (*y1 > clipY && *y2 > clipY) {
    	*y1 = *y2 = clipY;
    	return;
    } else if (*y1 > clipY && *y2 < clipY) {
    	*x1 += clipY * ((*x2 - *x1) / (*y2 - *y1));
    	*y1 = clipY;
    } else if (*y1 < clipY && *y2 > clipY) {
    	*x2 += clipY * ((*x2 - *x1) / (*y2 - *y1));
    	*y2 = clipY;
    }
}

static void clipBelow(int clipY, int *x1, int *y1, int *x2, int *y2)
{
    if (*y1 > clipY && *y2 > clipY) {
    	return;
    else if (*y1 < clipY && *y2 < clipY) {
    	*y1 = *y2 = clipY;
    	return;
    } else if (*y1 < clipY && *y2 > clipY) {
    	*x1 += clipY * ((*x2 - *x1) / (*y2 - *y1));
    	*y1 = clipY;
    } else if (*y1 > clipY && *y2 < clipY) {
    	*x2 += clipY * ((*x2 - *x1) / (*y2 - *y1));
    	*y2 = clipY;
    }
}
