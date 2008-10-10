#include "parser.h"
/**
 ** Position conversion routines.  Convert offset i, from s1 to s2, 
 ** using the specified ordering.
 **
 **
 **	BIL:	XZY
 **	BSQ:	XYZ
 **	BIP:	ZXY
 **/


#define RPOS(x1, y1, z1, x2, y2, z2) \
	ab = V_SIZE(s1)[0] * V_SIZE(s1)[1]; \
	z1 = i/ab; \
	x1 = i % V_SIZE(s1)[0]; \
	y1 = (i - z1*ab)/V_SIZE(s1)[0]; \
	x2 = x2 % V_SIZE(s2)[0]; \
	y2 = y2 % V_SIZE(s2)[1]; \
	z2 = z2 % V_SIZE(s2)[2]; \
	i = x2 + V_SIZE(s2)[0]*(y2 + V_SIZE(s2)[1]*z2);\
	return(i);

int
__BIL2BIP(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(x,z,y, z,x,y);
}

int
__BIL2BSQ(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(x,z,y, x,y,z);
}

int
__BSQ2BIL(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(x,y,z, x,z,y);
}

int
__BSQ2BIP(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(x,y,z, z,x,y);
}
int
__BIP2BIL(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(z,x,y, x,z,y);
}

int
__BIP2BSQ(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(z,x,y, x,y,z);
}

/**
 ** These do essentially nothing, for conversion from and to the same type
 **/

int
__BIP2BIP(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(z,x,y,z,x,y);
}
int
__BSQ2BSQ(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

 	RPOS(x,y,z,x,y,z); 
}
int
__BIL2BIL(Var *s1, Var *s2, int i)
{
	int x,y,z;
	int ab;

	RPOS(x,z,y,x,z,y);
}
