#include "parser.h"

//This is probably overriden in config.h
#ifndef GPLOT_CMD
#define GPLOT_CMD "gnuplot -xrm 'gnuplot*background:black' -xrm 'gnuplot*axisColor:white' -xrm 'gnuplot*textColor:white' -xrm 'gnuplot*borderColor:white'"
#endif

#ifdef GPLOT_CMD
extern FILE *pfp;
#endif

static char *make_gnuplot_file_path(char *dir);
static int name_check(char *actual_input, char *name, int limit);
int send2plot(char *s);
static void findAxis(char *R, Var * Obj, int flag);
static int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, char *dir);
static int handle_errorbars(Var *errorb, Var *v, int *Mode, char *style);
static int *make_errorbar_indices(Var *v, int *Mode, int *CE);



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
int send2plot(char *s)
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





static int name_check(char *actual_input, char *name, int limit) {

  int  namelen = 0;
  int  inputlen = 0;
  int  i,j;

  if(actual_input == NULL)
    return(-3);

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

  int      ac;                          // argument count
  Var    **av;                          // argument Var
  Var     *v = NULL;
  Var     *Xaxis = NULL;
  char    *Axis = NULL;
  char    *dir = NULL;
  int      i,j,k;                       // loop index
  int      start_ct = 0;
  int      end_ct = 0;
  char     CommandBuffer[8192] = { 0 }; //command buffer sent to Gnuplot
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
    parse_error("         [, errorbars = VAL]    (x and/or y errorbars)");
    parse_error("         [, separate = BOOL]    (default = 0)");
    parse_error("         [, label = STRING]     (default = \'vector #\')");
    parse_error("         [, width = INT]        (thickness of plot)");
    parse_error("         [, color = INT]        (color of plot)");
    parse_error("         [, style = lines|points|dots|linespoints|steps|");
    parse_error("                    boxes|xerrorbars|yerrorbars|xyerrorbars]");
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






int plot_chopper(Var **av, int start_ct, int end_ct, int Onum, char *CommandBuffer, Var *Xaxis, char *Axis, char *dir)
{
  
  /* The code of plot_chopper is the canibalized descendent of xplot()*/
  
  Var   *s, *v;                       // generic temporary Vars
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

  int    i, j, k;                     // loop indices
  char   axs;                         // temporary axis string
  FILE  *fp = NULL;                   // filepointer to Gnuplot
  char  *fname = NULL;                // filename

  float  ignore;                      // ignore value for data
  float  ignore_above;                // 'ignore above' value for data
  float  ignore_below;                // 'ignore below' value for data
  int    iflag = 0;                   // was there an ignore value?
  int    iaflag = 0;                  // was there an iabove value?
  int    ibflag = 0;                  // was there an ibelow value?
  int    sep = 0;                     // was separate used?
  int    color = -10;                 // color of plot
  int    width = -10;                 // width of plot
  char  *smooth = NULL;               // smooth the data?  csplines or bezier
  char  *style = NULL;                // style of plot
  char  *label = NULL;                // label of plot
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

    //av[i] isn't a keyword, but it is a STRUCT
    //so pull out the keyword elements
    if(V_TYPE(av[i]) == ID_UNK) {
      s = eval(av[i]);
      if(s == NULL) {
	label = V_NAME(av[i]);
	parse_error("Variable not found: %s", label);
        return(1);
      }

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

	  if(strcasecmp(axis_i,"x") && 
	     strcasecmp(axis_i,"y") &&
	     strcasecmp(axis_i,"z")) {
	    parse_error("Invalid axis designation. x y z only.");
	    return(1);
	  }

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

	  if(strcasecmp(Axis,"x") && 
	     strcasecmp(Axis,"y") &&
	     strcasecmp(Axis,"z")) {
	    parse_error("Invalid Axis designation. x y z only.");
	    return(1);
	  }

	  v = NULL;
	}

	find_struct(s,"errorbars",&v);
	if(v!=NULL) {
	  errorb = v;
	  v = NULL;
	}

	find_struct(s,"label",&v);
	if(v!=NULL) {
	  label = strdup(V_STRING(v));
	  v = NULL;
	}

	find_struct(s,"smooth",&v);
	if(v!=NULL) {
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

	  v = NULL;
	}

	//here's for stupid emcal
	find_struct(s,"sample_name",&v);
	if(v!=NULL) {
	  label = strdup(V_STRING(v));
	  v = NULL;
	}

	find_struct(s,"width",&v);
	if(v!=NULL) {
	  width = extract_int(v,0);
	  v = NULL;
	}

	find_struct(s,"style",&v);
	if(v!=NULL) {
	  style = strdup(V_STRING(v));
	  v = NULL;

	  if(name_check(style,"lines",1) && 
	     name_check(style,"linespoints",6) &&
	     name_check(style,"dots",1) && 
	     name_check(style,"points",1) &&
	     name_check(style,"boxes",1) && 
	     name_check(style,"steps",1) &&
	     name_check(style,"impulses",1) && 
	     name_check(style,"xerrorbars",2) &&
	     name_check(style,"yerrorbars",2) &&
	     name_check(style,"xyerrorbars",3)) {
	    parse_error("Invalid designation for style");
	    parse_error("Only: lines, points, dots, linespoints, boxes, steps, impulses");
	    parse_error("      xerrorbars, yerrorbars or xyerrorbars");
	    parse_error("Continuing plot with default style");
	    style = NULL;
	  }
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
	  iflag = 1;
	  v = NULL;
	}

	find_struct(s,"iabove",&v);
	if(v!=NULL) {
	  ignore_above = extract_float(v,0);
	  iaflag = 1;
	  v = NULL;
	}

	find_struct(s,"ibelow",&v);
	if(v!=NULL) {
	  ignore_below = extract_float(v,0);
	  ibflag = 1;
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

	if(strcasecmp(axis_i,"x") && 
	   strcasecmp(axis_i,"y") &&
	   strcasecmp(axis_i,"z")) {
	  parse_error("Invalid axis designation. x y z only.");
	  return(1);
	}

	
      } else if (!(name_check(av[i]->name, "errorbars", 1))) {
	errorb = eval(V_KEYVAL(av[i]));
	if (errorb == NULL) {
	  parse_error("Variable not found: \'errorbars\'");
	  free(av);
	  return (1);
	}
	
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
	
      } else if (!(name_check(av[i]->name, "width", 1))) {
	if (V_KEYVAL(av[i]) != NULL) {
	  width = (int)extract_float(V_KEYVAL(av[i]),0);
	} else {
	  parse_error("Invalid designation for line width");
	  parse_error("Continuing plot with default width");
	}
	
      } else if (!(name_check(av[i]->name, "style", 2))) {
	v = V_KEYVAL(av[i]);
	if (V_TYPE(v) == ID_STRING){
	  style = V_STRING(v);
	} else {
	  style = strdup(V_NAME(v));
	}

	if(name_check(style,"lines",1) && 
	   name_check(style,"linespoints",6) &&
	   name_check(style,"dots",1) && 
	   name_check(style,"points",1) &&
	   name_check(style,"boxes",1) && 
	   name_check(style,"steps",1) &&
	   name_check(style,"impulses",1) && 
	   name_check(style,"xerrorbars",2) &&
	   name_check(style,"yerrorbars",2) &&
	   name_check(style,"xyerrorbars",3)) {
	  parse_error("Invalid designation for style");
	  parse_error("Only: lines, points, dots, linespoints, boxes, steps, impulses");
	  parse_error("      xerrorbars, yerrorbars or xyerrorbars");
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
  if (xaxis_i == NULL && Xaxis != NULL)
    xaxis_i = Xaxis;
  
  if (xaxis_i != NULL) {
    XOrd[0] = GetSamples(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[1] = GetLines(V_SIZE(xaxis_i), V_ORG(xaxis_i));
    XOrd[2] = GetBands(V_SIZE(xaxis_i), V_ORG(xaxis_i));
  }

  if (xaxis_i != NULL)
    xFlag = 1;

  //Here's our next object in argument list
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

    // Here is the case where axis wasn't given
    if (axis_i == NULL) {
      
      if(Axis != NULL)
	axis_i = Axis;

      else if(xaxis_i != NULL) {
	findAxis(&axs, xaxis_i, 0);
	axis_i = strdup(&axs);
      }
	  
      else {
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
      fname = make_gnuplot_file_path(dir);
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
	parse_error("Length of x-axis vector is different than object vector");
	free(av);
	return (1);
	
      } else if ((XOrd[1] != 1 && XOrd[2] != 1) && 
		 (XOrd[1] != Ord[Mode[1]] || XOrd[2] != Ord[Mode[2]])) {
	parse_error("Given x-axis doesn't agree with given data set");
	parse_error("Dimensions of the xaxis array must be 1");
	parse_error("or match the object array.");
	free(av);
	return (1);
      }
    }

    //Perform a check to make sure style asks for errorbars
    if (style) {
      if(name_check(style,"xerrorbars",2) && 
	 name_check(style,"yerrorbars",2) &&
	 name_check(style,"xyerrorbars",2)) {
	errorb = NULL;
      }
    }

    //Errorbars are being used
    if (errorb) {
      i = handle_errorbars(errorb, v, Mode, style);
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
	
	// Here we're looping through the elements along the axis
	for (k = 0; k < Ord[Mode[0]]; k++) {

	  CE[Mode[2]] = i;
	  CE[Mode[1]] = j;
	  CE[Mode[0]] = k;

	  EI = make_errorbar_indices(v, Mode, CE);

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
	    switch (V_FORMAT(v)) {
	    case BYTE:
	    case SHORT:
	    case INT:
	      Err1[k] = (float)extract_int(errorb, cpos(EI[0], EI[1], EI[2], errorb));
	      if(Err2 != NULL)
		Err2[k] = (float)extract_int(errorb, cpos(EI[3], EI[4], EI[5], errorb));
	      if(Err3 != NULL){
		Err3[k] = (float)extract_int(errorb, cpos(EI[6], EI[7], EI[8], errorb));
		Err4[k] = (float)extract_int(errorb, cpos(EI[9], EI[10], EI[11], errorb));
	      }
	    case FLOAT:
	    case DOUBLE:
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
	  
	  //Here we finally fucking handle the glorious ignore values!
	  if ((iaflag != 0 && y[k] > ignore_above) ||
	      (ibflag != 0 && y[k] < ignore_below) ||
	      (iflag != 0 && y[k] == ignore)) {
	    fprintf(fp, "\n");
	  } else {
	    if(errorb == NULL) {
	      fprintf(fp, "%g\t %g\n", x[k], y[k]);
	    } else {
	      if(Err1 && Err2 == NULL) {
		fprintf(fp, "%g\t %g\t %g\n", x[k], y[k], Err1[k]);
	      } else if(Err1 && Err2 && Err3 == NULL) {
		fprintf(fp, "%g\t %g\t %g\t %g\n", x[k], y[k], Err1[k], Err2[k]);
	      } else {
		fprintf(fp, "%g\t %g\t %g\t %g\t %g\t %g\n", x[k], y[k], Err1[k], Err2[k], Err3[k], Err4[k]);
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
	  else 
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
	} else
	  fprintf(fp, "\n");
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
      else
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






static int handle_errorbars(Var *errorb, Var *v, int *Mode, char *style) {

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

  if(e_dim[AX] != v_dim[AX]) {
    parse_error("Length of errorbar vectors are different than object vectors.");
    return(0);
  }

  // x = 1, 2 or 4 and y doesn't match or y = 1, 2 or 4 and x doesn't match 
  if ((EM1 == 1 || EM1 == 2 || EM1 == 4 && EM2 == v_dim[Mode[2]]) || 
      (EM2 == 1 || EM2 == 2 || EM2 == 4 && EM1 == v_dim[Mode[1]])) {
  } else {
    parse_error("Given errorbars don't agree with given data set");
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
  }

  if(e_dim[EM1] == 4) {
    if(name_check(style,"xyerrorbars",2)){
      parse_error("Invalid style. Using style=xyerrorbars");
      sprintf(style,"xyerrorbars");
    }
  }

  AX = e_dim[EM1];
  return(AX);  
}





int *make_errorbar_indices(Var *v, int *Mode, int *CE){

  int   *IE=NULL;
  int    v_dim[3];
  int    erroraxis;
  int    i;

  v_dim[0] = GetX(v);
  v_dim[1] = GetY(v);
  v_dim[2] = GetZ(v);

  erroraxis = Mode[1];
  if(v_dim[Mode[1]] != 1)
    erroraxis = Mode[2];

  IE = calloc(12, sizeof(int));

  if(erroraxis == 0) {
    IE[0] = CE[0];
    IE[1] = CE[1];
    IE[2] = CE[2];
    IE[3] = CE[0]+1;
    IE[4] = CE[1];
    IE[5] = CE[2];
    IE[6] = CE[0]+2;
    IE[7] = CE[1];
    IE[8] = CE[2];
    IE[9] = CE[0]+3;
    IE[10] = CE[1];
    IE[11] = CE[2];
  } else if (erroraxis == 1) {
    IE[0] = CE[0];
    IE[1] = CE[1];
    IE[2] = CE[2];
    IE[3] = CE[0];
    IE[4] = CE[1]+1;
    IE[5] = CE[2];
    IE[6] = CE[0];
    IE[7] = CE[1]+2;
    IE[8] = CE[2];
    IE[9] = CE[0];
    IE[10] = CE[1]+3;
    IE[11] = CE[2];
  } else {
    IE[0] = CE[0];
    IE[1] = CE[1];
    IE[2] = CE[2];
    IE[3] = CE[0];
    IE[4] = CE[1];
    IE[5] = CE[2]+1;
    IE[6] = CE[0];
    IE[7] = CE[1];
    IE[8] = CE[2]+2;
    IE[9] = CE[0];
    IE[10] = CE[1];
    IE[11] = CE[2]+3;
  }

  return(IE);
}





