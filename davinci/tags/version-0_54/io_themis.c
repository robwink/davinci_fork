#include "parser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <values.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __MSDOS__
#include <sys/mman.h>
#endif


#define	_BANDS	3
#define	_FRAMES	4
#define _DATA	6


enum _CMD {BAND,FRAME,SINGLE,ALL};

typedef enum _CMD CMD;
int Skip_To_Stop_Sync(int *i, char *buf,int len);
int Skip_To_Start_Sync(int *i, char *buf,int len);
int Keep(unsigned char B, unsigned short F, CMD Cmd, int Frame, int Band);

#ifndef LITTLE_E
unsigned short Start_Sync = { 0xF0CA };
unsigned short Stop_Sync = { 0xAB8C };
#else
unsigned short Start_Sync = { 0xCAF0 };
unsigned short Stop_Sync = { 0x8CAB };
#endif

int Skip_To_Stop_Sync(int *i, char *buf,int len)
{
    int count=0;
    int Tmp=*i;
	short V;

    while ((*i)<len-1){
		V=*((short *)(buf+(*i)));
        if (memcmp(&V, &Stop_Sync, 2) == 0) {
            return (count);
        }
        else {
            (*i)++;
            count++;
        }
    }
    parse_error("Premature EOF.\n");
    return(-1);
}


int Skip_To_Start_Sync(int *i, char *buf,int len)
{
    int Tmp=*i;
	short V;

    while ((*i)<len-1){
		V=*((short *)(buf+(*i)));
        if (memcmp(&V, &Start_Sync, 2) == 0) {
            return(1);
        }
        else
            (*i)++;
    }

    return(-1);
}


int Keep(unsigned char B, unsigned short F, CMD Cmd, int Frame, int Band)
{
    switch (Cmd) {

    case BAND:
        if(Band==(((int)B) & 0x0F))
            return (1);
        break;

    case FRAME:
        if (Frame==(((int)F) & 0xFFFF) && 
            (B!=0x0F && B!=0xE))
            return (1);
        break;

    case ALL:
        if (B!=0x0F && B!=0xE)
            return(1);
        break;
    }

    return(0);
}


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

	if (GetGSEHeader(infile,&header) == 0) {
		parse_error("Your choice is not a valid GSE visible spectrum (ddd) file");
		return (NULL);
	}

	data=read_qube_data(fileno(infile), &header);

	fclose(infile);

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


Var *
ff_PAKI_Read(vfuncptr func, Var * arg)
{
#ifdef __MSDOS__
	extern unsigned char *mmap(void *, size_t , int, int , int , size_t );
	extern void munmap(unsigned char *,int);
	typedef	void*	caddr_t;
#endif
	void		*data;
	char 		*filename,*fname,fname2[256];
	int 		Frame=-1;
	int 		Band=-1;
	int 		report=0;
	int		len=0;
	int 		i, j, a, b;
	int 		fd;
	struct 		stat sbuf;
	unsigned 	char *buf;
	unsigned 	char  c_Band;
	unsigned 	short c_Frame;	
	CMD		Cmd;
	int		Output=0;
	int		Done=0;
	int		Col=0;
	int		Lines=0;
	int		Data_Only=0;
	int		Index=0;

    	Var **av, *v;
    	int ac;
    	Alist alist[6];

    	alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
    	alist[1] = make_alist("frame", INT, NULL, &Frame);
    	alist[2] = make_alist("band", INT, NULL, &Band);
    	alist[3] = make_alist("report", INT, NULL, &report);
    	alist[4] = make_alist("nosig", INT, NULL, &Data_Only);
    	alist[5].name = NULL;


    	make_args(&ac, &av, func, arg);
    	if (parse_args(ac, av, alist))
       	 	return (NULL);

    	if (filename == NULL) {
        	parse_error("No filename specified.");
        	return (NULL);
    	}


    	if (Frame < 0 && Band < 0)
		Cmd=ALL;
	else if (Frame < 0 && Band >=0)
		Cmd=BAND;
	else if (Frame >= 0 && Band <0)
		Cmd=FRAME;
	else {
		parse_error("Ignoring Band, using Frame\n");
		Cmd=FRAME;
		Band=-1;
	}
#ifndef __MSDOS__
	if ((fd = open(filename, O_RDONLY))==NULL){
#else
	if ((fd = open(filename, O_RDONLY | O_BINARY))==-1){
#endif
		fprintf(stderr,"Can't open file: %s...aborting\n",filename);
		return(NULL);
	}


	fstat(fd, &sbuf);
	len = sbuf.st_size;
	data=malloc(len);
	buf = mmap((caddr_t)0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
//#ifndef LITTLE_E
	swab(buf, buf, len);
//#endif


	if (!report) {
		i = 0;
		while ((i < len) && !(Done)) {
			c_Band=0;
			c_Frame=0;
			if (Skip_To_Start_Sync(&i,buf,len) == -1) {
				Done = 1;
			} 
			else {
				c_Band=*(buf+i+_BANDS);
				if (c_Band==0xE)  /*Last Frame*/
					Done=1;
				c_Frame=(((*(buf+i+_FRAMES) & 0xFF)<< 8) | (*(buf+i+_FRAMES+1) & 0xFF));
				if ((Output=Skip_To_Stop_Sync(&i,buf,len)) == -1) {
					Done = 1;
				} 
				else {
					if (Keep(c_Band,c_Frame,Cmd,Frame,Band)){
						if (Data_Only){
							memcpy(((char *)data+Index),(buf+i-Output+_DATA),Output);
							Index+=(Output-_DATA);
							Col=Output-_DATA;
						}
						else {
							memcpy(((char *)data+Index),(buf+i-Output),Output+2);
							Index+=Output+2;
							Col=Output+2;
						}
						Lines++;
					}
				}
			}
		}
	} 
	else {
		int *band = (int *)calloc(32, sizeof(int));
		unsigned int minframe = MAXINT;
		int maxframe = -1;
		i = 0;
		while ((i < len) && !(Done)) {
			c_Band=0;
			c_Frame=0;
			if (Skip_To_Start_Sync(&i,buf,len) >= 0) {
				c_Band=*(buf+i+_BANDS);
				if (c_Band==0xE) /*Last Frame*/
					Done=1;
				c_Frame=(((*(buf+i+_FRAMES) & 0xFF)<< 8) | (*(buf+i+_FRAMES+1) & 0xFF));
				if ((Output=Skip_To_Stop_Sync(&i,buf,len)) >= 0) {
					if (c_Frame > maxframe) maxframe = c_Frame;
					if (c_Frame < minframe) minframe = c_Frame;
					band[c_Band]++;
				} 
				else {
					Done++;
				}
			} 
			else {
				Done++;
			}
		}
		printf("File: %s\n", filename);
		printf("First frame: %d\n", minframe);
		printf("Last Frame:  %d\n", maxframe);
		printf("Bands:       ");
		for (i = 0 ; i< 32 ; i++) {
			if (band[i]) printf("%d ", i);
		}
		printf("\n");
	}
	
	munmap(buf,len);
	close(fd);
	return(newVal(BSQ,Col,Lines,1,BYTE,data)); 

}

