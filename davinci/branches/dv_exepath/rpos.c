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

size_t
__BIL2BIP(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(x,z,y, z,x,y);
}

size_t
__BIL2BSQ(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(x,z,y, x,y,z);
}

size_t
__BSQ2BIL(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(x,y,z, x,z,y);
}

size_t
__BSQ2BIP(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(x,y,z, z,x,y);
}
size_t
__BIP2BIL(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(z,x,y, x,z,y);
}

size_t
__BIP2BSQ(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(z,x,y, x,y,z);
}

/**
 ** These do essentially nothing, for conversion from and to the same type
 **/

size_t
__BIP2BIP(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(z,x,y,z,x,y);
}
size_t
__BSQ2BSQ(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

 	RPOS(x,y,z,x,y,z); 
}
size_t
__BIL2BIL(Var *s1, Var *s2, size_t i)
{
	size_t x,y,z;
	size_t ab;

	RPOS(x,z,y,x,z,y);
}
