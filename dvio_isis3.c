/*
 * a = load_isis3("/u/ddoerres/work/Project/themis_dbtools/themis_dbtools/trunk/makerdr/I01001001.cub")
 * b = load_isis3("/local/cube/SE_500K_0_0_SIMP.cub")
 * c = load_isis3("/local/cube/ESP_038117_1385_RED.cub")             <-- This is the really big cube
 * d = load_isis3("/u/cedwards/davinci_test/I01001001.lev1.cub")
 * e = load_isis3("/local/cube/32bit_msb_float.cub")
 *
 */

/*
 * drd
 *
 * 13 April--replaced long long by size_t
 * look for "drd history"
 * Total seven places
 * to bring history back in line
 * rather than separate
 */

/*
 * drd
 *
 * Bug 2171 - Davinci did not have a native method to write out ISIS3 files
 * This bug fixes that.
 * Made changes to the header files func.h and ff.h.
 * Added write_isis3() and write_ISIS3() to dvio_isis3.c (this file)
 * This initial version has minimal header information. BandSequential and Tile
 * are supported.  Both lsb and msb are supported
 * Thanks to size_t types, structures larger than can be accommodated by 32 bit
 * variables ARE possible
 */


// #define or #undef
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#undef PRINTINPUT
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#include "dvio_isis3.h"

#include "dvio.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>

#define _FILE_OFFSET_BITS 64

static Var *do_loadISIS3(vfuncptr func, char *filename, int data, int use_names,
        int use_units); // drd static so only visible in this file
static Var *do_writeISIS3(Var *obj, char *ISIS3_filename, char *bsqTile, int valTs, int valTl); // drd static so only visible in this file

static int parenManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *nambuf);
static int dQuoteManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf);
static void make_unique_name(char *buffer, Var *dv_struct);
static char get_keyword_datatype(char *value, char *name);
static void add_converted_to_struct(Var *dv_struct, char * name, char * value);
static int stringFinder(FILE *fp);
static int cNameMaker(char *inputString);
static int readNextLine(FILE *fp);
static int getCubeParams(FILE *fp);
static Var *readCube(FILE *fp);
static int writeCube(FILE *fp, minIsisInfo *isisinfo, unsigned char *data);
static char inputString[2048];
static char continuationString[2048];
static char string1[200], string2[200], string3[200], string4[200];
static char string1_2[80], string2_2[80], string3_2[80], string4_2[80];
static char interString[80];

static int StartByte = 1;
static int Format = 1;
static size_t Samples = 1;
static size_t Lines = 1;
static size_t totalBands = 1;
static size_t sampleSize = 1;   // "Type"
static int ByteOrder = 1;
static double Base = 0.0;
static double Multiplier = 1.0;

static size_t tileSize = 1;
static size_t TileSamples = 1;
static size_t TileLines = 1;

static size_t subTileSize = 1;
static size_t tiles = 1;
static size_t row = 1;
static size_t Row = 1;
static size_t Bands = 1;
static size_t tilesPerRow = 1;
static size_t subTileSamples = 1;
static size_t totalRows = 1;
static size_t subTileLines = 1;

static char cubeStub[][160] = {
		{ "Object = IsisCube" },                // 00
		{ "  Object = Core" },                  // 01
		{ "    StartByte   = " },               // 02 65537
		{ "    Format      = " },               // 03 Tile or BSQ
		{ "    TileSamples = " },               // 04
		{ "    TileLines   = " },               // 05
		{ "" },                                 // 06
		{ "    Group = Dimensions" },           // 07
		{ "      Samples = " },                 // 08
		{ "      Lines   = " },                 // 09
		{ "      Bands   = " },                 // 10
		{ "    End_Group" },                    // 11
		{ "" },                                 // 12
		{ "    Group = Pixels" },               // 13
		{ "      Type       = " },              // 14
		{ "      ByteOrder  = " },              // 15 Lsb, Msb -- be careful of this if it is NOT Lsb
		{ "      Base       = " },              // 16 0.0
		{ "      Multiplier = " },              // 17 1.0
		{ "    End_Group" },                    // 18
		{ "  End_Object" },                     // 19
		{ "End_Object" },                       // 20
		{ "" },                                 // 21
		{ "Object = Label" },                   // 22
		{ "  Bytes = " },                       // 23 65536
		{ "End_Object" },                       // 24
		{ "End" }                               // 25
		};
/*

 Group = BandBin
 Name = Gray
 End_Group
 End
 };
 */

#ifdef PRINTINPUT
static int countFix = 32;
#endif

/*
 * a = load_isis3("/u/ddoerres/work/Project/themis_dbtools/themis_dbtools/trunk/makerdr/I01001001.cub")
 * write_isis3(a, "Cow", force = 1)
 * b = load_isis3("Cow")
 * write_isis3(b, "Cow", force = 1)
 * b = load_isis3("Cow")
 * write_isis3(b, "Cow", force = 1, "bsQ", valTs=320, valTl=449) // Note that valTs and valTl are ignored
 * b = load_isis3("Cow")
 * write_isis3(b, "Cow", force = 1, "tiLE", valTs=320, valTl=449)
 * b = load_isis3("Cow")
 * write_isis3(b, "Cow", force = 1, "TiLe", valTs=160, valTl=449)
 * b = load_isis3("Cow")
 * write_isis3(a, "Cow", force = 1, "TiLe", valTs=133, valTl=111)
 *
 */

Var* WriteISIS3(vfuncptr func, Var * arg)   // drd proto for this is in func.h
{
	Var *obj = NULL;
	char *filename = NULL;
	int force = 0;
	char *bsqTile;
	char bsqTileBuffer[80];
	int ts = -1;
	int tl = -1;

	static char bsqItem[] = "BandSequential";
	static char tileItem[] = "Tile";
	static char nothing[] = "nothing";

	bsqTile = &nothing[0];

	if (arg == NULL) {
		parse_error("No parameter list supplied--must supply at least an ISIS3 object and file name.\n");
		return (NULL);
	}

	Alist alist[7];
	alist[0] = make_alist("obj", ID_UNK, NULL, &obj);
	alist[1] = make_alist("filename", ID_STRING, NULL, &filename);
	alist[2] = make_alist("force", DV_INT32, NULL, &force);
	alist[3] = make_alist("bsqTile", ID_STRING, NULL, &bsqTile);
	alist[4] = make_alist("valTs", DV_INT32, NULL, &ts);
	alist[5] = make_alist("valTl", DV_INT32, NULL, &tl);
	alist[6].name = NULL;

	if (parse_args(func, arg, alist) == 0) {
		parse_error("No useful command line, should have: ISIS3object, \"filename\", [force=0,1], [[\"BSQ\",\"Tile\"], [valTs=value], [valTl=value]]");
		return NULL;
	}

	if (obj == NULL) {
		parse_error("No ISIS3 object specified.");
		return NULL;
	}

	if (filename == NULL) {
		parse_error("No filename specified.");
		return NULL;
	}

	if (!force && access(filename, F_OK) == 0) {
		parse_error("%s: File %s already exists.", func->name, filename);
		return (NULL);
	}

	if (V_TYPE(obj) == ID_STRUCT) {
		int i;
		strcpy(bsqTileBuffer, bsqTile);
		for(i = 0; i < strlen(bsqTile) ; i ++) {
			bsqTileBuffer[i] = toupper(bsqTileBuffer[i]);
		}
		if( strcmp("BSQ", bsqTileBuffer) == 0) {
			strcpy(bsqTileBuffer, bsqItem);
			/*
			 * If we are BSQ, we are NOT tiled
			 * so set these to negative
			 */
			ts=-1;
			tl=-1;
		}
		/*
		 * If we are Tiled, we must have
		 * positive ts and positive tl
		 */
		if( strcmp("TILE", bsqTileBuffer) == 0) {
			if( (ts < 0) || (tl < 0) ) {
				strcpy(bsqTileBuffer, bsqItem);
				ts=-1;
				ts=-1;
			}
			else {
				strcpy(bsqTileBuffer, tileItem);
			}
		}
		bsqTile = bsqTileBuffer;
		return (do_writeISIS3(obj, filename, bsqTile, ts, tl));
	} else {
		printf("V_Type(obj)=%d\n", V_TYPE(obj));
		parse_error("You have submitted an invalid object to be written out as an ISIS3 file");
		return NULL;
	}
}


Var* do_writeISIS3(Var *obj, char *ISIS3_filename, char *bsqTile, int valTs, int valTl)
{
	int count;
	int writeResult = 1;
	int i, j;
	char *name;
	char buffer[160];
	char *ISIS3label = NULL;
	unsigned char *isis3Data = NULL;

	size_t allData = 0;

	Var *objIsisCube = NULL;
	Var *objCore = NULL;
	Var *objDimensions = NULL;
	Var *objPixels = NULL;
	Var *objLabel = NULL;
	Var *objTemp = NULL;

	FILE *fp;

	minIsisInfo isisinfo; // in isis3Include.h

	if ((fp = fopen(ISIS3_filename, "wb")) == NULL) {
		fprintf(stderr, "Unable to open file: %s\n", ISIS3_filename);
		return NULL;
	}

	i = find_struct(obj, "IsisCube", &objIsisCube);

	if (i >= 0) {
		i = find_struct(objIsisCube, "Core", &objCore);
	} else {
		fprintf(stderr, "Object is not an ISIS3 cube\n");
		return NULL;
	}


	if (i >= 0) {

		// in case these are not there, default
		isisinfo.TileSamples = valTs;
		isisinfo.TileLines = valTl;

		count = get_struct_count(objCore);
		for (j = 0; j < count; j++) {
			get_struct_element(objCore, j, &name, &objTemp);

			if (strcmp("StartByte", name) == 0) {
				isisinfo.StartByte = ((int *) V_DATA(objTemp))[0];
				//printf("StartByte is %d\n", isisinfo.StartByte);
			}

			else if (strcmp("Format", name) == 0) {
		   	    if(strcmp(bsqTile, "NOTHING") == 0) {
		   	    	isisinfo.Format = V_STRING(objTemp);
		   	    }
		   	    else {
		   	    	isisinfo.Format = bsqTile;
		   	    }
				//printf("Format is %s\n", isisinfo.Format);
		   	}

			else if (strcmp("TileSamples", name) == 0) {
				if( (isisinfo.TileSamples < 0) &&
					strcmp (isisinfo.Format, "Tile") == 0) {

					isisinfo.TileSamples = ((int *) V_DATA(objTemp))[0];
					//printf("TileSamples is %d\n", isisinfo.TileSamples);
				}
			}

			else if (strcmp("TileLines", name) == 0) {
				if( (isisinfo.TileLines < 0) &&
					strcmp (isisinfo.Format, "Tile") == 0) {

					isisinfo.TileLines = ((int *) V_DATA(objTemp))[0];
					//printf("TileLines is %d\n", isisinfo.TileLines);
				}
			}
		}
	}

	if (i >= 0) {
		i = find_struct(objCore, "Dimensions", &objDimensions);
		if (i >= 0) {
			count = get_struct_count(objDimensions);
			for (j = 0; j < count; j++) {

				get_struct_element(objDimensions, j, &name, &objTemp);

				if (strcmp("Samples", name) == 0) {
					isisinfo.Samples = ((int *) V_DATA(objTemp))[0];
					//printf("Samples is %d\n", isisinfo.Samples);
				}
				else if (strcmp("Lines", name) == 0) {
					isisinfo.Lines = ((int *) V_DATA(objTemp))[0];
					//printf("Lines is %d\n", isisinfo.Lines);
				}
				else if (strcmp("Bands", name) == 0) {
					isisinfo.Bands = ((int *) V_DATA(objTemp))[0];
					//printf("Bands is %d\n", isisinfo.Bands);
				}
			}
		}
	}

	if (i >= 0) {
		i = find_struct(objCore, "Pixels", &objPixels);
		if (i >= 0) {
			count = get_struct_count(objPixels);
			for (j = 0; j < count; j++) {
				get_struct_element(objPixels, j, &name, &objTemp);

				if (strcmp("Type", name) == 0) {
					isisinfo.Type = V_STRING(objTemp);
					//printf("Type is %s\n", isisinfo.Type);
				}
				else if (strcmp("ByteOrder", name) == 0) {
					isisinfo.ByteOrder = V_STRING(objTemp);
					//printf("ByteOrder is %s\n", isisinfo.ByteOrder);
				}
				else if (strcmp("Base", name) == 0) {
					isisinfo.Base = ((double *) V_DATA(objTemp))[0];
					//printf("Base is %.1f\n", isisinfo.Base);
				}
				else if (strcmp("Multiplier", name) == 0) {
					isisinfo.Multiplier = ((double *) V_DATA(objTemp))[0];
					//printf("Multiplier is %.1f\n", isisinfo.Multiplier);
				}
			}
		}
	}

	if (i >= 0) {
		i = find_struct(objIsisCube, "Label", &objLabel);
		if (i >= 0) {
			count = get_struct_count(objLabel);
			for (j = 0; j < count; j++) {
				get_struct_element(objLabel, j, &name, &objTemp);

				if (strcmp("Bytes", name) == 0) {
					isisinfo.Bytes = ((int *) V_DATA(objTemp))[0];
					//printf("Bytes is %d\n", isisinfo.Bytes);
				}
			}
		}
	}


	if (i >= 0) {
		ISIS3label = calloc(0x10000, sizeof(unsigned char));
		if(ISIS3label == NULL) {
			fprintf(stderr, "Could not allocate space for building label");
			return NULL;
		}

		strcpy(ISIS3label, &cubeStub[0][0]);                            //  { "Object = IsisCube" },                // 00
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[1][0]);                            //  { "  Object = Core" },                  // 01
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[2][0]);                            //  { "    StartByte   = " },               // 02
		sprintf(buffer, "%d\n", isisinfo.StartByte);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[3][0]);                            //  { "    Format      = " },               // 03
		sprintf(buffer, "%s\n", isisinfo.Format);
		strcat(ISIS3label, buffer);

		 /*
		  * We must be Tile Format or
		  * these don't occur
		  */

		if(strcmp(isisinfo.Format, "Tile") == 0) {

			strcat(ISIS3label, &cubeStub[4][0]);                    //  { "    TileSamples = " },               // 04
			sprintf(buffer, "%d\n", isisinfo.TileSamples);
			strcat(ISIS3label, buffer);

			strcat(ISIS3label, &cubeStub[5][0]);                    //  { "    TileLines = " },                 // 05
			sprintf(buffer, "%d\n", isisinfo.TileLines);
			strcat(ISIS3label, buffer);
		}


		strcat(ISIS3label, &cubeStub[6][0]);                            //  { "" }                                  // 06
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[7][0]);                            //  { "    Group = Dimensions" },           // 07
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[8][0]);                            //  { "      Samples = " },                 // 08
		sprintf(buffer, "%d\n", isisinfo.Samples);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[9][0]);                            //  { "      Lines   = " },                 // 09
		sprintf(buffer, "%d\n", isisinfo.Lines);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[10][0]);                           //  { "      Bands   = " },                 // 10
		sprintf(buffer, "%d\n", isisinfo.Bands);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[11][0]);                            //  { "    End_Group" },                   // 11
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[12][0]);                            //  { "" }                                 // 12
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[13][0]);                            //  { "    Group = Pixels" },              // 13
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[14][0]);                            //  { "      Type       = " },             // 14
		sprintf(buffer, "%s\n", isisinfo.Type);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[15][0]);                            //  { "      ByteOrder  = " },             // 15 Lsb, Msb
		sprintf(buffer, "%s\n", isisinfo.ByteOrder);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[16][0]);                            //  { "      Base       = " },             // 16 0.0
		sprintf(buffer, "%.1f\n", isisinfo.Base);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[17][0]);                            //  { "      Multiplier = " },             // 17 1.0
		sprintf(buffer, "%.1f\n", isisinfo.Multiplier);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[18][0]);                            //  { "    End_Group" },                   // 18
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[19][0]);                            //  { "  End_Object" },                    // 19
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[20][0]);                            //  { "End_Object" },                      // 20
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[21][0]);                            //  { "" }                                 // 21
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[22][0]);                            //  { "Object = Label" },                  // 22
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[23][0]);                            //  { "  Bytes = " },                      // 23 65536
		sprintf(buffer, "%d\n", isisinfo.Bytes);
		strcat(ISIS3label, buffer);

		strcat(ISIS3label, &cubeStub[24][0]);                            //  { "End_Object" },                      // 24
		strcat(ISIS3label, "\n");

		strcat(ISIS3label, &cubeStub[25][0]);                            //  { "End" },                             // 25
		strcat(ISIS3label, "\n");


		fwrite(ISIS3label, 0x10000, 1, fp);

		// fprintf(stdout, "\n\n%s", ISIS3label);
		// fflush(stdout);

		free(ISIS3label);

		get_struct_element(obj, 0, &name, &objTemp);

		isis3Data = (unsigned char *) V_DATA(objTemp);

		writeResult = writeCube(fp, &isisinfo, isis3Data);

		fclose(fp);

	}
	else {
		fprintf(stderr, "Malformed ISIS3 Cube");
	}

	return NULL;

}

Var* ReadISIS3(vfuncptr func, Var * arg)   // drd proto for this is in func.h
{
	Var *fn = NULL;
	char *filename = NULL;
	int data = 1;  // parse the cube
	int use_names = 1; // reverse name and info if 1
	int use_units = 0; // use the units field if 1
	int i;

	/*
	 * If the user inputs load_isis3() or load_ISIS3()
	 * with empty paren's, bail out
	 */

	if (arg == NULL) {
		parse_error(
				"No parameter list supplied--must at least supply an input file name.\n");
		return (NULL);
	}

	Alist alist[5];
	alist[0] = make_alist("filename", ID_UNK, NULL, &fn);
	alist[1] = make_alist("data", DV_INT32, NULL, &data);
	alist[2] = make_alist("use_names", DV_INT32, NULL, &use_names);
	alist[3] = make_alist("use_units", DV_INT32, NULL, &use_units);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) {
		return (NULL);
	}

	/* Handle loading many filenames */
	if (V_TYPE(fn) == ID_TEXT) {
		Var *s = new_struct(V_TEXT(fn).Row);
		for (i = 0; i < V_TEXT(fn).Row; i++) {
			filename = strdup(V_TEXT(fn).text[i]);
			Var *t = do_loadISIS3(func, filename, data, use_names, use_units);
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
		return (do_loadISIS3(func, filename, data, use_names, use_units));
	} else {
		parse_error("Illegal argument to function %s(%s), expected STRING",
				func->name, "filename");
		return (NULL);
	}
}

static Var* do_loadISIS3(vfuncptr func, char *filename, int data, int use_names, int use_units)
{

	char *fname;
	FILE *fp;
	Var *v = new_struct(0);
	Var *cubeVar = new_struct(0);
	Var *IsisCube = new_struct(0);
	Var *naifKeywordsObj = NULL; // NULL for logic tests, address of a Var * is copied to it if used
	Var *historyObj = NULL; // NULL for logic tests, address of a Var * is copied to it if used
	Var *originalLabelObj = NULL; // NULL for logic tests, address of a Var * is copied to it if used
	Var *Object[5]; // Can nest up to 5 deep
	Var *Group = new_struct(0);
	Var *stub = new_struct(0);
	Var *numArray = new_struct(0);
	Var *charBlob = new_struct(0);
	int originalLabelRows = 0;
	char **olRows = NULL;
	void *localData = NULL;
	char *originalLabelData = NULL;
	char *ObjectName[5];
	int i, k, step, count;
	int lineInCount = 0;

	volatile int stopCount = 32;  // find stop
	double *testD;
	unsigned char *testC;
	unsigned char inFix[8];
	int objectNest = -1;
	char *workingObject = NULL;
	char *workingGroup = NULL;
	char *fileIndicator = NULL;
	char namebuf[DV_NAMEBUF_MAX];
	static char noGroup[] = "NONE";
	int inGroup = 0;
	int position = 0;
	long whereInFile;  // Next read starts here
	long savedFileLocation = 0; // Save location
	long startOfHistoryRead = 0;
	char *historyStrings = NULL;
	long historyOffset;
	long historySize;
	long currentLocation;
	int inHistory = 0;
	int inOriginalLabel = 0;
	int doneReadingHistory = 1;
	int historyObjectLevel = -2;
	int retVal = 0;
	int weNeedToRead = 1;
	int doneFlag = 0;
	int inNaifKeywords = 0;
	int naifObjectLevel = -2;
	int localInTable = 0;  // 0 until we enter a table, 0 when we end a table
	unsigned int localStartByte = 0;
	unsigned int localBytes = 0;
	unsigned int localRecords = 0;
	unsigned int olLocalBytes = 0; // for OriginalLabel
	unsigned int olLocalStartByte = 0; // for OriginalLabel
	int localByteOrder = 2; // 0 will be Lsb, 1 will be Msb, 2 is initial condition
	int localFieldCount = 0;
	char localTableName[256];
	char localLine[256];
	int localSize = 0;  // This will be size of the item
	/* from parser.h, and right now ignoring others:
	 #define DV_INT32			3
	 #define DV_FLOAT		4
	 #define DOUBLE		8
	 We are assuming ONE type per table.  If more than one, will flag it and not process it
	 */
	int localProcess = 1; // Set to zero to NOT process
	char *nptr; // for strtoxx conversions

	for (i = 0; i < 5; i++) {
		Object[i] = NULL;
	}

	/*
	 * If the filename is NULL, davinci crashes before this code is reached.
	 * So it is handled in the calling function
	 */

	//if (filename == NULL) {
	//    parse_error("%s: No filename specified\n", func->name);
	//   return (NULL);
	//}
	if ((fname = dv_locate_file(filename)) == (char*) NULL) {
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

	//parse_error("/**************************************************/\n");
	if (data == 1) { // we parse cube info
		// parse_error("Parsing Cube Parameters...\n");
		int check;
		check = getCubeParams(fp);
		if (check != -1) {
			cubeVar = readCube(fp);
		} else {
			fclose(fp);
			return NULL;
		}
		fseeko(fp, 0, SEEK_SET);

	}

	for (k = 0; k < 2048; k++) { // 2048 seems a reasonable limit

		if (weNeedToRead == 1) {

			// This is the main line input
			if ((inHistory == 1) && (doneReadingHistory == 1)) {
				fileIndicator = NULL;
			} else {
				fileIndicator = fgets(inputString, 2046, fp);

#ifdef PRINTINPUT
				lineInCount++;
				if(lineInCount == stopCount) {
					printf("Eureka!\n");
				}
#endif
				if (inHistory == 1) {
					currentLocation = ftello(fp);
					if ((currentLocation - startOfHistoryRead) == historySize) {
						doneReadingHistory = 1;
					}

#ifdef PRINTINPUT
					printf("The difference is %ld, max difference is %ld\n", currentLocation-startOfHistoryRead, historySize);
#endif
				}

			}

#ifdef PRINTINPUT
			if(fileIndicator != NULL) {
				printf("%3d : %3d : %s", lineInCount, strlen(inputString), inputString);
				if (lineInCount == countFix) {
					printf("lineInCount = %d caught\n", countFix);
				}
			}
			else {
				printf("\n\nReached End of History\n\n");
			}
#endif

			if (fileIndicator == NULL) {
				fseeko(fp, savedFileLocation, SEEK_SET);
				inHistory = -1; // drd history
				historyObjectLevel = -2;
				continue;
			}

			whereInFile = ftello(fp);
			i = strlen(inputString);
		}

		if (doneFlag == 0) { // Only do this if we are not done
			if (i > 1) { // There may be "blank" lines with only a \n
				if (weNeedToRead) {
					// Note that there is a '\n' at the end of the input string
					i -= 1;
					if (inputString[i] == '\n') {
						inputString[i] = '\0';
					}

					/*
					 * For the most part:
					 * string1 is item
					 * string2 is '='
					 * string3 is value
					 * string4 is optional and should be units in <> brackets
					 *
					 * There are instances of lines with no '='s in them
					 * UNLESS the line is an End_ of something
					 * In this annoying case, we will reorganize:
					 */
					if ((strstr(inputString, "=") == NULL)
							&& (strstr(inputString, "End_") == NULL)) {
						if (strcmp(inputString, "End") != 0) {
							// printf("Here is %s\n", inputString);
							strcpy(string1, "comment = ");
							strcat(string1, inputString);
							strcpy(inputString, string1);
						}
					}
					count = sscanf(inputString, "%s%s%s%s", string1, string2,
							string3, string4);
				} else {
					weNeedToRead = 1;
				}
				if (count >= 2) {
					position = stringFinder(fp); // Position of start of string3
				}

				// The last thing in the header is "End"
				// once we get here -- we are done
				//
				if ((strcmp("End", string1) == 0) && (count == 1)) {
					doneFlag = 1;
#ifdef  PRINTINPUT
					printf("\nFound the end of the header\n");
#endif
					continue;
				}

				/* Let's see if we are starting an Object */
				if (strcmp("Object", string1) == 0) {

					if (strcmp("OriginalLabel", string3) == 0) {
						inOriginalLabel = 1; // the example I'm working with is a PDS header
					}

					if (strcmp("Table", string3) == 0) {
						int locali = 0;

						if (use_names == 1) {
							locali = readNextLine(fp); // conditional!
						}
						if (locali == 1) {
							weNeedToRead = 0;
						}

						if (localInTable == 0) { // drd table processing
							localInTable = 1;
							localStartByte = 0;
							localBytes = 0;
							localRecords = 0;
							localByteOrder = 2; // 0 is Lsb, 1 is Msb, this 2 is an initial condition
							localSize = 0;
							localProcess = 1;
							localFieldCount = 0;
							if (use_names == 1) {
								/*
								 * We have read another input line;
								 * string3 right now contains the name we want
								 * to use for the table values
								 */
								strcpy(localTableName, "Table");
								// strcpy(localTableName, "Table_"); // Option is to put this back in and
								// strcat(localTableName, string3); // have a longish table name
							} else {
								strcpy(localTableName, "NOTHING");
							}
						} // end localInTable == 0
					}  // end strcmp("Table", string3)

					strcpy(namebuf, string3);
					if (objectNest >= 0) {
						make_unique_name(namebuf, Object[objectNest]);
					} else {
						make_unique_name(namebuf, IsisCube);
					}
					workingObject = strdup(namebuf);
					if (strcmp("IsisCube", string3) == 0) {
						workingGroup = noGroup;
						inGroup = 0;
					} else {
						objectNest++; // this was because IsisCube was at -1 level
						if (objectNest >= 5) {
							return NULL;
						}

						Object[objectNest] = new_struct(0);
						ObjectName[objectNest] = workingObject;

						if (strcmp("NaifKeywords", ObjectName[objectNest])
								== 0) {
							inNaifKeywords = 1;
							naifObjectLevel = objectNest;
						}

// This is the original OriginalLabel Processing Code
						/*
						 if( strcmp("OriginalLabel", ObjectName[objectNest]) == 0) {
						 inOriginalLabel = 1;
						 fgets(inputString, 2046, fp); // Name = IsisCube
						 fgets(inputString, 2046, fp); // StartByte = 92020737
						 count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
						 sscanf(string3, "%ld", &historyOffset);
						 fgets(inputString, 2046, fp); // Bytes = 3210
						 savedFileLocation = ftell(fp); // This is where we return to parsing later
						 count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
						 sscanf(string3, "%ld", &historySize);
						 fseek(fp, historyOffset-1, SEEK_SET);
						 historyStrings = (char *)malloc(historySize + 1);  // The +1 is in case we need room for a \0
						 if(historyStrings == NULL) {
						 parse_error("Could not malloc space for OriginalLabel\n");
						 return NULL;
						 }

						 //Here we read bytes instead of one big chunk to allow for
						 //any kind of historySize error--hopefully something is better than nothing

						 fread(historyStrings, 1, historySize, fp);
						 historyStrings[historySize] = '\0'; // Force '\0' at the end
						 stub = newString(historyStrings); // All the History in a text blob with a '\0' at the end
						 add_struct(Object[objectNest], "OriginalLabel", stub);
						 fseek(fp, savedFileLocation, SEEK_SET);
						 // The next read is "End_Object"
						 // and this will be handled by the existing machinery
						 }
						 */
						if (strcmp("History", ObjectName[objectNest]) == 0) {
							/*
							 * The time has come to parse History!
							 */
							// parse_error("Reading History and storing as text...\n");
							fgets(inputString, 2046, fp); // Name = IsisCube
							fgets(inputString, 2046, fp); // StartByte = 92032424
							count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
							sscanf(string3, "%ld", &historyOffset);
							fgets(inputString, 2046, fp); // Bytes = 8292
							savedFileLocation = ftello(fp); // This is where we return to parsing later
							count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
							sscanf(string3, "%ld", &historySize);
							fseeko(fp, historyOffset - 1, SEEK_SET);
							startOfHistoryRead = ftello(fp);
							inHistory = 1;
							doneReadingHistory = 0;
							historyObjectLevel = objectNest;
							// drd history putting history back in line
							// historyObj = Object[objectNest];
#ifdef PRINTINPUT
							printf("\n\nNow jumping to History\n");
#endif

						}
					}
					if (weNeedToRead == 0) {
						strcpy(string1, string1_2);
						strcpy(string3, string3_2);
						strcpy(inputString, "=");
						strcat(inputString, string3);

					}

				} /* End Let's see if we are starting an Object */

				/* Let's see if we are starting a Group */
				else if (strcmp("Group", string1) == 0) {
					if (strcmp("Field", string3) == 0) {
						int locali = 0;
						if (localInTable == 1) {
							localFieldCount += 1;
						}

						if (use_names == 1) {
							locali = readNextLine(fp); // conditional!
						}

						if (locali == 1) {
							weNeedToRead = 0;
						}
					}
					strcpy(namebuf, string3);
					if (objectNest >= 0) {
						make_unique_name(namebuf, Object[objectNest]);
					} else {
						make_unique_name(namebuf, IsisCube);
					}
					workingGroup = strdup(namebuf);
					inGroup = 1;
					Group = new_struct(0);
					if (weNeedToRead == 0) {
						strcpy(string1, string1_2);
						strcpy(string3, string3_2);
						strcpy(inputString, "=");
						strcat(inputString, string3);

					}
				}
				/* Let's see if we are at an End_Object */
				else if (strcmp("End_Object", string1) == 0) {

					// drd history "object"
					if (inHistory == -1) {
						inHistory = 0; // drd history
						stub = newString(strdup("history"));
					} else {
						stub = newString(strdup("object")); // drd history this is all there was before putting history back inline
					}

					if (objectNest >= 0) {
						if (localInTable == 1) {
							localInTable = 0;

#ifdef PRINTINPUT
							printf("\nlocalInTable set to 0\n");
							printf("localStartByte = %d\n", localStartByte);
							printf("localBytes = %d\n", localBytes);
							printf("localRecords = %d\n", localRecords);
							printf("localByteOrder = %d\n", localByteOrder); // 0 is Lsb, 1 is Msb, this 2 is an initial condition
							printf("localSize = %d\n", localSize);
							printf("localProcess = %d\n", localProcess);
							printf("localFieldCount = %d\n", localFieldCount);
							printf("localTableName = %s\n", localTableName);
#endif
							localData = malloc(localBytes);
							savedFileLocation = ftello(fp);
							fseeko(fp, localStartByte - 1, SEEK_SET); // REMEMBER the 1 offset for the addresses!!!!!
							fread(localData, localBytes, 1, fp);
							fseeko(fp, savedFileLocation, SEEK_SET);

							if (localByteOrder == 1) { // 0 is Lsb, 1 is Msb, this 2 is an initial condition

								testC = (unsigned char *) localData;

								for (i = 0; i < localBytes; i += localSize) {
									for (k = 0; k < localSize; k++) {
										inFix[k] = testC[i + k];
										// printf("inFix[%d] = testC[%d + %d]\n", k, i, k);
									}

									for (k = 0; k < localSize; k++) {
										testC[i + k] = inFix[localSize - 1 - k];
										// printf("testC[%d + %d] = inFix[%d]\n", i, k, localSize-1-k);
									}
								}
							}

							numArray = newVal(BSQ, localFieldCount, localRecords, 1, localSize, localData);
							add_struct(Object[objectNest], localTableName, numArray);
						}  // end localInTable == 1

						else if (inOriginalLabel == 1) {
							inOriginalLabel = 0;
							parse_error("\n\"OriginalLabel\" is not processed by davinci, it is stored as a Text Buffer\n"); // was "as a string blob\n");
							originalLabelData = (char *) malloc(olLocalBytes + 3); // we will force a \n','\0' at the end
							if (originalLabelData == NULL) {
								parse_error("Could not malloc space for OriginalLabel\n");
								return NULL;
							}

							savedFileLocation = ftello(fp);
							fseeko(fp, olLocalStartByte - 1, SEEK_SET); // REMEMBER the 1 offset for the addresses!!!!!
							//Here we read bytes instead of one big chunk to allow for
							//any kind of historySize error--hopefully something is better than nothing
							fread(originalLabelData, 1, olLocalBytes, fp);
							fseeko(fp, savedFileLocation, SEEK_SET);
							originalLabelData[olLocalBytes + 2] = '\0'; // Force '\0' at the very end
							originalLabelData[olLocalBytes + 1] = '\n'; // Force a '\n' at the end of text
							// Var *newText(int rows, char **text)
							originalLabelRows = 0;
							for (k = 0; k < olLocalBytes + 2; k++) {
								if (originalLabelData[k] == '\n') {
									originalLabelRows++;
								}
							}
							olRows = malloc(originalLabelRows * sizeof(char**));
							// drd compile in the next line to test failed malloc()
							// olRows = NULL;
							if (olRows != NULL) {
								k = 0;
								for (i = 0; i < originalLabelRows; i++) {
									step = 0;
									while ((k < olLocalBytes) && (originalLabelData[k] != '\n')) {
										localLine[step++] = originalLabelData[k++];
									}

									localLine[step] = '\0'; // Want a '\0' on the end and not a '\n'
									olRows[i] = strdup(localLine);
									k++;
									// printf("line %d: %s\n", i+1, olRows[i]);  // drd to print out the strings as they are added to the list

								}
								//  charBlob = newString(originalLabelData); // All the originalLabel in a text blob with a '\0' at the end
								//             if you use the big blob, you don't need the list of strings in olRows[]
								//             so choose either newString() or newText()
								//             I have done this both ways--right now, Chris Edwards prefers the list of strings
								charBlob = newText(originalLabelRows, olRows); // All the originalLabel as a list of strings
								add_struct(Object[objectNest], "TextBuffer", charBlob);
							} // end malloc not NULL
							else { // We could not malloc space for the array of pointers to pointers
								stub = newString(strdup(
								    "Could not malloc space to store OriginalLabel as list of strings"));
								add_struct(Object[objectNest], "OriginalLabel", stub);
							}
							free(originalLabelData); // Don't need this anymore
						} // end of inOriginalLabel == 1

						if (inNaifKeywords == 1) {
							naifKeywordsObj = Object[objectNest--];
							inNaifKeywords = 0;
							naifObjectLevel = -2;
						}

						else if (inHistory == 1) {
							if (objectNest < 1) {
								printf("\n\n\nThis is not supposed to happen!!!!!!!\n");
								printf("System is unstable!");
							}
							add_struct(Object[objectNest - 1], ObjectName[objectNest], Object[objectNest]);
							objectNest--;
						} else {  //drd history putting History back in line
							// if(strcmp("History", ObjectName[objectNest]) != 0)  // drd was
							{ // History points to historyObj, historyObj is attached later
								add_struct(IsisCube, ObjectName[objectNest], Object[objectNest]);
							}

							objectNest--;
						}
					}  // end objectNest >= 0
					else {
						add_struct(IsisCube, "isis_struct_type", stub);
					}
				}  // end string1 == "End_Object"
				/* Let's see if we are at an End_Group */
				else if (strcmp("End_Group", string1) == 0) {
					stub = newString(strdup("group"));
					add_struct(Group, "isis_struct_type", stub);
					if (objectNest >= 0) {
						add_struct(Object[objectNest], workingGroup, Group);
					} else {
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
					/*
					 * We may have Table info handling
					 */
					if ((localInTable == 1) && (localProcess == 1)) {
						if ((strcmp("NOTHING", localTableName) == 0) && (strcmp("Name", string1) == 0)) {
							strcpy(localTableName, "Table_");
							strcat(localTableName, string3);
						}
						if (strcmp("StartByte", string1) == 0) {
							localStartByte = strtoul(string3, &nptr, 10);
						}
						if (strcmp("Bytes", string1) == 0) {
							localBytes = strtoul(string3, &nptr, 10);
						}
						if (strcmp("Records", string1) == 0) {
							localRecords = strtoul(string3, &nptr, 10);
						}
						if (strcmp("ByteOrder", string1) == 0) {
							if (strcmp("Lsb", string3) == 0) {
								localByteOrder = 0;
							} // end Lsb
							if (strcmp("Msb", string3) == 0) {
								localByteOrder = 1;
							} // end Msb
						} // end ByteOrder
						if ((localSize == 0) && strcmp("Type", string1) == 0) { // drd setting only once per table
							for (i = 0; i < strlen(string3); i++) {
								string3[i] = toupper(string3[i]);
							}
							if (strcmp("DOUBLE", string3) == 0) {
								localSize = 8;
							} else if (strcmp("DV_FLOAT", string3) == 0) {
								localSize = 4;
							} else if (strcmp("DV_INT32", string3) == 0) {
								localSize = 4;
							}
						}
					} // end of table info handling
					/*
					 * We may have OriginalLabel processing
					 */
					if (inOriginalLabel == 1) {
						if (strcmp("StartByte", string1) == 0) {
							olLocalStartByte = strtoul(string3, &nptr, 10);
						}
						if (strcmp("Bytes", string1) == 0) {
							olLocalBytes = strtoul(string3, &nptr, 10);
						}
					}
					/* Make a unique name; either for the Group or Object we are in or the base IsisCube */
					cNameMaker(string1); // Fix some problems observed in the input name
					strcpy(namebuf, string1);

					if (inGroup == 1) {
						make_unique_name(namebuf, Group);
					} else if (objectNest >= 0) {
						make_unique_name(namebuf, Object[objectNest]);
					} else {
						make_unique_name(namebuf, IsisCube);
					}

					// do a little research on continuation lines
					// First is string lists enclosed by '(' and ')'
					// These may be either text string lists or text strings representing number lists
					// We don't consider mixed lists
					//

					if ((string2[0] == '=') && (string3[0] == '(')) {
						if (inGroup == 1) {
							retVal = parenManager(inputString, fp, Group, namebuf);
						} else {
							retVal = parenManager(inputString, fp, Object[objectNest], namebuf);
						}

					}
					// The next is a series of strings starting with the character pair '"
					// This is just a long string                             0x27 is a '
					else if ((string2[0] == '=') && (string3[0] == '\'') && (string3[1] == '"')) {
						if (inGroup == 1) {
							retVal = dQuoteManager(inputString, fp, Group, namebuf);
						} else {
							retVal = dQuoteManager(inputString, fp, Object[objectNest], namebuf);
						}
					}

					else {

						// Next is the possibility of a long string with no delimiters that may extend to yet another line
						// If there is another part to read in, we will concatenate it to string3
						if ((string2[0] == '=') && (string3[strlen(string3) - 1] == '-') &&
						    (strlen(string3) > 58)) { // The '-' is a hyphen for continuation

							string3[strlen(string3) - 1] = '\0'; // get rid of the '-'

							fgets(continuationString, 2046, fp); // read in some more

							// More than likely a newline on the end
							i = strlen(continuationString);
							if (continuationString[i - 1] == '\n') {
								continuationString[i - 1] = '\0';
							}

							// remove leading spaces
							i = 0;
							while (continuationString[i] == ' ') {
								i++;
							}
							// concat if not too big-
							if (strlen(&continuationString[i]) < (-1 + sizeof(string3) - strlen(string3))) {
								strcat(string3, &continuationString[i]);
							} else {
								; // nothing...right now, if it is too big, just drop it
							}
							strcpy(&inputString[position], string3);
						}
						// Whether or not we read in an extra snippet, just continue

						if (inGroup == 1) {
							/* There may be <units> in string4. Units are strings enclosed by a '<' and a '>',
							 * or string4 may simply be a part of a string3 that has spaces in it.
							 * Also, by default, we are now not showing units
							 */
							if ((use_units == 1) && (count == 4) && (string4[0] == '<') &&
							    (string4[-1 + strlen(string4)] == '>')) {

								add_converted_to_struct(Group, namebuf,string3);
								strcat(namebuf, "Units");
								add_struct(Group, namebuf, newString(strdup(string4)));
							} else if ((use_units == 0) && (count == 4) && (string4[0] == '<') &&
							           (string4[-1 + strlen(string4)] == '>')) {
								add_converted_to_struct(Group, namebuf, string3);
							} else { // &inputString[position] is the input string past the '=' with leading and trailing double quotes removed
								add_converted_to_struct(Group, namebuf, &inputString[position]);
							}
						} else {
							/* There may be <units> in string4. Units are strings enclosed by a '<' and a '>',
							 * or string4 may simply be a part of a string3 that has spaces in it.
							 * Also, by default, we are now not showing units
							 */
							if ((use_units == 1) && (count == 4) && (string4[0] == '<') &&
							    (string4[-1 + strlen(string4)] == '>')) {
								add_converted_to_struct(Object[objectNest], namebuf, string3);
								strcat(namebuf, "Units");
								add_struct(Object[objectNest], namebuf, newString(strdup(string4)));
							} else if ((use_units == 0) && (count == 4) && (string4[0] == '<') &&
							           (string4[-1 + strlen(string4)] == '>')) {
								add_converted_to_struct(Object[objectNest], namebuf, string3);
							} else { // &inputString[position] is the input string past the '=' with leading and trailing double quotes removed
								if (inOriginalLabel == 1) { // todo -- is this only here, or is it needed in the "Group" one above, too?
									//add_struct(Object[objectNest], "Note", newString(strdup("This item is not processed by davinci")));
								}
								add_converted_to_struct(Object[objectNest], namebuf, &inputString[position]);
							}
						}
					}
				} // end of not at beginning or end
			} // end of length > 1
		} // end of not done yet
		else {
			// parse_error("Found End of label...\n");
			// parse_error("/**************************************************/\n");
			break;
		}
	} // End of header info for loop

	fclose(fp);

	/*************************************************************************

	 Done parsing file

	 ************************************************************************/
	// fop = malloc(4*4*4*4);
	// cubeVar = newVal(BSQ, 4, 4, 4, DV_FLOAT, fop);
	// pp_print(cubeVar);
	if ((cubeVar != NULL) && (data != 0)) {
		add_struct(v, "cube", cubeVar);
	}
	add_struct(v, "IsisCube", IsisCube);
	if (naifKeywordsObj != NULL) {
		add_struct(v, "NaifKeywords", naifKeywordsObj);
	}

	if (historyObj != NULL) {
		add_struct(v, "History", historyObj); // Naming the history object
	}

	if (originalLabelObj != NULL) {
		add_struct(v, "OriginalLabel", originalLabelObj);
	}
	return (v);
}

/**************************************************************************/
void make_unique_name(char * buffer, Var * dv_struct)
{
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
	strncpy(work_buf, buffer, DV_NAMEBUF_MAX - 1);
	while (find_struct(dv_struct, work_buf, &dummy_sptr) != -1) {
		if (snprintf(work_buf, DV_NAMEBUF_MAX - 1, "%s_%i", buffer, start_count++) >= DV_NAMEBUF_MAX - 1) {
			parse_error("Warning: attempt to find unique name for '%s' results in truncated strings.\n"
			            "Cannot rename. Please use shorter names in your ISIS files!");
			return;
		}
	}
	strncpy(buffer, work_buf, DV_NAMEBUF_MAX - 1);
}

char get_keyword_datatype(char * value, char * name)
{
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
	if (errno)
		is_int = 0;
	else
		is_int = 1;
	if (endptr != value + strlen(value))
		is_int = 0;

	/* check for float value. Note that like the int, the whole string must convert
	 to be a float or it is more likely a string like a date/time value */

	floatval = strtod(value, &endptr);
	if (errno)
		is_double = 0;
	else
		is_double = 1;
	if (endptr != value + strlen(value))
		is_double = 0;

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
	/* drd
	 * If the name contains the string 'SpacecraftClockCount', keep this as a string
	 */

	if (name != NULL && strstr(name, "SpacecraftClockCount") != NULL) {
		is_int = 0;
		is_double = 0;
	}

	/* drd
	 * If the value is an empty string of "", keep it a string
	 */

	if ((name != NULL) && (value[0] == '\"') && (value[1] == '\"')) {
		is_int = 0;
		is_double = 0;
		value[0] = '\0'; // The output routine will give us the desired "" for "Nothing"
	}

	if (name != NULL) {
		if (is_int && is_double) {
			if (intval == floatval) {
				is_double = 0;
				// drd added one more test -- if there is a decimal point, leave it a double
				if (strstr(value, ".") != NULL)
					is_double = 1;
			} else {
				is_int = 0;
			}
		}
	}

	if (is_double)
		return 'd';
	if (is_int)
		return 'i';
	return 'c';

}

void add_converted_to_struct(Var * dv_struct, char * name, char * value)
{
	/* Attempts to convert keyword values to integers and floats to determine if
	 the value supplied is a numeric value. The rules are:
	 Try to make it an int and a float.
	 Compare the values and if they are equal, make it an int
	 UNLESS they are equal and the string has a "." in it // drd added this line
	 If they are not, make it a float.
	 If only one conversion worked, use that conversion.
	 If neither conversion worked, then make it a string.
	 */

	char datatype;

	datatype = get_keyword_datatype(value, name);

	if (datatype == 'i') {
		add_struct(dv_struct, name, newInt(atol(value)));
	} else if (datatype == 'd') {
		add_struct(dv_struct, name, newDouble(atof(value)));
	} else {
		add_struct(dv_struct, name, newString(strdup(value)));
	}
}

/*
 * drd
 * Some comments on CSV strings I have seen:  There are lots of CSV strings in the cubes.  The longs ones are wrapped with leading spaces.
 * The tools parenManager() and dQuoteManager remove leading spaces from the wrapped lines and concatenate the strings.  This may mean that there will be
 * no white space in a few instances:
 * one, two, three,four, five
 *                ^ comma, but no space white for example
 * BUT, in at least one case I found a long string wrapped with no comma.  Forcing a space between concatenations may thus add a space to a string
 * not intended to have spaces.  Thus, I have elected to strip leading spaces from read in strings.
 * Note that all Tokens are based on ','s and not white spaces.
 */

int parenManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf)
{
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
	int endHyphen = 0;  // 0 means there is no hyphen at the end of the string
	int endSpace = 1; // 1 means we add one space to the end of the string before strcat()'ing on next string
	int conversionCount = 0;
	double *numberArrayDouble = NULL;
	int *numberArrayInt = NULL;
	double checkDouble = 99.9; // Any value would do
	int checkInt = 99; // Any value will do
	Var *localStub = new_struct(0);
	Var *subItem = new_struct(0);

	while (stringItem[i] != '(') {
		i++;
	}
	i++; // Now we are past the leading '('

	length = strlen(&stringItem[i]);

	while ((stringItem[i] == 0x20) && (i < length)) { // 0x20 is a space, may be leading spaces
		i++;
	}

	workingString = strdup(&stringItem[i]);
	length = 2 + strlen(workingString); // '2 + ' is accounting for a space to be added and '\0' at the end of the string

	while (strstr(workingString, ")") == NULL) {
		fgets(localBuff, 400, fp);
		retVal += strlen(localBuff);
		addedLength = strlen(localBuff) - 1;
		localBuff[addedLength] = '\0'; // delete the '\n'

		i = 0;
		while ((localBuff[i] == 0x20) && (i < addedLength)) { // removing leading spaces
			i++;
		}

		/*
		 * We have run across lines ending in hyphens.
		 * This indicates continuation. If there is continuation,
		 * we are not adding a space next time through the loop
		 */

		if (localBuff[i - 1 + strlen(&localBuff[i])] == '-') {
			localBuff[i - 1 + strlen(&localBuff[i])] = '\0';
			endHyphen = 1;
		}

		length = length + strlen(&localBuff[i]);
		if (endSpace == 1) {
			localString = realloc(workingString, length + 1); // + 1 because we are adding a space
		} else {
			localString = realloc(workingString, length);
		}

		if (localString != NULL) {
			workingString = localString;
			if (endSpace == 1) {
				strcat(workingString, " "); // a space at the end
			}
			if (endSpace == 0) {
				endSpace = 1;
			}
			strcat(workingString, &localBuff[i]);
			if (endHyphen == 1) {
				endSpace = 0;
				endHyphen = 0;
			}
			// printf("Realloc worked %d,%d,%s\n", length, strlen(workingString),workingString);
		} else {
			retVal = -1;
			return retVal;
		}
	}

	/*
	 * We now have a CSV list of items in one string.
	 * Most leading spaces have been deleted.
	 * Replace the closing ')' with a ','
	 */

	for (i = 0; i < strlen(workingString); i++) {
		if (workingString[i] == ',') {
			commaCount++;
		}

		if (workingString[i] == ')') {
			workingString[i] = ',';
			commaCount++; // There was one more element than there were ','s until we just added this ','
		}
	}

	token = strtok(workingString, ",");
	i = 0;
	length = strlen(token);
	while ((token[i] == 0x20) && (i < length)) { // remove leading spaces
		i++;
	}

	/*
	 * The question is, is this a list of numbers,
	 * or is it a list of strings?
	 */

	conversionCount = sscanf(&token[i], "%lf", &checkDouble);

	if (conversionCount == 1) { // We have a list of numbers
		sscanf(&token[i], "%d", &checkInt);
		if ((checkInt == checkDouble) && (strstr(&token[i], ".") == NULL)) { // if the values are the same and no ".", assume ints
			numberArrayInt = malloc(commaCount * sizeof(int));
			if (numberArrayInt == NULL) {
				retVal = -1;
				return retVal;
			}
			numberArrayInt[0] = checkInt;
			for (i = 1; i < commaCount; i++) {
				token = strtok(NULL, ",");
				sscanf(token, "%d", &numberArrayInt[i]);
			}

		}

		else { // if values are not the same, we have doubles
			numberArrayDouble = malloc(commaCount * sizeof(double));
			if (numberArrayDouble == NULL) {
				retVal = -1;
				return retVal;
			}
			numberArrayDouble[0] = checkDouble;
			for (i = 1; i < commaCount; i++) {
				token = strtok(NULL, ",");
				sscanf(token, "%lf", &numberArrayDouble[i]);
			}
		}

		if (numberArrayDouble != NULL) {
			localStub = newVal(BSQ, commaCount, 1, 1, DOUBLE,
					numberArrayDouble);
		} else {
			localStub = newVal(BSQ, commaCount, 1, 1, DV_INT32, numberArrayInt);
		}
		add_struct(GroupOrObject, namebuf, localStub);

	}

	else { // We have a list of strings

		localStub = newString(strdup(&token[i]));
		add_struct(subItem, NULL, localStub);
		for (i = 1; i < commaCount; i++) {
			token = strtok(NULL, ",");
			addedLength = strlen(token);
			k = 0;
			while ((token[k] == 0x20) && (k < addedLength)) { // remove leading spaces
				k++;
			}
			localStub = newString(strdup(&token[k]));
			add_struct(subItem, NULL, localStub);
		}
		add_struct(GroupOrObject, namebuf, subItem);
	}
	free(workingString);
	return retVal;
}

int dQuoteManager(char *stringItem, FILE *fp, Var *GroupOrObject, char *namebuf)
{
	char *workingString = NULL;
	char *localString = NULL;
	char localBuff[400];
	int length = 0;
	int lengthInBuf = 0;
	int retVal = 0;
	int i = 0;
	Var *localStub = new_struct(0);

	/* drd
	 *  This next looks like it might fail, but we got here because there was the character pair > '" <
	 *  in the string that brought us here.  We will find a \' !
	 */

	while (stringItem[i] != '\'') {
		i++;
	}
	strcpy(localBuff, &stringItem[i]);
	strcat(localBuff, "\n");
	workingString = strdup(localBuff);
	length = 1 + strlen(workingString); // '1 +' to allow for terminating '\0'

	while (strstr(workingString, "\"'") == NULL) { // begins with '" and ends with "'
		fgets(localBuff, 400, fp);
		retVal += strlen(localBuff);
		lengthInBuf = strlen(localBuff);

		i = 0;

		while ((localBuff[i] == 0x20) && (i < lengthInBuf)) { // removing leading spaces
			i++;
		}

		length = length + strlen(&localBuff[i]);

		localString = realloc(workingString, length);

		if (localString != NULL) {
			workingString = localString;
			strcat(workingString, &localBuff[i]);
		} else {
			retVal = -1;
			return retVal;
		}
	}

	/*
	 * We now have a longish string.
	 */
	workingString[strlen(workingString) - 3] = '\0'; // discard final > " ' newline <

	localStub = newString(strdup(&workingString[2])); // discard starting > ' " <

	free(workingString);

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
int stringFinder(FILE* fp)
{
	int i = 0;
	int sub_i = 0;
	int innerString;
	int length;
	char inBuffer[100];
	char moreBuffer[4096];

// So what about the case where there is no '=' in here?
// This was fixed before sscanf() is called

	while (inputString[i] != '=') {
		i++;
	}
	i++; // Now we are past the '='
// There might be 'spaces'
	while (inputString[i] == 0x20) { // 0x20 is a space, remove leading spaces
		i++;
	}

	/* drd
	 * What we have found is strings of the form "", just an empty string
	 * This we will just return
	 */

	if ((inputString[i] == '\"') && (inputString[i + 1] == '\"')) {
		return i;
	}

// There might be strings with embedded spaces
// These strings might be enclosed in double quotes
// I don't want the double quotes
// If these do remain, then strings with embedded spaces will print like
//                  ""This is a string with embedded spaces""
// What I want is
//                   "This is a string with embedded spaces"
// drd added:
// Also, I have found cases where the string is continued on the next line(s)
// This means I need to read more to get the rest of the string
	if (inputString[i] == 0x22) { // 0x22 is a "
		inputString[i] = 0x20; // now the 0x22 is a space
		i++; // now past where the " was

		// there must be another 0x22, or we need to read more

		if (strstr(inputString, "\"") == NULL) {
			strcpy(moreBuffer, &inputString[i]);
			strcat(moreBuffer, "\n"); // This was removed before we got here
			i = 0; // Now we go to the beginning

			do {  // you just don't see do-while loops very often
				sub_i = 0;
				fgets(inBuffer, 99, fp);

				while (inBuffer[sub_i] == 0x20) { // 0x20 is a space, remove leading space
					sub_i++;
				}

				strcat(moreBuffer, &inBuffer[sub_i]);
			} while (strstr(&inBuffer[sub_i], "\"") == NULL);

			strcpy(inputString, moreBuffer);
		}

		length = strlen(inputString);
		for (innerString = i; innerString < length; innerString++) {
			if (inputString[innerString] == 0x22) {
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
int cNameMaker(char *inputString)
{
	int result = 0;
	int i;

	for (i = 0; i < strlen(inputString); i++) {
		switch (inputString[i]) {
		case '-':   // a minus sign
		case '.':   // a dot
		case ':':   // a colon
		case '+':   // a plus sign
		case ';':   // a semicolon
		case ' ':   // a space, not very possible
			inputString[i] = '_'; // each individually replace by an underbar
			break;
		default:
			; // nothing, just leave it alone
		}

	}

	return result; // I don't see a failure possible here, so for now, always good
}

int readNextLine(FILE *fp)
{
	int locali;

	fgets(inputString, 2046, fp);
	locali = strlen(inputString);
	locali -= 1;
	inputString[locali] = '\0';
	sscanf(inputString, "%s%s%s%s", string1_2, string2_2, string3_2, string4_2);
	locali = 0;
	if (strcmp("Name", string1_2) == 0) {
		strcpy(interString, string3);
		strcpy(string3, string3_2);
		strcpy(string3_2, interString);
		locali = 1;
	}
	return locali;

}

Var *readCube(FILE *fp)
{

	#define CUBEDEBUG
	#ifdef CUBEDEBUG

	FILE *fpc;

	#endif

	Var *dv_struct = new_struct(0);
	unsigned char *cubeData = NULL;
	unsigned char inBuffer[4];
	unsigned char swapBuffer[4];
	int cubeElement = 0;
	int x, y, z;
	size_t element = 0;
	size_t linearOffset = 0;
	size_t offset = 0;
	size_t allSized = 0;

	if (Format == I3Tile) {
		tileSize = TileSamples * TileLines * sampleSize;
		tilesPerRow = Samples / TileSamples;
		subTileSamples = Samples % TileSamples; // What is left over going across
		subTileSize = subTileSamples * TileLines * sampleSize;
		totalRows = Lines / TileLines;
		subTileLines = Lines % TileLines; // What is left going down

		if (subTileSamples != 0) {
			tilesPerRow += 1;
		}
		if (subTileLines != 0) {
			totalRows += 1;
		}
	}
	allSized = (size_t)Samples * (size_t)Lines * (size_t)totalBands * (size_t)sampleSize;

	if (allSized >= 2147483647ll) {
		char biggie[64];
		sprintf(biggie,"%lu", allSized);
		commaize(biggie);
		parse_error(
				"Warning:  Size is %s bytes.\nThis is larger than the 32bit integer max of 2,147,483,647.\nProceeding...\n",
				biggie);
// for now, we are done
//  return NULL;
	}
	cubeData = malloc(allSized);

	if (cubeData == NULL) {
		parse_error("Cannot malloc() enough space to hold Cube\n");
		return NULL;
	}

	if (Format == BandSequential) {

		fseeko(fp, StartByte - 1, SEEK_SET); // Need only the one fseeko() to set up all
		offset = 0;  // This is the offset in to the storage structure
		for (z = 0; z < totalBands; z++) {
			for (y = 0; y < Lines; y++) {
				for (x = 0; x < Samples; x++) {
					if (1 == fread(inBuffer, sampleSize, 1, fp)) {

						if (ByteOrder == Msb) {
							switch (sampleSize) {
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
						} else {
							for (element = 0; element < sampleSize; element++) {
								cubeData[offset + element] = inBuffer[element];
							}
						}

						offset += sampleSize; // offset increases by sample size

					} // end of good read
					else {
						parse_error("Bad Read for Band Sequential Cube");
						return NULL;
					}
				} // end x
			} // end y
		} // end z
	} // end BandSequential

	if (Format == I3Tile) {
		linearOffset = 0;  // Where the write starts to the storage area
		// printf("totalRows = %lld\n", totalRows);
		for (Bands = 0; Bands < totalBands; Bands++) {
			for (Row = 0; Row < totalRows; Row++) {
				for (row = 0; row < TileLines; row++) {
					for (tiles = 0; tiles < tilesPerRow; tiles++) {
						offset = StartByte - 1;
						offset += Bands * (Samples * Lines * sampleSize); // How many whole "sheets" we have read
						offset += (Row * tilesPerRow * tileSize); // This is the number of full Rows of full tiles
						offset += (tiles * tileSize); // This is the offset of the number of full tiles already read in the Row
						offset += (row * TileSamples * sampleSize); // How many full rows we have read so far in this tile

						fseeko(fp, offset, SEEK_SET); // for Tiled, fseeko() each TileSample start

						for (x = 0; x < TileSamples; x++) { // Reading across a tile
							if (1 == fread(inBuffer, sampleSize, 1, fp)) {

								if ((subTileSamples != 0)
										&& (tiles == (tilesPerRow - 1))) {
									if (x >= subTileSamples) {
										continue; // We read it, we just don't put it in the structure
									}
								}

								if ((subTileLines != 0)
										&& (Row == (totalRows - 1))) {
									if (row >= subTileLines) {
										continue; // We read it, we just don't put it in the structure
									}
								}

								if (ByteOrder == Msb) {
									switch (sampleSize) {
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
										cubeData[linearOffset + element] =
												swapBuffer[element];
									}
								} else {
									for (element = 0; element < sampleSize; element++) {
										cubeData[linearOffset + element] = inBuffer[element];
									}
								}
								linearOffset += sampleSize; // linearOffset into the RAM data just keeps incrementing

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
		cubeElement = DV_UINT8;
		break;
	case 2:
		cubeElement = DV_INT16;
		break;
	case 4:
		cubeElement = DV_FLOAT;
		break;
	default:
		cubeElement = DV_UINT8;
	}

	dv_struct = newVal(BSQ, Samples, Lines, totalBands, cubeElement, cubeData);

	#ifdef CUBEDEBUG
	fpc = fopen("bincube.bin", "wb");
	fwrite(cubeData, allSized, 1, fpc );
	fclose(fpc);
	#endif

	cubeData = NULL;

	return dv_struct;
}

int getCubeParams(FILE *fp)
{
	int retVal = 1;
	int count;
	fgets(inputString, 2046, fp);
	sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
	if (strcmp("IsisCube", string3) != 0) {
		parse_error("This is not an Isis3 Cube\n");
		return -1;
	}

	while (strcmp("End_Object", string1) != 0) {
		fgets(inputString, 2046, fp);
		count = sscanf(inputString, "%s%s%s%s", string1, string2, string3, string4);
		if (strlen(inputString) < 3) {  // Some blank lines
			continue;
		}

		if (count != 3) { // There something to parse out only if there are three strings
			continue;
		}

		if (strcmp("StartByte", string1) == 0) {
			sscanf(string3, "%d", &StartByte);
		}

		else if (strcmp("Format", string1) == 0) {
			if (strcmp("Tile", string3) == 0) {
				Format = I3Tile;
			} else {
				Format = BandSequential;
			}
		}

		else if (strcmp("TileSamples", string1) == 0) {
			sscanf(string3, "%zd", &TileSamples);
		}

		else if (strcmp("TileLines", string1) == 0) {
			sscanf(string3, "%zd", &TileLines);
		}

		else if (strcmp("Samples", string1) == 0) {
			sscanf(string3, "%zd", &Samples);
		}

		else if (strcmp("Lines", string1) == 0) {
			sscanf(string3, "%zd", &Lines);
		}

		else if (strcmp("Bands", string1) == 0) {
			sscanf(string3, "%zd", &totalBands);
		}

		else if (strcmp("Type", string1) == 0) {
			if (strcmp("UnsignedByte", string3) == 0) {
				sampleSize = 1;
			} else if (strcmp("SignedWord", string3) == 0) {
				sampleSize = 2;
			} else {
				sampleSize = 4; // only other choice is Real which is really float of size 4
			}
		}

		else if (strcmp("ByteOrder", string1) == 0) {
			if (strcmp("Lsb", string3) == 0) {
				ByteOrder = Lsb;
			} else {
				ByteOrder = Msb;
			}
		}

		else if (strcmp("Base", string1) == 0) {
			sscanf(string3, "%lf", &Base);
		}

		else if (strcmp("Multiplier", string1) == 0) {
			sscanf(string3, "%lf", &Multiplier);
		}

		else {
			; // nothing for now
		}
	} // end of the while loop

	// parse_error("Completed parsing cube parameters...\n");

	return retVal;
}

int writeCube(FILE *fp, minIsisInfo *isisinfo, unsigned char *data)
{
	unsigned char *cubeData = NULL;
	unsigned char outBuffer[4];
	unsigned char swapBuffer[4];
	unsigned char fakeBuffer[] = {0,0,0,0};
	int cubeElement = 0;
	int x, y, z;
	int writeCount = 0;
	int wrtSampleSize = 0;
	size_t wrtTileSize = 0;
	size_t wrtTileSamples = 0;

	size_t wrtSubTileSize = 0;
	size_t wrtTiles = 0;
	size_t wrt_row = 0;
	size_t wrtRow = 0;
	size_t wrtBands = 0;
	size_t wrtTilesPerRow = 0;
	size_t wrtSubTileSamples = 0;
	size_t wrtTotalRows = 0;
	size_t wrtSubTileLines = 0;

	size_t wrtElement = 0;
	size_t wrtOffset = 0;
	size_t wrtAllSized = 0;
#undef OUTDEBUG
#ifdef OUTDEBUG
	FILE *snog;
	snog = fopen("data.txt","wt");
#endif
	if (strcmp("UnsignedByte", isisinfo->Type) == 0) {
		wrtSampleSize = 1;
	} else if (strcmp("SignedWord", isisinfo->Type) == 0) {
		wrtSampleSize = 2;
	} else {
		wrtSampleSize = 4; // only other choice is Real which is really float of size 4
	}

	wrtAllSized = (size_t) isisinfo->Samples * (size_t) isisinfo->Lines * (size_t) isisinfo->Bands * (size_t) wrtSampleSize;

	if (wrtAllSized >= 2147483647ll) {
		char biggie[64];
		sprintf(biggie, "%zu", wrtAllSized);
		commaize(biggie);
		parse_error("Warning:  Size is %s bytes.\n", biggie);
	}

	cubeData = data;

	if (strcmp (isisinfo->Format, "BandSequential") == 0) {

		wrtOffset = 0;  // This is the offset in to the storage structure
		for (z = 0; z < isisinfo->Bands; z++) {
			for (y = 0; y < isisinfo->Lines; y++) {
				for (x = 0; x < isisinfo->Samples; x++) {
					for (wrtElement = 0; wrtElement < wrtSampleSize; wrtElement++) {
						 outBuffer[wrtElement] = cubeData[wrtOffset + wrtElement];
					}
					if ( strcmp(isisinfo->ByteOrder, "Lsb")== 0) {
						writeCount = fwrite(outBuffer, wrtSampleSize, 1, fp);
					}
					else {
						switch (wrtSampleSize) {
							case 2:
								swapBuffer[1] = outBuffer[0];
								swapBuffer[0] = outBuffer[1];
								break;
							case 4:
								swapBuffer[3] = outBuffer[0];
								swapBuffer[2] = outBuffer[1];
								swapBuffer[1] = outBuffer[2];
								swapBuffer[0] = outBuffer[3];
								break;
							default:
								;
						}
						writeCount = fwrite(swapBuffer, wrtSampleSize, 1, fp);
					}
					if (writeCount != 1) {
						parse_error("Bad Write for Band Sequential Cube");
						return 0;
					}

					wrtOffset += wrtSampleSize;

				} // end x
			} // end y
		} // end z
	} // end BandSequential

	else { // Tiled

		wrtTileSize = isisinfo->TileSamples * isisinfo->TileLines * wrtSampleSize;
		wrtTilesPerRow = isisinfo->Samples / isisinfo->TileSamples;
		wrtSubTileSamples = isisinfo->Samples % isisinfo->TileSamples; // What is left over going across
		wrtSubTileSize = wrtSubTileSamples * isisinfo->TileLines * wrtSampleSize;
		wrtTotalRows = isisinfo->Lines / isisinfo->TileLines;
		wrtSubTileLines = isisinfo->Lines % isisinfo->TileLines; // What is left going down

		if (wrtSubTileSamples != 0) {
			wrtTilesPerRow += 1;
		}
		if (wrtSubTileLines != 0) {
			wrtTotalRows += 1;
		}

		for (wrtBands = 0; wrtBands < isisinfo->Bands; wrtBands++) {
			for (wrtRow = 0; wrtRow < wrtTotalRows; wrtRow++) {
				for (wrtTiles = 0; wrtTiles < wrtTilesPerRow; wrtTiles++) {
					for (wrt_row = 0; wrt_row < isisinfo->TileLines; wrt_row++) {
						wrtOffset = 0;
						wrtOffset += wrtBands * (isisinfo->Samples * isisinfo->Lines * wrtSampleSize); // How many whole "sheets" we have read
						wrtOffset += (wrtRow * wrtTilesPerRow * wrtTileSize); // This is the number of full Rows of full tiles
						wrtOffset += (wrtTiles * isisinfo->TileSamples * wrtSampleSize); // This is the X offset from the left, tile widths
						wrtOffset += (wrt_row * isisinfo->TileSamples * (wrtTilesPerRow) * wrtSampleSize); // This is the Y offset

#ifdef OUTDEBUG
 fprintf(snog,"Bands=%d, Rows=%d, row=%3d, Tiles=%d, wrtOffset=%8d\n", wrtBands, wrtRow, wrt_row, wrtTiles,wrtOffset);
#endif
						writeCount = 1;

						for (x = 0; x < isisinfo->TileSamples; x++) { // Reading across a tile

							if (writeCount != 1) {
								parse_error("Bad Write for Tiled Data");
								return 0;
							}

							if ( (wrtTiles == (wrtTilesPerRow - 1)) &&
								 (wrtSubTileSamples != 0) ) {

								if (x >= wrtSubTileSamples) {
									writeCount = fwrite(fakeBuffer, wrtSampleSize, 1, fp);
									continue;
								}
							}

							if ( (wrtRow == (wrtTotalRows - 1)) &&
								 (wrtSubTileLines != 0) ) {

								if (wrtRow >= wrtSubTileLines) {
									writeCount = fwrite(fakeBuffer, wrtSampleSize, 1, fp);
									continue;
								}
							}

							for (wrtElement = 0; wrtElement < wrtSampleSize; wrtElement++) {
								outBuffer[wrtElement] = cubeData[wrtOffset + wrtElement];
							}
							if ( strcmp(isisinfo->ByteOrder, "Lsb") == 0) {
								writeCount = fwrite(outBuffer, wrtSampleSize, 1, fp);
							}
							else {
								switch (wrtSampleSize) {
									case 2:
										swapBuffer[1] = outBuffer[0];
										swapBuffer[0] = outBuffer[1];
										break;
									case 4:
										swapBuffer[3] = outBuffer[0];
										swapBuffer[2] = outBuffer[1];
										swapBuffer[1] = outBuffer[2];
										swapBuffer[0] = outBuffer[3];
										break;
									default:
										;
								}
								writeCount = fwrite(swapBuffer, wrtSampleSize, 1, fp);
							}

							wrtOffset += wrtSampleSize;

						} // end TileSamples
					} // end tilesPerRow
				} // end rows
			} // end Rows
		}  // end Bands
	} // end tiled
#ifdef OUTDEBUG
	fclose(snog);
#endif
	cubeData = NULL;
	return 1;
}
