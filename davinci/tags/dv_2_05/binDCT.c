#include "parser.h"

void jpeg_fdct_bin_c1(int *);

void bindct_c1(int *data);
void binrdct_c1(int *coef_block, int *output_buf);
void jpeg_fdct_bin_c1(int *data);
void jpeg_idct_bin_c1(int *coef_block, int *output_buf);

Var *ff_bindct(vfuncptr func, Var * arg)
{
  Var *obj = NULL;
  int x, y, z, i, j, k;
  int *data;
  int *odata;
  Var *out;

  Alist alist[2];
  alist[0] = make_alist("object", ID_VAL, NULL, &obj);
  alist[1].name = NULL;

  if (parse_args(func, arg, alist) == 0)
    return (NULL);

  if (obj == NULL) {
    parse_error("%s: No object specified\n", func->name);
    return (NULL);
  }

  x = GetSamples(V_SIZE(obj), V_ORG(obj));
  y = GetLines(V_SIZE(obj), V_ORG(obj));
  z = GetBands(V_SIZE(obj), V_ORG(obj));

  if (x != 8 || y != 8) {
    parse_error("binDCT only works on 8x8 element arrays");
    return(NULL);
  }

  data = calloc(64, sizeof(int));
  odata = calloc(x*y*z, sizeof(int));

  for (k = 0 ; k < z ; k++) {
    for (j = 0 ; j < 8 ; j++) {
      for (i = 0 ; i < 8 ; i++) {
        data[i+j*8] = extract_int(obj, cpos(i,j,k, obj));
      }
    }

    if (!strcmp(func->name, "bindct")) {
      jpeg_fdct_bin_c1(data);
    } else {
      jpeg_idct_bin_c1(data,data);
    }


    for (j = 0 ; j < 8 ; j++) {
      for (i = 0 ; i < 8 ; i++) {
        odata[cpos(i,j,k,obj)] = data[i+j*8];
      }
    }
  }

  out = newVal(V_ORG(obj), V_SIZE(obj)[0], V_SIZE(obj)[1], V_SIZE(obj)[2],
               INT, odata);

  return(out);
}


void
bindct_c1(int *data)
{
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int *dataptr;

/****************************************************************************/
    /* Case 2: lossless binDCT: Use new butterflies. */

    dataptr = data;

    tmp7 = dataptr[0] - dataptr[7];
    tmp0 = dataptr[0] - ((tmp7) >> 1);

    tmp6 = dataptr[1] - dataptr[6];
    tmp1 = dataptr[1] - ((tmp6) >> 1);

    tmp5 = dataptr[2] - dataptr[5];
    tmp2 = dataptr[2] - ((tmp5) >> 1);

    tmp4 = dataptr[3] - dataptr[4];
    tmp3 = dataptr[3] - ((tmp4) >> 1);

    /* Even part */
    tmp13 = tmp0 - tmp3;
    tmp10 = tmp0 - ((tmp13) >> 1);

    tmp12 = tmp1 - tmp2;
    tmp11 = tmp1 - ((tmp12) >> 1);

    dataptr[4] = tmp10 - tmp11;
    dataptr[0] = tmp10 - ((dataptr[4]) >> 1);

    /* 3pi/8: 13/32, 11/32. */
    dataptr[6] = (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - tmp12;
    dataptr[2] =
        tmp13 -
        (((dataptr[6] << 3) + (dataptr[6] << 1) + dataptr[6]) >> 5);

    /* Odd part */

    /* pi/4 = 13/32, 11/16, 13/32 */
    tmp10 = tmp5 - (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);
    tmp6 = tmp6 + tmp10 - (((tmp10 << 2) + tmp10) >> 4);
    tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp10;

    /* butterflies: */
    tmp11 = tmp4 - tmp5;
    tmp10 = tmp4 - ((tmp11) >> 1);

    tmp12 = tmp7 - tmp6;
    tmp13 = tmp7 - ((tmp12) >> 1);

    /* 7pi/16 = 3/16u -3/16d: alter the sign to get positive scaling factor */
    dataptr[7] = (((tmp13 << 1) + tmp13) >> 4) - tmp10;
    dataptr[1] = tmp13 - (((dataptr[7] << 1) + dataptr[7]) >> 4);

    /* 3pi/16 = */
    /* new version: 11/16, 15/32 */
    dataptr[5] = tmp11 + (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);
    dataptr[3] = tmp12 - (((dataptr[5] << 4) - dataptr[5]) >> 5);
}


void
binrdct_c1(int *coef_block, int *output_buf)
{
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int *inptr;
    int *wsptr;

/****************************************************************************/
/* Case 2: lossless binDCT: descale by 2 immediately after inverse butterfly. */

    /* Pass 1: process columns from input, store into work array. */

    inptr = coef_block;
    wsptr = output_buf;

    tmp0 = inptr[0];
    tmp1 = inptr[4];
    tmp2 = inptr[6];
    tmp3 = inptr[2];
    tmp4 = inptr[7];
    tmp5 = inptr[5];
    tmp6 = inptr[3];
    tmp7 = inptr[1];

    /* X[0] and X[4] */
    tmp10 = tmp0 + ((tmp1) >> 1);
    tmp11 = tmp10 - tmp1;

    /* X[6] and X[2]: 11/32, 13/32 */
    tmp13 = tmp3 + (((tmp2 << 3) + (tmp2 << 1) + tmp2) >> 5);
    tmp12 = (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - tmp2;

    /* lossless binDCT: use new butterflies. */
    tmp0 = tmp10 + ((tmp13) >> 1);
    tmp3 = tmp0 - tmp13;

    tmp1 = tmp11 + ((tmp12) >> 1);
    tmp2 = tmp1 - tmp12;

    /* X[7] and X[1]: */
    /* 7pi/16 = 3/16d 3/16u */
    tmp13 = tmp7 + (((tmp4 << 1) + tmp4) >> 4);
    tmp10 = (((tmp13 << 1) + tmp13) >> 4) - tmp4;

    /* X[5] and X[3] */
    /* new 15/32 and 11/16 */
    tmp12 = tmp6 + (((tmp5 << 4) - tmp5) >> 5);
    tmp11 = tmp5 - (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);

    /* lossless binDCT: use new butterflies. */
    tmp4 = tmp10 + ((tmp11) >> 1);
    tmp5 = tmp4 - tmp11;

    tmp7 = tmp13 + ((tmp12) >> 1);
    tmp6 = tmp7 - tmp12;

    /* pi/4 = 13/32, 11/16, 13/32 */
    tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp5;
    tmp6 = tmp6 - tmp5 + (((tmp5 << 2) + tmp5) >> 4);
    tmp5 = tmp5 + (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);

    /* last stage: butterfly */
    wsptr[0] = tmp0 + ((tmp7) >> 1);
    wsptr[7] = wsptr[0] - tmp7;

    wsptr[1] = tmp1 + ((tmp6) >> 1);
    wsptr[6] = wsptr[1] - tmp6;

    wsptr[2] = tmp2 + ((tmp5) >> 1);
    wsptr[5] = wsptr[2] - tmp5;

    wsptr[3] = tmp3 + ((tmp4) >> 1);
    wsptr[4] = wsptr[3] - tmp4;
}

#define DCTSIZE 8
#define DCTSIZE2 DCTSIZE*DCTSIZE

void jpeg_fdct_bin_c1(int *data)
{
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int *dataptr;
    int ctr;

/****************************************************************************/
    /* Case 2: lossless binDCT: Use new butterflies. */

    dataptr = data;
    for (ctr = DCTSIZE - 1; ctr >= 0; ctr--) {

        tmp7 = dataptr[0] - dataptr[7];
        tmp0 = dataptr[0] - ((tmp7) >> 1);

        tmp6 = dataptr[1] - dataptr[6];
        tmp1 = dataptr[1] - ((tmp6) >> 1);

        tmp5 = dataptr[2] - dataptr[5];
        tmp2 = dataptr[2] - ((tmp5) >> 1);

        tmp4 = dataptr[3] - dataptr[4];
        tmp3 = dataptr[3] - ((tmp4) >> 1);

        /* Even part */
        tmp13 = tmp0 - tmp3;
        tmp10 = tmp0 - ((tmp13) >> 1);

        tmp12 = tmp1 - tmp2;
        tmp11 = tmp1 - ((tmp12) >> 1);

        dataptr[4] = tmp10 - tmp11;
        dataptr[0] = tmp10 - ((dataptr[4]) >> 1);

        /* 3pi/8: 13/32, 11/32. */
        dataptr[6] = (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - tmp12;
        dataptr[2] =
            tmp13 -
            (((dataptr[6] << 3) + (dataptr[6] << 1) + dataptr[6]) >> 5);

        /* Odd part */

        /* pi/4 = 13/32, 11/16, 13/32 */
        tmp10 = tmp5 - (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);
        tmp6 = tmp6 + tmp10 - (((tmp10 << 2) + tmp10) >> 4);
        tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp10;

        /* butterflies: */
        tmp11 = tmp4 - tmp5;
        tmp10 = tmp4 - ((tmp11) >> 1);

        tmp12 = tmp7 - tmp6;
        tmp13 = tmp7 - ((tmp12) >> 1);

        /* 7pi/16 = 3/16u -3/16d: alter the sign to get positive scaling factor */
        dataptr[7] = (((tmp13 << 1) + tmp13) >> 4) - tmp10;
        dataptr[1] = tmp13 - (((dataptr[7] << 1) + dataptr[7]) >> 4);

        /* 3pi/16 = */
        /* new version: 11/16, 15/32 */
        dataptr[5] = tmp11 + (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);
        dataptr[3] = tmp12 - (((dataptr[5] << 4) - dataptr[5]) >> 5);

        dataptr += DCTSIZE;     /* advance pointer to next row */
    }

    /* Pass 2: process columns.
     */

    dataptr = data;
    for (ctr = DCTSIZE - 1; ctr >= 0; ctr--) {

        tmp7 = dataptr[DCTSIZE * 0] - dataptr[DCTSIZE * 7];
        tmp0 = dataptr[DCTSIZE * 0] - ((tmp7) >> 1);

        tmp6 = dataptr[DCTSIZE * 1] - dataptr[DCTSIZE * 6];
        tmp1 = dataptr[DCTSIZE * 1] - ((tmp6) >> 1);

        tmp5 = dataptr[DCTSIZE * 2] - dataptr[DCTSIZE * 5];
        tmp2 = dataptr[DCTSIZE * 2] - ((tmp5) >> 1);

        tmp4 = dataptr[DCTSIZE * 3] - dataptr[DCTSIZE * 4];
        tmp3 = dataptr[DCTSIZE * 3] - ((tmp4) >> 1);

        /* Even part */

        tmp13 = tmp0 - tmp3;
        tmp10 = tmp0 - ((tmp13) >> 1);

        tmp12 = tmp1 - tmp2;
        tmp11 = tmp1 - ((tmp12) >> 1);

        dataptr[DCTSIZE * 4] = tmp10 - tmp11;
        dataptr[DCTSIZE * 0] = tmp10 - ((dataptr[DCTSIZE * 4]) >> 1);

        /*  3pi/8: 13/32, 11/32 */
        dataptr[DCTSIZE * 6] =
            (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - tmp12;
        dataptr[DCTSIZE * 2] =
            tmp13 -
            (((dataptr[DCTSIZE * 6] << 3) + (dataptr[DCTSIZE * 6] << 1) +
              dataptr[DCTSIZE * 6]) >> 5);

        /* Odd part */

        /* pi/4 = 13/32, 11/16, 13/32 */
        tmp10 = tmp5 - (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);
        tmp6 = tmp6 + tmp10 - (((tmp10 << 2) + tmp10) >> 4);
        tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp10;

        /* butterflies */
        tmp11 = tmp4 - tmp5;
        tmp10 = tmp4 - ((tmp11) >> 1);

        tmp12 = tmp7 - tmp6;
        tmp13 = tmp7 - ((tmp12) >> 1);

        /* 7pi/16 = 3/16u -3/16d: alter sign to get positive scaling factor */
        dataptr[DCTSIZE * 7] = (((tmp13 << 1) + tmp13) >> 4) - tmp10;
        dataptr[DCTSIZE * 1] =
            tmp13 -
            (((dataptr[DCTSIZE * 7] << 1) + dataptr[DCTSIZE * 7]) >> 4);

        /* 3pi/16 = */
        /* new : 11/16 and 15/32 */
        dataptr[DCTSIZE * 5] =
            tmp11 + (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);
        dataptr[DCTSIZE * 3] =
            tmp12 -
            (((dataptr[DCTSIZE * 5] << 4) - dataptr[DCTSIZE * 5]) >> 5);

        dataptr++;              /* advance pointer to next column */
    }
}

void
jpeg_idct_bin_c1(int *coef_block, int *output_buf)
{
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int *inptr;
    int *wsptr;
    int *outptr;
    int ctr;
    int workspace[DCTSIZE2];    /* buffers data between passes */

/****************************************************************************/
/* Case 2: lossless binDCT: descale by 2 immediately after inverse butterfly. */

    /* Pass 1: process columns from input, store into work array. */

    inptr = coef_block;
    wsptr = workspace;
    for (ctr = DCTSIZE; ctr > 0; ctr--) {

        tmp0 = inptr[DCTSIZE * 0];
        tmp1 = inptr[DCTSIZE * 4];
        tmp2 = inptr[DCTSIZE * 6];
        tmp3 = inptr[DCTSIZE * 2];
        tmp4 = inptr[DCTSIZE * 7];
        tmp5 = inptr[DCTSIZE * 5];
        tmp6 = inptr[DCTSIZE * 3];
        tmp7 = inptr[DCTSIZE * 1];

        /* X[0] and X[4] */
        tmp10 = tmp0 + ((tmp1) >> 1);
        tmp11 = tmp10 - tmp1;

        /* X[6] and X[2]: 11/32, 13/32 */
        tmp13 = tmp3 + (((tmp2 << 3) + (tmp2 << 1) + tmp2) >> 5);
        tmp12 = (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - tmp2;

        /* lossless binDCT: use new butterflies. */
        tmp0 = tmp10 + ((tmp13) >> 1);
        tmp3 = tmp0 - tmp13;

        tmp1 = tmp11 + ((tmp12) >> 1);
        tmp2 = tmp1 - tmp12;

        /* X[7] and X[1]: */
        /* 7pi/16 = 3/16d 3/16u */
        tmp13 = tmp7 + (((tmp4 << 1) + tmp4) >> 4);
        tmp10 = (((tmp13 << 1) + tmp13) >> 4) - tmp4;

        /* X[5] and X[3] */
        /* new 15/32 and 11/16 */
        tmp12 = tmp6 + (((tmp5 << 4) - tmp5) >> 5);
        tmp11 = tmp5 - (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);

        /* lossless binDCT: use new butterflies. */
        tmp4 = tmp10 + ((tmp11) >> 1);
        tmp5 = tmp4 - tmp11;

        tmp7 = tmp13 + ((tmp12) >> 1);
        tmp6 = tmp7 - tmp12;

        /* pi/4 = 13/32, 11/16, 13/32 */
        tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp5;
        tmp6 = tmp6 - tmp5 + (((tmp5 << 2) + tmp5) >> 4);
        tmp5 = tmp5 + (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);

        /* last stage: butterfly */
        wsptr[DCTSIZE * 0] = tmp0 + ((tmp7) >> 1);
        wsptr[DCTSIZE * 7] = wsptr[DCTSIZE * 0] - tmp7;

        wsptr[DCTSIZE * 1] = tmp1 + ((tmp6) >> 1);
        wsptr[DCTSIZE * 6] = wsptr[DCTSIZE * 1] - tmp6;

        wsptr[DCTSIZE * 2] = tmp2 + ((tmp5) >> 1);
        wsptr[DCTSIZE * 5] = wsptr[DCTSIZE * 2] - tmp5;

        wsptr[DCTSIZE * 3] = tmp3 + ((tmp4) >> 1);
        wsptr[DCTSIZE * 4] = wsptr[DCTSIZE * 3] - tmp4;

        inptr++;                /* advance pointers to next column */
        wsptr++;
    }

    /* Pass 2: process rows from work array, store into output array. */
    /* Note that we must descale the results by a factor of 8 == 2**3, */
    /* and also undo the PASS1_BITS scaling. */

    /* fprintf(stderr, "\nAfter inverse DCT:\n"); */

    wsptr = workspace;
    for (ctr = 0; ctr < DCTSIZE; ctr++) {
        outptr = &output_buf[ctr * DCTSIZE];

        /* Even part: reverse the even part of the forward DCT. */

        /* Even part */

        /* X[0] and X[4] */
        tmp10 = wsptr[0] + ((wsptr[4]) >> 1);
        tmp11 = tmp10 - wsptr[4];

        /* X[6] and X[2]:11/32, 13/32 */
        tmp13 =
            wsptr[2] +
            (((wsptr[6] << 3) + (wsptr[6] << 1) + wsptr[6]) >> 5);
        tmp12 = (((tmp13 << 3) + (tmp13 << 2) + tmp13) >> 5) - wsptr[6];

        /* lossless binDCT: use new nutterflies. */
        tmp0 = tmp10 + ((tmp13) >> 1);
        tmp3 = tmp0 - tmp13;

        tmp1 = tmp11 + ((tmp12) >> 1);
        tmp2 = tmp1 - tmp12;

        /* 7pi/16 = -3/16d 3/16u */
        tmp13 = wsptr[1] + (((wsptr[7] << 1) + wsptr[7]) >> 4);
        tmp10 = (((tmp13 << 1) + tmp13) >> 4) - wsptr[7];

        /* 3pi/16 = 1/2d -7/8u */
        /* new 15/32 and -11/16 */
        tmp12 = wsptr[3] + (((wsptr[5] << 4) - wsptr[5]) >> 5);
        tmp11 = wsptr[5] - (((tmp12 << 3) + (tmp12 << 1) + tmp12) >> 4);

        /* lossless binDCT: use new butterflies. */
        tmp4 = tmp10 + ((tmp11) >> 1);
        tmp5 = tmp4 - tmp11;

        tmp7 = tmp13 + ((tmp12) >> 1);
        tmp6 = tmp7 - tmp12;

        /* pi/4 = 13/32, 11/16, 13/32 */
        tmp5 = (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5) - tmp5;
        tmp6 = tmp6 - tmp5 + (((tmp5 << 2) + tmp5) >> 4);
        tmp5 = tmp5 + (((tmp6 << 3) + (tmp6 << 2) + tmp6) >> 5);

        /* last stage: butterfly */

        /* Final output stage: scale down by a factor of 8 and range-limit */
        tmp10 = tmp0 + ((tmp7) >> 1);
        tmp11 = tmp10 - tmp7;
        outptr[0] = tmp10;
        outptr[7] = tmp11;

        if (tmp10 > 4096 || tmp10 < -4096 || tmp11 > 4096 || tmp11 < -4096) {
            fprintf(stderr, "Possible IDCT overflow!\n");
        }

        tmp10 = tmp1 + ((tmp6) >> 1);
        tmp11 = (tmp10 - tmp6);
        outptr[1] = tmp10;
        outptr[6] = tmp11;

        if (tmp10 > 4096 || tmp10 < -4096 || tmp11 > 4096 || tmp11 < -4096) {
            fprintf(stderr, "Possible IDCT overflow!\n");
        }

        tmp10 = tmp2 + ((tmp5) >> 1);
        tmp11 = (tmp10 - tmp5);
        outptr[2] = tmp10;
        outptr[5] = tmp11;

        if (tmp10 > 4096 || tmp10 < -4096 || tmp11 > 4096 || tmp11 < -4096) {
            fprintf(stderr, "Possible IDCT overflow!\n");
        }

        tmp10 = tmp3 + ((tmp4) >> 1);
        tmp11 = (tmp10 - tmp4);
        outptr[3] = tmp10;
        outptr[4] = tmp11;

        wsptr += DCTSIZE;       /* advance pointer to next row */
    }
}
