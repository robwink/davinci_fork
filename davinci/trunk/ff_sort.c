#include "parser.h"

void quicksort(void *, size_t, size_t, int (*)(), int *, int, int);
void *reorgByIndex(Var *, Var *, int *);


int cmp_string(const void *a, const void *b)
{
  return strcmp(* (char * const *) a, * (char * const *) b);
}


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




// Move data [from] object, [to] data, using size to expand the offset as needed
#define reorg(to, data, from, object, size)			\
  memcpy(	(void*)((long)data 	 + to 	* size),	\
		(void*)((long)object + from * size),		\
		size);





/*Reorganizes the object, using the index object and its respective sortList.
 * object - Data that is being manipulated
 * index - Source for the sorted sortList
 * sortList  - Sorted sortList, describing where blocks of object are placed
 */
void *reorgByIndex(Var *object, Var *index, int *sortList)
{
  int   t;			// temporary integer
  int   x, y, z;		// x y z for object
  int   i, j, k;		// Loop counters;  i - x, j - y, k - z
  int   doX = 1;		// Determines if
  int   doY = 1;		// X, Y, or Z
  int   doZ = 1;		// axis points remain contiguous
  int   from, to;		// from / to, memory offsets for cpos
  int   format = 0;	        // object's format
  int   dsize = 0;	        // object's size
  int   sortListNum = 0;	// index and sortList's size
  int   cntr = 0;		// sortList counter
  void *data;			// Returned data block

  x = GetX(object);
  y = GetY(object);
  z = GetZ(object);
  
  //Any axis on index with a depth of '1' remains in a contiguous block
  // if the axis is greater than '1', it is not contiguous
  if (GetX(index) > 1) doX = 0;
  if (GetY(index) > 1) doY = 0;
  if (GetZ(index) > 1) doZ = 0;
  
  //If object and index do not share a similar depth on its axis
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
	  to = cpos(i, j, k, object);
	  from = cpos(i, j, sortList[k], object);
	  
	  reorg(to, data, from, V_DATA(object), NBYTES(format));
	}
  // sort(object, index=[1, ,1])
  else if(doX && doZ)
    for(j = 0; j < sortListNum; j++)
      for(i = 0; i < x; i++)
	for(k = 0; k < z; k++) {
	  to = cpos(i, j, k, object);
	  from = cpos(i, sortList[j], k, object);
	  
	  reorg(to, data, from, V_DATA(object), NBYTES(format));
	}
  // sort(object, index=[ ,1,1])
  else if(doY && doZ)
    for(i = 0; i < sortListNum; i++)
      for(j = 0; j < y; j++)
	for(k = 0; k < z; k++) {
	  to = cpos(i, j, k, object);
	  from = cpos(sortList[i], j, k, object);
	  
	  reorg(to, data, from, V_DATA(object), NBYTES(format));
	}
  // sort(object, index=[1, , ])
  else if(doX)
    for(cntr = 0; cntr < sortListNum; cntr++)
      for(i = 0; i < x; i++) {
	to = cpos(i, cntr % y, cntr / y, object);
	from = cpos(i, sortList[cntr] % y, sortList[cntr] / y, object);
	
	reorg(to, data, from, V_DATA(object), NBYTES(format));
      }
  // sort(object, index=[ ,1, ])
  else if(doY)
    for(cntr = 0; cntr < sortListNum; cntr++)
      for(j = 0; j < y; j++) {
	to = cpos(cntr % x, j, cntr / x, object);
	from = cpos(sortList[cntr] % x, j, sortList[cntr] / x, object);
	
	reorg(to, data, from, V_DATA(object), NBYTES(format));
      }
  // sort(object, index=[ , ,1])
  else if(doZ)
    for(cntr = 0; cntr < sortListNum; cntr++)
      for(k = 0; k < z; k++) {
	to = cpos(cntr % x, cntr / x, k, object);
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






static int check_sort_objects(Var *object, Var *byObj)
{
  int  Ox=0, Oy=0, Oz=0;
  int  Bx=0, By=0, Bz=0;
  int  i=0;

  if(V_TYPE(object) == ID_VAL) {
    Ox = GetX(object);
    Oy = GetY(object);
    Oz = GetZ(object);
  }

  if(V_TYPE(object) == ID_TEXT) {
    Ox = 1;
    Oy = V_TEXT(object).Row;
    Oz = 1;
  }

  if(V_TYPE(byObj) == ID_VAL) {
    Bx = GetX(byObj);
    By = GetY(byObj);
    Bz = GetZ(byObj);
  }

  if(V_TYPE(byObj) == ID_TEXT) {
    Bx = 1;
    By = V_TEXT(byObj).Row;
    Bz = 1;
  }

  if((Bx != Ox && Bx != 1) || (By != Oy && By != 1) || 
     (Bz != Oz && Bz != 1) || (Bx*By*Bz == 1)) {
    parse_error("Dimensions of \'object\' and \'by\' are incompatible");
    i = -1;
  }

  return(i);
}





Var *ff_sort(vfuncptr func, Var * arg)
{
  Var   *object = NULL;
  Var   *sortVar = NULL;
  Var   *byObj = NULL;
  Var   *args = NULL;
  Var   *result = NULL;
  char  *oneline = NULL;
  char **tlines = NULL;
  int   *indexList = NULL;
  int    format, sformat;
  int    dsize, ssize;
  int    i, j;
  int    rows = 0;
  float  descend = 0;
  void  *data, *sortSet;
  int  (*cmp) (const void *, const void *);
  
  Alist alist[4];
  alist[0] = make_alist("object",  ID_UNK, NULL, &object);
  alist[1] = make_alist("by",      ID_UNK, NULL, &byObj);
  alist[2] = make_alist("descend", FLOAT,  NULL, &descend);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return (NULL);
  
  if (object == NULL){
    parse_error("\nsort() - Alpha-numeric sorting");
    parse_error("\nSyntax: sort(object=VAL[,by=VAL][,descend=INT])");
    parse_error("Example: sort(a, by=a[2])");
    parse_error("Example: sort(a, by=b.labels)");
    parse_error("Example: sort(a, descend=1)");
    parse_error("  'object'  - a numeric array or text array");
    parse_error("  'by'      - optional object used to sort the data object");
    parse_error("  'descend' - option to order by decreasing value. Default = 0 (ascending)");
    parse_error("01/01/2008");
    return NULL;
  }

  /* Check object and byObj are compatible sizes */
  if(byObj) {
    i = check_sort_objects(object,byObj);
    if(i==-1) 
      return(NULL);
  }

  /* Assign sortVar */
  sortVar = object;
  if(byObj)
    sortVar = byObj;

  if(V_TYPE(sortVar) == ID_VAL) {

    format = V_FORMAT(sortVar);
    dsize = V_DSIZE(sortVar);
  
    switch (format) {
    case BYTE:		cmp = cmp_byte;		break;
    case SHORT:		cmp = cmp_short;	break;
    case INT:		cmp = cmp_int;		break;
    case FLOAT:		cmp = cmp_float;	break;
    case DOUBLE:	cmp = cmp_double;	break;
    }

    if (V_ORG(sortVar) != BSQ) {
      args = create_args(1, NULL, sortVar, NULL, NULL);
      sortVar = V_func("bsq", args);
    }

    /* Create memory for sortSet -- avoid touching the user's data */
    sortSet = calloc(dsize, NBYTES(format));
    memcpy(sortSet, V_DATA(sortVar), dsize * NBYTES(format));

    /* sortList will record the index of items that are moved */
    indexList = calloc(dsize, sizeof(int));
    for (i = 0; i < dsize; i++)
      indexList[i] = i;

    /* Sort and create indexList */
    quicksort(sortSet, dsize, NBYTES(format), cmp, indexList, 0, dsize - 1);
    
    /* flip the bitches? */
    if(descend > 0) {
      for(i=0;i<(dsize/2);i+=1) {
				j = indexList[i];
				indexList[i] = indexList[dsize-i-1];
				indexList[dsize-i-1] = j;
      }
    }

    if(byObj == NULL) {
      //parse_error("Object = ID_VAL, byObj = NULL");
      //parse_error("works");
      result = newVal(V_ORG(object), V_SIZE(object)[0], V_SIZE(object)[1], 
											V_SIZE(object)[2], format, sortSet);
      
      free(indexList);
      return(result);
			
    } else {
			
      free(sortSet);
			
      if(V_TYPE(object) == ID_VAL) {
				//parse_error("Object = ID_VAL, byObj = ID_VAL");
	//parse_error("works");
        data = reorgByIndex(object, byObj, indexList);
	
	result = newVal(V_ORG(object), V_SIZE(object)[0], V_SIZE(object)[1], 
			V_SIZE(object)[2], format, data);
	free(indexList);
	return(result);

      } else if(V_TYPE(object) == ID_TEXT) {
	//parse_error("Object = ID_TEXT, byObj = ID_VAL");
       	//parse_error("works");
	rows = V_TEXT(object).Row;
	tlines = (char **) calloc(rows, sizeof(char *));

	for(i=0; i<rows; i+=1) {
	  j=indexList[i];
	  tlines[i] = strdup(V_TEXT(object).text[j]);
	}
	result = newText(rows, tlines);
	free(indexList);
	return(result);
      }
    }
  } else if (V_TYPE(sortVar) == ID_TEXT) {

    cmp = cmp_string;
    rows = V_TEXT(sortVar).Row;
    tlines = (char **) calloc(rows, sizeof(char *));
    
    for (i = 0; i < rows; i++) {
      tlines[i] = strdup(V_TEXT(sortVar).text[i]);
      if (tlines[i] == NULL)
	tlines[i] = strdup("");
    }

    indexList = calloc(rows, sizeof(int));

    for (i = 0; i < rows; i++)
      indexList[i] = i;
    
    quicksort(tlines, rows, sizeof(char *), cmp_string, indexList, 0, rows - 1);

    /* flip the bitches? */
    if(descend > 0) {
      for(i=0;i<(rows/2);i+=1){
	oneline = tlines[i];
	tlines[i] = tlines[rows-i-1];
	tlines[rows-i-1] = oneline;
	
	j=indexList[i];
	indexList[i] = indexList[rows-i-1];
	indexList[rows-i-1] = j;
      }
    }

    if(byObj == NULL){
      //parse_error("Object = ID_TEXT, byObj = NULL");
      //parse_error("works");
      free(indexList);
      result = newText(rows, tlines);
      return(result);

    } else {
      if(V_TYPE(object) == ID_VAL) {
	//parse_error("Object = ID_VAL, byObj = ID_TEXT");
	//parse_error("works, but has possible memory leak");
	args = newVal(BSQ, 1, rows, 1, INT, indexList);
	data = reorgByIndex(object, args, indexList);
	result = newVal(V_ORG(object), V_SIZE(object)[0], V_SIZE(object)[1], 
			V_SIZE(object)[2], V_FORMAT(object), data);
	//free(indexList);
	return(result);

      } else if(V_TYPE(object) == ID_TEXT) {
	//parse_error("Object = ID_TEXT, byObj = ID_TEXT");
	//parse_error("works");
	rows = V_TEXT(object).Row;

	for(i=0; i<rows; i+=1) {
	  j=indexList[i];
	  tlines[i] = strdup(V_TEXT(object).text[j]);
	}
	result = newText(rows, tlines);
	free(indexList);
	return(result);
      }
    }
    
  } else {
    parse_error("Incompatible object type. Must be TEXT or VAL.");
    return NULL;
  }
}






Var *
ff_unique(vfuncptr func, Var * arg)
{
  Var   *object = NULL;                /* the object */
  Var   *byObj = NULL;                 /* optional object to find unique elements in */
  Var   *searchVar = NULL;
  Var   *result = NULL;
  char **uniqText = NULL;
  float *uniqData = NULL;
  int   *indexList = NULL;
  int    x,y,z;
  int    sx,sy,sz;
  int    i, j, k, l;                   /* general array counters */
  int    uniqElems = 0;
  float  m;
  
  Alist alist[3];
  alist[0] = make_alist("object", ID_UNK, NULL,	&object);
  alist[1] = make_alist("by",     ID_UNK, NULL, &byObj);
  alist[2].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (object == NULL){
    parse_error("\n unique() - Find all unique elements in a VAL or TEXT array.");
    parse_error(" Allows finding all rows that are unique in a");
    parse_error(" single row, column or z-channel.\n");
    parse_error(" Syntax:  unique(object=VAR [,by=VAR])");
    parse_error(" example: c = unique(a, by=b)");
    parse_error(" \'object\' - may be a numeric or TEXT array");
    parse_error(" \'by\' - an optional variable in which to search for");
    parse_error("          unique values and use to sort \'object\'");
    parse_error(" \'by\' must be a 1-D object with compatible dimensions with \'object\'");
    return NULL;
  }

  /* Check object and byObj are compatible sizes */
  if(byObj) {
    i = check_sort_objects(object,byObj);
    if(i==-1) 
      return(NULL);
  }

  searchVar = object;
  if(byObj)
    searchVar = byObj;

  if(V_TYPE(searchVar) == ID_TEXT) {
    sx = 1;
    sy = V_TEXT(searchVar).Row;
    sz = 1;

    indexList = malloc(sizeof(int)*sy);
    for(j=0;j<sy;j+=1)
      indexList[j] = 1;
    
    /* Compare all elements marking duplicates with a 0 in indexList */
    for(j=0;j<sy;j+=1) {
      if(indexList[j] == 1) {
	for(i=j+1;i<sy;i+=1)
	  if(strcmp(V_TEXT(searchVar).text[j],V_TEXT(searchVar).text[i]) == 0)
	    indexList[i] = 0;
      }
    }

  } else if (V_TYPE(searchVar) == ID_VAL) {
    sx = GetX(searchVar);
    sy = GetY(searchVar);
    sz = GetZ(searchVar);

    k=sx*sy*sz;
    indexList = calloc(sizeof(int),k);
    for(i=0;i<(sx*sy*sz);i+=1)
      indexList[i] = 1;

    /* Compare all elements marking duplicates with a 0 in indexList */
    for(k=0;k<sz*sy*sx;k+=1) {
      if(indexList[k] == 1) {
	m = extract_float(searchVar,k);
	for(j=k+1;j<sz*sy*sx;j+=1)
	  if(m == extract_float(searchVar,j))
	    indexList[j] = 0;
      }
    }

    if(!byObj) {
      uniqElems = 0;
      for(i=0;i<sx*sy*sz;i+=1)
	uniqElems=uniqElems+indexList[i];

      uniqData = calloc(uniqElems,sizeof(float));

      j=0;
      for(k=0;k<sz*sy*sx;k++) {
	if(indexList[k] == 1) {
	  uniqData[j] = extract_float(searchVar,k);
	  j++;
	}
      }

      result = newVal(BSQ, 1, uniqElems, 1, FLOAT, uniqData);
      free(indexList);
      return(result);
    }

  } else {
    parse_error("Incompatible variable type! Only TEXT or VAL allowed.");
    return(NULL);
  }

  if(V_TYPE(object) == ID_TEXT) {

    uniqElems = 0;
    for(i=0;i<sy;i+=1)
      uniqElems=uniqElems+indexList[i];

    uniqText = (char **) calloc(k, sizeof(char *));
    
    j=0;
    for(i=0; i<sy; i++) {
      if(indexList[i] == 1) {
	uniqText[j] = strdup(V_TEXT(object).text[i]);
	j++;
      }
    }

    result = newText(uniqElems, uniqText);
    free(indexList);
    return(result);

  } else if (V_TYPE(object) == ID_VAL) {

    k=sx*sy*sz;
    
    if(k/sz != 1 && k/sy != 1 && k/sx != 1) {
      parse_error("Error: \'by\' object must be 1 dimensional.");
      return(NULL);
    }

    x = GetX(object);
    y = GetY(object);
    z = GetZ(object);

    uniqElems = 0;
    for(i=0; i<k; i+=1)
      uniqElems=uniqElems+indexList[i];

    if(sx != 1) {
      uniqData = calloc(uniqElems*y*z, NBYTES(V_FORMAT(object)));

      l=-1;
      for(i=0;i<x;i++) {
	if(indexList[i] == 1) {
	  l+=1;
	  for(k=0;k<z;k++) {
	    for(j=0;j<y;j++) {  
	      uniqData[k*y*uniqElems + j*uniqElems + l] = extract_float(object,cpos(i,j,k,object));
	    }
	  }
	}
      }
      result = newVal(BSQ,uniqElems,y,z,FLOAT,uniqData);
      return(result);

    } else if(sy != 1) {
      uniqData = calloc(uniqElems*x*z, NBYTES(V_FORMAT(object)));

      l=-1;
      for(j=0;j<y;j++) {
	if(indexList[j] == 1) {
	  l+=1;
	  for(k=0;k<z;k++) {
	    for(i=0;i<x;i++) {
	      uniqData[k*uniqElems*x + l*x + i] = extract_float(object,cpos(i,j,k,object));
	    }
	  }
	}
      }
      result = newVal(BSQ,x,uniqElems,z,FLOAT,uniqData);
      return(result);

    } else if(sz != 1) {
      uniqData = calloc(uniqElems*x*z, NBYTES(V_FORMAT(object)));
      
      l=-1;
      for(k=0;k<z;k++) {
	if(indexList[k] == 1) {
	  l+=1;
	  for(j=0;j<y;j++) {
	    for(i=0;i<x;i++) {
	      uniqData[l*y*x + j*x + i] = extract_float(object,cpos(i,j,k,object));
	    }
	  }
	}
      }
      result = newVal(BSQ,x,y,uniqElems,FLOAT,uniqData);
      return(result);
    }
  }
}




Var *ff_sort_old(vfuncptr func, Var * arg)
{
  Var   *object = NULL;	     // Value of current object 
  Var   *rVal = NULL;	     // Returned value/structure
  Var   *index = NULL;	     // Sorting index from user
  Var   *args = NULL;
  int   *sortList = NULL;    // Sorted list (from index)
  int    format, dsize;
  int    sformat, ssize, i;
  void  *data, *sortSet;
  int  (*cmp) (const void *, const void *);
  
  Alist alist[3];
  alist[0] = make_alist("object", ID_VAL, NULL, &object);
  alist[1] = make_alist("index",  ID_VAL, NULL, &index);
  alist[2].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return (NULL);
  
  if (object == NULL) {
    parse_error("Scott Gonyea's version of sort (still)");
    return (NULL);
  }

  format = V_FORMAT(object);
  dsize = V_DSIZE(object);
  
  switch (format) {
  case BYTE:		cmp = cmp_byte;		break;
  case SHORT:		cmp = cmp_short;	break;
  case INT:		cmp = cmp_int;		break;
  case FLOAT:		cmp = cmp_float;	break;
  case DOUBLE:	        cmp = cmp_double;	break;
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
    
    rVal = newVal(V_ORG(rVal), V_SIZE(rVal)[0], V_SIZE(rVal)[1], 
		  V_SIZE(rVal)[2], format, data);
    
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
  quicksort(sortSet, ssize, NBYTES(sformat), cmp, sortList, 0, ssize - 1);
  
  // Reorganize the object by its respective index
  data = reorgByIndex(object, index, sortList);
  
  rVal = newVal(V_ORG(object), V_SIZE(object)[0], V_SIZE(object)[1], 
		V_SIZE(object)[2], format, data);
  
  // Release memory and return
  free(sortSet);	sortSet  = NULL;
  free(sortList);	sortList = NULL;
  
  return rVal;
}



