// pnm_module.c : Gary Rigg : January 2002
//
// Port of the PNM suite of utilities for DaVinci
// 

#include "parser.h"
#include "ff_modules.h"

// Main CUT function - Static so can't be extern'd
static Var *ff_pnmcut(vfuncptr f, Var *args);

// Main CUT Algorithm - None static so is reusable
Var *pnmcut(Var* obj, int iLeft, int iTop, int iWidth, int iHeight);

// CUT support functions - ARE ALL STATIC
static void print_cut_usage(void);
static int  convert_and_check_ranges(int *iLeft, int *iTop, 
		                     int iWidth, int iHeight, 
				     int x, int y);
static Var* go_and_cut(Range r, Var* obj);

static dvModuleFuncDesc exported_list[] = {
    {"cut", (void *)ff_pnmcut} /*,*/
};

static dvModuleInitStuff is = {
    exported_list, 1,
    NULL, 0
};

DV_MOD_EXPORT int
dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
	parse_error("*******************************************************");
	parse_error("*******   pnm_mod.c initialized with %-10s   ********", name);
	parse_error("*******************************************************");

    *init_stuff = is;
    
    return 1; /* return initialization success */
}

// CUT - Extracts a rectangle from either X,Y or X,Y,Z images.
// 
// INPUTS:  OBJECT - The image (matrix) to be cut
//          LEFT   - The leftmost column to be included in output
//                   Negative values imply from the RIGHT
//          TOP    - The topmost row to be included in the output
//                   Negative values imply from the BOTTOM
//          WIDTH  - Number of columns in the output (must be >0)
//          HEIGHT - Number of rows in the output (must be >0)
//
// RETURNS: The cropped image as output.
//
static Var *
ff_pnmcut(vfuncptr f, Var *args)
{
    Var *obj = NULL;
    int left, top, width, height = 0;

    Alist alist[6];
    alist[0] = make_alist("object", ID_VAL, NULL, &obj);
    alist[1] = make_alist("left", INT, NULL, &left);
    alist[2] = make_alist("top", INT, NULL, &top);
    alist[3] = make_alist("width", INT, NULL, &width);
    alist[4] = make_alist("height", INT, NULL, &height);
    alist[5].name = NULL;

    // Check arguments are valid
    if (0 == parse_args(f, args, alist))
    {
	    print_cut_usage();
	    return(NULL);
    }

    // Echo to user
    printf("Left:     %d\t", left);
    printf("Top:      %d\t", top);
    printf("Width:    %d\t", width);
    printf("Height:   %d\n", height);

    return pnmcut(obj, left, top, width, height);
}

Var* pnmcut(Var* obj, int iLeft, int iTop, int iWidth, int iHeight)
{
    int x,y,z = 0;
    Range r;

    // Read our Matrix
    x = GetX(obj); printf("Object X: %d\t",x);
    y = GetY(obj); printf("Object Y: %d\t",y);
    z = GetZ(obj); printf("Object Z: %d\n",z);
    
    // SENG RULE #1 - Assume the user is stupid. ;-D
    // So, lets check that their left/top and width/height values
    // are at least compatible!
    if (!convert_and_check_ranges(&iLeft, &iTop, iWidth, iHeight, x, y))
	    return (NULL);

    // Ok, all being well, we're good to go...
    // Remember, USER input is ONE based, but in code we're ZERO based
    // We're assuming NO changes on the Z plane
    // Create new object and cut...
    
    // X, Y, Z - But note ranges ONE based
    r.lo[0]   = iLeft;
    r.hi[0]   = (iLeft+iWidth)-1;
    r.step[0] = 0;
    r.lo[1]   = iTop;
    r.hi[1]   = (iTop+iHeight)-1;
    r.step[1] = 0;
    r.lo[2]   = 1;
    r.hi[2]   = z;
    r.step[2] = 0;

    return (extract_array(obj, &r));
} 

void print_cut_usage(void)
{
    printf("Usage: cut(object, left, top, width, height) where:\n\n");
    printf("LEFT is the leftmost column (negative implies from right)\n");
    printf("TOP is the topmost row (negative implies from bottom)\n");
    printf("WIDTH is the number of columns in the output.\n");
    printf("HEIGHT is the number of rows in the output.\n\n");
}

int convert_and_check_ranges(int *iLeft, int *iTop, int iWidth, int iHeight, int x, int y)
{
    // Convert NEGATIVES to equivalent POSITIVES
    if (*iLeft < 0)
	   *iLeft = x - (*iLeft*-1);
    if (*iTop < 0)
	   *iTop = y - (*iTop*-1);

    // Check values in RANGE.
    if (*iLeft > x || *iLeft <= 0)
    {
	    parse_error("\nBeginning Column number %d out of range (1 to %d)", *iLeft, x);
	    return 0;
    }
    if (*iTop > y || *iTop <= 0)
    {
	    parse_error("\nBeginning Row number %d out of range (1 to %d)", *iTop, y);
	    return 0;
    }

    if (iWidth > x  || iWidth <= 0)
    {
	    parse_error("Width %d is invalid (%d)", iWidth, x);
	    return 0;
    }
    if ( iHeight > y || iHeight <= 0 )
    {
	    parse_error("Height %d is invalid (%d)", iHeight, y);
	    return 0;
    }
    return 1; // Good job!
}

DV_MOD_EXPORT void
dv_module_fini(
    const char *name
)
{
	parse_error("*******************************************************");
	parse_error("********** pnm_mod.c finalized                ************");
	parse_error("*******************************************************");
}

