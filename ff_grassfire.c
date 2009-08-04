/** sedt : SEDT  in linear time
 *
 * David Coeurjolly (david.coeurjolly@liris.cnrs.fr) - Sept. 2004
 *
 * Version 0.3 : Feb. 2005
 *
 **/

/**
  =================================================
 * @file   sedt.cc
 * @author David COEURJOLLY <David Coeurjolly <dcoeurjo@liris.cnrs.fr>>
 * @date   Wed Sep 29 17:05:31 2004
 *
 * @brief  The Euclidean distance transform in linear time using the
 * Saito and Toriwaki algorithm with the Meijster/Roerdnik/Hesselink's
 * optimization
 *
 * Computational cost : O(n^3) if the input volume is nxnxn
 *
 * Memory requirement : O(n^3).
 *
 * More precisely : if nxnxn*(size of a char) is the size of the input volume,
 * the SDT requires  nxnxn*(size of a long int). Furthermore a temporary  nxnxn*(size of a long int)
 * is needed all along the process. Two vectors with size  n*(size of an int) (arrays s and q) are also used.
 *
 =================================================*/
#include "parser.h"

#define INFTY 100000001

/* operators : Basic arithmetic operation using INFTY numbers */
/* David Coeurjolly (david.coeurjolly@liris.cnrs.fr) - Sept. 2004 */


/////////Basic functions to handle operations with INFTY
/**
 **************************************************
 * @b sum
 * @param a Long number with INFTY
 * @param b Long number with INFTY
 * @return The sum of a and b handling INFTY
 **************************************************/
static long sum(long a, long b)
{
  if ((a==INFTY) || (b==INFTY))
    return INFTY;
  else
    return a+b;
}

/**
 **************************************************
 * @b prod
 * @param a Long number with INFTY
 * @param b Long number with INFTY
 * @return The product of a and b handling INFTY
 **************************************************/
static long prod(long a, long b)
{
  if ((a==INFTY) || (b==INFTY))
    return INFTY;
  else
    return a*b;
}
/**
 **************************************************
 * @b opp
 * @param a Long number with INFTY
 * @return The opposite of a  handling INFTY
 **************************************************/
static long opp (long a) {
  if (a == INFTY) {
    return INFTY;
  }
  else {
    return -a;
  }
}

/**
 **************************************************
 * @b intdivint
 * @param divid Long number with INFTY
 * @param divis Long number with INFTY
 * @return The division (integer) of divid out of divis handling INFTY
 **************************************************/
  static long intdivint (long divid, long divis) {
    if (divis == 0)
      return  INFTY;
    if (divid == INFTY)
      return  INFTY;
    else
      return  divid / divis;
  }
//////////

/**
  =================================================
 * @file   operators.cc
 * @author David COEURJOLLY <David Coeurjolly <dcoeurjo@liris.cnrs.fr>>
 * @date   Thu Sep 30 09:22:19 2004
 *
 * @brief  Basic implementation of arithmetical operations using INFTY numbers.
 *
 *
 =================================================*/

////////// Functions F and Sep for the SDT labelling

/*
 * @return Definition of a parabola
 */
static long F(int x, int i, long gi2) { return sum((x-i)*(x-i), gi2); }

/*
 * @return The abscissa of the intersection point between two parabolas
 */
static long Sep(int i, int u, long gi2, long gu2) {
  return intdivint(sum( sum((long) (u*u - i*i), gu2), opp(gi2) ), 2*(u-i));
}

/**
 **************************************************
 * @b phaseSaitoX
 * @param V Input volume
 * @param sdt_x SDT along the x-direction
 **************************************************/
//First step of  the saito  algorithm
// (Warning   : we  store the  EDT instead of the SDT)


void phaseSaitoX(Var *v, int ignore, Var *vx) {
  int dx = GetX(vx);
  int dy = GetY(vx);
  int x, y;

  int *sdt_x = V_DATA(vx);

  for (y = 0; y < dy ; y++) {
    if (extract_int(v, cpos(0, y, 0, v)) == ignore) {
      sdt_x[cpos(0, y, 0, vx)] = 0;
    } else {
      sdt_x[cpos(0, y, 0, vx)] = INFTY;
    }

    // Forward scan
    for (x = 1; x < dx ; x++) {
      if (extract_int(v, cpos(x, y, 0, v)) == ignore) {
        sdt_x[cpos(x, y, 0, vx)] = 0;
      } else {
        sdt_x[cpos(x, y, 0, vx)]= sum(1, sdt_x[cpos(x-1, y, 0, vx)]);
      }
    }

    //Backward scan
    for (x = dx -2; x >= 0; x--) {
      if (sdt_x[cpos(x+1, y, 0, vx)] < sdt_x[cpos(x, y, 0, vx)]) {
        sdt_x[cpos(x, y, 0, vx)]= sum(1, sdt_x[cpos(x+1, y, 0, vx)]);
      }
    }
  }
}


/**
 **************************************************
 * @b phaseSaitoY
 * @param sdt_x the SDT along the x-direction
 * @param sdt_xy the SDT in the xy-slices
 **************************************************/
//Second      Step   of    the       saito   algorithm    using    the
//[Meijster/Roerdnik/Hesselink] optimization
void phaseSaitoY(Var *vx, Var *vxy) {
  int dx = GetX(vx);
  int dy = GetY(vx);
  int s[dy]; //Center of the upper envelope parabolas
  int t[dy]; //Separating index between 2 upper envelope parabolas
  int q;
  int w;
  int x, u;

  int *sdt_x = V_DATA(vx);
  int *sdt_xy = V_DATA(vxy);

  for (x = 0; x < dx ; x++) {
    q = 0;
    s[0] = 0;
    t[0] = 0;

    //Forward Scan
    for (u = 1; u < dy ; u++) {
      while ((q >= 0) &&
             (F(t[q], s[q],
                prod(sdt_x[cpos(x, s[q], 0, vx)], sdt_x[cpos(x, s[q], 0, vx)])) >
              F(t[q], u,
                prod(sdt_x[cpos(x, u, 0, vx)], sdt_x[cpos(x, u, 0, vx)]))))
        q--;

      if (q<0) {
        q = 0;
        s[0] = u;
      } else {
        w = 1 + Sep(s[q], u,
                    prod(sdt_x[cpos(x, s[q], 0, vx)],
                         sdt_x[cpos(x, s[q], 0, vx)]),
                    prod(sdt_x[cpos(x, u, 0, vx)],
                         sdt_x[cpos(x, u, 0, vx)]));

        if (w < dy) {
          q++;
          s[q] = u;
          t[q] = w;
        }
      }
    }

    //Backward Scan
    for (u = dy-1; u >= 0; --u) {
      sdt_xy[cpos(x, u, 0, vxy)] = F(u, s[q],
                                     prod(sdt_x[cpos(x, s[q], 0, vx)],
                                          sdt_x[cpos(x, s[q], 0, vx)]));
      if (u ==t[q]) q--;
    }
  }
}

/**
 **************************************************
 * @b phaseSaitoZ
 * @param sdt_xy the SDT in the xy-slices
 * @param sdt_xyz the final SDT
 **************************************************/
//Third   Step      of     the    saito   algorithm     using      the
//[Meijster/Roerdnik/Hesselink] optimization
void phaseSaitoZ(Var *vxy, Var *vxyz) {
  int dx = GetX(vxy);
  int dy = GetY(vxy);
  int *sdt_xy = V_DATA(vxy);
  int *sdt_xyz = V_DATA(vxyz);
  int x, y;
  for (y = 0; y< dy ; y++) {
    for (x = 0; x < dx ; x++) {
      sdt_xyz[cpos(x, y, 0, vxyz)] = sqrt(F(0, 0, sdt_xy[cpos(x, y, 0, vxy)]));
    }
  }
}

Var *saito_grassfire(Var *input, int ignore) {
  // Euclidian distance computation
  size_t dx = GetX(input);
  size_t dy = GetY(input);

  Var *sdt_x = newVal(BSQ, dx, dy, 1, INT, calloc(dx*dy, sizeof(int)));
  if (sdt_x == NULL) {
    parse_error("Unable to allocate memory.");
    return(NULL);
  }
  Var *sdt_xy = newVal(BSQ, dx, dy, 1, INT, calloc(dx*dy, sizeof(int)));
  if (sdt_xy == NULL) {
    parse_error("Unable to allocate memory.");
    free(sdt_x);
    return(NULL);
  }

  phaseSaitoX(input, ignore, sdt_x);
  phaseSaitoY(sdt_x, sdt_xy);
  phaseSaitoZ(sdt_xy, sdt_x); //We reuse sdt_x to store the final result!!

  mem_claim(sdt_xy);
  mem_free(sdt_xy);
  return (sdt_x);
}

Var *vw_grassfire(Var *vsrc, int ignore) {
  size_t dx = GetX(vsrc);
  size_t dy = GetY(vsrc);
  int val;
  int i, j;

  int *dst = calloc(dx*dy, sizeof(int));
  if (dst == NULL) {
    parse_error("Unable to allocate memory.");
    return(NULL);
  }
  Var *vdst = newVal(BSQ, dx, dy, 1, INT, dst);

  // First row
  j = 0;
  for (i = 0 ; i < dx ; i++) {
    val = extract_int(vsrc, cpos(i, j, 0, vsrc));
    dst[cpos(i, j, 0, vdst)] = (val == ignore ? 0 : 1);
  }

  for (j = 1 ; j < dy-1 ; j++) {
    // first column
    i = 0;
    val = extract_int(vsrc, cpos(i, j, 0, vsrc));
    dst[cpos(i, j, 0, vdst)] = (val == ignore ? 0 : 1);

    // middle columns
    for (i = 1 ; i < dx-1 ; i++) {
      val = extract_int(vsrc, cpos(i, j, 0, vsrc));
      dst[cpos(i, j, 0, vdst)] = (val == ignore ? 0 :
                                  1 + min(dst[cpos(i-1, j, 0, vdst)],
                                          dst[cpos(i, j-1, 0, vdst)]));
    }

    // last column
    val = extract_int(vsrc, cpos(i, j, 0, vsrc));
    dst[cpos(i, j, 0, vdst)] = (val == ignore ? 0 : 1);
  }

  // last row
  for (i = 0 ; i < dx ; i++) {
    val = extract_int(vsrc, cpos(i, j, 0, vsrc));
    dst[cpos(i, j, 0, vdst)] = (val == ignore ? 0 : 1);
  }

  // Now the other direction
  for (j = dy-2 ; j >= 0 ; --j) {
    for (i = dx-2 ; i >= 0 ; --i) {
      if (dst[cpos(i, j, 0, vdst)] != 0) {
        int m = min(dst[cpos(i+1, j, 0, vdst)], dst[cpos(i, j+1, 0, vdst)]);
        if (m < dst[cpos(i, j, 0, vdst)]) dst[cpos(i, j, 0, vdst)] = m+1;
      }
    }
  }

  return(vdst);
}


// This marks all the pixels inside the outermost extents.  Not exactly
// grassfire, but closely related.
Var *bounding_box(Var *vsrc, int ignore) {
  size_t dx = GetX(vsrc);
  size_t dy = GetY(vsrc);
  int left, right, i, j;
  int val;

  int *dst = calloc(dx*dy, sizeof(int));
  if (dst == NULL) {
    parse_error("Unable to allocate memory.");
    return(NULL);
  }
  Var *vdst = newVal(BSQ, dx, dy, 1, INT, dst);

  for (j = 0 ; j < dy ; j++) {
    for (left = 0 ; left < dx ; left++) {
      val = extract_int(vsrc, cpos(left, j, 0, vsrc));
      if (val == ignore) {
        dst[cpos(left, j, 0, vdst)] = 0;
      } else {
        break;
      }
    }

    for (right = dx-1 ; right >= 0 ; right--) {
      val = extract_int(vsrc, cpos(right, j, 0, vsrc));
      if (val == ignore) {
        dst[cpos(right, j, 0, vdst)] = 0;
      } else {
        break;
      }
    }
    for (i = left ; i < right ; i++) {
      dst[cpos(i, j, 0, vdst)] = 1;
    }
  }
  return (vdst);
}

Var *ff_grassfire(vfuncptr func, Var * arg)
{
  Var *obj = NULL;
  int ignore = MAXINT;
  const char *options[] = { "euclidian", "manhattan", "bounding", NULL };
  char *type = (char *)options[0];

  Alist alist[4];
  alist[0] = make_alist("obj",    ID_VAL,  NULL, &obj);
  alist[1] = make_alist("ignore", INT,     NULL, &ignore);
  alist[2] = make_alist("type",   ID_ENUM, options, &type);
  alist[3].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if (obj == NULL) {
    parse_error( "%s: No value specified for keyword: object.", func->name);
    return(NULL);
  }

  if (!strcmp(type, "euclidian")) {
    return(saito_grassfire(obj, ignore));
  } else if (!strcmp(type, "manhattan")) {
    return(vw_grassfire(obj, ignore));
  } else if (!strcmp(type, "bounding")) {
    return(bounding_box(obj, ignore));
  } else {
    parse_error("%s: Unknown algorithm.", func->name);
    return (NULL);
  }
}
