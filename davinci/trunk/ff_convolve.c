#include "parser.h"
#if 0
vector convolve_BRUTE(const vector& vec1, const vector& vec2)
{
        const vector* v1;
        const vector* v2;
        vector v(vec1.size + vec2.size - 1, 0.0);

        if(vec1.size <= vec2.size)
        {
                v1 = &vec1;
                v2 = &vec2;
        }
        else
        {
                v1 = &vec2;
                v2 = &vec1;
        }

        long int i, j;
        /* part 1 */
        for(i = 0; i <= (v1->size - 1); ++i)
                for(j = 0; j <= i; ++j)
                        v.data[i] += v1->data[j] * v2->data[i - j];
        /* part 2 */
        if(v2->size > v1->size)
                for(i = v1->size; i <= (v2->size - 1); ++i)
                        for(j = 0; j <= (v1->size - 1); ++j)
                                v.data[i] += v1->data[j] * v2->data[i - j];
        /*  part 3 */
        for(i = 1; i <= (v1->size - 1); ++i)
                for(j = i; j <= (v1->size - 1); ++j)
                        v.data[(v2->size - 1) + i] +=
                        v1->data[j] * v2->data[(v2->size - 1) + i - j];

        return v;
}

#endif

Var *
ff_self_convolve(vfuncptr func, Var * arg)
{
	Var *v1 = NULL, *v2 = NULL;
	int m, n, i, j, d1, d2;
	float *out;

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist( "obj1",    ID_VAL,    NULL,     &v1);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (v1 == NULL) {
		parse_error("%s: No obj1 specified\n", func->name);
		return(NULL);
	}

	d1 = V_DSIZE(v1);

	n = d1 *2-1;
	out = (float *)calloc(sizeof(float), n);

	for (m = 0 ; m < d1 ; m++) {
		for (n = 0 ; n < d1 ; n++) {
			d2 = d1/2 -1 -n +m;
			if (d2 < 0 || d2 >= n) continue;
			out[m] += extract_float(v1, n) * extract_float(v1, d2);
		}
	}

	return(newVal(V_ORG(v1), V_SIZE(v1)[0], V_SIZE(v1)[1], V_SIZE(v1)[2], FLOAT, out));
}
