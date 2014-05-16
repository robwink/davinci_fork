#include "isis3Include.h"

#include "dvio.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>

static Var *do_loadISIS3(vfuncptr func, char *filename, int data, int transpose_info); // drd static so only visible in this file

static int parenManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *nambuf);
static int dQuoteManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf);
static void make_unique_name(char *buffer, Var *dv_struct);
static char get_keyword_datatype(char *value, char *name);
static void add_converted_to_struct(Var *dv_struct, char * name, char * value);
static int stringFinder(void);
static int cNameMaker(char *inputString);
static int readNextLine(FILE *fp);
static int getCubeParams(FILE *fp);
static Var *readCube(FILE *fp);
static char inputString[2048];
static char string1[200],  string2[200],  string3[200],  string4[200];
static char string1_2[80], string2_2[80], string3_2[80], string4_2[80];
static char interString[80];

static int StartByte = 1;
static int Format = 1;
static int Samples = 1;
static int Lines = 1;
static int totalBands = 1;
static int sampleSize = 1;   // "Type"
static int ByteOrder = 1;
static double Base = 0.0;
static double Multiplier = 1.0;


static int tileSize = 1;
static int TileSamples = 1;
static int TileLines = 1;


static int subTileSize = 1;
static int tiles = 1;
static int row = 1;
static int Row = 1;
static int Bands = 1;
static int tilesPerRow = 1;
static int subTileSamples = 1;
static int totalRows = 1;
static int subTileLines = 1;

Var *ReadISIS3(vfuncptr func, Var * arg)   // drd proto for this is in func.h
{
    Var *fn = NULL;
    char *filename = NULL;
    int data = 1;  // parse the cube
    int transpose_info = 1; // reverse name and info
    int i;

    /*
     * If the user inputs load_isis3() or load_ISIS3()
     * with empty paren's, bail out
     */

    if(arg == NULL) {
    	 parse_error("No parameter list supplied--must at least supply an input file name.\n");
    	 return(NULL);
    }

    Alist alist[4];
    alist[0] = make_alist("filename", ID_UNK, NULL, &fn);
    alist[1] = make_alist("data", INT, NULL, &data);
    alist[2] = make_alist("transpose_info", INT, NULL, &transpose_info);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0) {
        return (NULL);
    }

/* Handle loading many filenames */
    if (V_TYPE(fn) == ID_TEXT) {
        Var *s = new_struct(V_TEXT(fn).Row);
        for (i = 0; i < V_TEXT(fn).Row; i++) {
        filename = strdup(V_TEXT(fn).text[i]);
        Var *t = do_loadISIS3(func, filename, data, transpose_info);
        if (t) {
            add_struct(s, filename, t);
    	}
    }
    if (get_struct_count(s)) {
        return (s);
        } else {
            free_struct(s);
            return (NULL);
        }
    } else if (V_TYPE(fn) == ID_STRING) {
        filename = V_STRING(fn);
        return(do_loadISIS3(func, filename, data, transpose_info));
    } else {
        parse_error("Illegal argument to function %s(%s), expected STRING", func->name, "filename");
        return (NULL);
        }
    }

static Var *
do_loadISIS3(vfuncptr func, char *filename, int data, int transpose_info)
{

    // char *err_file = NULL;
    char *fname;
    FILE *fp;
    Var *v = new_struct(0);
    Var *cubeVar = new_struct(0);
    Var *IsisCube = new_struct(0);
    Var *Object[5]; // Can nest up to 5 deep
    	Var *Group = new_struct(0);
			Var *stub = new_struct(0);

    int i,k,count;
    int objectNest = -1;
    char *workingObject = NULL;
    char *workingGroup = NULL;
    char namebuf[DV_NAMEBUF_MAX];
    static char noGroup[] = "NONE";
    int inGroup = 0;
    int position = 0;
    long whereInFile;  // Next read starts here
    long savedFileLocation; // Save location
    char *historyStrings = NULL;
    long historyOffset;
    long historySize;
    int inHistory = 0;
    int retVal = 0;
    int weNeedToRead = 1;
    int doneFlag = 0;

    /*
     * If the filename is NULL, davinci crashes before this code is reached.
     * So it is handled in the calling function    
     */

    //if (filename == NULL) {
    //    parse_error("%s: No filename specified\n", func->name);
    //   return (NULL);
    //}

    if ((fname = dv_locate_file(filename)) == (char*)NULL) {
        parse_error("%s: Unable to expand filename %s\n", func->name, filename);
        return (NULL);
    }

    if (access(fname, R_OK) != 0) {
        parse_error("%s: Unable to find file %s.", func->name, filename);
        return (NULL);
    }

    /**
    *** What about compression?
    **/
    if ((fp = fopen(fname, "rb")) != NULL) {
        if (iom_is_compressed(fp)) {
        /* fprintf(stderr, "is compressed\n");    FIX: remove */
        fclose(fp);
        fname = iom_uncompress_with_name(fname);
        fp = fopen(fname, "rb");
        }
    }
    


    parse_error("/**************************************************/\n");
	if(data == 1) { // we parse cube info
		parse_error("Parsing Cube Parameters...\n");
		data = getCubeParams(fp);
		if(data == 1) {
			parse_error("Reading the Cube...\n");
			cubeVar = readCube(fp);
            // pp_print(cubeVar);
		}

		fseek(fp, 0, SEEK_SET);

	}
	parse_error("Parsing Label...\n");
    for (k = 0; k < 2048; k++) { // 2048 seems a reasonable limit

    	if(weNeedToRead == 1) {
    		fgets(inputString, 2046, fp);
    	    whereInFile = ftell(fp);
    	    i = strlen(inputString);
    	}

        if(doneFlag == 0) { // Only do this if we are not done
            if(i > 1) { // There may be "blank" lines with only a \n
            if(weNeedToRead) {
            	// Note that there is a '\n' at the end of the input string
            	i-=1;
            	inputString[i] = '\0';

           	/*
          	 * For the most part:
          	 * string1 is item
          	 * string2 is '='
          	 * string3 is value
          	 * string4 is optional and should be units in <> brackets
           	 */

            	count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
            }
            else {
            	weNeedToRead = 1;
            }
            if(count >=2) {
            	position = stringFinder(); // Position of start of string3
            }

            // The last thing in the header is "End"
            // once we get here -- we are done
            //
            if( (strcmp("End", string1) == 0) && (count == 1)) {
                doneFlag = 1;
                continue;
            }

            /* Let's see if we are starting an Object */
            if(strcmp("Object", string1) == 0) {
                if(strcmp("Table", string3) == 0 ) {
                    int locali = 0;
                    if(transpose_info == 1) {
                    	locali = readNextLine(fp); // conditional!
                    }
                    if(locali == 1) {
                        weNeedToRead = 0;
                    }
                }
                strcpy(namebuf, string3);
    			if(objectNest >= 0) {
    				make_unique_name(namebuf, Object[objectNest]);
			   	}
				else {
					make_unique_name(namebuf, IsisCube);
				}
				workingObject = strdup(namebuf);
            	if(strcmp("IsisCube", string3) == 0) {
            	    workingGroup = noGroup;
            		inGroup = 0;
            	}
            	else {
            		objectNest++;  // this was because IsisCube was at -1 level
            		if(objectNest >=5) {
            			return NULL;
                    }

            		Object[objectNest] = new_struct(0);
            		if(strcmp("History", workingObject) == 0) {
            			/*
            			 * The time has come to parse History!
            			 */
            			parse_error("Reading History and storing as text...\n");
            			fgets(inputString, 2046, fp); // Name = IsisCube
                        fgets(inputString, 2046, fp); // StartByte = 92032424
                        count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
                        sscanf(string3, "%ld", &historyOffset);
                        fgets(inputString, 2046, fp); // Bytes = 8292
                        savedFileLocation = ftell(fp); // This is where we return to parsing later
                        count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
                        sscanf(string3, "%ld", &historySize);
                        fseek(fp, historyOffset-1, SEEK_SET);
                        inHistory = 1;
                        historyStrings = (char *)malloc(historySize + 1);  // The +1 is in case we need room for a \0
                        if(historyStrings == NULL) {
                            parse_error("Could not malloc space for History\n");
                            return NULL;
                        }
                        /*
                         * Here we read bytes instead of one big chunk to allow for
                         * any kind of historySize error--hopefully something is better than nothing
                         */
                        fread(historyStrings, 1, historySize, fp);
                        historyStrings[historySize] = '\0'; // Force '\0' at the end
                        stub = newString(historyStrings); // All the History in a text blob with a '\0' at the end
                        add_struct(Object[objectNest], "History", stub);
                        parse_error("Completed Reading History...\n");
                        fseek(fp, savedFileLocation, SEEK_SET);
                        // The next read is "End_Object"
                        // and this will be handled by the existing machinery

            		}
            	}
            if(weNeedToRead == 0) {
                strcpy(string1, string1_2);
                strcpy(string3, string3_2);
                strcpy(inputString,"=");
                strcat(inputString, string3);
            
            }


            }
			/* Let's see if we are starting a Group */
            else if (strcmp("Group", string1) == 0) {
                if(strcmp("Field", string3) == 0) {
                    int locali = 0;
                    if(transpose_info == 1) {
                    	locali = readNextLine(fp); // conditional!
                    }
                    if(locali == 1) {
                        weNeedToRead = 0;
                    }
                }
                strcpy(namebuf, string3);
                if(objectNest >= 0) {
                    make_unique_name(namebuf, Object[objectNest]);
                }
                else {
                    make_unique_name(namebuf, IsisCube);
                }
                workingGroup = strdup(namebuf);
            	inGroup = 1;
            	Group = new_struct(0);
                if(weNeedToRead == 0) {
                    strcpy(string1, string1_2);
                    strcpy(string3, string3_2);
                    strcpy(inputString,"=");
                    strcat(inputString, string3);

                }
            }
			/* Let's see if we are at an End_Object */
            else if (strcmp("End_Object", string1) == 0) {
                stub = newString(strdup("Object"));
               	if(objectNest >= 0 ) {
 					add_struct(Object[objectNest], "isis_struct_type", stub);
               		add_struct(IsisCube, workingObject, Object[objectNest--]);
               	}
               	else {
               		add_struct(IsisCube, "isis_struct_type", stub);
               	}
            }
			/* Let's see if we are at an End_Group */
            else if (strcmp("End_Group", string1) == 0) {
           		stub = newString(strdup("Group"));
        		add_struct(Group, "isis_struct_type", stub);
        		if(objectNest >= 0) {
            		add_struct(Object[objectNest], workingGroup, Group);
        		}
        		else {
            		add_struct(IsisCube, workingGroup, Group);
        	    }
        		// We are done with this Group
        		workingGroup = noGroup; // May not get another Group for a while, point to noGroup
        		inGroup = 0;
            }

			/* 
				We are not at the Beginning or End of something, so add the item to 
				what we have.
			*/
            else {
           		/* Make a unique name; either for the Group or Object we are in or the base IsisCube */
               	cNameMaker(string1); // Fix some problems observed in the input name
            	strcpy(namebuf, string1);
               
                if(inGroup == 1) {
            	    make_unique_name(namebuf, Group);
                }
                else if(objectNest >= 0) {
            	    make_unique_name(namebuf, Object[objectNest]);
                }
                else {
               		make_unique_name(namebuf, IsisCube);
                }

               	// do a little research on continuation lines
				// First is string lists enclosed by '(' and ')'
               	// These may be either text string lists or text strings representing number lists
               	// We don't consider mixed lists

               	if(strstr(inputString, "(") != NULL) {
               	    if(inGroup == 1) {
               	    	retVal = parenManager(inputString, fp, Group, namebuf);
               		}
               		else {
               			retVal = parenManager(inputString, fp, Object[objectNest], namebuf);
               		}
                    if(inHistory == 1) {
                        historySize -= retVal;
               	    }
                }                
                // The next is a series of strings starting with the character pair '"
                // This is just a long string
               	else if(strstr(inputString, "'\"") != NULL) {
                	    if(inGroup == 1) {
               	    	retVal = dQuoteManager(inputString, fp, Group, namebuf);
               		}
               		else {
               			retVal = dQuoteManager(inputString, fp, Object[objectNest], namebuf);
               		}         
               	    if(inHistory == 1) {
                        historySize -= retVal;
                    }	
               	}
               	else {
               		if(inGroup == 1) {
               			/* There may be <units> in string4. Units are strings enclosed by a '<' and a '>',
               			 * or string4 may be a part of a string3 that has spaces in it.
               			 *
               			 */
               			if ( (count == 4) && (string4[0] == '<') && (string4[-1 + strlen(string4)] == '>') ) {
               				add_converted_to_struct(Group, namebuf, string3);
               				strcat(namebuf, "Units");
               				add_struct(Group, namebuf, newString(strdup(string4)));
               			}
               			else {// &inputString[position] is the input string past the '=' with leading and trailing double quotes removed
               				add_converted_to_struct(Group, namebuf, &inputString[position]);
               			}
               		}
					else { 
              			/* There may be <units> in string4. Units are strings enclosed by a '<' and a '>' */
               			if ( (count == 4) && (string4[0] == '<') && (string4[-1 + strlen(string4)] == '>') ) {
               				add_converted_to_struct(Object[objectNest], namebuf, string3);
               				strcat(namebuf, "Units");
               				add_struct(Object[objectNest], namebuf, newString(strdup(string4)));
               			}
               			else {// &inputString[position] is the input string past the '=' with leading and trailing double quotes removed
               				add_converted_to_struct(Object[objectNest], namebuf, &inputString[position]);
               			}
   					}
               	}
            } // end of not at beginning or end
        } // end of length > 1
    } // end of not done yet
  	else {
  		parse_error("Found End of label...\n");
  		parse_error("/**************************************************/\n");
  		break;
  		}
  	} // End of header info for loop

  fclose(fp);

/*************************************************************************

Done parsing file

************************************************************************/
 // fop = malloc(4*4*4*4);
 // cubeVar = newVal(BSQ, 4, 4, 4, FLOAT, fop);
 // pp_print(cubeVar);
  if( (cubeVar != NULL) && (data != 0)) {
	  add_struct(v, "cube", cubeVar);
  }
  add_struct(v, "IsisCube", IsisCube);
  return (v);
}



/**************************************************************************/
void make_unique_name(char * buffer, Var * dv_struct) {
    /*

        Make unique names to be inserted in the davinci structure dv_struct by
        checking to see if the candidate name already exists. If it does not,
        the name remains unchanged.  If it does, it appends a _ and an integer
        value (starting at 2) and keeps trying, incrementing the integer by one,
        until a unique name is found.

        The buffer value is changed to the unique name by this function.

    */
    int start_count = 2;
    char work_buf[DV_NAMEBUF_MAX];
    Var * dummy_sptr;
    strncpy(work_buf, buffer, DV_NAMEBUF_MAX-1);
    while (find_struct(dv_struct, work_buf, &dummy_sptr) != -1) {
        if (snprintf(work_buf, DV_NAMEBUF_MAX-1, "%s_%i", buffer, start_count++) >= DV_NAMEBUF_MAX-1) {
            parse_error("Warning: attempt to find unique name for '%s' results in truncated strings.\nCannot rename. Please use shorter names in your ISIS files!");
            return;
        }
    }
    strncpy(buffer, work_buf, DV_NAMEBUF_MAX-1);
}



char get_keyword_datatype(char * value, char * name) {
    /* Determines the davinci datatype to apply to a value string, given its value
       and its name for hints. */
    char * endptr;
    int intval;
    double floatval;
    int is_int = 0, is_double = 0;
    errno = 0;

    /* check for int value. Note that the whole string must convert to be an int
    or it is more likely a string like a date/time value */

    intval = strtol(value, &endptr, 10);
    if (errno) is_int = 0; else is_int = 1;
    if (endptr != value+strlen(value)) is_int = 0;

    /* check for float value. Note that like the int, the whole string must convert
    to be a float or it is more likely a string like a date/time value */

    floatval = strtod(value, &endptr);
    if (errno) is_double = 0; else is_double = 1;
    if (endptr != value+strlen(value)) is_double = 0;

    /* if both tests pass, check to see if they are numerically equal. Floating point
       values only occasionally integral, and davinci can do the appropriate casting
       in any case, so it's a good compromise. */

    /*  If the name of the field contains the string 'Version', don't translate it
        to a number, but keep it as a string.
    */
    if (name != NULL && strstr(name, "Version") != NULL) {
        is_int = 0;
        is_double = 0;
    }
    if (name != NULL && strstr(name, "SpacecraftClockCount") != NULL) {
        is_int = 0;
        is_double = 0;
    }
    if (is_int && is_double) {
        if (intval == floatval) {
            is_double = 0;
        }
        else {
            is_int = 0;
        }
    }

    if (is_double) return 'd';
    if (is_int) return 'i';
    return 'c';

}

void add_converted_to_struct(Var * dv_struct, char * name, char * value) {
    /* Attempts to convert keyword values to integers and floats to determine if
    the value supplied is a numeric value. The rules are:
    Try to make it an int and a float.
    If they are both, compare the values and if they are equal, make it an int.
    If they are not, make it a float.
    If only one conversion worked, use that conversion.
    If neither conversion worked, then make it a string.
    */

    char datatype;

    datatype = get_keyword_datatype(value, name);

    if (datatype == 'i') {
        add_struct(dv_struct, name, newInt(atol(value)));
    }
    else if (datatype == 'd') {
        add_struct(dv_struct, name, newDouble(atof(value)));
    }
    else {
        add_struct(dv_struct, name, newString(strdup(value)));
    }
}

/*
 * Some comments on CSV strings I have seen:  There are lots of CSV strings in the cubes.  The longs ones are wrapped with leading spaces.
 * The tools parenManager() and dQuoteManager remove leading spaces from the wrapped lines and concatenate the strings.  This may mean that there will be
 * no white space in a few instances:
 * one, two, three,four, five
 *                ^ comma, but no space white for example
 * BUT, in at least one case I found a long string wrapped with no comma.  Forcing a space between concatenations may thus add a space to a string
 * not intended to have spaces.  Thus, I have elected to strip leading spaces from read in strings.
 * Note that all Tokens are based on ','s and not white spaces.
 */


int parenManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf) {
    char *workingString = NULL;
    char *localString = NULL;
    char *token = NULL;
    char localBuff[400];
    int length = 0;
    int addedLength = 0;
    int commaCount = 0;
    int retVal = 0;
    int i = 0;
    int k = 0;
    int conversionCount = 0;
    double *numberArray = NULL;
    double checkDouble = 99.9; // Any value would do
   // char **stringArray = NULL;
    Var *localStub = new_struct(0);
    Var *subItem = new_struct(0);

    while (stringItem[i]  != '(') {
    	i++;
    }
    i++; // Now we are past the leading '('

    length  = strlen(&stringItem[i]);

    while( (stringItem[i] == 0x20) && (i < length)) { // 0x20 is a space, may be a leading space
        i++;
    }

    workingString = strdup(&stringItem[i]);
    length = 1 + strlen(workingString); // '1 + ' is accounting for '\0' at the end of the string

    while (strstr(workingString, ")") == NULL) {
    	fgets(localBuff, 400, fp);
        retVal += strlen(localBuff);
    	addedLength = strlen(localBuff) - 1;
    	localBuff[addedLength] = '\0'; // delete the '\n'

    	i = 0;
    	while( (localBuff[i] == 0x20) && (i < addedLength)) { // removing leading spaces
    		i++;
    	}
    	length = length + strlen(&localBuff[i]);
    	localString =	realloc(workingString, length);
    	if (localString != NULL) {
    		workingString = localString;
    		strcat(workingString, &localBuff[i]);
    		// printf("Realloc worked %d,%d,%s\n", length, strlen(workingString),workingString);
    		}
    	else {
    		retVal = -1;
    		return retVal;
    		}
    	}

    /*  
     * We now have a CSV list of items in one string.
     * Most leading spaces have been deleted.
     * Replace the closing ')' with a ','
     */

    for(i = 0; i< strlen(workingString); i++) {
	    if(workingString[i] == ',') {
    		commaCount++;
	    }

	    if(workingString[i] == ')') {
            workingString[i] = ',';
            commaCount++;// There was one more element than there were ','s until we just added this ','
	    }
    }


    token = strtok(workingString, ",");
    i = 0;
    length = strlen(token);
    while ( (token[i] == 0x20) && (i < length) ) {
	    i++;
    }


    /*
    * The question is, is this a list of numbers,
    * or is it a list of strings?
    */

    conversionCount = sscanf(&token[i], "%lf", &checkDouble);

    if(conversionCount == 1) { // We have a list of numbers--we will just use doubles
	    numberArray = malloc(commaCount * sizeof(double));
	    if(numberArray == NULL) {
		    retVal = -1;
		    return retVal;
	    }
	    numberArray[0] = checkDouble;
	    for(i = 1; i < commaCount; i++) {
		    token = strtok(NULL, ",");
		    sscanf(token, "%lf", &numberArray[i]);
	    }


    	free(workingString);

    	localStub = newVal(BSQ, commaCount, 1,1, DOUBLE, numberArray);
	    add_struct(GroupOrObject, namebuf, localStub);

    }

    else { // We have a list of strings

    	//stringArray = malloc(commaCount * sizeof(char*));
	    //if(stringArray == NULL) {
		//    retVal = -1;
		//    return retVal;
	    //}

        localStub = newString(strdup(&token[i]));
        add_struct(subItem, NULL, localStub);
        for(i = 1; i < commaCount; i++) {
  	        token = strtok(NULL, ",");
  	        addedLength = strlen(token);
  	        k = 0;
  	        while( (token[k] == 0x20) && (k < addedLength)) {
  	        	k++;
  	        }
  	        localStub = newString(strdup(&token[k]));
  	        add_struct(subItem, NULL, localStub);
        }

     	free(workingString);

        add_struct(GroupOrObject, namebuf, subItem);
    }
    return retVal;
}

int dQuoteManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf) {
    char *workingString = NULL;
    char *localString = NULL;
    char localBuff[400];
    int length = 0;
    int addedLength = 0;
    int retVal = 0;
    int i = 0;
    Var *localStub = new_struct(0);

    while (stringItem[i]  != '\'') {
    	i++;
    }

    workingString = strdup(&stringItem[i]);
    length = 1 + strlen(workingString); // '1 +' to allow space for a '\0'

    while (strstr(workingString, "\"'") == NULL) { // begins with '" and ends with "'
    	fgets(localBuff, 400, fp);
        retVal += strlen(localBuff);
    	addedLength = strlen(localBuff) - 1;

    	localBuff[addedLength] = '\0';
    	i = 0;
    	while( (localBuff[i] == 0x20) && (i < addedLength)) { // removing leading spaces
    		i++;
    	}
    	length = length + strlen(&localBuff[i]);
    	localString = realloc(workingString, length);
    	if (localString != NULL) {
    		workingString = localString;
    		strcat(workingString, &localBuff[i]);
    		}
    	else {
    		retVal = -1;
    		return retVal;
    		}
    	}

    /*  
     * We now have a longish string.
     */
    
    localStub = newString(workingString);
    add_struct(GroupOrObject, namebuf, localStub);
    return retVal;
}


/***********************************************************/
/*
 * What we have are item-=-value-units constructs like:
 * string1    string2    string3    string4
 *    item          =      value    units
 * The thing is, string3 may have spaces in it.
 * Units is optional.  If string3 has spaces in it,
 * then string4 is just the next text word of what should be part of string3, and string3 is truncated.
 * So we have this function which returns an offset to the first letter in the first word past the string2 '=' sign
 */
int stringFinder(void) {
int i = 0;
int innerString;
int length;

while (inputString[i] != '=') {
		i++;
	}
i++; // Now we are past the '='
// There might be 'spaces'
while(inputString[i] == 0x20) { // 0x20 is a space
		i++;
	}

// There might be strings with embedded spaces
// These strings might be enclosed in double quotes
// I don't want the double quotes
// If these do remain, then strings with embedded spaces will print like
// ""This is a string with embedded spaces""
// What I want is
// "This is a string with embedded spaces"
if (inputString[i] == 0x22) { // 0x22 is a "
	i++;
	length = strlen(inputString);
	for(innerString = i; innerString < length; innerString++) {
		if(inputString[innerString] == 0x22) {
			inputString[innerString] = '\0';
		}
	}
}

return i;
}

/*
 * I have seen names in ISIS3 files that include minus signs and dots.
 * I have not seen all these other symbols, but they could cause a problem
 * Replace as shown, and the string becomes a C name that davinci can parse
 */
int cNameMaker(char *inputString) {
int result = 0;
int i;

for(i = 0; i < strlen(inputString); i++) {
	switch(inputString[i]) {
	case '-':	// a minus sign
	case '.':	// a dot
	case ':':	// a colon
	case '+':	// a plus sign
	case ';':	// a semicolon
	case ' ':   // a space, not very possible
		inputString[i] = '_'; // each individually replace by and underbar
	break;
	default:
		;// nothing, just leave it alone
	}

}


return result; // I don't see a failure possible here, so for now, always good
}



int readNextLine(FILE *fp) {
    int locali;

    fgets(inputString, 2046, fp);
    locali = strlen(inputString);
    locali-=1;
    inputString[locali] = '\0';
    sscanf(inputString, "%s%s%s%s", string1_2, string2_2, string3_2, string4_2);
    locali = 0;
    if(strcmp("Name", string1_2) == 0) {
        strcpy(interString, string3);
        strcpy(string3, string3_2);
        strcpy(string3_2, interString);
        locali = 1;
    }
    return locali;

}


Var *readCube(FILE *fp) {
Var *dv_struct = new_struct(0);
unsigned char *cubeData;
unsigned char inBuffer[4];
unsigned char swapBuffer[4];
int cubeElement = 0;
int x, y, z, element, offset, linearOffset;

if(Format == I3Tile) {
	tileSize = TileSamples * TileLines * sampleSize;
	tilesPerRow = Samples/TileSamples;
	subTileSamples = Samples%TileSamples;  // What is left over going across
	subTileSize = subTileSamples * TileLines * sampleSize;
	totalRows = Lines/TileLines;
	subTileLines = Lines%TileLines; // What is left going down

//	if( (subTileSamples !=0) || (subTileLines !=0) ) {
//		parse_error("Irregular Tile sizes--can't read this cube into RAM\n");
//		return NULL;
	if(subTileSamples != 0) {
		tilesPerRow+=1;
	}
	if(subTileLines != 0) {
		totalRows+=1;
	}
}

cubeData = malloc(Samples*Lines*totalBands*sampleSize);

if(cubeData == NULL) {
	parse_error("Cannot malloc() enough space to hold Cube\n");
	return NULL;
}

if(Format == BandSequential) {

	fseek(fp, StartByte-1, SEEK_SET);  // Need only the one fseek() to set up all
    offset = 0;  // This is the offset in to the storage structure
	for(z = 0; z < totalBands; z++) {
		for(y = 0; y < Lines; y++) {
			for (x = 0; x < Samples; x++) {
				if (1 == fread(inBuffer, sampleSize, 1, fp) ) {
				    
					if(ByteOrder == Msb) {
							switch(sampleSize){
							case 2:
								swapBuffer[1] = inBuffer[0];
								swapBuffer[0] = inBuffer[1];
								break;
							case 4:
								swapBuffer[3] = inBuffer[0];
								swapBuffer[2] = inBuffer[1];
								swapBuffer[1] = inBuffer[2];
								swapBuffer[0] = inBuffer[3];
								break;
							default:
								;
							}

						for (element = 0; element < sampleSize; element++) {
							cubeData[offset + element] = swapBuffer[element];
						}
					}
					else {
						for (element = 0; element < sampleSize; element++) {
							cubeData[offset + element] = inBuffer[element];
						}
					}

                    offset+=sampleSize; // offset increases by sample size

				} // end of good read
				else {
					parse_error("Bad Read for Band Sequential Cube\n");
					return NULL;
				}
			} // end x
		} // end y
	} // end z
} // end BandSequential

if(Format == I3Tile) {
	linearOffset = 0;  // Where the write starts to the storage area
    for(Bands = 0; Bands < totalBands; Bands++) {
    	for (Row = 0; Row < totalRows; Row++) {
    		for(row = 0; row < TileLines; row++) {
    			for( tiles = 0; tiles < tilesPerRow; tiles++) {
    				offset = StartByte - 1;
    				offset += Bands * (Samples * Lines * sampleSize);   // How many whole "sheets" we have read
    				offset += ( Row * tilesPerRow * tileSize);  // This is the number of full Rows of full tiles
      				offset += (tiles*tileSize); // This is the offset of the number of full tiles already read in the Row
    				offset += (row * TileSamples * sampleSize); // How many full rows we have read so far in this tile

    				fseek(fp, offset, SEEK_SET); // for Tiled, fseek() each TileSample start

    				for (x = 0; x < TileSamples; x++) { // Reading across a tile
    					if (1 == fread (inBuffer, sampleSize, 1, fp)) {

    						if((subTileSamples != 0) && (tiles == (tilesPerRow-1)) ) {
    							if(x >= subTileSamples) {
    								continue;  // We read it, we just don't put it in the structure
    							}
    						}

    						if((subTileLines != 0) && (Row == (totalRows-1)) ) {
    							if(row >= subTileLines) {
    								continue; // We read it, we just don't put it in the structure
    							}
    						}


   							if(ByteOrder == Msb) {
   								switch(sampleSize){
   								case 2:
   									swapBuffer[1] = inBuffer[0];
   									swapBuffer[0] = inBuffer[1];
   									break;
   								case 4:
   									swapBuffer[3] = inBuffer[0];
   									swapBuffer[2] = inBuffer[1];
   									swapBuffer[1] = inBuffer[2];
   									swapBuffer[0] = inBuffer[3];
   									break;
   								default:
   									;
   								}

   								for (element = 0; element < sampleSize; element++) {
   									cubeData[linearOffset + element] = swapBuffer[element];
   								}
   							}
   							else {
   								for (element = 0; element < sampleSize; element++) {
   									cubeData[linearOffset + element] = inBuffer[element];
   								}
   							}
   							linearOffset+=sampleSize; // linearOffset into the RAM data just keeps incrementing


    					} // end of good read

    					else {
    						parse_error("Bad Read for Tile Cube\n");
    						return NULL;
    					}
    				} // end TileSamples
    			} // end tilesPerRow
    		} // end rows
    	} // end Rows
    }  // end Bands
} // end I3Tile


switch (sampleSize) {
case 1:
	cubeElement = BYTE;
	break;
case 2:
	cubeElement = SHORT;
	break;
case 4:
	cubeElement = FLOAT;
	break;
default:
	cubeElement = BYTE;
}


dv_struct = newVal(BSQ, Samples, Lines, totalBands, cubeElement, cubeData);
// pp_print(dv_struct);

parse_error("Finished reading the Cube into RAM...\n");
return dv_struct;
}



int getCubeParams(FILE *fp) {
	int retVal = 1;
	int count;
	fgets(inputString, 2046, fp);
	sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
	if(strcmp("IsisCube", string3) !=0) {
		parse_error("This is not an Isis3 Cube\n");
		return 0;
	}

	while(strcmp("End_Object", string1) != 0) {
		fgets(inputString, 2046, fp);
		count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
		if(strlen(inputString) < 3) {  // Some blank lines
			continue;
		}

		if(count != 3) { // There something to parse out only if there are three strings
			continue;
		}

		if(strcmp("StartByte", string1) == 0 ) {
			sscanf(string3, "%d", &StartByte);
		}

		else if(strcmp("Format", string1) == 0 ) {
			if(strcmp("Tile", string3) == 0 ) {
				Format = I3Tile;
			}
			else {
				Format = BandSequential;
			}
		}

		else if(strcmp("TileSamples", string1) == 0 ) {
			sscanf(string3, "%d", &TileSamples);
		}

		else if(strcmp("TileLines", string1) == 0 ) {
			sscanf(string3, "%d", &TileLines);
		}

		else if(strcmp("Samples", string1) == 0 ) {
			sscanf(string3, "%d", &Samples);
		}

		else if(strcmp("Lines", string1) == 0 ) {
			sscanf(string3, "%d", &Lines);
		}

		else if(strcmp("Bands", string1) == 0 ) {
			sscanf(string3, "%d", &totalBands);
		}

		else if(strcmp("Type", string1) == 0 ) {
			if(strcmp("UnsignedByte", string3) == 0) {
				sampleSize = 1;
			}
			else if(strcmp("SignedWord", string3) == 0) {
				sampleSize = 2;
			}
			else {
				sampleSize = 4; // only other choice is Real which is really float of size 4
			}
		}

		else if(strcmp("ByteOrder", string1) == 0 ) {
			if(strcmp("Lsb", string3) == 0) {
				ByteOrder = Lsb;
			}
			else {
				ByteOrder = Msb;
			}
		}

		else if(strcmp("Base", string1) == 0 ) {
			sscanf(string3, "%lf", &Base);
		}

		else if(strcmp("Multiplier", string1) == 0 ) {
			sscanf(string3, "%lf", &Multiplier);
		}

		else {
			; // nothing for now
		}
	} // end of the while loop

	parse_error("Completed parsing cube parameters...\n");

	return retVal;


}
