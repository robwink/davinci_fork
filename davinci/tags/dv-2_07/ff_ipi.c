#include "parser.h"

static Var *do_ipi(Var *coords_array, Var *values_array){
  float coords[3]; /* more useable form of coordinate list     */
  float values[3]; /* more useable form of values list         */
  float result;    /* the coordinate of the lowest value       */
  int   i;         /* the ubiquituous i var for loops          */

  /* translate information from davinci Var structure to the array */
  for(i=0; i<3; i++){
    coords[i] = extract_float(coords_array,cpos(i,0,0,coords_array));
    values[i] = extract_float(values_array,cpos(i,0,0,values_array));
  }
  
  /* inverse parabolic interpolation calculation */
  result = coords[1] - 
    .5*(pow(coords[1]-coords[0],2)*(values[1]-values[2]) - pow(coords[1]-coords[2],2)*(values[1]-values[0])) /
    ((coords[1]-coords[0])*(values[1]-values[2]) - (coords[1]-coords[2])*(values[1]-values[0]));
  
  return(newFloat(result));
}



Var *ff_ipi(vfuncptr func, Var * arg){
  Var *coords_array = NULL; /* list of coordinates                                  */
  Var *values_array = NULL; /* list of values of f(x) where x is supplied by coords */
  
  Alist alist[3];
  alist[0] = make_alist("coordinates",  ID_VAL,   NULL,     &coords_array);
  alist[1] = make_alist("values",       ID_VAL,   NULL,     &values_array);
  alist[2].name = NULL;
  
  /* immediately exit function if the argument list is empty */
  if(parse_args(func, arg, alist) == 0) return(NULL);
  
  /* if no coordinates and/or values passed into the argument list */
  if((coords_array == NULL)||(values_array == NULL)){
    parse_error("\nNo data specified\n");
    return(NULL);
  }
  
  return(do_ipi(coords_array,values_array));
}
