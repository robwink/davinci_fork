#include "parser.h"

void quicksort(void *, size_t, size_t, int (*)(), int *, int, int);
void *reorgByIndex(Var *, Var *, int *);

int cmp_byte(const void *a, const void *b)
{
    if (*(u_char *) a > *(u_char *) b) return (1);
    if (*(u_char *) a < *(u_char *) b) return (-1);
    return (0);
}
int cmp_short(const void *a, const void *b)
{
    if (*(short *) a > *(short *) b) return (1);
    if (*(short *) a < *(short *) b) return (-1);
    return (0);
}
int cmp_int(const void *a, const void *b)
{
    if (*(int *) a > *(int *) b) return (1);
    if (*(int *) a < *(int *) b) return (-1);
    return (0);
}
int cmp_float(const void *a, const void *b)
{
    if (*(float *) a > *(float *) b) return (1);
    if (*(float *) a < *(float *) b) return (-1);
    return (0);
}
int cmp_double(const void *a, const void *b)
{
    if (*(double *) a > *(double *) b) return (1);
    if (*(double *) a < *(double *) b) return (-1);
    return (0);
}

/*	Last modified by Scott Gonyea on Fri Jun  9 13:51:11 MST 2006
 *
 *	Sort will maintain a list of all elements and where they were moved.  
 *	Given an index, blocks of elements in object will be moved to reflect
 *	the sorted order of index.
 */
Var *ff_sort(vfuncptr func, Var * arg)
{
    Var   *object = NULL,	// Value of current object 
		    *rVal = NULL,	// Returned value/structure
		   *index = NULL,	// Sorting index from user
	 	    *args = NULL;
	 	    
    int *sortList = NULL;	// Sorted list (from index)

    int format, dsize,
       sformat, ssize, i;

    void *data, *sortSet;

    int (*cmp) (const void *, const void *);

    Alist alist[3];
    alist[0] = make_alist("object", ID_VAL, NULL, &object);
    alist[1] = make_alist("index",  ID_VAL, NULL, &index);
    alist[2].name = NULL;

    if (parse_args(func, arg, alist) == 0)
		return (NULL);

    if (object == NULL)
		return (NULL);

    format = V_FORMAT(object);
    dsize = V_DSIZE(object);

    switch (format) {
		case BYTE:		cmp = cmp_byte;		break;
		case SHORT:		cmp = cmp_short;	break;
		case INT:		cmp = cmp_int;		break;
		case FLOAT:		cmp = cmp_float;	break;
		case DOUBLE:	cmp = cmp_double;	break;
    }

    // Convert index or object to BSQ format, so that the sorted object
    //  can be rearranged in a way that is understandable to the user.
    if(index)
    	rVal = index;
    else
    	rVal = object;
    
    // Convert to BSQ if not already
    if (V_ORG(rVal) != BSQ) {
		args = create_args(1, NULL, rVal, NULL, NULL);
		rVal = V_func("bsq", args);

		if (index) index = rVal;	// Treat index like it is BSQ
	}
	
	/*	If index is NULL, ensure BSQ org (above), qsort,	*
	 *		then convert back to original organization.		*
	 *		This is visually "more intuitive" to the user.	*/
	if (index == NULL) {
		data = calloc(dsize, NBYTES(format));
		memcpy(data, V_DATA(rVal), dsize * NBYTES(format));
	
		qsort(data, dsize, NBYTES(format), cmp);
	
		rVal = newVal(V_ORG(rVal),
				  V_SIZE(rVal)[0], V_SIZE(rVal)[1], V_SIZE(rVal)[2],
				  format, data);
	
		// Convert the object back to its native format, if it's not BSQ
		if (V_ORG(object) != BSQ) {
			args = create_args(1, NULL, rVal, NULL, NULL);
			if(V_ORG(object) == BIL)
				rVal = V_func("bil", args);
			else
				rVal = V_func("bip", args);
		}
		return rVal;
    }

    sformat = V_FORMAT(index);
    ssize = V_DSIZE(index);

	// Create memory for sortSet -- avoid touching the user's data
    sortSet = calloc(V_DSIZE(index), NBYTES(sformat));
    memcpy(sortSet, V_DATA(index), V_DSIZE(index) * NBYTES(sformat));

    // sortList will record the index of items that are moved
    sortList = calloc(ssize, sizeof(int));
    for (i = 0; i < ssize; i++)
		sortList[i] = i;

    // Sort it
    quicksort(sortSet, ssize, NBYTES(sformat), cmp, sortList, 0,
	      ssize - 1);

	// Reorganize the object by its respective index
    data = reorgByIndex(object, index, sortList);

    rVal = newVal(V_ORG(object),
		  V_SIZE(object)[0], V_SIZE(object)[1], V_SIZE(object)[2],
		  format, data);
	
	// Release memory and return
    free(sortSet);	sortSet  = NULL;
    free(sortList);	sortList = NULL;

    return rVal;
}

// Move data [from] object, [to] data, using size to expand the offset as needed
#define reorg(to, data, from, object, size) 		\
	memcpy(	(void*)((long)data 	 + to 	* size),	\
			(void*)((long)object + from * size),	\
			size);
			
/*	Reorganizes the object, using the index object and its respective sortList.
 *		object - Data that is being manipulated
 *		index - Source for the sorted sortList
 *		sortList  - Sorted sortList, describing where blocks of object are placed
 */
void *reorgByIndex(Var *object, Var *index, int *sortList)
{
    int t;				// temporary integer

    int x, y, z,		// x y z for object
    	i, j, k,		// Loop counters;  i - x, j - y, k - z
    	doX = 1,		// Determines if
		doY = 1,		// X, Y, or Z
		doZ = 1;		// axis points remain contiguous
	
	int from, to;		// from / to, memory offsets for cpos

    int    format = 0,	// object's format
		    dsize = 0,	// object's size
	  sortListNum = 0;	// index and sortList's size

    int cntr = 0;		// sortList counter

    void *data;			// Returned data block

    x = GetX(object);
    y = GetY(object);
    z = GetZ(object);

    //	Any axis on index with a depth of '1' remains in a contiguous block
    //   if the axis is greater than '1', it is not contiguous
    if (GetX(index) > 1) doX = 0;
    if (GetY(index) > 1) doY = 0;
    if (GetZ(index) > 1) doZ = 0;
    
    // If object and index do not share a similar depth on its axis
    //  then memcpy could reach outside the allocated memory.
    // Avoid this by comparing the depth for axis that were sorted.
    if( (!doX && x != GetX(index)) ||
    	(!doY && y != GetY(index)) ||
    	(!doZ && z != GetZ(index)) )
    	return NULL;

    format = V_FORMAT(object);
    dsize = V_DSIZE(object);
    sortListNum = V_DSIZE(index);

    // Alocate the data
    data = calloc(dsize, NBYTES(format));
    
    // sort(object, index=[1,1, ])
    if(doX && doY)
    	for(k = 0; k < sortListNum; k++)
    		for(i = 0; i < x; i++)
    			for(j = 0; j < y; j++) {
    				to 	 = cpos(i, j, k, object);
    				from = cpos(i, j, sortList[k], object);
    				
    				reorg(to, data, from, V_DATA(object), NBYTES(format));
    			}
   	// sort(object, index=[1, ,1])
  	else if(doX && doZ)
  		for(j = 0; j < sortListNum; j++)
  			for(i = 0; i < x; i++)
  				for(k = 0; k < z; k++) {
  					to	 = cpos(i, j, k, object);
  					from = cpos(i, sortList[j], k, object);
  					
  					reorg(to, data, from, V_DATA(object), NBYTES(format));
  				}
  	// sort(object, index=[ ,1,1])
    else if(doY && doZ)
    	for(i = 0; i < sortListNum; i++)
    		for(j = 0; j < y; j++)
    			for(k = 0; k < z; k++) {
    				to	 = cpos(i, j, k, object);
    				from = cpos(sortList[i], j, k, object);
    				
    				reorg(to, data, from, V_DATA(object), NBYTES(format));
    			}
   	// sort(object, index=[1, , ])
    else if(doX)
    	for(cntr = 0; cntr < sortListNum; cntr++)
			for(i = 0; i < x; i++) {
				to	 = cpos(i, cntr % y, cntr / y, object);
				from = cpos(i, sortList[cntr] % y, sortList[cntr] / y, object);
				
				reorg(to, data, from, V_DATA(object), NBYTES(format));
			}
	// sort(object, index=[ ,1, ])
    else if(doY)
    	for(cntr = 0; cntr < sortListNum; cntr++)
			for(j = 0; j < y; j++) {
				to	 = cpos(cntr % x, j, cntr / x, object);
				from = cpos(sortList[cntr] % x, j, sortList[cntr] / x, object);
				
				reorg(to, data, from, V_DATA(object), NBYTES(format));
			}
	// sort(object, index=[ , ,1])
    else if(doZ)
    	for(cntr = 0; cntr < sortListNum; cntr++)
			for(k = 0; k < z; k++) {
				to	 = cpos(cntr % x, cntr / x, k, object);
				from = cpos(sortList[cntr] % x, sortList[cntr] / x, k, object);
				
				reorg(to, data, from, V_DATA(object), NBYTES(format));
			}

    return data;
}

#define L sizeof(long)
//	QuickSort adapted from Kernighan and Ritchie, 'The C Programming Language'
static inline void qswap(void *base, int i, int j, int width, int *sortList)
{
    long tmp;

	// Maintain a sorted list of elements
    tmp = sortList[j];
    sortList[j] = sortList[i];
    sortList[i] = tmp;

	// Swap two elements, given the size of the elements
    for (; width > 0; i++, j++, width -= L) {
		tmp = *(long *) ((long) base + i * width);
		*(long *) ((long) base + i * width) =
			*(long *) ((long) base + j * width);
		*(long *) ((long) base + j * width) = tmp;
    }
}

//	QuickSort adapted from Kernighan and Ritchie, 'The C Programming Language'
void quicksort(void *base, size_t num, size_t width, int (*cmp) (),
	       int *sortList, int left, int right)
{
    int i, last, tmp;

    if (left > right)
		return;

    qswap(base, left, (left + right) / 2, width, sortList);

    last = left;

    for (i = left + 1; i <= right; i++)
		if (cmp(base + i * width, base + left * width) < 0)
			qswap(base, (++last), i, width, sortList);

    qswap(base, left, last, width, sortList);
    quicksort(base, num, width, cmp, sortList, left, last - 1);
    quicksort(base, num, width, cmp, sortList, last + 1, right);
}
