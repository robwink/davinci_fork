#include "parser.h"


Var *
ff_popen(vfuncptr func, Var * arg)
{

	int Row=1,Col=1,Xpos=200,Ypos=200,Xpixels=512,Ypixels=512;

        int ac;
        Var **av;
        Alist alist[8];
        alist[0] = make_alist("Row",		INT,	NULL,   &Row);
        alist[1] = make_alist("Col",		INT,	NULL,	&Col);
        alist[3] = make_alist("Xpos",		INT, 	NULL,	&Xpos);
        alist[4] = make_alist("Xpos",		INT, 	NULL,	&Ypos);
        alist[5] = make_alist("Xpixels",		INT, 	NULL,	&Xpixels);
        alist[6] = make_alist("Ypixels",		INT, 	NULL,	&Ypixels);
        alist[7].name = NULL;

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) return(NULL);

	c_plssub(Row,Col);
	c_plsdev("xtwin");
	c_plspage(0,0,Xpixels,Ypixels,Xpos,Ypos);
	c_plinit();

	return (NULL);
}

