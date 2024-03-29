#include "parser.h"
#include <math.h>

#define XAXIS 1
#define YAXIS 2
#define ZAXIS 4

/*
** sum, avg and stddev
**
** If the user passes the 'both' option, you get a struct with both a & s
*/

/*
** this is a one pass algorithm to find sum, avg and stddev.
*/
Var* ff_avg(vfuncptr func, Var* arg)
{
	Var *obj  = NULL, *v;
	char* ptr = NULL;
	int axis  = 0;
	size_t i, j, dsize;
	int in[3], out[3];
	double *sum, *sum2;
	size_t* count;
	const char* options[] = {"x",  "y",   "z",   "xy",  "yx",  "xz",  "zx",  "yz",
	                         "zy", "xyz", "xzy", "yxz", "yzx", "zxy", "zyx", NULL};
	size_t dsize2;
	double x;
	Var* both = NULL;
	Var *avg = NULL, *stddev = NULL;
	int f_avg = 0, f_stddev = 0, f_sum = 0;
	Var* ignore = NULL;
	double ignore_val;

	Alist alist[5];
	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("axis", ID_ENUM, options, &ptr);
	alist[2]      = make_alist("both", ID_VAL, NULL, &both);
	alist[3]      = make_alist("ignore", ID_VAL, NULL, &ignore);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (ptr == NULL) {
		axis = XAXIS | YAXIS | ZAXIS; /* all of them */
	} else {
		if (strchr(ptr, 'x') || strchr(ptr, 'X')) axis |= XAXIS;
		if (strchr(ptr, 'y') || strchr(ptr, 'Y')) axis |= YAXIS;
		if (strchr(ptr, 'z') || strchr(ptr, 'Z')) axis |= ZAXIS;
	}

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return (NULL);
	}

	dsize = V_DSIZE(obj);
	for (i = 0; i < 3; i++) {
		in[i] = out[i] = V_SIZE(obj)[i];
	}

	if (axis & XAXIS) out[orders[V_ORG(obj)][0]] = 1;
	if (axis & YAXIS) out[orders[V_ORG(obj)][1]] = 1;
	if (axis & ZAXIS) out[orders[V_ORG(obj)][2]] = 1;

	/*
	** Decide what operations to perform and setup output variables
	** v is a generic variable with the right output sizes.
	*/
	if (!strcmp(func->name, "sum")) {
		f_sum = 1;
		v     = newVal(V_ORG(obj), out[0], out[1], out[2], DV_DOUBLE, NULL);
	}

	if (!strncmp(func->name, "avg", 3) || both) {
		f_avg = 1;
		v = avg = newVal(V_ORG(obj), out[0], out[1], out[2], DV_DOUBLE, NULL);
	}

	if (!strcmp(func->name, "stddev") || both) {
		f_stddev = 1;
		v = stddev = newVal(V_ORG(obj), out[0], out[1], out[2], DV_DOUBLE, NULL);
	}

	sum                = (double*)calloc(V_DSIZE(v), sizeof(double));
	count              = (size_t*)calloc(V_DSIZE(v), sizeof(size_t));
	if (f_stddev) sum2 = (double*)calloc(V_DSIZE(v), sizeof(double));

	if (ignore) ignore_val = extract_double(ignore, 0);

	/*
	** The if avoids the x^2 computation on the case we don't need it.
	*/
	if (f_stddev) {
		/*
		** Sum the data
		*/
		for (i = 0; i < dsize; i++) {
			x = extract_float(obj, i);
			if (ignore && x == ignore_val) continue;
			j = rpos(i, obj, v);
			sum[j] += x;
			sum2[j] += x * x;
			count[j]++;
		}
	} else {
		for (i = 0; i < dsize; i++) {
			x = extract_float(obj, i);
			if (ignore && x == ignore_val) continue;
			j = rpos(i, obj, v);
			sum[j] += x;
			count[j]++;
		}
	}

	dsize2 = V_DSIZE(v);
	// n = (double)dsize / (double)dsize2;

	/*
	** Perform required computations with sums
	*/
	if (f_stddev) {
		for (i = 0; i < dsize2; i++) {
			if (count[i] > 1) {
				sum2[i] = sqrt((sum2[i] - (sum[i] * sum[i] / count[i])) / (count[i] - 1));
			} else {
				sum2[i] = 0;
			}
		}
		V_DATA(stddev) = sum2;
	}

	if (f_avg) {
		// n = (double)dsize2 / (double)dsize;
		for (i = 0; i < dsize2; i++) {
			if (count[i] > 0) {
				sum[i] = sum[i] / count[i];
			} else {
				sum[i] = 0;
			}
		}
		V_DATA(avg) = sum;
	}

	free(count);

	if (f_sum) {
		V_DATA(v) = sum;
		return (v);
	} else if (both) {
		both = new_struct(0);
		add_struct(both, "avg", avg);
		add_struct(both, "stddev", stddev);
		return (both);
	} else if (f_stddev) {
		return (stddev);
	} else if (f_avg) {
		return (avg);
	} else {
		parse_error("ff_avg: Shouldn't ever get here.\n");
		return (NULL);
	}
}

Var* fb_min(Var* obj, int axis, int direction, Var* ignore);

Var* ff_min(vfuncptr func, Var* arg)
{
	Var* obj              = NULL;
	char* ptr             = NULL;
	int axis              = 0;
	const char* options[] = {"x",  "y",   "z",   "xy",  "yx",  "xz",  "zx",  "yz",
	                         "zy", "xyz", "xzy", "yxz", "yzx", "zxy", "zyx", NULL};
	//float ignore = FLT_MIN;
	Var* ignore = NULL;;

	Alist alist[4];
	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("axis", ID_ENUM, options, &ptr);
	alist[2]      = make_alist("ignore", ID_UNK, NULL, &ignore);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (ptr == NULL) {
		axis = XAXIS | YAXIS | ZAXIS; /* all of them */
	} else {
		if (strchr(ptr, 'x') || strchr(ptr, 'X')) axis |= XAXIS;
		if (strchr(ptr, 'y') || strchr(ptr, 'Y')) axis |= YAXIS;
		if (strchr(ptr, 'z') || strchr(ptr, 'Z')) axis |= ZAXIS;
	}

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return NULL;
	}

	if (ignore) {
		if (V_TYPE(ignore) != ID_VAL) {
			parse_error("%s: ignore must be VAL type.\n", func->name);
			return NULL;
		}
		if (V_DSIZE(ignore) != 1) {
			parse_error("%s: ignore must be a single value (ie length(ignore)==1).\n", func->name);
			return NULL;
		}
	}

	if (ignore && V_TYPE(ignore) != ID_VAL) {
		parse_error("%s: ignore must be VAL type\n", func->name);
		return NULL;
	}

	return fb_min(obj, axis, (!strcmp(func->name, "min")) ? 0 : 1, ignore);
}

/*
** Do min/max.
**     axis:1, x axis
**     axis:2, y axis
**     axis:3, z axis
**     direction:  0 = min, 1 = max
*/
Var* fb_min(Var* obj, int axis, int direction, Var* ignore)
{
	Var* v;
	size_t dsize, dsize2, i, j;
	size_t in[3], out[3];

	dsize = V_DSIZE(obj);
	for (i = 0; i < 3; i++) {
		in[i] = out[i] = V_SIZE(obj)[i];
	}

	if (axis & XAXIS) out[orders[V_ORG(obj)][0]] = 1;
	if (axis & YAXIS) out[orders[V_ORG(obj)][1]] = 1;
	if (axis & ZAXIS) out[orders[V_ORG(obj)][2]] = 1;

	v      = newVal(V_ORG(obj), out[0], out[1], out[2], V_FORMAT(obj), NULL);
	dsize2 = V_DSIZE(v);

	// NOTE(rswinkle): Someone should decide what the default ignore values
	// should be for the different types.  I used the min values for each type
	// except for float and double because the old "always return float" version
	// used FLT_MIN which is actually the smallest positive value for float.
	// not the lowest value which would be -FLT_MAX.
	switch (V_FORMAT(obj)) {
	case DV_UINT8: {
		u8 tmp, ign = (ignore) ? extract_u32(ignore, 0) : 0;
		u8* u8data = V_DATA(v) = malloc(dsize2);
		for (i=0; i<dsize2; ++i) { u8data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_u32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (u8data[j] == ign || tmp < u8data[j]) u8data[j] = tmp;
			} else {
				if (u8data[j] == ign || tmp > u8data[j]) u8data[j] = tmp;
			}
		}
	} break;
	case DV_UINT16: {
		u16 tmp, ign = (ignore) ? extract_u32(ignore, 0) : 0;
		u16* u16data = V_DATA(v) = malloc(dsize2*sizeof(u16));
		for (i=0; i<dsize2; ++i) { u16data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_u32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (u16data[j] == ign || tmp < u16data[j]) u16data[j] = tmp;
			} else {
				if (u16data[j] == ign || tmp > u16data[j]) u16data[j] = tmp;
			}
		}
	} break;
	case DV_UINT32: {
		u32 tmp, ign = (ignore) ? extract_u32(ignore, 0) : 0;
		u32* u32data = V_DATA(v) = malloc(dsize2*sizeof(u32));
		for (i=0; i<dsize2; ++i) { u32data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_u32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (u32data[j] == ign || tmp < u32data[j]) u32data[j] = tmp;
			} else {
				if (u32data[j] == ign || tmp > u32data[j]) u32data[j] = tmp;
			}
		}
	} break;
	case DV_UINT64: {
		u64 tmp, ign = (ignore) ? extract_u64(ignore, 0) : 0;
		u64* u64data = V_DATA(v) = malloc(dsize2*sizeof(u64));
		for (i=0; i<dsize2; ++i) { u64data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_u64(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (u64data[j] == ign || tmp < u64data[j]) u64data[j] = tmp;
			} else {
				if (u64data[j] == ign || tmp > u64data[j]) u64data[j] = tmp;
			}
		}
	} break;

	case DV_INT8: {
		i8 tmp, ign = (ignore) ? extract_i32(ignore, 0) : INT8_MIN;
		i8* i8data = V_DATA(v) = malloc(dsize2);
		for (i=0; i<dsize2; ++i) { i8data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_i32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (i8data[j] == ign || tmp < i8data[j]) i8data[j] = tmp;
			} else {
				if (i8data[j] == ign || tmp > i8data[j]) i8data[j] = tmp;
			}
		}
	} break;
	case DV_INT16: {
		i16 tmp, ign = (ignore) ? extract_i32(ignore, 0) : INT16_MIN;
		i16* i16data = V_DATA(v) = malloc(dsize2*sizeof(i16));
		for (i=0; i<dsize2; ++i) { i16data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_i32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (i16data[j] == ign || tmp < i16data[j]) i16data[j] = tmp;
			} else {
				if (i16data[j] == ign || tmp > i16data[j]) i16data[j] = tmp;
			}
		}
	} break;
	case DV_INT32: {
		i32 tmp, ign = (ignore) ? extract_i32(ignore, 0) : INT32_MIN;
		i32* i32data = V_DATA(v) = malloc(dsize2*sizeof(i32));
		for (i=0; i<dsize2; ++i) { i32data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_i32(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (i32data[j] == ign || tmp < i32data[j]) i32data[j] = tmp;
			} else {
				if (i32data[j] == ign || tmp > i32data[j]) i32data[j] = tmp;
			}
		}
	} break;
	case DV_INT64: {
		i64 tmp, ign = (ignore) ? extract_i64(ignore, 0) : INT64_MIN;
		i64* i64data = V_DATA(v) = malloc(dsize2*sizeof(i64));
		for (i=0; i<dsize2; ++i) { i64data[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_i64(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (i64data[j] == ign || tmp < i64data[j]) i64data[j] = tmp;
			} else {
				if (i64data[j] == ign || tmp > i64data[j]) i64data[j] = tmp;
			}
		}
	} break;
	
	case DV_FLOAT: {
		float tmp, ign = (ignore) ? extract_float(ignore, 0) : FLT_MIN;
		float* fdata = V_DATA(v) = malloc(dsize2*sizeof(float));
		for (i=0; i<dsize2; ++i) { fdata[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_float(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (fdata[j] == ign || tmp < fdata[j]) fdata[j] = tmp;
			} else {
				if (fdata[j] == ign || tmp > fdata[j]) fdata[j] = tmp;
			}
		}
	} break;
	case DV_DOUBLE: {
		double tmp, ign = (ignore) ? extract_double(ignore, 0) : DBL_MIN;
		double* ddata = V_DATA(v) = malloc(dsize2*sizeof(double));
		for (i=0; i<dsize2; ++i) { ddata[i] = ign; }
		for (i=0; i<dsize; ++i) {
			j = rpos(i, obj, v);
			tmp = extract_double(obj, i);
			if (tmp == ign) continue;

			if (direction == 0) {
				if (ddata[j] == ign || tmp < ddata[j]) ddata[j] = tmp;
			} else {
				if (ddata[j] == ign || tmp > ddata[j]) ddata[j] = tmp;
			}
		}
	} break;

	}

	return v;
}

Var* ff_findmin(vfuncptr func, Var* arg)
{
	Var* obj = NULL;
	size_t dsize, i;
	float x, val;
	int do_min = 0, do_max = 0;
	size_t pos;
	float ignore = FLT_MIN;

	Alist alist[3];
	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("ignore", DV_FLOAT, NULL, &ignore);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return (NULL);
	}

	if (!strcmp(func->name, "minchan")) do_min = 1;
	if (!strcmp(func->name, "maxchan")) do_max = 1;

	dsize = V_DSIZE(obj);

	pos = -1;
	val = ignore;
	for (i = 0; i < dsize; i++) {
		x = extract_float(obj, i);
		if (x == ignore) continue;
		if (do_min && (x < val || val == ignore)) {
			val = x;
			pos = i;
		}
		if (do_max && (x > val || val == ignore)) {
			val = x;
			pos = i;
		}
	}

	if (((int)(pos + 1)) != (pos + 1)) {
		parse_error("%s: Overflow pos=%ld cannot be represented as an int.\n", func->name, (pos + 1));
	}

	return (newInt(pos + 1));
}

Var* ff_convolve(vfuncptr func, Var* arg)
{
	Var *obj = NULL, *kernel = NULL;
	int norm     = 1;
	float ignore = FLT_MIN;

	Alist alist[5];
	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("kernel", ID_VAL, NULL, &kernel);
	alist[2]      = make_alist("normalize", DV_INT32, NULL, &norm);
	alist[3]      = make_alist("ignore", DV_FLOAT, NULL, &ignore);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return (NULL);
	}
	if (kernel == NULL) {
		parse_error("%s: No kernel specified\n", func->name);
		return (NULL);
	}
	return (do_convolve(obj, kernel, norm, ignore));
}

Var* do_convolve(Var* obj, Var* kernel, int norm, float ignore)
{
	float *data, v1, v2;
	int* wt;

	size_t dsize, i, j, k;
	int a, b, c;
	int x_pos, y_pos, z_pos;
	int obj_x, obj_y, obj_z;
	int kernel_x_center, kernel_x;
	int kernel_y_center, kernel_y;
	int kernel_z_center, kernel_z;
	int x, y, z;
	obj_x = GetSamples(V_SIZE(obj), V_ORG(obj));
	obj_y = GetLines(V_SIZE(obj), V_ORG(obj));
	obj_z = GetBands(V_SIZE(obj), V_ORG(obj));

	kernel_x = GetSamples(V_SIZE(kernel), V_ORG(kernel));
	kernel_y = GetLines(V_SIZE(kernel), V_ORG(kernel));
	kernel_z = GetBands(V_SIZE(kernel), V_ORG(kernel));

	kernel_x_center = kernel_x / 2;
	kernel_y_center = kernel_y / 2;
	kernel_z_center = kernel_z / 2;

	dsize = V_DSIZE(obj);
	if ((data = (float*)calloc(dsize, sizeof(float))) == NULL) {
		parse_error("Unable to allocate memory");
		return (NULL);
	}
	if ((wt = (int*)calloc(dsize, sizeof(int))) == NULL) {
		parse_error("Unable to allocate memory");
		return (NULL);
	}

	for (i = 0; i < dsize; i++) {
		xpos(i, obj, &x, &y, &z); /* compute current x,y,z */
		for (a = 0; a < kernel_x; a++) {
			x_pos = x + a - kernel_x_center;
			if (x_pos < 0 || x_pos >= obj_x) continue;
			for (b = 0; b < kernel_y; b++) {
				y_pos = y + b - kernel_y_center;
				if (y_pos < 0 || y_pos >= obj_y) continue;
				for (c = 0; c < kernel_z; c++) {
					z_pos = z + c - kernel_z_center;
					if (z_pos < 0 || z_pos >= obj_z) continue;

					j  = cpos(x_pos, y_pos, z_pos, obj);
					k  = cpos(a, b, c, kernel);
					v1 = extract_float(kernel, k);
					v2 = extract_float(obj, j);
					if (v1 != ignore && v2 != ignore) {
						wt[i]++;
						data[i] += v1 * v2;
					}
				}
			}
		}
		if (norm && wt[i] > 0) {
			data[i] /= (float)wt[i];
		}
	}

	return (newVal(V_ORG(obj), V_SIZE(obj)[0], V_SIZE(obj)[1], V_SIZE(obj)[2], DV_FLOAT, data));
}

Var* ff_convolve3(vfuncptr func, Var* arg)
{
	Var *obj = NULL, *kernel = NULL;
	int norm = 1;
	float *data, *cache, val;
	float *Mask, *Weight;
	int* P;
	int wt;

	size_t dsize;
	int i, j, k;
	int q, r, s;
	int Init_Cache = 1;

	int Mode[3], Ce[3], Center[3];
	int Pos[3];
	int MajC, MidC, MinC;
	int mOrd[3]; /*Size of cube in order of: 0-X 1-Y 2-Z */
	int kOrd[3]; /*Size of Mask in order of: 0-X 1-Y 2-Z */
	int T, M[3]; /*Temp Variables*/

	size_t Mem_Index;
	size_t I;

	Alist alist[4];
	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("kernel", ID_VAL, NULL, &kernel);
	alist[2]      = make_alist("normalize", DV_INT32, NULL, &norm);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return (NULL);
	}
	if (kernel == NULL) {
		parse_error("%s: No kernel specified\n", func->name);
		return (NULL);
	}

	mOrd[0] = GetSamples(V_SIZE(obj), V_ORG(obj));
	mOrd[1] = GetLines(V_SIZE(obj), V_ORG(obj));
	mOrd[2] = GetBands(V_SIZE(obj), V_ORG(obj));

	Center[0] = (kOrd[0] = GetSamples(V_SIZE(kernel), V_ORG(kernel))) / 2;
	Center[1] = (kOrd[1] = GetLines(V_SIZE(kernel), V_ORG(kernel))) / 2;
	Center[2] = (kOrd[2] = GetBands(V_SIZE(kernel), V_ORG(kernel))) / 2;

	dsize = V_DSIZE(obj);
	if ((data = (float*)calloc(dsize, sizeof(float))) == NULL) {
		parse_error("Unable to allocate memory");
		return (NULL);
	}

	if ((cache = (float*)calloc(V_DSIZE(kernel), sizeof(float))) == NULL) {
		parse_error("Unable to allocate memory for cache");
		return (NULL);
	}

	if ((Mask = (float*)calloc(V_DSIZE(kernel), sizeof(float))) == NULL) {
		parse_error("Unable to allocate memory for Mask");
		return (NULL);
	}

	if ((Weight = (float*)calloc(V_DSIZE(kernel), sizeof(int))) == NULL) {
		parse_error("Unable to allocate memory for Weights");
		return (NULL);
	}

	/*****
	***	Find Axis lengths from shortest to longest and set up Mode:
	***	Mode[0]=Shortest
	***	Mode[1]=Middle
	***	Mode[2]=Longest.
	***	We move the cache through the data block along the longest axis
	******/

	for (i = 0; i < 3; i++) {
		Mode[i] = i;
		M[i]    = mOrd[i];
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 2; j++) {
			if (M[j] > M[j + 1]) {
				T           = M[j];
				M[j]        = M[j + 1];
				M[j + 1]    = T;
				T           = Mode[j];
				Mode[j]     = Mode[j + 1];
				Mode[j + 1] = T;
			}
		}
	}

	if ((P = (int*)calloc(kOrd[Mode[2]], sizeof(int))) == NULL) {
		parse_error("Unable to allocate memory for cache");
		return (NULL);
	}

	/***********************************************************************************
	**Copy data from Kernal into Mask so that Mask has the same orientation as the Cache.
	**This is a one time access penalty to Kernel.  It also makes the math between the
	**The Cache and the Kernel easier (IMHO).
	*************************************************************************************/
	for (i = 0; i < kOrd[Mode[0]]; i++) {
		for (j = 0; j < kOrd[Mode[1]]; j++) {
			for (k = 0; k < kOrd[Mode[2]]; k++) {
				Pos[Mode[0]] = i;
				Pos[Mode[1]] = j;
				Pos[Mode[2]] = k;
				I            = cpos(Pos[0], Pos[1], Pos[2], kernel);
				Mask[k * kOrd[Mode[1]] * kOrd[Mode[0]] + j * kOrd[Mode[0]] + i] =
				    extract_float(kernel, I);
			}
		}
	}

	/*************************************************************
	**Main body of loop
	**/
	for (i = 0; i < mOrd[Mode[0]]; i++) {
		for (j = 0; j < mOrd[Mode[1]]; j++) {
			for (k = 0; k < mOrd[Mode[2]]; k++) {

				Ce[Mode[0]] = i;
				Ce[Mode[1]] = j;
				Ce[Mode[2]] = k;

				if (Init_Cache) { /*Need to fully load cache block*/
					MajC = MidC = MinC = 0;
					val                = 0;
					wt                 = 0;
					for (s = k - Center[Mode[2]]; s <= k + Center[Mode[2]]; s++) {
						for (r = j - Center[Mode[1]]; r <= j + Center[Mode[1]]; r++) {
							for (q = i - Center[Mode[0]]; q <= i + Center[Mode[0]]; q++) {
								/*Load Cache Block*/
								if (q < 0 || r < 0 ||
								    s < 0) { /* Do a wrap check here in the furture */
									cache[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    0;
									Weight[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    0;
								} else if (q >= mOrd[Mode[0]] || r >= mOrd[Mode[1]] || s >= mOrd[Mode[2]]) {
									cache[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    0;
									Weight[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    0;
								} else {
									Pos[Mode[0]] = q;
									Pos[Mode[1]] = r;
									Pos[Mode[2]] = s;
									Mem_Index    = cpos(Pos[0], Pos[1], Pos[2], obj);
									cache[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    extract_float(obj, Mem_Index);
									Weight[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] =
									    1;
									wt++;
									val +=
									    cache[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC] *
									    Mask[MajC * kOrd[Mode[1]] * kOrd[Mode[0]] + MidC * kOrd[Mode[0]] + MinC];
								}
								MinC++;
							}
							MinC = 0;
							MidC++;
						}
						P[MajC] = MajC * kOrd[Mode[1]] * kOrd[Mode[0]];
						MidC    = 0;
						MajC++;
					}
					Init_Cache = 0;
					/*		  View_Cache(cache,P,kOrd[Mode[2]],kOrd[Mode[1]],kOrd[Mode[0]]); */
				}

				else { /*Load a new section and shift the pointers; also update the weight block*/
					MidC = MinC = 0;
					val         = 0;
					wt          = 0;
					s           = k + Center[Mode[2]];
					for (r = j - Center[Mode[1]]; r <= j + Center[Mode[1]]; r++) {
						for (q = i - Center[Mode[0]]; q <= i + Center[Mode[0]]; q++) {
							/*Load Portion of Cache Block*/
							if (q < 0 || r < 0 || s < 0) { /* Do a wrap check here in the furture */
								cache[P[0] + MidC * kOrd[Mode[0]] + MinC]  = 0;
								Weight[P[0] + MidC * kOrd[Mode[0]] + MinC] = 0;
							} else if (q >= mOrd[Mode[0]] || r >= mOrd[Mode[1]] || s >= mOrd[Mode[2]]) {
								cache[P[0] + MidC * kOrd[Mode[0]] + MinC]  = 0;
								Weight[P[0] + MidC * kOrd[Mode[0]] + MinC] = 0;
							} else {
								Pos[Mode[0]] = q;
								Pos[Mode[1]] = r;
								Pos[Mode[2]] = s;
								Mem_Index    = cpos(Pos[0], Pos[1], Pos[2], obj);
								cache[P[0] + MidC * kOrd[Mode[0]] + MinC] = extract_float(obj, Mem_Index);
								Weight[P[0] + MidC * kOrd[Mode[0]] + MinC] = 1;
								val += cache[P[0] + MidC * kOrd[Mode[0]] + MinC] *
								       Mask[(kOrd[Mode[2]] - 1) * kOrd[Mode[1]] * kOrd[Mode[0]] +
								            MidC * kOrd[Mode[0]] + MinC];
								wt++;
							}
							MinC++;
						}
						MinC = 0;
						MidC++;
					}
					/*Now adjust cache pointers and finish multiplying the Mask*/
					T = P[0];
					for (s = 0; s < kOrd[Mode[2]] - 1; s++) {
						P[s] = P[s + 1];
						for (r = 0; r < kOrd[Mode[1]]; r++) {
							for (q = 0; q < kOrd[Mode[0]]; q++) {
								wt += Weight[P[s] + r * kOrd[Mode[0]] + q];
								val += cache[P[s] + r * kOrd[Mode[0]] + q] *
								       Mask[s * kOrd[Mode[1]] * kOrd[Mode[0]] + r * kOrd[Mode[0]] + q];
							}
						}
					}
					P[kOrd[Mode[2]] - 1] = T;

					/*		  View_Cache(cache,P,kOrd[Mode[2]],kOrd[Mode[1]],kOrd[Mode[0]]); */
				}
				/*
				        val=0;
				        wt=0;
				        for (q=0;q<kOrd[Mode[0]];q++){
				          for (r=0;r<kOrd[Mode[1]];r++){
				            for (s=0;s<kOrd[Mode[2]];s++){
				            val+=cache[P[s]+r*kOrd[Mode[0]]+q]*
				                Mask[s*kOrd[Mode[1]]*kOrd[Mode[0]]+
				                     r*kOrd[Mode[0]]+q];
				            wt+=Weight[P[s]+r*kOrd[Mode[0]]+q];
				            }
				          }
				        }
				*/

				I       = cpos(Ce[0], Ce[1], Ce[2], obj);
				data[I] = val;
				if (norm) data[I] /= (float)wt;
			}
			Init_Cache = 1;
		}
	}
	return (newVal(V_ORG(obj), V_SIZE(obj)[0], V_SIZE(obj)[1], V_SIZE(obj)[2], DV_FLOAT, (float*)data));
}

void View_Cache(float* cache, int* P, int Major, int Middle, int Minor)
{
	int i, j, k;

	for (i = 0; i < Major; i++) {
		printf("Major:%d\n", i);
		for (j = 0; j < Middle; j++) {
			printf("\t");
			for (k = 0; k < Minor; k++) {
				printf("%d ", (int)cache[P[i] + j * Minor + k]);
			}
			printf("\n");
		}
	}
}

static void position_fill(int* pos, size_t elem, int iter, Var* obj);

Var* ff_maxpos(vfuncptr func, Var* arg)
{

	/* Transferred from thm module to davinci core 11/17/2007      **
	** Rewritten to offer user option to return pixel value - kjn  */

	Var* data    = NULL; /* the original data               */
	Var* out     = NULL; /* the output structure            */
	int* pos     = NULL; /* the output position             */
	float* posv  = NULL; /* the output position plus value  */
	float* vals  = NULL;
	float minval = -FLT_MAX; /* the most negative float value   */
	float ignore = minval;   /* null value                      */
	size_t i, j, k;          /* loop indices and flags          */
	int ck    = 0;
	float ni1 = 0, ni2 = 0, ni3 = 0; /* temp values and positions       */
	int iter         = 1;            /* number of iterations to include */
	size_t elems     = 0;
	size_t curr_elem = 0;
	int* elements    = NULL;
	int showval      = 0;      /* flag to return value with array */
	float lt         = minval; /* search for max values < this value */

	Alist alist[6];
	alist[0]      = make_alist("data", ID_VAL, NULL, &data);
	alist[1]      = make_alist("iter", DV_INT32, NULL, &iter);
	alist[2]      = make_alist("ignore", DV_FLOAT, NULL, &ignore);
	alist[3]      = make_alist("lt", DV_FLOAT, NULL, &lt);
	alist[4]      = make_alist("showval", DV_INT32, NULL, &showval);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (data == NULL) {
		parse_error("\nNo data provided\n");
		return NULL;
	}

	/* create array for position */
	pos      = (int*)calloc(sizeof(int), 3 * iter);
	vals     = (float*)calloc(sizeof(float), iter);
	elements = (int*)calloc(sizeof(int), iter);

	if (showval != 0) {
		posv = (float*)calloc(sizeof(float), 4 * iter);
	}

	/* size of the data */
	elems = V_DSIZE(data);

	/* set initial max values */
	ni2 = ni3 = minval;

	/* set ni3 for less than value */
	if (lt != minval) ni3 = lt;

	/* ni1 = current value     **
	** ni2 = current max value **
	** ni3 = old max val       */

	/* find the maximum point and its position */
	for (k = 0; k < iter; k++) {
		for (i = 0; i < elems; i++) {
			ni1 = extract_float(data, i);

			if (ni1 > ni2 && ni1 != ignore && ((lt != minval && k == 0 && ni1 <= ni3) ||
			                                   (k == 0 && lt == minval) || (ni1 <= ni3 && k > 0))) {

				/* check to see if it's the same element as a previous entry */
				ck = 0;
				if (k > 0) {
					for (j = k; j > 0; j--) {
						if (i == elements[j - 1]) ck = 1;
					}
				}

				if (ck == 0) {
					ni2       = ni1;
					curr_elem = i;
				}
			}
		}

		ni3         = ni2;
		elements[k] = curr_elem;
		position_fill(pos, curr_elem, k, data);
		vals[k]   = ni3;
		ni2       = minval;
		curr_elem = 0;
	}

	free(elements);

	/* Concatenate position with value if flagged */
	if (showval != 0) {
		for (j = 0; j < iter; j += 1) {
			posv[j * 4 + 0] = (float)pos[j * 3 + 0];
			posv[j * 4 + 1] = (float)pos[j * 3 + 1];
			posv[j * 4 + 2] = (float)pos[j * 3 + 2];
			posv[j * 4 + 3] = vals[j];

			free(vals);
			free(pos);

			out = newVal(BSQ, 4, iter, 1, DV_FLOAT, posv);
			return (out);
		}
	}

	/* return the findings */
	free(vals);

	out = newVal(BSQ, 3, iter, 1, DV_INT32, pos);
	return (out);
}

Var* ff_minpos(vfuncptr func, Var* arg)
{
	/* Transferred from thm module to davinci core 11/17/2007      **
	 ** Rewritten to offer user option to return pixel value - kjn  */

	Var* data    = NULL; /* the original data               */
	Var* out     = NULL; /* the output structure            */
	int* pos     = NULL; /* the output position             */
	float* vals  = NULL;
	float* posv  = NULL;    /* the output position plus value  */
	float maxval = FLT_MAX; /* the most negative float value   */
	float ignore = maxval;  /* null value                      */
	size_t i, j, k;         /* loop indices and flags          */
	int ck    = 0;
	float ni1 = 0, ni2 = 0, ni3 = 0; /* temp values and positions       */
	int iter         = 1;            /* number of iterations to include */
	size_t elems     = 0;
	size_t curr_elem = 0;
	int* elements    = NULL;
	int showval      = 0;      /* flag to return value with array */
	float gt         = maxval; /* search for min values > this value */

	Alist alist[6];
	alist[0]      = make_alist("data", ID_VAL, NULL, &data);
	alist[1]      = make_alist("iter", DV_INT32, NULL, &iter);
	alist[2]      = make_alist("ignore", DV_FLOAT, NULL, &ignore);
	alist[3]      = make_alist("gt", DV_FLOAT, NULL, &gt);
	alist[4]      = make_alist("showval", DV_INT32, NULL, &showval);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (data == NULL) {
		parse_error("\nNo data provided\n");
		return NULL;
	}

	/* create array for position */
	pos      = (int*)calloc(sizeof(int), 4 * iter);
	vals     = (float*)calloc(sizeof(float), iter);
	elements = (int*)calloc(sizeof(int), iter);

	if (showval != 0) {
		posv = (float*)calloc(sizeof(float), 4 * iter);
	}

	/* size of the data */
	elems = V_DSIZE(data);

	/* set initial max values */
	ni2 = ni3 = maxval;

	/* set ni3 for less than value */
	if (gt != maxval) ni3 = gt;

	/* ni1 = current value     **
	** ni2 = current min value **
	** ni3 = old min val       */

	/* find the minimum point and its position */
	for (k = 0; k < iter; k++) {
		for (i = 0; i < elems; i++) {
			ni1 = extract_float(data, i);

			//TODO(rswinkle) simplify logic ((k==0 && (gt == maxval || ni1 >= ni3)) || (ni1 >= ni3 && k > 0)
			if (ni1 < ni2 && ni1 != ignore && ((gt != maxval && k == 0 && ni1 >= ni3) ||
			                                   (k == 0 && gt == maxval) || (ni1 >= ni3 && k > 0))) {

				/* check to see if it's the same element as a previous entry */
				ck = 0;
				if (k > 0) {
					for (j = k; j > 0; j--) {
						if (i == elements[j - 1]) ck = 1;
					}
				}

				if (ck == 0) {
					ni2       = ni1;
					curr_elem = i;
				}
			}
		}

		ni3         = ni2;
		elements[k] = curr_elem;
		position_fill(pos, curr_elem, k, data);
		vals[k]   = ni3;
		ni2       = maxval;
		curr_elem = 0;
	}

	free(elements);

	/* Concatenate position with value if flagged */
	if (showval != 0) {
		for (j = 0; j < iter; j += 1) {
			posv[j * 4 + 0] = (float)pos[j * 3 + 0];
			posv[j * 4 + 1] = (float)pos[j * 3 + 1];
			posv[j * 4 + 2] = (float)pos[j * 3 + 2];
			posv[j * 4 + 3] = vals[j];

			free(vals);
			free(pos);

			out = newVal(BSQ, 4, iter, 1, DV_FLOAT, posv);
			return (out);
		}
	}

	/* return the findings */
	out = newVal(BSQ, 3, iter, 1, DV_INT32, pos);
	return (out);
}

Var* ff_valpos(vfuncptr func, Var* arg)
{

	Var* data    = NULL; /* the original data               */
	Var* out     = NULL; /* the output structure            */
	int* pos     = NULL; /* the output position             */
	float* posv  = NULL; /* the output position plus value  */
	float* vals  = NULL;
	float maxval = FLT_MAX; /* the most positive float value   */
	float ignore = maxval;  /* null value                      */
	size_t i, j, k;         /* loop indices and flags          */
	int ck    = 0;
	float ni1 = 0, ni2 = 0; /* temp values and positions       */
	float ni3 = 0, ni4 = 0;
	int iter         = 1; /* number of iterations to include */
	size_t elems     = 0;
	size_t curr_elem = 0;
	int* elements    = NULL;
	int showval      = 0;   /* flag to return value with array */
	float myval      = 0.0; /* the damned value you want       */

	Alist alist[6];
	alist[0]      = make_alist("data", ID_VAL, NULL, &data);
	alist[1]      = make_alist("value", DV_FLOAT, NULL, &myval);
	alist[2]      = make_alist("iter", DV_INT32, NULL, &iter);
	alist[3]      = make_alist("ignore", DV_FLOAT, NULL, &ignore);
	alist[4]      = make_alist("showval", DV_INT32, NULL, &showval);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (data == NULL) {
		parse_error("\nNo data provided\n");
		return NULL;
	}

	/* create array for position */
	pos      = (int*)calloc(sizeof(int), 3 * iter);
	vals     = (float*)calloc(sizeof(float), iter);
	elements = (int*)calloc(sizeof(int), iter);

	if (showval != 0) {
		posv = (float*)calloc(sizeof(float), 4 * iter);
	}

	/* size of the data */
	elems = V_DSIZE(data);

	/* set initial max values */
	ni3 = maxval;
	ni4 = 0;

	/* ni1 = current value                             **
	** ni2 = current difference  abs(ni1-myval)        **
	** ni3 = current closest val                       **
	** ni4 = old closest difference                    */

	/* find the closest value and its position */
	for (k = 0; k < iter; k++) {
		ni3 = maxval;
		for (i = 0; i < elems; i++) {
			ni1 = extract_float(data, i);
			ni2 = fabs((float)ni1 - (float)myval);

			if (ni2 < fabs((float)ni3 - (float)myval) && ni1 != ignore && ni2 >= ni4) {

				/* check to see if it's the same element as a previous entry */
				ck = 0;
				if (k > 0) {
					for (j = k; j > 0; j--) {
						if (i == elements[j - 1]) ck = 1;
					}
				}

				if (ck == 0) {
					ni3       = ni1;
					curr_elem = i;
				}
			}
		}

		ni4         = fabs((float)ni3 - (float)myval);
		elements[k] = curr_elem;
		position_fill(pos, curr_elem, k, data);
		vals[k]   = ni3;
		ni2       = maxval;
		curr_elem = 0;
	}

	free(elements);

	/* Concatenate position with value if flagged */
	if (showval != 0) {
		for (j = 0; j < iter; j += 1) {
			posv[j * 4 + 0] = (float)pos[j * 3 + 0];
			posv[j * 4 + 1] = (float)pos[j * 3 + 1];
			posv[j * 4 + 2] = (float)pos[j * 3 + 2];
			posv[j * 4 + 3] = vals[j];
		}

		free(vals);
		free(pos);

		out = newVal(BSQ, 4, iter, 1, DV_FLOAT, posv);
		return (out);
	}

	/* return the findings */
	free(vals);

	out = newVal(BSQ, 3, iter, 1, DV_INT32, pos);
	return (out);
}

void position_fill(int* pos, size_t elem, int iter, Var* obj)
{
	int x, y, z;
	int org;
	int i, j, k;

	x   = GetX(obj);
	y   = GetY(obj);
	z   = GetZ(obj);
	i   = 0;
	j   = 0;
	k   = 0;
	org = V_ORG(obj);

	if (org == 0) { // bsq organization
		k = elem / (x * y);
		j = (elem - k * x * y) / x;
		i = elem - k * x * y - j * x;
	} else if (org == 1) { // bil organization
		j = elem / (x * z);
		k = (elem - j * x * z) / x;
		i = elem - j * x * z - k * x;
	} else if (org == 2) { // bip organization
		j = elem / (z * x);
		i = (elem - j * z * x) / z;
		k = elem - j * z * x - k * z;
	}
	pos[iter * 3 + 2] = (float)(k + 1); // z position
	pos[iter * 3 + 1] = (float)(j + 1); // y position
	pos[iter * 3 + 0] = (float)(i + 1); // x position
}
