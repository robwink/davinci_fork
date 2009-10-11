#include "parser.h"

//This is probably overriden in config.h
#ifndef GPLOT_CMD
#define GPLOT_CMD "gnuplot -xrm 'gnuplot*background:black' -xrm 'gnuplot*axisColor:white' -xrm 'gnuplot*textColor:white' -xrm 'gnuplot*borderColor:white'"
#endif

#ifdef GPLOT_CMD
extern FILE *pfp;
#endif

static int name_check(const char *actual_input, const char *name, int limit);
static void findAxis(char *R, Var * Obj, int flag);
static int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, float *globalNums, char *Style, char *dir);
static int handle_errorbars(Var *errorb, Var *v, int *Mode, char *style, int Onum);
static int *make_errorbar_indices(Var *v, int *Mode, int *CE, Var *errorb);


// Opens a single session of gnuplot and sends it a plot command
// NOTE(gorelick): Why not use send_to_plot?
int send2plot(const char *s)
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
    send2plot("set style data lines\n");
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


static void findAxis(char *R, Var *Obj, int flag)
{
  int axis = 0;
  int x, y, z;

  x = GetX(Obj);
  y = GetY(Obj);
  z = GetZ(Obj);

  if (x == 1)
    axis |= 1;
  if (y == 1)
    axis |= 2;
  if (z == 1)
    axis |= 4;

  if (axis == 7)
    *R = '\0';

  if (axis == 6)
    *R = 'X';
  else if (axis == 5)
    *R = 'Y';
  else if (axis == 3)
    *R = 'Z';
  else if(flag){
    *R = 'Y';
    if(x>y && x>z)
      *R = 'X';
    else if(z>x && z>y)
      *R = 'Z';
  } else
    *R = '\0';
}





static int name_check(const char *actual_input, const char *name, int limit) {

  int  namelen = 0;
  int  inputlen = 0;

  if(actual_input == NULL)
    return(-3);

  namelen = strlen(name);
  inputlen = strlen(actual_input);
  if(inputlen < limit) return(-2);

  if(inputlen > namelen) return(-1);
  else if(inputlen == namelen) return(strcmp(actual_input,name));
  else return(strncmp(actual_input,name,inputlen));
}


Var *
ff_vplot(vfuncptr func, Var *arg)
{

  int      ac;                          // argument count
  Var    **av;                          // argument Var
  Var     *v = NULL;
  Var     *Xaxis = NULL;
  char    *Axis = NULL;
  char    *Style = NULL;
  char    *dir = NULL;
  int      i,j;                       // loop index
  int      start_ct = 0;
  int      end_ct = 0;
  char     CommandBuffer[8192] = { 0 }; //command buffer sent to Gnuplot
  int      plotchoppererror = 0;
  int      Onum = 0;
  int      gcommand = 0;
  float   *globalNums = NULL;

  make_args(&ac, &av, func, arg);

  if (ac == 1) {
    parse_error("\nType plot(?) for help");
    return(NULL);
  }

  /* 9/14/2008 version:
    -fixed a segmentation fault involving calling axis with an improperly called structure.
    -added xerrorlines, yerrorlines, xyerrorlines to permissible styles

     Neditation version:
    -Fixed a bug with errorbars that was pretty important.  All code changed was in make_errorbar_indices()
    -Fixed a bug with dir.  Wasn't calling the function make_temp_file_path_in_dir(char *).
    -Altered Style so that it accepts errorbar options for all vectors.
  */

  // start the command string
  strcpy(CommandBuffer, "plot ");

  if (ac == 2 && V_TYPE(av[1]) == ID_STRING) {
    strcpy(CommandBuffer, V_STRING(av[1]));
    send2plot(CommandBuffer);
    return(NULL);
  }

  // Set up globalNums array.  It is a 16 element float array to hold
  // flags and values for numeric global keywords
  globalNums = calloc(16, sizeof(float));

  /* Loop through the entire argument list extracting Xaxis, Axis, **
  ** dir and gcommand if they exist.                               */
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

      if(!name_check(V_NAME(av[i]),"Xaxis",1)) {

	Xaxis = eval(V_KEYVAL(av[i]));
	if (Xaxis == NULL) {
	  parse_error("Variable not found: \'Xaxis\'");
	  free(av);
	  return(NULL);
	}
	if (V_TYPE(Xaxis) != ID_VAL) {
	  parse_error("Xaxis must be a numeric array.\n");
	  free(av);
	  return(NULL);
	}

      } else if(!name_check(V_NAME(av[i]),"Axis",1)) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING)
	  Axis = strdup(V_STRING(v));
	else
	  Axis = strdup(V_NAME(v));
	
	if(Axis != NULL &&
	   strcasecmp(Axis,"x") &&
	   strcasecmp(Axis,"y") &&
	   strcasecmp(Axis,"z")) {
	  parse_error("Invalid Axis designation. x y z only.");
	  return(NULL);
	}

      } else if (!name_check(V_NAME(av[i]), "Ignore", 2)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[0] = 1;
	      globalNums[1] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[0] = 1;
	      globalNums[1] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Ignore");
	    parse_error("Keyword 'Ignore' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Ignore");
	  return(NULL);
	}

      } else if (!name_check(V_NAME(av[i]), "Iabove", 2)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[2] = 1;
	      globalNums[3] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[2] = 1;
	      globalNums[3] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Iabove");
	    parse_error("Keyword 'Iabove' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Iabove");
	  return(NULL);
	}
	
      } else if (!name_check(V_NAME(av[i]), "Ibelow", 2)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[4] = 1;
	      globalNums[5] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[4] = 1;
	      globalNums[5] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Ibelow");
	    parse_error("Keyword 'Ibelow' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Ibelow");
	  return(NULL);
	}

      } else if (!name_check(V_NAME(av[i]), "Ixabove", 3)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[6] = 1;
	      globalNums[7] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[6] = 1;
	      globalNums[7] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Ixabove");
	    parse_error("Keyword 'Ixabove' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Value for Ixabove does not exist");
	  return(NULL);
	}
	
      } else if (!name_check(V_NAME(av[i]), "Ixbelow", 3)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[8] = 1;
	      globalNums[9] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[8] = 1;
	      globalNums[9] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Ixbelow");
	    parse_error("Keyword 'Ixbelow' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Ignore");
	  return(NULL);
	}

      } else if (!name_check(V_NAME(av[i]), "Color", 1)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  v = V_KEYVAL(av[i]);
	  if(V_TYPE(v) == ID_VAL) {
	    globalNums[10] = 1;
	    globalNums[11] = (int)extract_float(v,0);
	  } else {
	    parse_error("Invalid designation for Color");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Color");
	  return(NULL);
	}
	
      } else if (!name_check(V_NAME(av[i]), "Width", 3)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  v =V_KEYVAL(av[i]);
	  if(V_TYPE(v) == ID_VAL) {
	    globalNums[12] = 1;
	    globalNums[13] = (int)extract_float(v,0);
	  } else {
	    parse_error("Invalid designation for line Width");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Width");
	  return(NULL);
	}
	
      } else if (!name_check(V_NAME(av[i]), "Style", 2)) {
	if(V_KEYVAL(av[i]) != NULL) {
	  v = V_KEYVAL(av[i]);
	
	  if(V_TYPE(v) == ID_STRING)
	    Style = V_STRING(v);
	  else
	    Style = strdup(V_NAME(v));
	
	  if(name_check(Style,"lines",1) &&
	     name_check(Style,"linespoints",6) &&
	     name_check(Style,"dots",1) &&
	     name_check(Style,"points",1) &&
	     name_check(Style,"boxes",1) &&
	     name_check(Style,"steps",1) &&
	     name_check(Style,"impulses",1) &&
	     name_check(Style,"xerrorbars",7) &&
	     name_check(Style,"yerrorbars",7) &&
	     name_check(Style,"xyerrorbars",8) &&
	     name_check(Style,"xerrorlines",7) &&
	     name_check(Style,"yerrorlines",7) &&
	     name_check(Style,"xyerrorlines",8)) {
	    parse_error("Invalid designation for Style");
	    parse_error("Only: lines, points, dots, linespoints, boxes, steps, impulses,");
	    parse_error("      xerrorbars, yerrorbars, xyerrorbars");
	    parse_error("      xerrorlines, yerrorlines, xyerrorlines");
	    Style = NULL;
	  }
	  v = NULL;
	} else {
	  parse_error("Cannot find value for Style");
	  return(NULL);
	}

      } else if (!name_check(V_NAME(av[i]), "Separate", 2)) {
	globalNums[14] = 1;

      } else if (!name_check(V_NAME(av[i]), "Offset", 1)) {
	if (V_KEYVAL(av[i]) != NULL) {
	  if(V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	    v = V_KEYVAL(av[i]);
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      globalNums[15] = (float)extract_int(v,0);
	    } else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      globalNums[15] = extract_float(v,0);
	    }
	  } else {
	    parse_error("Invalid designation for Offset");
	    parse_error("Keyword 'Offset' must be a numeric value");
	    return(NULL);
	  }
	} else {
	  parse_error("Cannot find value for Offset");
	  return(NULL);
	}

      } else if(!name_check(V_NAME(av[i]),"dir",1)) {
	if(V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_STRING){
	  dir = strdup(V_STRING(V_KEYVAL(av[i])));
	}

      } else if(!name_check(V_NAME(av[i]),"gcommand",1))
	gcommand = 1;
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

    plotchoppererror = plot_chopper(av, start_ct, end_ct, Onum, CommandBuffer, Xaxis, Axis, globalNums, Style, dir);
    j+=1;

    // There was an error somewhere in plot_chopper()
    if(plotchoppererror == 1)
      return(NULL);

    // plot_chopper() completed without error. Find the next subset of
    // the command buffer significant to the next plot object
    else {
      start_ct = end_ct + 1;
      end_ct = 0;
      i = start_ct + 1;
      while (i<ac) {
	if(V_TYPE(av[i]) != ID_KEYWORD && V_TYPE(av[i]) != ID_STRING && end_ct == 0)
	  end_ct = i-1;
	i++;
      }

      if(i==ac && end_ct == 0)
	end_ct = ac-1;
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






int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, float *globalNums, char *Style, char *dir)
{

  /* The code of plot_chopper is the canibalized descendent of xplot()*/

  Var   *s=NULL, *v=NULL;    // generic temporary Vars
  int    count = 0;                   //
  Var   *xaxis_i = NULL;              // Var holding xaxis for individual object
  char  *axis_i = NULL;               // string holding axis designation
  int    Mode[3];                     // mode of the object (defined in code)
  int    Ord[3];                      // order of the object
  int    XOrd[3];                     // order of the xaxis
  int    CE[3];                       //
  int    xFlag = 0;                   // was there an xaxis designated?
  int    obj_index;                   // object index in the argument string
  float *x = NULL, *y = NULL;         // the x and y data extracted from object

  Var   *errorb = NULL;               // Var holding errorbars
  float *Err1 = NULL, *Err2 = NULL;   // Vars to be used if errorbars are used
  float *Err3 = NULL, *Err4 = NULL;   // Vars to be used if errorbars are used
  int   *EI = NULL;
  int    sef = 0;                     // sum of error flags
  int    sef2 = 0;

  int    i, j, k;                     // loop indices
  char   axs;                         // temporary axis string
  FILE  *fp = NULL;                   // filepointer to Gnuplot
  char  *fname = NULL;                // filename

  float  offset = 0;                  // value to offset successive vectors
  float  yval = 0;                    // offset y-value
  float  e1=0, e2=0;                  // offset error values

  float  ignore;                      // ignore value for data
  float  ignore_above;                // 'ignore above' value for data
  float  ignore_below;                // 'ignore below' value for data
  float  ignore_x_above;              // 'ignore above' x value for data
  float  ignore_x_below;              // 'ignore above' y value for data
  int    iflag = 0;                   // was there an ignore value?
  int    iaflag = 0;                  // was there an iabove value?
  int    ibflag = 0;                  // was there an ibelow value?
  int    ixaflag = 0;
  int    ixbflag = 0;
  int    sep = 0;                     // was separate used?
  int    color = -10;                 // color of plot
  int    width = -10;                 // width of plot
  char  *smooth = NULL;               // smooth the data?  csplines or bezier
  char  *style = NULL;                // style of plot
  char  *label = NULL;                // label of plot
  Var   *lbl = NULL;                  // in case label is a TEXT buffer
  int    lblrows = 0;                 // a count of the number of rows in lbl
  int    vnum = 1;                    // current vector number of the object

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

    if(V_TYPE(av[i]) == ID_UNK || V_TYPE(av[i]) == ID_STRUCT) {

      if(V_TYPE(av[i]) == ID_UNK)
	s = eval(av[i]);
      else
	s = av[i];

      //variable was empty
      if(s == NULL) {
	parse_error("Variable not found: %s", label);
        return(1);
      }

      //av[i] isn't a keyword, but it is a STRUCT
      //so pull out the keyword elements
      if(V_TYPE(s) == ID_STRUCT){
	v = NULL;

	find_struct(s,"data",&v);
	if(v == NULL) {
	  parse_error("This struct doesn't have a conforming architecture.");
	  parse_error("It must contain at least the element .data to be plotted");
	  return(1);
	}
	v = NULL;

	find_struct(s,"xaxis",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    xaxis_i = v;
	  } else {
	    parse_error("Invalid xaxis element in structure: Object %d",Onum);
	    parse_error("xaxis must be a numeric array. Continuing with default xaxis.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"axis",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_STRING) {
	    axis_i = V_STRING(v);
	
	    if(axis_i != NULL &&
	       strcasecmp(axis_i,"x") &&
	       strcasecmp(axis_i,"y") &&
	       strcasecmp(axis_i,"z")) {
	      parse_error("Invalid axis designation. x y z only.");
	    return(1);
	    }
	  } else {
	    parse_error("Invalid axis element in structure: Object %d",Onum);
	    parse_error("axis must be a string. Continuing with default axis.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"errorbars",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL)
	    errorb = v;
	  else {
	    parse_error("Invalid errorbars element in structure: Object %d",Onum);
	    parse_error("errorbars must be a numeric array. Continuing without errorbars.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"label",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_STRING) {
	    label = strdup(V_STRING(v));
	    v = NULL;
	  } else if(V_TYPE(v) == ID_TEXT) {
	    lbl = v;
	    lblrows = V_TEXT(lbl).Row;
	    v = NULL;
	  } else {
	    parse_error("Invalid label element in structure: Object %d",Onum);
	    parse_error("label must be a STRING or TEXT. Continuing with default label.\n");
	  }
	  v = NULL;
	}

	//here's for stupid emcal
	find_struct(s,"sample_name",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_STRING) {
	    label = strdup(V_STRING(v));
	    v = NULL;
	  } else if(V_TYPE(v) == ID_TEXT) {
	    lbl = v;
	    lblrows = V_TEXT(lbl).Row;
	    v = NULL;
	  } else {
	    parse_error("Invalid label element in structure: Object %d",Onum);
	    parse_error("label must be a STRING or TEXT. Continuing with default label.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"smooth",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_STRING) {
	    smooth = strdup(V_STRING(v));
	
	    if(name_check(smooth,"bezier",1) &&
	       name_check(smooth,"csplines",1)) {
	      parse_error("smooth must be bezier, csplines");
	      parse_error("continuing without smooth");
	      smooth = NULL;
	      return(1);
	    }
	  } else {
	    parse_error("Invalid smooth element in structure: Object %d",Onum);
	    parse_error("smooth must be string \'bezier\' or \'csplines\'. Continuing without smooth.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"width",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	      width = extract_int(v,0);
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	      width = (int)extract_float(v,0);
	  } else {
	    parse_error("Invalid width element in structure: Object %d",Onum);
	    parse_error("width must be an integer. Continuing with default width.\n");
	  }	
	  v = NULL;
	}


	find_struct(s,"style",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_STRING) {
	    style = strdup(V_STRING(v));
	
	    if(name_check(style,"lines",1) &&
	       name_check(style,"linespoints",6) &&
	       name_check(style,"dots",1) &&
	       name_check(style,"points",1) &&
	       name_check(style,"boxes",1) &&
	       name_check(style,"steps",1) &&
	       name_check(style,"impulses",1) &&
	       name_check(style,"xerrorbars",7) &&
	       name_check(style,"yerrorbars",7) &&
	       name_check(style,"xyerrorbars",8) &&
	       name_check(style,"xerrorlines",7) &&
	       name_check(style,"yerrorlines",7) &&
	       name_check(style,"xyerrorlines",8)) {
	      parse_error("Invalid designation for style");
	      parse_error("Only: lines, points, dots, linespoints, boxes, steps, impulses");
	      parse_error("      xerrorbars, yerrorbars, xyerrorbars");
	      parse_error("      xerrorlines, yerrorlines, xyerrorlines");
	      parse_error("Continuing plot with default style");
	      style = NULL;
	    }
	  } else {
	    parse_error("Invalid style element in structure: Object %d",Onum);
	    parse_error("style must be a string. Continuing with default style.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"color",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	      color = extract_int(v,0);
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	      color = (int)extract_float(v,0);	
	  } else {
	    parse_error("Invalid color element in structure: Object %d",Onum);
	    parse_error("color must be an integer. Continuing with default color.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"separate",&v);
	if(v!=NULL)
	  sep = 1;
	v = NULL;


	find_struct(s,"ignore",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      ignore = (float)extract_int(v,0);
	      iflag = 1;
	    }
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      ignore = extract_float(v,0);
	      iflag = 1;
	    }
	    else {
	      parse_error("Invalid format for ignore in Object %d",Onum);
	    }
	  } else {
	    parse_error("Invalid ignore element in structure: Object %d",Onum);
	    parse_error("ignore must be a number. Continuing without ignore.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"iabove",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      ignore_above = (float)extract_int(v,0);
	      iaflag = 1;
	    }
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      ignore_above = extract_float(v,0);
	      iaflag = 1;
	    }
	    else {
	      parse_error("Invalid format for iabove in Object %d",Onum);
	    }
	  } else {
	    parse_error("Invalid iabove element in structure: Object %d",Onum);
	    parse_error("iabove must be a number. Continuing without iabove.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"ibelow",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      ignore_below = (float)extract_int(v,0);
	      ibflag = 1;
	    }
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      ignore_below = extract_float(v,0);
	      ibflag = 1;
	    }
	    else {
	      parse_error("Invalid format for ibelow in Object %d",Onum);
	    }
	  } else {
	    parse_error("Invalid ibelow element in structure: Object %d",Onum);
	    parse_error("ibelow must be a number. Continuing without ibelow.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"ixabove",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      ignore_x_above = (float)extract_int(v,0);
	      ixaflag = 1;
	    }
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      ignore_x_above = extract_float(v,0);
	      ixaflag = 1;
	    }
	    else {
	      parse_error("Invalid format for ixabove in Object %d",Onum);
	    }
	  } else {
	    parse_error("Invalid ixabove element in structure: Object %d",Onum);
	    parse_error("ixabove must be a number. Continuing without ixabove.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"ixbelow",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	      ignore_x_below = (float)extract_int(v,0);
	      ixbflag = 1;
	    }
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	      ignore_x_below = extract_float(v,0);
	      ixbflag = 1;
	    }
	    else {
	      parse_error("Invalid format for ixbelow in Object %d",Onum);
	    }
	  } else {
	    parse_error("Invalid ixbelow element in structure: Object %d",Onum);
	    parse_error("ixbelow must be a number. Continuing without ixbelow.\n");
	  }
	  v = NULL;
	}

	find_struct(s,"offset",&v);
	if(v!=NULL) {
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	      offset = (float)extract_int(v,0);

	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	      offset = extract_float(v,0);

	    else
	      parse_error("Invalid format for offset in Object %d",Onum);

	  } else {
	    parse_error("Invalid offset element in structure: Object %d",Onum);
	    parse_error("offset must be a number. Continuing without offset.\n");
	  }
	  v = NULL;
	}
      }
    }

    if (V_TYPE(av[i]) == ID_KEYWORD) {
      if (!(name_check(av[i]->name, "Xaxis", 1))) {
      } else if (!(name_check(av[i]->name, "Axis", 1))) {
      } else if (!(name_check(av[i]->name, "Ignore", 2))) {
      } else if (!(name_check(av[i]->name, "Iabove", 2))) {
      } else if (!(name_check(av[i]->name, "Ibelow", 2))) {
      } else if (!(name_check(av[i]->name, "Ixabove", 3))) {
      } else if (!(name_check(av[i]->name, "Ixbelow", 3))) {
      } else if (!(name_check(av[i]->name, "Color", 1))) {
      } else if (!(name_check(av[i]->name, "Width", 1))) {
      } else if (!(name_check(av[i]->name, "Style", 2))) {
      } else if (!(name_check(av[i]->name, "Separate", 2))) {
      } else if (!(name_check(av[i]->name, "Offset", 1))) {
      } else if (!(name_check(av[i]->name, "dir", 1))) {
      } else if (!(name_check(av[i]->name, "gcommand", 1))) {

      } else if (!(name_check(av[i]->name, "xaxis", 1))) {
	xaxis_i = eval(V_KEYVAL(av[i]));
	if (xaxis_i == NULL) {
	  parse_error("Variable not found: \'xaxis\'");
	  free(av);
	  return(1);
	}
	if (V_TYPE(xaxis_i) != ID_VAL) {
	  parse_error("xaxis in Object %d does not conform!",Onum);
	  parse_error("xaxis must be a numeric array.\n");
	  free(av);
	  return(1);
	}
	
      } else if (!(name_check(av[i]->name, "axis",1))) {

	if(V_KEYVAL(av[i]) == NULL) {
	  parse_error("Invalid axis designation. x y z only.");
	  free(av);
	  return(1);
	}

	v = V_KEYVAL(av[i]);

	if (V_TYPE(v) == ID_STRING) {
	  axis_i = V_STRING(v);
	} else {
	  axis_i = V_NAME(v);
	}
	
	if(axis_i != NULL &&
	   strcasecmp(axis_i,"x") &&
	   strcasecmp(axis_i,"y") &&
	   strcasecmp(axis_i,"z")) {
	  parse_error("Invalid axis designation. x y z only.");
	  free(av);
	  return(1);
	}

	
      } else if (!(name_check(av[i]->name, "errorbars", 1))) {
	errorb = eval(V_KEYVAL(av[i]));
	if (errorb == NULL) {
	  parse_error("Variable not found: \'errorbars\'");
	  free(av);
	  return (1);
	}
	if (V_TYPE(errorb) != ID_VAL) {
	  parse_error("errorbars in Object %d does not conform!",Onum);
	  parse_error("errorbars must be a numeric array.\n");
	  free(av);
	  return(1);
	}
	
      } else if (!(name_check(av[i]->name, "separate",2))) {
	sep = 1;
	
      } else if (!(name_check(av[i]->name, "ignore",2))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  v = V_KEYVAL(av[i]);
	  if(V_TYPE(v) == ID_VAL) {
	    if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	      ignore = (float)extract_int(v,0);
	    else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	      ignore = extract_float(v,0);
	    iflag = 1;
	  } else {
	    parse_error("Invalid ignore value for Object %d",Onum);
	    parse_error("ignore must be a number. Continuing without ignore.\n");
	    free(av);
	    return (1);
	  }
	}

      } else if (!(name_check(av[i]->name, "iabove", 2))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  v = V_KEYVAL(av[i]);
	  if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	    ignore_above = (float)extract_int(v,0);
	  else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	    ignore_above = extract_float(v,0);
	  iaflag = 1;
	} else {
	  parse_error("Invalid designation for iabove");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "ibelow", 2))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  v = V_KEYVAL(av[i]);
	  if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	    ignore_below = (float)extract_int(v,0);
	  else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	    ignore_below = extract_float(v,0);
	  ibflag = 1;
	} else {
	  parse_error("Invalid designation for ibelow");
	  parse_error("Continuing plot without value");
	}

      } else if (!(name_check(av[i]->name, "ixabove", 3))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  v = V_KEYVAL(av[i]);
	  if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	    ignore_x_above = (float)extract_int(v,0);
	    ixaflag = 1;
	  }
	  else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	    ignore_x_above = extract_float(v,0);
	    ixaflag = 1;
	  }
	} else {
	  parse_error("Invalid designation for ixabove");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "ixbelow", 3))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  v = V_KEYVAL(av[i]);
	  if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT) {
	    ignore_x_below = (float)extract_int(v,0);
	    ixbflag = 1;
	  }
	  else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE) {
	    ignore_x_below = extract_float(v,0);
	    ixbflag = 1;
	  }
	} else {
	  parse_error("Invalid designation for ixbelow");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "offset", 1))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  v = V_KEYVAL(av[i]);
	  if(V_FORMAT(v)>=BYTE && V_FORMAT(v)<=INT)
	    offset = (float)extract_int(v,0);
	  else if(V_FORMAT(v)>=FLOAT && V_FORMAT(v)<=DOUBLE)
	    offset = extract_float(v,0);
	} else {
	  parse_error("Invalid designation for offset");
	  parse_error("Continuing plot without value");
	}
	
      } else if (!(name_check(av[i]->name, "color", 1))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
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
	
      } else if (!(name_check(av[i]->name, "width", 1))) {
	if (V_KEYVAL(av[i]) != NULL && V_TYPE(V_KEYVAL(av[i])) == ID_VAL) {
	  width = (int)extract_float(V_KEYVAL(av[i]),0);
	} else {
	  parse_error("Invalid designation for line width");
	  parse_error("Continuing plot with default width");
	}
	
      } else if (!(name_check(av[i]->name, "style", 2))) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING){
	  style = V_STRING(v);
	} else if (V_TYPE(v) != ID_VAL) {
	  style = strdup(V_NAME(v));
	} else {
	  parse_error("Invalid designation for style");
	  return(1);
	}

	if(name_check(style,"lines",1) &&
	   name_check(style,"linespoints",6) &&
	   name_check(style,"dots",1) &&
	   name_check(style,"points",1) &&
	   name_check(style,"boxes",1) &&
	   name_check(style,"steps",1) &&
	   name_check(style,"impulses",1) &&
	   name_check(style,"xerrorbars",7) &&
	   name_check(style,"yerrorbars",7) &&
	   name_check(style,"xyerrorbars",8) &&
	   name_check(style,"xerrorlines",7) &&
	   name_check(style,"yerrorlines",7) &&
	   name_check(style,"xyerrorlines",8)) {
	  parse_error("Invalid designation for style");
	  parse_error("Only: lines, points, dots, linespoints, boxes, steps, impulses");
	  parse_error("      xerrorbars, yerrorbars, xyerrorbars");
	  parse_error("      xerrorlines, yerrorlines, xyerrorlines");
	  parse_error("Continuing plot with default style");
	  style = NULL;
	}
	
      } else if (!(name_check(av[i]->name, "smooth",1))) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING)
	  smooth = V_STRING(v);
	else
	  smooth = V_NAME(v);

	if(name_check(smooth,"bezier",1) &&
	   name_check(smooth,"csplines",1)) {
	  parse_error("smooth must be bezier, csplines");
	  parse_error("continuing without smooth");
	  smooth = NULL;
	  return(1);
	}
	
      } else if (!(name_check(av[i]->name, "label", 1))) {

	v = eval(V_KEYVAL(av[i]));
	if (v == NULL) {
	  parse_error("Variable not found: \'label\'");
	  return(1);
	}

	if(V_TYPE(V_KEYVAL(av[i])) == ID_STRING){
	  label = strdup(V_STRING(V_KEYVAL(av[i])));

	} else if(V_TYPE(V_KEYVAL(av[i])) == ID_TEXT){
	  lbl = V_KEYVAL(av[i]);
	  lblrows = V_TEXT(lbl).Row;

	} else if(V_TYPE(V_KEYVAL(av[i])) == ID_UNK) {
	  v=eval(V_KEYVAL(av[i]));
	  if(V_TYPE(v) == ID_TEXT) {
	    lbl = v;
	    lblrows = V_TEXT(lbl).Row;
	  } else if(V_TYPE(v) == ID_STRING) {
	    label = strdup(V_STRING(v));
	  } else {
	    parse_error("Illegal label designation. STRINGS or TEXT only.a");
	    return(1);
	  }

	} else {
	  parse_error("Illegal label designation. STRINGS or TEXT only.b");
	  return(1);
	}

      } else {
	parse_error("Illegal keyword %s\n", av[i]->name);
	free(av);
	return (1);
      }
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
   ** Section C: File and command creation      **
   ***********************************************/

  // Find or set xaxis_i
  if (Xaxis != NULL)
    xaxis_i = Xaxis;

  if (xaxis_i != NULL) {
    XOrd[0] = GetSamples(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[1] = GetLines(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[2] = GetBands(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    xFlag = 1;
  }

  //Override of local keywords by global keywords
  if(Axis != NULL)
    axis_i = Axis;

  if(globalNums[0]) {
    iflag = 1;
    ignore = globalNums[1];
  }

  if(globalNums[2]) {
    iaflag = 1;
    ignore_above = globalNums[3];
  }

  if(globalNums[4]) {
    ibflag = 1;
    ignore_below = globalNums[5];
  }

  if(globalNums[6]) {
    ixaflag = 1;
    ignore_x_above = globalNums[7];
  }

  if(globalNums[8]) {
    ixbflag = 1;
    ignore_x_below = globalNums[9];
  }

  if(globalNums[10])
    color = globalNums[11];

  if(globalNums[12])
    width = globalNums[13];

  if(globalNums[14])
    sep = 1;

  if(globalNums[15])
    offset = globalNums[15];

  if(Style != NULL)
    style = Style;


  //Here's our next object in argument list
  s = av[start_ct];

  if (V_TYPE(s) == ID_UNK) {

    if ((s = eval(s)) == NULL) {
      label = V_NAME(s);
      parse_error("%s does not exist\n", label);
      free(av);
      return(1);
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

    // Here is the case where axis wasn't given
    if (axis_i == NULL) {
      if(xaxis_i != NULL) {
	findAxis(&axs, xaxis_i, 0);
	axis_i = strdup(&axs);

      } else {
	findAxis(&axs, s, 1);
	axis_i = strdup(&axs);
      }
    }

    switch (*axis_i) {
    case 'X':  case 'x':
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
      Mode[1] = 1;
      Mode[2] = 0;
      break;
    default:
      Mode[0] = 2;
      Mode[1] = 0;
      Mode[2] = 1;
      break;
    }

    if (!(sep)) {
      fname = make_temp_file_path_in_dir(dir);
      if (fname == NULL || (fp = fopen(fname, "w")) == NULL) {
	parse_error("Unable to open temp file");
	if (fname)
	  free(fname);
	free(av);
	return(1);
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
	parse_error("Length of x-axis vector is different than Object %d vector",Onum);
	free(av);
	return (1);
	
      } else if ((XOrd[1] != 1 && XOrd[2] != 1) &&
		 (XOrd[1] != Ord[Mode[1]] || XOrd[2] != Ord[Mode[2]])) {
	parse_error("Given x-axis doesn't agree with Object %d",Onum);
	parse_error("Dimensions of the xaxis array must be 1");
	parse_error("or match the object array.");
	free(av);
	return (1);
      }
    }

    //Perform a check to make sure style asks for errorbars
    if (style) {
      if(name_check(style,"xerrorbars",7) &&
	 name_check(style,"yerrorbars",7) &&
	 name_check(style,"xyerrorbars",8) &&
	 name_check(style,"xerrorlines",7) &&
	 name_check(style,"yerrorlines",7) &&
	 name_check(style,"xyerrorlines",8)) {
	errorb = NULL;
      }
    }

    //Errorbars are being used
    if (errorb) {
      i = handle_errorbars(errorb, v, Mode, style, Onum);
      if(i == 0) {
	free(av);
	return(1);
      }

      Err1 = calloc(Ord[Mode[0]], sizeof(float));
      if(i >= 2)
	Err2 = calloc(Ord[Mode[0]], sizeof(float));
      if(i >= 3) {
	Err3 = calloc(Ord[Mode[0]], sizeof(float));
        Err4 = calloc(Ord[Mode[0]], sizeof(float));
      }
    }

    x = calloc(Ord[Mode[0]], sizeof(float));
    y = calloc(Ord[Mode[0]], sizeof(float));

    //Loop through the two non-axis dimensions to plot all vectors
    for (i = 0; i < Ord[Mode[2]]; i++) {
      for (j = 0; j < Ord[Mode[1]]; j++) {

	if (sep) {
	  fname = make_temp_file_path_in_dir(dir);
	  if (fname == NULL || (fp = fopen(fname, "w")) == NULL) {
	    parse_error("Unable to open temp file");
	    if (fname) {
	      free(fname);
	    }
	    free(av);
	    return (1);
	  }
	}
	
	// Here we're looping through the elements along the axis
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

	  if (errorb) {

	    EI = make_errorbar_indices(v, Mode, CE, errorb);

	    switch (V_FORMAT(v)) {
	    case BYTE:
	    case SHORT:
	    case INT:
	      if(V_DSIZE(errorb) == 1)
		Err1[k] = (float)extract_int(errorb,0);
	      else {
		Err1[k] = (float)extract_int(errorb, cpos(EI[0], EI[1], EI[2], errorb));
		if(Err2 != NULL)
		  Err2[k] = (float)extract_int(errorb, cpos(EI[3], EI[4], EI[5], errorb));
		if(Err3 != NULL){
		  Err3[k] = (float)extract_int(errorb, cpos(EI[6], EI[7], EI[8], errorb));
		  Err4[k] = (float)extract_int(errorb, cpos(EI[9], EI[10], EI[11], errorb));
		}
	      }
	    case FLOAT:
	    case DOUBLE:
	      if(V_DSIZE(errorb) == 1)
		Err1[k] = extract_float(errorb,0);
	      else {
		Err1[k] = extract_float(errorb, cpos(EI[0], EI[1], EI[2], errorb));
		if(Err2 != NULL){
		  Err2[k] = extract_float(errorb, cpos(EI[3], EI[4], EI[5], errorb));
		}
		if(Err3 != NULL){
		  Err3[k] = extract_float(errorb, cpos(EI[6], EI[7], EI[8], errorb));
		  Err4[k] = extract_float(errorb, cpos(EI[9], EI[10], EI[11], errorb));
		}
	      }
	    }
	  }
	
	  //Here we finally fucking handle the glorious ignore values!
	  sef = 0; // Sum of the Error Flags

	  sef = (iflag != 0 && y[k] == ignore)?sef+1:sef;
	  sef = (iaflag != 0 && y[k] > ignore_above)?sef+1:sef;
	  sef = (ibflag != 0 && y[k] < ignore_below)?sef+1:sef;
	  sef = (ixaflag != 0 && ixbflag == 0 && x[k] > ignore_x_above)?sef+1:sef;
	  sef = (ixaflag == 0 && ixbflag != 0 && x[k] < ignore_x_below)?sef+1:sef;
	  sef = (ixaflag != 0 && ixbflag != 0 && x[k] > ignore_x_above && x[k] < ignore_x_below)?sef+1:sef;

	  //places a break in the plot if ignore_x block occurs between data points
	  sef2 = 0;
	  sef2 = (k>0 && ixaflag+ixbflag == 2 && x[k-1] < ignore_x_above && x[k] > ignore_x_below)?sef2+1:sef2;
	  sef2 = (k>0 && ixaflag+ixbflag == 2 && x[k-1] > ignore_x_below && x[k] < ignore_x_above)?sef2+1:sef2;

	  if(sef2)
	    fprintf(fp, "\n");


	  if (sef) {
	    fprintf(fp, "\n");

	  } else {

	    yval = y[k]-(float)(vnum-1)*offset;

	    if(errorb == NULL) {
	      fprintf(fp, "%g\t %g\n", x[k], yval);

	    } else {

	      if(Err1 && Err2 == NULL) {
		fprintf(fp, "%g\t %g\t %g\n", x[k], yval, Err1[k]);

	      } else if(Err1 && Err2 && Err3 == NULL) {

                // NOTE(gorelick): This is an outright error.
		e1 = (style && offset && (style == "yerrorbars" || style == "yerrorlines"))?Err1[k]-(float)(vnum-1)*offset:Err1[k];
		e2 = (style && offset && (style == "yerrorbars" || style == "yerrorlines"))?Err2[k]-(float)(vnum-1)*offset:Err2[k];

		fprintf(fp, "%g\t %g\t %g\t %g\n", x[k], yval, e1, e2);

	      } else {

		e1 = Err3[k]-(float)(vnum-1)*offset;
		e2 = Err4[k]-(float)(vnum-1)*offset;

		fprintf(fp, "%g\t %g\t %g\t %g\t %g\t %g\n", x[k], yval, Err1[k], Err2[k], e1, e2);
	      }
	    }
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
	  else if (lbl != NULL) {
	    if(vnum <= lblrows) {
	      label = strdup(V_TEXT(lbl).text[vnum-1]);
	      sprintf(CommandBuffer + strlen(CommandBuffer), " title '%s'", label);
	      label = NULL;
	    }
	  } else
	    sprintf(CommandBuffer + strlen(CommandBuffer), " title 'Obj %d vector %d'",Onum,vnum);
	
	  vnum+=1;

	  // smooth?
	  if(smooth != NULL)
	    sprintf(CommandBuffer + strlen(CommandBuffer), " smooth %s",smooth);
	
	  // Add width
	  if(width > 0)
	    sprintf(CommandBuffer + strlen(CommandBuffer), " lw %d",width);
	
	  // Add style
	  if(style != NULL)
	    sprintf(CommandBuffer + strlen(CommandBuffer), " with %s",style);
	
	  free(fname);

	} else {
	  fprintf(fp, "\n");
	  vnum+=1;
	}
      }
    }
    free(x);
    free(y);

    if(Err1)
      free(Err1);
    if(Err2)
      free(Err2);
    if(Err3) {
      free(Err3);
      free(Err4);
    }

    if (!(sep)) {
      fclose(fp);
      if (count++) {
	strcat(CommandBuffer, ",");
      }
      sprintf(CommandBuffer + strlen(CommandBuffer), "'%s'", fname);

      // Properly title the object
      if (label != NULL)
	sprintf(CommandBuffer + strlen(CommandBuffer), " title '%s'", label);
      else if(lbl != NULL) {
	label = strdup(V_TEXT(lbl).text[0]);
	sprintf(CommandBuffer + strlen(CommandBuffer), " title '%s'", label);
	label = NULL;
      } else
	sprintf(CommandBuffer + strlen(CommandBuffer), " title 'Obj %d'",Onum);

      // smooth?
      if(smooth != NULL)
	sprintf(CommandBuffer + strlen(CommandBuffer), " smooth %s",smooth);

      // Add color
      if(color > -2)
	sprintf(CommandBuffer + strlen(CommandBuffer), " lt %d",color);

      // Add width
      if(width > 0)
	sprintf(CommandBuffer + strlen(CommandBuffer), " lw %d",width);

      // Add style
      if(style != NULL)
	sprintf(CommandBuffer + strlen(CommandBuffer), " with %s",style);

      free(fname);

    }
  }
  return(0);
}






static int handle_errorbars(Var *errorb, Var *v, int *Mode, char *style, int Onum) {

  int    AX;
  int    EM1, EM2;
  int    v_dim[3];
  int    e_dim[3];

  v_dim[0] = GetX(v);
  v_dim[1] = GetY(v);
  v_dim[2] = GetZ(v);

  e_dim[0] = GetX(errorb);
  e_dim[1] = GetY(errorb);
  e_dim[2] = GetZ(errorb);

  //this be the axis
  AX = Mode[0];

  //these ain't
  EM1 = e_dim[Mode[1]];
  EM2 = e_dim[Mode[2]];

  // The case where errorbars was just a single number, not an array
  if(e_dim[0] == 1 && e_dim[1] == 1 && e_dim[2] == 1) {
    if(!name_check(style,"xyerrorbars",2)) {
      parse_error("You only provided a single error value for a style that requires 2 values.\nReverting to style=yerrorbars");
      sprintf(style,"yerrorbars");
    }
    return(EM1);
  }

  if(e_dim[0] == 1 && e_dim[1] == 1 && e_dim[2] == 1) {
    if(!name_check(style,"xyerrorlines",8)) {
      parse_error("You only provided a single error value for a style that requires 2 values.\nReverting to style=yerrorlines");
      sprintf(style,"yerrorlines");
    }
    return(EM1);
  }


  //Errorbars is an array, check it!
  if(e_dim[AX] != v_dim[AX]) {
    parse_error("Length of errorbar vectors are different than for Object %d.", Onum);
    return(0);
  }

  // x = 1, 2 or 4 and y doesn't match or y = 1, 2 or 4 and x doesn't match
  if (((EM1 == 1 || EM1 == 2 || EM1 == 4) && EM2 == v_dim[Mode[2]]) ||
      ((EM2 == 1 || EM2 == 2 || EM2 == 4) && EM1 == v_dim[Mode[1]])) {
  } else {
    parse_error("Given errorbars don't agree with Object %d", Onum);
    parse_error("There are the correct number of errorbar inputs,");
    parse_error("but a different number of errorbar vectors than object vectors");
    parse_error("Must have 1,2 or 4 errorbar inputs per vector.");
    return(0);	
  }

  //There still exists the problem of 2x4xN or 2x2xN errorbar vectors. So I'm going
  //to have to add this limitation to users:
  if (v_dim[Mode[1]] != 1 && v_dim[Mode[2]] != 1) {
    parse_error("I'm sorry, but if you're going to use errorbars, the");
    parse_error("dimension of your object must be 1 in the dimension");
    parse_error("of errorbars that contains the inputs for that vector");
    parse_error("Example: obj[11,1,188], yerrorbars[11,2,188].");
    parse_error("yerrorbars has inputs ylow in [,1,] and yhigh in [,2,]");
    return(0);
  }

  //Reuse EM1 as the mode of object and errorbars that are relevant
  if(v_dim[Mode[1]] == 1) {
    EM1 = Mode[1]; //order of object where it has dimension of 1
  }
  else if(v_dim[Mode[2]] == 1) {
    EM1 = Mode[2];
  }

  if(e_dim[EM1] == 1){
    if(!name_check(style,"xyerrorbars",2)) {
      parse_error("Invalid style for number of errorbar inputs.\nUsing style=yerrorbars");
      sprintf(style,"yerrorbars");
    }
    if(!name_check(style,"linesxybars",7)) {
      parse_error("Invalid style for number of errorbar inputs.\nUsing style=linesybars");
      sprintf(style,"linesybars");
    }
  }

  if(e_dim[EM1] == 4) {
    if(!name_check(style,"xerrorbars",1) || !name_check(style,"yerrorbars",1)){
      parse_error("Invalid style for number of errorbar values. Using style=xyerrorbars");
      sprintf(style,"xyerrorbars");
    }

    if(!name_check(style,"xerrorlines",7) || !name_check(style,"yerrorlines",7)) {
      parse_error("Invalid style for number of errorbar values. Using style=xyerrorlines");
      sprintf(style,"xyerrorlines");
    }
  }

  AX = e_dim[EM1];
  return(AX);
}





int *make_errorbar_indices(Var *v, int *Mode, int *CE, Var *errorb){

  int   *IE = NULL;
  int    v_dim[3];
  int    erroraxis = 0;
  int    ex=0,ey=0,ez=0;

  ex = GetX(errorb)-1;
  ey = GetY(errorb)-1;
  ez = GetZ(errorb)-1;

  v_dim[0] = GetX(v);
  v_dim[1] = GetY(v);
  v_dim[2] = GetZ(v);

  erroraxis = Mode[1];
  if(v_dim[Mode[1]] != 1)
    erroraxis = Mode[2];

  IE = calloc(12, sizeof(int));

  if(erroraxis == 0) {
    IE[0] = (CE[0] <= ex)?CE[0]:ex;
    IE[1] = (CE[1] <= ey)?CE[1]:ey;
    IE[2] = (CE[2] <= ez)?CE[2]:ez;
    IE[3] = (CE[0]+1 <= ex)?CE[0]+1:ex;
    IE[4] = (CE[1] <= ey)?CE[1]:ey;
    IE[5] = (CE[2] <= ez)?CE[2]:ez;
    IE[6] = (CE[0]+2 <= ex)?CE[0]+2:ex;
    IE[7] = (CE[1] <= ey)?CE[1]:ey;
    IE[8] = (CE[2] <= ez)?CE[2]:ez;
    IE[9] = (CE[0]+3 <= ex)?CE[0]+3:ex;
    IE[10] = (CE[1] <= ey)?CE[1]:ey;
    IE[11] = (CE[2] <= ez)?CE[2]:ez;
  } else if (erroraxis == 1) {
    IE[0] = (CE[0] <= ex)?CE[0]:ex;
    IE[1] = (CE[1] <= ey)?CE[1]:ey;
    IE[2] = (CE[2] <= ez)?CE[2]:ez;
    IE[3] = (CE[0] <= ex)?CE[0]:ex;
    IE[4] = (CE[1]+1 <= ey)?CE[1]+1:ey;
    IE[5] = (CE[2] <= ez)?CE[2]:ez;
    IE[6] = (CE[0] <= ex)?CE[0]:ex;
    IE[7] = (CE[1]+2 <= ey)?CE[1]+2:ey;
    IE[8] = (CE[2] <= ez)?CE[2]:ez;
    IE[9] = (CE[0] <= ex)?CE[0]:ex;
    IE[10] = (CE[1]+3 <= ey)?CE[1]+3:ey;
    IE[11] = (CE[2] <= ez)?CE[2]:ez;
  } else {
    IE[0] = (CE[0] <= ex)?CE[0]:ex;
    IE[1] = (CE[1] <= ey)?CE[1]:ey;
    IE[2] = (CE[2] <= ez)?CE[2]:ez;
    IE[3] = (CE[0] <= ex)?CE[0]:ex;
    IE[4] = (CE[1] <= ey)?CE[1]:ey;
    IE[5] = (CE[2]+1 <= ez)?CE[2]+1:ez;
    IE[6] = (CE[0] <= ex)?CE[0]:ex;
    IE[7] = (CE[1] <= ey)?CE[1]:ey;
    IE[8] = (CE[2]+2 <= ez)?CE[2]+2:ez;
    IE[9] = (CE[0] <= ex)?CE[0]:ex;
    IE[10] = (CE[1] <= ey)?CE[1]:ey;
    IE[11] = (CE[2]+3 <= ez)?CE[2]+3:ez;
  }

  return(IE);
}


Var *
ff_ploop(vfuncptr func, Var *arg)
{

  int      ac;                          // argument count
  Var    **av;                          // argument Var

  make_args(&ac, &av, func, arg);

  if (ac == 1) {
    parse_error("    plotloop() - 9/13/2008\n");
    parse_error(" Loop through a VAL, plotting one spectrum (or slice) at a time.");
    parse_error(" Syntax: plotloop(VAL[, axis = x|y|z][, xaxis = VAL][, keep = VAL])");
    parse_error(" Example: plotloop(a, axis=y, keep=b)");
  }
  return(NULL);
}
