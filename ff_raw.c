#include "parser.h"
#include "iomedley.h"


Var *
ff_raw(vfuncptr func, Var * arg)
{
	unsigned char *buf;
	char *filename=NULL;
	FILE *fp;
	int x = -1,y = -1,z =-1,header=0;
	char *org = NULL;
	char *format =NULL;
	struct iom_iheader h;
	char *data;
	Var *v;

	char *orgs[] = { "bil", "bip", "bsq" ,NULL};
	char *formats[] = { 
		"byte", "msb_short", "msb_int", "msb_float", "msb_double" ,
		        "lsb_short", "lsb_int", "lsb_float", "lsb_double" , NULL
		};

	Alist alist[11];
	alist[0] = make_alist( "filename",  ID_STRING,  NULL,     &filename);
	alist[1] = make_alist( "x", INT,NULL,&x);
	alist[2] = make_alist( "y", INT,NULL,&y);
	alist[3] = make_alist( "z", INT,NULL,&z);
	alist[4] = make_alist( "org", ID_ENUM,orgs,&org);
	alist[5] = make_alist( "format", ID_ENUM,formats,&format);
	alist[6] = make_alist( "header", INT,NULL,&header);
	alist[7].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (filename == NULL) {
		parse_error("No filename specified to load()");
		return (NULL);
	}
	if (x == -1 ||  y == -1 || z ==-1 || org == NULL || format==NULL) {
		parse_error("usage: %s(filename=,x=,y=,z=,org=,format=,[header=])", 
			func->name);
		return(NULL);
	}

	if ((fp=fopen(filename,"rb"))==NULL){
		parse_error("Can't find file: %s",filename);
		return(NULL);
	}

	iom_init_iheader(&h);

	if (!strcmp(format, "byte")) {
		h.eformat = iom_MSB_INT_1;
		h.format = iom_BYTE;
	} else if (!strcmp(format, "msb_short")) {
		h.eformat = iom_MSB_INT_2;
		h.format = iom_SHORT;
	} else if (!strcmp(format, "lsb_short")) {
		h.eformat = iom_LSB_INT_2;
		h.format = iom_SHORT;
	} else if (!strcmp(format, "msb_int")) {
		h.eformat = iom_MSB_INT_4;
		h.format = iom_INT;
	} else if (!strcmp(format, "lsb_int")) {
		h.eformat = iom_LSB_INT_4;
		h.format = iom_INT;
	} else if (!strcmp(format, "msb_float")) {
		h.eformat = iom_MSB_IEEE_REAL_4;
		h.format = iom_FLOAT;
	} else if (!strcmp(format, "lsb_float")) {
		h.eformat = iom_LSB_IEEE_REAL_4;
		h.format = iom_FLOAT;
	} else if (!strcmp(format, "msb_double")) {
		h.eformat = iom_MSB_IEEE_REAL_8;
		h.format = iom_DOUBLE;
	} else if (!strcmp(format, "lsb_double")) {
		h.eformat = iom_LSB_IEEE_REAL_8;
		h.format = iom_DOUBLE;
	} else {
		parse_error("Unrecognized format: %s\n", format);
		return(NULL);
	}

	if (!strcmp(org, "bsq")) {
		h.size[0] = x;
		h.size[1] = y;
		h.size[2] = z;
		h.org = iom_BSQ;
	} else if (!strcmp(org, "bil")) {
		h.size[0] = x;
		h.size[1] = z;
		h.size[2] = y;
		h.org = iom_BIL;
	} else if (!strcmp(org, "bip")) {
		h.size[0] = z;
		h.size[1] = x;
		h.size[2] = y;
		h.org = iom_BIP;
	} else {
		parse_error("Unrecognized org: %s\n", org);
		return(NULL);
	}

	h.dptr = header;
	h.gain = 1.0;
	h.offset = 0.0;
	h.prefix[0] = h.prefix[1] = h.prefix[2] = 0;
	h.suffix[0] = h.suffix[1] = h.suffix[2] = 0;

	data = iom_read_qube_data(fileno(fp), &h);
	v = iom_iheader2var(&h);
	V_DATA(v)  = data;

	return(v);
}
