#include "parser.h"
#include <sys/types.h>
#include <sys/stat.h>

/*extern read_qube_data(int fp, struct _iheader *h); */

int GetGSEHeader (FILE *fp, struct _iheader *h)
{
	unsigned char	Buf[1000];
	unsigned int	Num[6];
	struct stat 	filebuf;

	int	Col,Row;
	int	Size;
	int 	Time;
	float	Exposure;
	int	i=0;
	int	nb=1;
	char	Nibs[9][24];/*Header has 9 short words in it */
	int	plane;

	rewind(fp);
	fstat(fileno(fp),&filebuf);
	Size=(filebuf.st_size)-1024;

	if (Size <=0){
		return(0);
	}

#if 0
	if (fread((void *)Num,sizeof (unsigned int),6,fp)!=6){
		return (0);
	}

	fscanf(fp,"%s %s %s %s %s %s %s %s %s",
		Nibs[0],Nibs[1],Nibs[2],Nibs[3],
		Nibs[4],Nibs[5],Nibs[6],Nibs[7],Nibs[8]);

	Col=Num[2];
	Row=Num[1];


/*Validate Header (this test might change) */

	if (strcmp(Nibs[0],"ms98") || strcmp(Nibs[1],"image")){
		return(NULL);
	}

/*
	if (strcmp(Nibs[8],"ci1656.bin"))
		nb=1;
	else
		nb=2;
*/
#endif
	/**
	*** I have hard-coded these values becase the header word sucks.
	**/
	Col = 1032;
	Row = 1024;
	nb = 2;

	plane=(Row*Col*nb);
	if (Size % plane) {
		parse_error("File contains incomplete frame");
	}


/*Okay, the file header is a GSE visible image; load the _iheader structure with info */
	
	h->org=BSQ;
	h->size[0]=Col;
	h->size[1]=Row;
	h->size[2]=(Size/(Row*Col*nb));
	h->format=((nb==1) ? (BYTE):(SHORT));
	h->dptr=1024;
	h->gain=0.0;
	h->corner=0;
	for (i=0;i<3;i++){
		h->suffix[i]=0;
		h->prefix[i]=0;
		h->s_lo[i]=0;
		h->s_hi[i]=h->size[i];
		h->s_skip[i]=0;
	}
	return (1);
}

Var *ff_GSE_VIS_Read(vfuncptr func, Var * arg)
{
	FILE	*infile;
	void	*data;
struct	_iheader header;
        int i,j;
    	int ac;
    	Var **av, *v;
    	Alist alist[2];
	char	*filename,*fname,fname2[256];

    	alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
    	alist[1].name = NULL;


    	make_args(&ac, &av, func, arg);
    	if (parse_args(ac, av, alist))
       	 	return (NULL);

    	if (filename == NULL) {
        	parse_error("No filename specified.");
        	return (NULL);
    	}


    	if ((fname = locate_file(filename)) == NULL ||
        	(infile = fopen(fname, "r")) == NULL) {
        	fprintf(stderr, "Unable to open file: %s\n", filename);
        	return (NULL);
    	}

    	strcpy(fname2, fname);
	free(fname);
    	fname = fname2;

	if (GetGSEHeader(infile,&header)==NULL){
		parse_error("Your choice is not a valid GSE visible spectrum (ddd) file");
		return (NULL);
	}

	data=read_qube_data(fileno(infile), &header);

	return(newVal(header.org,header.size[0],header.size[1],header.size[2],header.format,data));
}




Var *
ff_Frame_Grabber_Read(vfuncptr func, Var * arg)
{
	FILE	*infile;

	unsigned char *data_words;
	unsigned short *sh_data_words;
	unsigned char n;
        int i,j,nvals, nframes,nbytes;
        unsigned short fileType, nrows, npixels;
        unsigned short GSEarg[6];
        float ieeeVals[15];
        char c, dateTime[32], pad[14];

    	int ac;
    	Var **av, *v;
    	Alist alist[2];
	char	*filename,*fname,fname2[256];
	int	X,Y,Z;
	int	num_bytes;

    	alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
    	alist[1].name = NULL;


    	make_args(&ac, &av, func, arg);
    	if (parse_args(ac, av, alist))
       	 	return (NULL);

    	if (filename == NULL) {
        	parse_error("No filename specified.");
        	return (NULL);
    	}

    	if ((fname = locate_file(filename)) == NULL ||
        	(infile = fopen(fname, "r")) == NULL) {
        	fprintf(stderr, "Unable to open file: %s\n", filename);
        	return (NULL);
    	}

    	strcpy(fname2, fname);
	free(fname);
    	fname = fname2;

	fread (&fileType, sizeof(unsigned short), 1, infile);
	fread (&nrows, sizeof(unsigned short), 1, infile);
	fread (&npixels, sizeof(unsigned short), 1, infile);
	fread (&nframes, sizeof(int), 1, infile);
	fread (GSEarg, sizeof(unsigned short), 6, infile);
	fread (ieeeVals, sizeof(float), 15, infile);
	fread (dateTime, sizeof(char), 32, infile);
	fread (pad, sizeof(char), 14, infile);
	
	switch (fileType)
                        {
                        case 00 :
                        case 04 :
                                /* full frame 8 bits */
                                /* compute number of bytes of data */
                                nbytes = nrows*npixels*nframes;
                                nvals = npixels;
                                data_words = (unsigned char *) calloc(sizeof(unsigned char), nbytes);
                                fread (data_words, sizeof(unsigned char), nbytes, infile);
				X=npixels;
				Y=nrows;
				Z=nframes;
				num_bytes=1;
				break;
                        case 01 :
                                /* line mode 8 bits */
                                /* compute number of bytes of data */
                                nbytes = 2*npixels*nframes;
                                nvals = 2*npixels;
                                data_words = (unsigned char *) calloc(sizeof(unsigned char), nbytes);
                                fread (data_words, sizeof(unsigned char), nbytes, infile);
				X=npixels;
				Y=2;
				Z=nframes;
				num_bytes=1;
				break;
 			case 02 :
                                /* Pixel mode 8 bits */
                                /* compute number of bytes of data */
                                nbytes = nrows*nframes;
                                nvals = nrows;
                                data_words = (unsigned char *) calloc(sizeof(unsigned char), nbytes);
                                fread (data_words, sizeof(unsigned char), nbytes, infile);
				X=1;
				Y=nrows;
				Z=nframes;
				num_bytes=1;
				break;
                        case 03 :
                                /* 9 Pixel mode 8 bits */
                                /* compute number of bytes of data */
                                nbytes = 36*nframes;
                                data_words = (unsigned char *) calloc(sizeof(unsigned char), nbytes);
                                fread (data_words, sizeof(unsigned char), nbytes, infile);
				X=9;
				Y=3;
				Z=nframes;
				num_bytes=1;	
				break;
                        case 10 :
                        case 14 :
                                /* full frame 12 bits */
                                /* compute number of bytes of data */
                                nbytes = nrows*npixels*nframes*2;
                                nvals = 2*npixels;
                                sh_data_words = (unsigned short *) calloc(sizeof(unsigned char), nbytes);
                                fread (sh_data_words, sizeof(unsigned short), nbytes, infile);
				X=npixels;
				Y=nrows;
				Z=nframes;
				num_bytes=2;
                                break;
                        case 20 :
                                parse_error("IEEE data file: (Consists of just header)");
                                break;
                        } /* switch fileType */

                fclose(infile);

		return(newVal(BSQ,X,Y,Z,num_bytes,((num_bytes==2) ? ((void *)sh_data_words) : ((void *)data_words))));

}
