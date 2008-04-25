#include "parser.h"



//This is probably overriden in config.h
#ifndef GPLOT_CMD
#define GPLOT_CMD "gnuplot -xrm 'gnuplot*background:black' -xrm 'gnuplot*axisColor:white' -xrm 'gnuplot*textColor:white' -xrm 'gnuplot*borderColor:white'"
#endif

#ifndef GPLOT_VERBOSE
#define GPLOT_VERBOSE 1
#endif

#ifdef GPLOT_CMD
extern FILE *pfp;
#endif

static char *make_gnuplot_file_path(char *dir);
static int name_check(char *actual_input, char *name, int limit);
static int send2plot(char *s);
static void findAxis(char *R, Var * Obj);
static int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, char *dir);



/* List of keywords recognized by vplot(). */
static char *known_kw[] = {
  "Xaxis",       // (X)  a common, or default Xaxis to be used for all data objects
  "Axis",        // (A)  a common, or default Axis to be used for all data objects
  "xaxis",       // (x)  an object-specific xaxis designated after it's data object
  "axis",        // (a)  an object-specific axis designated after it's data object
  "label",       // (l)  an o-s label
  "color",       // (c)  an o-s line color
  "weight",      // (w)  an o-s line weight
  "style",       // (st) an o-s line style
  "separate",    // (se) an o-s separate command
  "ignore",      // (ig) an o-s ignore value
  "iabove",      // (ia) an o-s ignore-above value
  "ibelow",      // (ib) an o-s ignore-below value
  "errxbars",    //      a flag to use x-low,x-high ordered error bars
  "errxdelta",   //      a flag to use x-delta error bars
  "errybars",    //      a flag to use y-low,y-high ordered error bars
  "errydelta",   //      a flag to use y-delta error bars
  "errxybars",   //      a flag to use x-low,x-high,y-low,y-high ordered error bars
  "eerxydelta",  //      a flag to use x-delta,y-delta error bars
  "dir",         // (d)  a directory in which to write the vplot command and files
  "gcommand",    // (g)  a flag to print the full Gnuplot command to the screen
  NULL
};



// list of line styles in gnuplot
static char *line_styles[] = {
  "lines",
  "linespoints",
  "dots",
  "points",
  "steps",
  "impulses",
  "boxes",
  NULL
};




static char *make_gnuplot_file_path(char *dir)
{
  int fd;
  unsigned int uretval;
  char pathbuf[256];
  char *tmpdir = getenv("TMPDIR");
  
  if (tmpdir == NULL) tmpdir = "/tmp";
  if (dir != NULL) tmpdir = dir;

  sprintf(pathbuf, "%s/XXXXXX", tmpdir);
  
  /** To handle racing issues, since mkstemp does not exist in MINGW **/
#ifdef __MINGW32__
  uretval = GetTempFileName(tmpdir, // directory for tmp files
			    "dv",        // temp file name prefix 
			    0,            // create unique name 
			    pathbuf);  // buffer for name 
  if (uretval == 0){
    return(NULL);
  }
#else
  fd = mkstemp(pathbuf);
#endif
  
  if (fd == -1) {
    
    return(NULL);
  }
  close(fd);
  return(strdup(pathbuf));
}




// Opens a single session of gnuplot and sends it a plot command
static int send2plot(char *s)
{
#ifdef GPLOT_CMD
  char *gplot_cmd;
  
  if (pfp == NULL) {
    if (getenv("GPLOT_CMD") != NULL) {
      gplot_cmd = strdup(getenv("GPLOT_CMD"));
    } else {
      gplot_cmd = GPLOT_CMD;
    }
    if ((pfp = popen(gplot_cmd, "w")) == NULL) {
      fprintf(stderr, "Unable to open gplot.\n");
      return (0);
    }
    send2plot("set style data linespoints\n");
    send2plot("set parametric\n");
    send2plot("set mouse\n");
  }

  if (write(fileno(pfp), s, strlen(s)) < 0) {
    pfp = NULL;
    send2plot(s);
  }
  write(fileno(pfp), "\n", 1);
#endif
  return (1);
}






static void findAxis(char *R, Var *Obj)
{
  int axis = 0;
  
  if (GetSamples(V_SIZE(Obj), V_ORG(Obj)) == 1)
    axis |= 1;
  if (GetLines(V_SIZE(Obj), V_ORG(Obj)) == 1)
    axis |= 2;
  if (GetBands(V_SIZE(Obj), V_ORG(Obj)) == 1)
    axis |= 4;
  
  
  if (axis == 7)
    *R = '\0';
  
  if (axis == 6)
    *R = 'X';
  else if (axis == 5)
    *R = 'Y';
  else if (axis == 3)
    *R = 'Z';
  else
    *R = 'Y';
}





static int name_check(char *actual_input, char *name, int limit) {

  int  namelen = 0;
  int  inputlen = 0;
  int  i,j;

  namelen = strlen(name);
  inputlen = strlen(actual_input);
  if(inputlen < limit) return(-2);

  if(inputlen > namelen) return(-1);
  if(inputlen == namelen) return(strcmp(actual_input,name));
  if(inputlen < namelen) return(strncmp(actual_input,name,inputlen));
}




      

Var *
ff_vplot(vfuncptr func, Var *arg)
{

  int      ac;                        // argument count
  Var    **av;                        // argument Var
  Var     *v = NULL;
  Var     *Xaxis = NULL;
  char    *Axis = NULL;
  char    *dir = NULL;
  int      i,j,k;                     // loop index
  char    *command = NULL;            // command buffer sent to Gnuplot
  int      start_ct = 0;
  int      end_ct = 0;
  char     CommandBuffer[8192] = { 0 };
  int      plotchoppererror = 0;
  int      Onum = 0;
  int      gcommand = 0;

  make_args(&ac, &av, func, arg);

  if (ac == 1) {
    parse_error("    plot() - A portal to Gnuplot plotting.\n");
    parse_error(" Syntax: plot(VAL");
    parse_error("         [, Xaxis = VAL]        (Xaxis to use on all plots)");
    parse_error("         [, Axis = x|y|z|X|Y|Z] (Axis to use on all plots)");
    parse_error("         [, xaxis = VAL]        (axis to use on one plot)");
    parse_error("         [, axis = x|y|z|X|Y|Z] (xaxis to use on one plot)");
    parse_error("         [, ignore = VAL]       (value to skip while plotting)");
    parse_error("         [, iabove = VAL]       (ignore above)");
    parse_error("         [, ibelow = VAL]       (ignore below)");
    parse_error("         [, separate = BOOL]    (default = 0)");
    parse_error("         [, label = STRING]     (default = \'vector #\')");
    parse_error("         [, weight = INT]       (thickness of plot)");
    parse_error("         [, color = INT]        (color of plot)");
    parse_error("         [, style = lines|points|dots|linespoints|steps|boxes]");
    parse_error("         [, dir = STRING]       (path to store files)");
    parse_error("         [, gcommand = BOOL]    (print Gnuplot command to screen])");
    parse_error(" or");
    parse_error("         plot(string) - to send string commands to Gnuplot\n");
    return(NULL);
  }

  // start the command string
  strcpy(CommandBuffer, "plot ");

  if (ac == 2 && V_TYPE(av[1]) == ID_STRING) {
    strcpy(CommandBuffer, V_STRING(av[1]));
    send2plot(CommandBuffer);
    return(NULL);
  }

  /* Loop through the entire argument list extracting Xaxis, Axis, **
  ** dir and verbose if they exist.                                */
  i=1;
  while (i<ac) {
    if(V_TYPE(av[i]) != ID_KEYWORD && V_TYPE(av[i]) != ID_STRING) {
      if(start_ct == 0)
	start_ct = i;
      else if (end_ct == 0)
	end_ct = i-1;
    }
    i+=1;
  }

  if(end_ct == 0)
    end_ct = ac-1;

  for(i=1; i<ac; i+=1) {

    if(V_TYPE(av[i]) == ID_KEYWORD) {
      if(name_check(V_NAME(av[i]),"Xaxis",1) == 0) {
	if(V_KEYVAL(av[i]) != NULL){
	  Xaxis = eval(V_KEYVAL(av[i]));
	  if(V_TYPE(Xaxis) == ID_STRING || 
	     V_TYPE(Xaxis) == ID_STRUCT || 
	     V_TYPE(Xaxis) == ID_TEXT) {
	    parse_error("Invalid Xaxis type. Numeric arrays only!");
	    return(NULL);
	  }
	}
      }

      if(name_check(V_NAME(av[i]),"Axis",1) == 0) {
	v = V_KEYVAL(av[i]);
	if(!strcasecmp(V_NAME(v),"x") || 
	   !strcasecmp(V_NAME(v),"y") ||
	   !strcasecmp(V_NAME(v),"z")) {
	  Axis = strdup(V_NAME(v));
	} else {
	  parse_error("Invalid Axis designation. x y z only.");
	  return(NULL);
	}
      }

      if(name_check(V_NAME(av[i]),"dir",1) == 0) {
	if(V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_STRING){
	  dir = strdup(V_STRING(V_KEYVAL(av[i])));
	}
      }

      if(name_check(V_NAME(av[i]),"gcommand",1) == 0) {
	gcommand = 1;
      }
    }
  }

  /* Loop through the elements from start_ct to end_ct dealing with each **
  ** plot object independently. plot_chopper() will create a file for    **
  ** each object and add to the CommandBuffer.                           */

  j=0;

  while(end_ct < ac && start_ct < ac) {

    Onum += 1;
    if(j==1) {
      sprintf(CommandBuffer + strlen(CommandBuffer),",");
      j = 0;
    }

    plotchoppererror = plot_chopper(av, start_ct, end_ct, Onum, CommandBuffer, Xaxis, Axis, dir);
    j+=1;

    // There was an error somewhere in plot_chopper()
    if(plotchoppererror == 1)
      return(NULL);

    // plot_chopper() completed without error. Find the next subset of
    // the command buffer significant to the next plot object
    else {
      start_ct = end_ct + 1;
      end_ct = ac-1;
      i = start_ct + 1;
      while (i<ac) {
	if(V_TYPE(av[i]) != ID_KEYWORD && V_TYPE(av[i]) != ID_STRING) {
	  end_ct = i-1;
	}
	i++;
      }
    }
  }

  if (strlen(CommandBuffer) > 5) {
    sprintf(CommandBuffer + strlen(CommandBuffer),"\n");
    send2plot(CommandBuffer);
  }

  if(gcommand != 0)
    printf("\n%s\n",CommandBuffer);

  free(av);
  return(NULL);

}






int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, char *dir)
{
  
  /* The code of plot_chopper is the canibalized descendent of xplot()*/
  
  Var   *s, *v;
  int    count = 0;
  Var   *xaxis_i = NULL;
  char  *axis_i = NULL;
  int    Mode[3];
  int    Ord[3];
  int    XOrd[3];
  int    CE[3];
  int    xFlag = 0;
  int    obj_index;
  float *x = NULL, *y = NULL;
  int    i, j, k;
  int    idx;
  int    sep = 0;
  char   axs;
  int    argcount = 0;
  FILE  *fp = NULL;
  char  *fname = NULL;
  float  ignore;
  float  ignore_above;
  float  ignore_below;
  int    iflag = 0;
  int    iaflag = 0;
  int    ibflag = 0;
  int    color = -10;
  int    weight = -10;
  char  *style = NULL;
  char  *label = NULL;
  int    vnum = 1;

  /***********************************************
   ** Section A: Keyword Extraction             **
   **                                           **
   ** Loop through the limited argument list    **
   ** extracting all object-specific keywords   **
   ***********************************************/

  for (i = start_ct; i <= end_ct; i++) {
    
    //this should never happen, but a good catch nonetheless
    if (av[i] == NULL)
      continue;

    //av[i] isn't a keyword, but it is a STRUCT
    //so pull out the keyword elements
    if(V_TYPE(av[i]) == ID_UNK) {
      s = eval(av[i]);
      if(V_TYPE(s) == ID_STRUCT){

	find_struct(s,"xaxis",&v);
	if(v!=NULL) {
	  xaxis_i = v;
	  v = NULL;
	}

	find_struct(s,"axis",&v);
	if(v!=NULL) {
	  if (V_TYPE(v) == ID_STRING)
	    axis_i = V_STRING(v);
	  else
	    axis_i = V_NAME(v);

	  v = NULL;
	}

	find_struct(s,"Xaxis",&v);
	if(v!=NULL) {
	  Xaxis = v;
	  v = NULL;
	}

	find_struct(s,"Axis",&v);
	if(v!=NULL) {
	  if (V_TYPE(v) == ID_STRING)
	    Axis = V_STRING(v);
	  else
	    Axis = V_NAME(v);

	  v = NULL;
	}

	find_struct(s,"label",&v);
	if(v!=NULL) {
	  label = strdup(V_STRING(v));
	  v = NULL;
	}

	//here's for stupid emcal
	find_struct(s,"sample_name",&v);
	if(v!=NULL) {
	  label = strdup(V_STRING(v));
	  v = NULL;
	}

	find_struct(s,"weight",&v);
	if(v!=NULL) {
	  weight = extract_int(v,0);
	  v = NULL;
	}

	find_struct(s,"style",&v);
	if(v!=NULL) {
	  style = V_NAME(v);
	  v = NULL;
	}	

	find_struct(s,"color",&v);
	if(v!=NULL) {
	  color = extract_int(v,0);
	  v = NULL;
	}

	find_struct(s,"separate",&v);
	if(v!=NULL) {
	  sep = 1;
	  v = NULL;
	}

	find_struct(s,"ignore",&v);
	if(v!=NULL) {
	  ignore = extract_float(v,0);
	  v = NULL;
	}

	find_struct(s,"iabove",&v);
	if(v!=NULL) {
	  ignore_above = extract_float(v,0);
	  v = NULL;
	}

	find_struct(s,"ibelow",&v);
	if(v!=NULL) {
	  ignore_below = extract_float(v,0);
	  v = NULL;
	}
      }
    }
    
    if (V_TYPE(av[i]) == ID_KEYWORD) {
      
      if (!(name_check(av[i]->name, "Xaxis", 1))) {
      } else if (!(name_check(av[i]->name, "Axis", 1))) {
      } else if(!(name_check(av[i]->name, "gcommand", 1))) {
      } else if(!(name_check(av[i]->name, "dir", 1))) {

      } else if (!(name_check(av[i]->name, "xaxis", 1))) {
	xaxis_i = eval(V_KEYVAL(av[i]));
	if (xaxis_i == NULL) {
	  parse_error("Variable not found: \'xaxis\'");
	  free(av);
	  return (1);
	}
	
      } else if (!(name_check(av[i]->name, "axis",1))) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING)
	  axis_i = V_STRING(v);
	else
	  axis_i = V_NAME(v);
	
      } else if (!(name_check(av[i]->name, "separate",2))) {
	sep = 1;
	
      } else if (!(name_check(av[i]->name, "ignore",2))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  ignore = extract_float(V_KEYVAL(av[i]), 0);
	  iflag = 1;
	} else {
	  parse_error("Bad value for ignore");
	  free(av);
	  return (1);
	}
	
      } else if (!(name_check(av[i]->name, "iabove", 2))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  ignore_above = extract_float(V_KEYVAL(av[i]),0);
	  iaflag = 1;
	} else {
	  parse_error("Invalid designation for iabove");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "ibelow", 2))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  ignore_below = extract_float(V_KEYVAL(av[i]),0);
	  ibflag = 1;
	} else {
	  parse_error("Invalid designation for ibelow");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "color", 1))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_FORMAT(av[i]) == FLOAT || V_FORMAT(av[i]) == DOUBLE)
	    color = (int)extract_float(V_KEYVAL(av[i]),0);
	  else
	    color = extract_int(V_KEYVAL(av[i]),0);
	} else {
	  parse_error("Invalid designation for color");
	  parse_error("Continuing plot with default color");
	}
	if(color < -1) {
	  parse_error("color must be an integer value -1 or greater");
	  return(1);
	}
	
      } else if (!(name_check(av[i]->name, "weight", 1))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  weight = (int)extract_float(V_KEYVAL(av[i]),0);
	} else {
	  parse_error("Invalid designation for line weight");
	  parse_error("Continuing plot with default weight");
	}
	
      } else if (!(name_check(av[i]->name, "style", 2))) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING){
	  style = V_STRING(v);
	} else {
	  style = strdup(V_NAME(v));
	}

	if(name_check(style,"lines",1) && name_check(style,"linespoints",6) &&
	   name_check(style,"dots",1) && name_check(style,"points",1)) {
	  parse_error("Invalid designation for style");
	  parse_error("Only: lines, points, dots, linespoints");
	  parse_error("Continuing plot with default style");
	  style = NULL;
	}
	
      } else if (!(name_check(av[i]->name, "label", 1))) {      
	//possibility uno for label designation
	if(V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_STRING){
	  label = strdup(V_STRING(V_KEYVAL(av[i])));
	} else {
	  parse_error("Illegal label designation. STRINGS only.");
	  return(1);
	}

	//this stupid keyname is to take care of fucking emcal
      } else if (!(name_check(av[i]->name, "sample_name", 2))) {      
	//possibility uno for label designation
	if(V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_STRING){
	  label = strdup(V_STRING(V_KEYVAL(av[i])));
	} else {
	  parse_error("Illegal label designation. STRINGS only.");
	  return(1);
	}

      } else {
	parse_error("Illegal keyword %s\n", av[i]->name);
	free(av);
	return (1);
      }
      argcount++;
    }

    if(V_TYPE(av[i]) == ID_STRING) {
      //possibility dos for label designation 
      label = strdup(V_STRING(av[i]));
    }
  }

  
  /************************************************
   **                END Section A               **
   ************************************************/

  /***********************************************
   ** Section B: Axis and Xaxis                 **
   **                                           **
   ** Find axis and xaxis if they were given    **
   ** if not, create them appropriately         **
   ** I'm going to add code to convert Axis into**
   ** axis and Xaxis into xaxis as defaults     **
   ***********************************************/

  
  // Here is the case where axis wasn't given
  if (axis_i == NULL) {

    if(Axis != NULL)
      axis_i = Axis;

    else {
      s  = av[start_ct];
      findAxis(&axs, s);
      axis_i = strdup(&axs);
    }
  }

  switch (*axis_i) {
  case 'X':
  case 'x':
    Mode[0] = 0;
    Mode[1] = 1;
    Mode[2] = 2;
    break;
  case 'Y':
  case 'y':
    Mode[0] = 1;
    Mode[1] = 0;
    Mode[2] = 2;
    break;
  case 'Z':
  case 'z':
    Mode[0] = 2;
    Mode[1] = 0;
    Mode[2] = 1;
    break;
  default:
    Mode[0] = 1;
    Mode[1] = 0;
    Mode[2] = 2;
    break;
  }


  // Find or set xaxis_i
  if (xaxis_i == NULL && Xaxis != NULL)
    xaxis_i = Xaxis;
  
  if (xaxis_i != NULL) {
    XOrd[0] = GetSamples(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[1] = GetLines(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[2] = GetBands(V_SIZE(xaxis_i), V_ORG(xaxis_i));
  }

  if (xaxis_i != NULL)
    xFlag = 1;


  /************************************************
   **                END Section B               **
   ************************************************/

  /***********************************************
   ** Section C: File and command creation      **
   ***********************************************/

  s = av[start_ct];

  if (V_TYPE(s) == ID_UNK) {

    if ((s = eval(s)) == NULL) {
      label = V_NAME(s);
      parse_error("%s does not exist\n", label);
      free(av);
      return (1);
    }
  }

  // Let's get that standard struct data out of that struct!
  if(V_TYPE(s) == ID_STRUCT){
    find_struct(s,"data",&v);
    if(v!=NULL) {
      s = v;  //kill all other elements and make 's' point to the data
      v = NULL;
    }
  }

  // Hooray! We have a possible plotable object
  if(V_TYPE(s) == ID_VAL){
    if (!(sep)) {
      fname = make_gnuplot_file_path(dir);
      if (fname == NULL || (fp = fopen(fname, "w")) == NULL) {
	parse_error("Unable to open temp file");
	if (fname)
	  free(fname);
	free(av);
	return(NULL);
      }
    }
    if ((v = eval(s)) == NULL)
      v = s;
    
    Ord[0] = GetSamples(V_SIZE(v), V_ORG(v));
    Ord[1] = GetLines(V_SIZE(v), V_ORG(v));
    Ord[2] = GetBands(V_SIZE(v), V_ORG(v));
    
    //An xaxis was designated, one way or another.
    //Check the xaxis against the object.
    if (xFlag) {
      if (XOrd[Mode[0]] != Ord[Mode[0]]) {
	parse_error("Given x-axis doesn't agree with given data set");
	free(av);
	return (1);
	
      } else if ((XOrd[1] != 1 && XOrd[2] != 1) && 
		 (XOrd[1] != Ord[Mode[1]] && XOrd[2] != Ord[Mode[2]])) {
	parse_error("Given x-axis doesn't agree with given data set");
	free(av);
	return (1);
      }
    }
    
    x = calloc(Ord[Mode[0]], sizeof(float));
    y = calloc(Ord[Mode[0]], sizeof(float));
    
    for (i = 0; i < Ord[Mode[2]]; i++) {
      for (j = 0; j < Ord[Mode[1]]; j++) {
	
	if (sep) {
	  fname = make_temp_file_path(dir);
	  if (fname == NULL || (fp = fopen(fname, "w")) == NULL) {
	    parse_error("Unable to open temp file");
	    if (fname) {
	      free(fname);
	    }
	    free(av);
	    return (1);
	  }
	}

	for (k = 0; k < Ord[Mode[0]]; k++) {
	  CE[Mode[2]] = i;
	  CE[Mode[1]] = j;
	  CE[Mode[0]] = k;
	  obj_index = cpos(CE[0], CE[1], CE[2], v);
	  switch (V_FORMAT(v)) {
	  case BYTE:
	  case SHORT:
	  case INT:
	    y[k] = (float) extract_int(v, obj_index);
	  case FLOAT:
	  case DOUBLE:
	    y[k] = extract_float(v, obj_index);
	  }
	  
	  if (xFlag) {
	    switch (V_FORMAT(v)) {
	    case BYTE:
	    case SHORT:
	    case INT:
	      x[k] = (float)extract_int(xaxis_i, rpos(obj_index, v, xaxis_i));
	    case FLOAT:
	    case DOUBLE:
	      x[k] = extract_float(xaxis_i, rpos(obj_index, v, xaxis_i));
	    }
	  } else {
	    x[k] = (float) k;
	  }
	  
	  //Here we finally fucking handle the glorious ignore values!
	  if ((iaflag != 0 && y[k] > ignore_above) ||
	      (ibflag != 0 && y[k] < ignore_below) ||
	      (iflag != 0 && y[k] == ignore)) {
	    fprintf(fp, "\n");
	  } else {
	    fprintf(fp, "%g\t %g\n", x[k], y[k]);
	  }
	}
	
	// Close the file if each vector is separated
	if (sep) {
	  fclose(fp);
	  if (count++)
	    strcat(CommandBuffer, ",");
	  sprintf(CommandBuffer + strlen(CommandBuffer), "'%s'", fname);

	  // Properly title the fucking object and vector
	  if (label != NULL) 
	    sprintf(CommandBuffer + strlen(CommandBuffer), " title '%s vector %d'", label,vnum);
	  else 
	    sprintf(CommandBuffer + strlen(CommandBuffer), " title 'Obj %d vector %d'",Onum,vnum);

	  vnum+=1;

	  // Add weight
	  if(weight > 0) 
	    sprintf(CommandBuffer + strlen(CommandBuffer), " lw %d",weight);
	  
	  // Add style
	  if(style != NULL)
	    sprintf(CommandBuffer + strlen(CommandBuffer), " with %s",style);

	  free(fname);
	} else
	  fprintf(fp, "\n");
      }
    }
    free(x);
    free(y);
    if (!(sep)) {
      fclose(fp);
      if (count++) {
	strcat(CommandBuffer, ",");
      }
      sprintf(CommandBuffer + strlen(CommandBuffer), "'%s'", fname);

      // Properly title the object
      if (label != NULL) 
	sprintf(CommandBuffer + strlen(CommandBuffer), " title '%s'", label);
      else
	sprintf(CommandBuffer + strlen(CommandBuffer), " title 'Obj %d'",Onum);

      // Add color
      if(color > -2)
	sprintf(CommandBuffer + strlen(CommandBuffer), " lt %d",color);

      // Add weight
      if(weight > 0) 
	sprintf(CommandBuffer + strlen(CommandBuffer), " lw %d",weight);
      
      // Add style
      if(style != NULL)
	sprintf(CommandBuffer + strlen(CommandBuffer), " with %s",style);
      
      free(fname);
      
    }
  }
  
  return(0);
}
