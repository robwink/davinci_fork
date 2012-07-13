/*
 * Extract portions of a VAL orthogonal to a given mask, at every point the
 * mask is set.
 *
 * This routine will extract all portions of the VAL orthogonal to the given
 * mask.  If a VAL and mask have dimensions 5x5x5 and 5x1x1, respectively, the
 * algorithm will extract a piece of the original VAL for every mask bit that
 * is set.  Each such piece would be 1x5x5, orthogonal to the extent of the VAL
 * dimensions, essentially a copy of the plane x=maskx, where maskx is the x
 * position in the mask where a bit was set.
 *
 * If multiple bits are set these planes are automatically stacked together in
 * the order they were in, but intervening planes are not in the output.
 *
 * If a VAL and mask have dimensions 5x5x5 and 5x5x1, respectively, then each
 * extracted piece is 1x1x5.  These can be stacked together to form an output
 * object of dimension 1xNx5 or Nx1x5, where N is the number of bits set in the
 * mask.  Either is valid, but the program needs the user to specify the axis
 * to grow along.  This axis will always be one for which the mask has a
 * dimension greater than one.
 *
 * When the dimensionality of the VAL and mask are such that the program does
 * not automatically know how to combine the output, the user must specify an
 * axis for the data to "grow" along.  This axis must be one the mask spans.
 *
 * A VAL and a mask have a certain number of dimensions greater than one.  To
 * use this program at least one of those dimensions must be greater than one.
 * All non-one dimensions in the mask must agree with the corresponding
 * dimension in the VAL.  The VAL/mask combinations are listed below, with
 * example sizes in the last three columns.  The N used in the outputs is the
 * number of mask bits set.
 *
 * VAL  Mask  Pieces  VAL    Mask   Output
 * 1D   1D    points  5x1x1  5x1x1  Nx1x1 (axis="x")
 * 2D   2D    points  5x5x1  5x5x1  Nx1x1 (axis="x")
 * 3D   3D    points  5x5x5  5x5x5  Nx1x1 (axis="x")
 * 2D   1D    lines   5x5x1  1x5x1  5xNx1 (axis="y")
 * 3D   2D    lines   5x5x5  5x5x1  Nx1x5 (axis="x")
 * 3D   1D    planes  5x5x5  1x1x5  5x5xN (axis does not have to be specified)
 * 1D   2D    n/a     n/a    n/a    n/a
 * 1D   3D    n/a     n/a    n/a    n/a
 * 2D   3D    n/a     n/a    n/a    n/a
 *
 */

#include "parser.h"
#include "func.h"

/* definitions for grow direction */
#define AXIS_NONE -1
#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

/* Prototypes */

/* Functions */

Var *
ff_extract (vfuncptr func, Var * arg)
{
	Var *val = NULL, *mask = NULL, *axisvar = NULL, *result = NULL;
	size_t vx, vy, vz, mx, my, mz, rx, ry, rz;
	int axis = AXIS_NONE;
	size_t i,j,k, count, dims, length, size;
	size_t in, out, outstep;
	void *data;
	unsigned char *inbase, *outbase;
	size_t maskIdx, p,q,r, a,b,c;
	int minx, maxx, miny, maxy, minz, maxz;

	Alist alist[4];
	alist[0] = make_alist( "object", ID_VAL, NULL, &val);
	alist[1] = make_alist( "mask", ID_VAL, NULL, &mask);
	alist[2] = make_alist( "axis", ID_UNK, NULL, &axisvar);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (val == NULL) {
		parse_error("Object is null");
		return(NULL);
	}

	if (mask == NULL) {
		parse_error("Mask is null");
		return (NULL);
	}
	
	if (axisvar != NULL) {
		if (V_TYPE(axisvar) == ID_STRING && V_STRING(axisvar) != NULL) {
			if (0 == strcasecmp (V_STRING(axisvar), "x"))
				axis = AXIS_X;
			else if (0 == strcasecmp (V_STRING(axisvar), "y"))
				axis = AXIS_Y;
			else if (0 == strcasecmp (V_STRING(axisvar), "z"))
				axis = AXIS_Z;
			else {
				parse_error ("Axis must be a string equal to 'x', 'y', or 'z'");
				return NULL;
			}
		}
	}

	vx = GetX (val);
	vy = GetY (val);
	vz = GetZ (val);
	mx = GetX (mask);
	my = GetY (mask);
	mz = GetZ (mask);

	if (vx < 2 && vy < 2 && vz < 2) {
		parse_error ("Object must be at least 2 in one or more dimensions");
		return (NULL);
	}

	if (mx < 2 && my < 2 && mz < 2) {
		parse_error ("Mask must be at least 2 in one or more dimensions");
		return (NULL);
	}

	if ((mx > 1 && mx != vx)
	||  (my > 1 && my != vy)
	||  (mz > 1 && mz != vz)) {
		parse_error ("Mask dimensions greater than one must equal corresponding object dimension");
		return (NULL);
	}

	/* dimension parallel to mask will be equal to 'count' */
	count = 0;
	for (i=0; i<V_DSIZE(mask); i++)
		if (extract_int(mask, i) != 0)
			count ++;

	/* compute size of return object */

	/* user must specify which of the mask's axes (greater than one) the data
	   will be grown along (when there is only one such axis, the axis parameter
	   may be omitted, but if included it must be correct */

	dims = 0;
	if (mx > 1) dims ++;
	if (my > 1) dims ++;
	if (mz > 1) dims ++;

	switch (dims) {
	case 1:
		if (axis != AXIS_NONE) {
			if ((mx > 1 && axis != AXIS_X)
			||  (my > 1 && axis != AXIS_Y)
			||  (mz > 1 && axis != AXIS_Z)) {
				parse_error ("Axis must be 'x', 'y', or 'z', and correspond to a mask dimension > 1");
				return (NULL);
			}

		}

		if (mx > 1) axis = AXIS_X;
		if (my > 1) axis = AXIS_Y;
		if (mz > 1) axis = AXIS_Z;

		rx = vx;
		ry = vy;
		rz = vz;
		if (mx > 1) {
			rx = count;
		} else if (my > 1) {
			ry = count;
		} else { /* mz > 1 */
			rz = count;
		}
		break;
	case 2:
	case 3:
		if (axis == AXIS_NONE) {
			parse_error ("Axis parameter required to specify stacking direction");
			return (NULL);
		} else {
			if (mx > 1 && axis == AXIS_X) {
				rx = count;
				if (my > 1) {
					/* mask in x,y, stack in x */
					ry = 1;
					rz = dims < 3 ? vz : 1;
				} else {
					/* mask in x,z, stack in x */
					ry = dims < 3 ? vy : 1;
					rz = 1;
				}
			} else if (my > 1 && axis == AXIS_Y) {
				ry = count;
				if (mx > 1) {
					/* mask in x,y, stack in y */
					rx = 1;
					rz = dims < 3 ? vz : 1;
				} else {
					/* mask in y,z, stack in y */
					rx = dims < 3 ? vx : 1;
					rz = 1;
				}
			} else if (mz > 1 && axis == AXIS_Z) {
				rz = count;
				if (mx > 1) {
					/* mask in x,z, stack in z */
					rx = 1;
					ry = dims < 3 ? vy : 1;
				} else {
					/* mask in y,z, stack in z */
					rx = dims < 3 ? vx : 1;
					ry = 1;
				}
			} else {
				parse_error ("Axis must be 'x', 'y', or 'z', and correspond to a mask dimension > 1");
				return (NULL);
			}
		}
		break;
	default:
		parse_error ("Unable to determine dimensionality of output object.");
		return (NULL);
	}

	/* allocate data and VAL for the result */
	length = rx * ry * rz;
	size = NBYTES(V_FORMAT(val));
	data = calloc (length, size);
	result = newVal (V_ORG(val), rx, ry, rz, V_FORMAT(val), data);

	inbase = (unsigned char *) V_DATA (val);
	outbase = (unsigned char *) data;

	/* loop over the mask, copying as necessary when a set bit is found */
	outstep = 0;
	for (i = 0; i < mx; i++) {
		for (j = 0; j < my; j++) {
			for (k = 0; k < mz; k++) {
				maskIdx = cpos (i,j,k, mask);

				if (0 != extract_int (mask, maskIdx)) {
					minx = (mx == vx ? i : 0);
					maxx = (mx == vx ? i : vx-1);
					miny = (my == vy ? j : 0);
					maxy = (my == vy ? j : vy-1);
					minz = (mz == vz ? k : 0);
					maxz = (mz == vz ? k : vz-1);

					for (p = minx; p <= maxx; p ++) {
						for (q = miny; q <= maxy; q ++) {
							for (r = minz; r <= maxz; r ++) {
								in = cpos (p,q,r, val);

								a = (rx == vx ? p : 0);
								b = (ry == vy ? q : 0);
								c = (rz == vz ? r : 0);
								switch (axis) {
									case AXIS_X: a = outstep; break;
									case AXIS_Y: b = outstep; break;
									case AXIS_Z: c = outstep; break;
								}
								out = cpos (a,b,c, result);

								memcpy (outbase + out*size, inbase + in*size, size);
							}
						}
					}

					outstep ++;
				}
			}
		}
	}

	return result;
}
