#include "parser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <values.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __MSDOS__
#include <sys/mman.h>
#endif

#define	_IMAGEID	2
#define	_BANDS	3
#define	_FRAMES	4
#define  _DATA		6
#define 	_SIGINFO	8
#define	MAXDATA	320
#define	MINDATA	128
#define NUMBER_OF_GUESSES 4
#define	INVALID_DATA	0

int Compressed;


enum _CMD {BAND,FRAME,SINGLE,ALL};
int FrameBandStart[]={16,32,58,84,110,136,162,188,213,239};

typedef enum _CMD CMD;
int Skip_To_Stop_Sync(int *i, unsigned char *buf,int len);
int Skip_To_Start_Sync(int *i, unsigned char *buf,int len);
int Keep(unsigned char B, unsigned short F, CMD Cmd, int Frame, int Band);
int Read_Ahead_For_Best_Collumn_Guess(int *Width,unsigned char *buf,int len);

#ifndef LITTLE_E
unsigned short Start_Sync = { 0xF0CA };
unsigned short Stop_Sync = { 0xAB8C };
unsigned int StopStart_Sync = {0xAB8CF0CA};
#else
unsigned short Start_Sync = { 0xCAF0 };
unsigned short Stop_Sync = { 0x8CAB };
unsigned int StopStart_Sync = {0x8CABCAF0};
#endif


#ifdef HAVE_LIBUSDS
extern unsigned char *Themis_Entry(unsigned char*, int *);
#endif

int 
Read_Ahead_For_Best_Collumn_Guess(int *Width,unsigned char *buf,int len)
{
    int i,j;
    int Done=0;

    int LineCount=0;
    int Guesses[NUMBER_OF_GUESSES];

    i=0;
	

    while (!Done) {

        if(Skip_To_Start_Sync(&i,buf,len) < 0){
            return(++Done);
        }
			
        if( (Guesses[LineCount]=Skip_To_Stop_Sync(&i,buf,len)) < 0){
            return(++Done);
        }

        if (Guesses[LineCount] <= (MAXDATA+_SIGINFO) &&
            Guesses[LineCount] >= (MINDATA+_SIGINFO)){
            LineCount++;
        }

        if (LineCount == NUMBER_OF_GUESSES) {
            for (j=0;j<NUMBER_OF_GUESSES-1;j++){
                if(Guesses[j]!=Guesses[j+1]){
                    j=NUMBER_OF_GUESSES;
                    LineCount=0;/*Start again*/
                }
            }
            if (LineCount) {/*Got a good guess!*/
                *Width=(Guesses[0]+2)/*Add two for the stop sync*/;
                return(0);
            }
        }
    }
    return (1);
}	
			



#if 0
int Skip_To_Stop_Sync(int *i, unsigned char *buf,int len)
{
    int count=0;
    int Tmp=*i;
	 int result;	

    while ((*i)<len-1){
        if (memcmp((buf+(*i)), &Stop_Sync, 2) == 0) {
				if (!(Compressed)){
					if (*i-Tmp+2==328)
						return(count);
					else {
						int Tmp2=*i;
						result=Skip_To_Start_Sync(&Tmp2,buf,len);
						if (result < 0) {
							return(-1);
						}
						if (Tmp2-Tmp==328){/*Found a false stop sync*/
							*i=Tmp2-2;/*Set it to real stop sync*/
							return(*i-Tmp);
						}
						else if (Tmp2-Tmp > 328) {/*We have a junk packet somewhere*/
							return(count);/*return original count, fingure it out from tere*/
						}


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
#endif


int Skip_To_Stop_Sync(int *i, unsigned char *buf,int len)
{
    int count=0;
    int Tmp=*i;
	 int result;	

    while ((*i)<len-1){
        if (memcmp((buf+(*i)), &Stop_Sync, 2) == 0) {
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


int Skip_To_StopStart_Sync(int *i, unsigned char *buf,int len)
{
    int count=0;
    int Tmp=*i;
	 int result;	

    while ((*i)<len-3){
        	if (memcmp((buf+(*i)), &StopStart_Sync, 4) == 0) {
				/*Set i (and count) to be past the stop sync, but
					at the begining of the start sync
				*/
				(*i)+=2;
				count+=2;

				return(count);
			}
			else {
				count++;
				(*i)++;
			}
	 }
/*If we're here we've run past the end of file:
	End packet with only have a Stop, so
	just look for that
*/
	*i=Tmp;
	return(Skip_To_Stop_Sync(i,buf,len));
}



int Skip_To_Start_Sync(int *i, unsigned char *buf,int len)
{
    int Tmp=*i;
    short V;

    while ((*i)<len-1){
        if (memcmp((buf+(*i)), &Start_Sync, 2) == 0) {
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
        parse_error("File contains incomplete frame\n");
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
    int dsize;
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
    if (header.format=SHORT){ /*Data is actually 12-bit and needs the upper 4 bits cleaned off*/
        dsize=header.size[0]*header.size[1]*header.size[2];
        for (i=0;i<dsize*2;i+=2){
            ((short *)(data))[i] &= 4095;
        }
    }

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

static int Themis_Sort(const void *vA, const void *vB)
{
	char *A = (char *)vA;
	char *B = (char *)vB;
    unsigned char A_c_Band;
    unsigned short A_c_Frame;
    unsigned char B_c_Band;
    unsigned short B_c_Frame;

    A_c_Band=*(A+_BANDS)&0xf;
    B_c_Band=*(B+_BANDS)&0xf;

    if (A_c_Band > B_c_Band)
        return(1);
    else if (A_c_Band < B_c_Band)
        return(-1);
    else {
   	A_c_Frame=(((*(A+_FRAMES) & 0xFF)<< 8) | (*(A+_FRAMES+1) & 0xFF));
   	B_c_Frame=(((*(B+_FRAMES) & 0xFF)<< 8) | (*(B+_FRAMES+1) & 0xFF));

        if (A_c_Frame > B_c_Frame)
            return(1);
        else if (A_c_Frame < B_c_Frame) 
            return(-1);
        else
            return(0);
    }

    return(0);
}

void
Add_Fake_Frame(void *newbuf,int band, int frame, int num_frames,int width)
{
    int i;
    unsigned char pseudo_ID=99;
    unsigned short new_frame;
    char *blank=malloc(width-8);
    memset(blank,INVALID_DATA,(width-8));

    for (i=0;i<num_frames;i++){
        new_frame=frame+i;	
        memcpy((((unsigned char *)newbuf)+i*width),&Start_Sync,2);
        memcpy((((unsigned char *)newbuf)+i*width+2),&pseudo_ID,1);
        memcpy((((unsigned char *)newbuf)+i*width+_BANDS),&band,1);
        memcpy((((unsigned char *)newbuf)+i*width+_FRAMES),&new_frame,2);
        memcpy((((unsigned char *)newbuf)+i*width+_DATA),blank,(width-8));
        memcpy((((unsigned char *)newbuf)+i*width+(width-2)),&Stop_Sync,2);
    }

    free(blank);
}


	

Var *
ff_PACI_Read(vfuncptr func, Var * arg)
{
#ifdef __MSDOS__
    extern unsigned char *mmap(void *, size_t , int, int , int , size_t );
    extern void munmap(unsigned char *,int);
    typedef	void*	caddr_t;
#endif
    void		*data;
    void		*newbuf;
    char 		*filename,*fname,fname2[256];
    unsigned char *decompress=NULL;
    int 		Frame=-1;
    int 		Band=-1;
    int 		report=0;
    int		len=0;
    int		old_len=0;
    int 		i, j, a, b;
    int 		fd;
	 int		swap_flag=1;
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
    int		BandCount[16]={0};
    int		Bad_Flag=0;
    int		Max_Band=-1;
    int		Max_Frame_Count=0;
	 int		band_frame_count;
	 int		Total_Frame_Count;
    int		offset;

    int		debug=0;

    Var **av, *v;
    int ac;
    Alist alist[7];

    alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
    alist[1] = make_alist("frame", INT, NULL, &Frame);
    alist[2] = make_alist("band", INT, NULL, &Band);
    alist[3] = make_alist("report", INT, NULL, &report);
    alist[4] = make_alist("nosig", INT, NULL, &Data_Only);
    alist[5] = make_alist("swap", INT, NULL, &swap_flag);
    alist[6].name = NULL;


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
    if ((fd = open(filename, O_RDONLY))==-1){
        fprintf(stderr,"Can't open file: %s...aborting\n",filename);
        return(NULL);
    }
#else
    if ((fd = open(filename, O_RDONLY | O_BINARY))==-1){
        fprintf(stderr,"Can't open file: %s...aborting\n",filename);
        return(NULL);
    }
#endif


    fstat(fd, &sbuf);
    old_len=len = sbuf.st_size;


    buf = mmap((caddr_t)0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(swap_flag)
      swab(buf, buf, len);

	 len+=328;		 /*Add room for an additional packet; this will prevent
							 an unitialized read from a file that is still being
							 written to*/

    if ((data=malloc(len))==NULL){
        parse_error("Couldn't allocate temporary buffer...aborting\n");
        return(NULL);
    }


    if (!report) {
        i = 0;

#ifdef HAVE_LIBUSDS
	Col=320+_SIGINFO;
#else
        Col=Output+2;			
#endif

		
/*Find Start of data transmission*/

       if (Skip_To_Start_Sync(&i,buf,old_len) == -1) {/*Most likely not a themis file*/
               parse_error("Doesn't look like this is a themis file...Aborting\n");
					return(NULL);
       }

        while ((i < (old_len-_DATA)) && !(Done)) {
            c_Band=0;
            c_Frame=0;
            c_Band=*(buf+i+_BANDS)&0xf;
            if (c_Band < 14 && c_Band > Max_Band) Max_Band=c_Band;
            if (c_Band==0xE)  /*Last Frame*/
                Done=1;

            c_Frame=(((*(buf+i+_FRAMES) & 0xFF)<< 8) | (*(buf+i+_FRAMES+1) & 0xFF));
            if ((Output=Skip_To_StopStart_Sync(&i,buf,old_len)) == -1) {
                break;
            } 

            if (Keep(c_Band,c_Frame,Cmd,Frame,Band) && ((Output) <= 328 )){

#ifdef HAVE_LIBUSDS

                if (Data_Only && Cmd!=ALL){
                    if (Output-_SIGINFO < 320){
                        offset=Output-_SIGINFO;
                        if (decompress!=NULL)
                            free(decompress);
                        decompress=Themis_Entry((buf+i-Output+_DATA),&offset);
                        if (offset==320){
									 Compressed=1;
                            memcpy((data+Index),decompress,offset);
                            Index+=(offset);
                            Lines++;
                				 BandCount[c_Band]++;
                        }
                    }
                    else {
								Compressed=0;
                        memcpy((data+Index),(buf+i-Output+_DATA),Output-_SIGINFO);
                        Index+=(Output-_SIGINFO);
                        Lines++;
                			BandCount[c_Band]++;
                    }
                } 
					 else { 
                    if (Output-_SIGINFO < 320){

                        if (len < (Lines*Col+320)) { 	/*We're running out of room!*/
                            int guess=(len/(Output));/*Guess # of total lines*/
                            len = guess * Col; 			/*How big it needs to be*/
                            parse_error("Increasing Buffer sizes\n");
                            data=realloc(data,len);
                        }

                        offset=Output-_SIGINFO;
                        if (decompress!=NULL)
                            free(decompress);
                        decompress=Themis_Entry((buf+i-Output+_DATA),&offset);
                        if (offset==320){
									 Compressed=1;
                            memcpy((data+Index),(buf+i-Output),_DATA);
                            Index+=_DATA;
                            memcpy((data+Index),decompress,offset);
                            Index+=(offset);
                            memcpy((data+Index),(buf+i-2),2);/*Stop sync*/
                            Index+=2;
                            Lines++;
                				 BandCount[c_Band]++;
                        }
                    }
                    else {
								Compressed=0;
                        memcpy(((char *)data+Index),(buf+i-Output),Output);
                        Index+=Output;
                        Lines++;
                			BandCount[c_Band]++;
                    }
                }
#else
                if (Data_Only && Cmd!=ALL){
                    memcpy(((unsigned char *)data)+Index,
							((unsigned char *)buf)+i-Output+_DATA,
							Output-_SIGINFO);
                    Index+=(Output-_SIGINFO);
                    Lines++;
	                 BandCount[c_Band]++;
                } else { 
                    memcpy(((char *)data+Index),(buf+i-Output),Output);
                    Index+=Output;
                    Lines++;
	                 BandCount[c_Band]++;
                }
#endif
            }
        }

        if (debug) {
            FILE *wp=fopen("debug.dump","wb");
            fwrite(data,sizeof(char),len,wp);
            fclose(wp);
        }

        if (Cmd==ALL) {
            int extra_frames=0;
            qsort((char *)data,Lines,Col,Themis_Sort);
				/*Pack Bands: this throws out empty bands*/
				{
					int Bc[16];
					int idx=0;
					int i;
					for (i=0;i<=Max_Band;i++){
						if (BandCount[i]){
							Bc[idx]=BandCount[i];
							idx++;
							printf("Mapping Band:%d to Davinci slot:%d\n",i,idx);
						}
						else {
							printf("Band %d has no data, throwing it out\n",i);
						}
					}
					Max_Band=idx-1;
					for(i=0;i<=Max_Band;i++){
						BandCount[i]=Bc[i];
					}
				}
	
            Max_Frame_Count=BandCount[0];
            for (i=0;i<Max_Band;i++){
                if (BandCount[i]!=BandCount[i+1]) {
                    Bad_Flag=1;
                }
                if (Max_Frame_Count < BandCount[i+1]) Max_Frame_Count=BandCount[i+1];
			   	
            }
            for (i=0;i<=Max_Band;i++){ /*Now, gotta count the missing frames!*/
                extra_frames+=(Max_Frame_Count-BandCount[i]);
            }
            len+=Col*extra_frames;

            if ((newbuf=malloc(len))==NULL){
      			parse_error("Couldn't allocate transfer buffer...aborting\n");
      			return(NULL);
            }	

/*            if (Bad_Flag){ We have uneven Band counts! Gotta do things differently! */
				{
               unsigned char c_band;
               unsigned char current_band=255;
               unsigned short c_frame;
					int i;
					band_frame_count=Max_Frame_Count+1;
					Total_Frame_Count=0;


					for (i=0;i<Lines;i++){
                    	c_band=*((char *)data+(i*Col)+_BANDS)&0xf;
                    	c_frame=(((*((char *)data+(i*Col)+_FRAMES) & 0xFF)<< 8) | 
                             (*((char *)data+(i*Col)+_FRAMES+1) & 0xFF));
							
							if (current_band!=c_band) {
								if (band_frame_count < Max_Frame_Count) {/*Missing end frames*/
                        	printf("Extending buffer:%d\n",(Col*(Max_Frame_Count-band_frame_count)));
                        	len+=(Col*(Max_Frame_Count-band_frame_count));
                        	if((newbuf=realloc(newbuf,len))==NULL){
                        	    printf("Couldn't extend buffer...you're hosed\n");
                        	    return(NULL);
                        	}
                    

                        	Add_Fake_Frame((((unsigned char *)newbuf)+Total_Frame_Count*Col),
											(int)c_band, (FrameBandStart[current_band]+band_frame_count), 
											(Max_Frame_Count-band_frame_count), Col);

									parse_error("Adding End frames: %d-%d in Band: %d\n",
										(FrameBandStart[current_band]+band_frame_count),
										(FrameBandStart[current_band]+Max_Frame_Count-1), c_band);

                            Total_Frame_Count+=(Max_Frame_Count-band_frame_count);
                        }
								band_frame_count=0;
								current_band=c_band;
							}
					
							if (c_frame > (FrameBandStart[current_band]+band_frame_count)){ /*Missing frames*/
                       printf("Extending buffer: %d\n",(c_frame-(FrameBandStart[current_band]+
																			band_frame_count))*Col);
                       len+=(c_frame-(FrameBandStart[current_band]+band_frame_count))*Col;

                       if((newbuf=realloc(newbuf,len))==NULL){
                       	    printf("Couldn't extend buffer...you're hosed\n");
                       	    return(NULL);
                       }
                    		

								Add_Fake_Frame((((unsigned char *)newbuf)+Total_Frame_Count*Col),
									(int)c_band, (FrameBandStart[current_band]+band_frame_count),
									(c_frame-(FrameBandStart[current_band]+band_frame_count)), Col);
					
								parse_error("Adding frames: %d-%d in Band: %d\n",
									(FrameBandStart[current_band]+band_frame_count),
									(c_frame-1),c_band);

								Total_Frame_Count+=c_frame-(FrameBandStart[current_band]+band_frame_count);
								band_frame_count+=c_frame-(FrameBandStart[current_band]+band_frame_count);
							}

							else if (c_frame < (FrameBandStart[current_band]+band_frame_count)) {
								parse_error("Corrupted Frame Sequence...Aborting\n");
								return(NULL);
							}

				
                    memcpy((((unsigned char *)newbuf)+(Total_Frame_Count*Col)),
							(((unsigned char *)data)+i*Col),
							Col);
                    band_frame_count++;
						  Total_Frame_Count++;
						
			
                }

					if (band_frame_count < Max_Frame_Count) {/*Missing end frames*/

                 	Add_Fake_Frame((((unsigned char *)newbuf)+Total_Frame_Count*Col),
							(int)c_band, (FrameBandStart[current_band]+band_frame_count), 
							(Max_Frame_Count-band_frame_count), Col);

						parse_error("Adding End frames: %d-%d in Band: %d\n",
							(FrameBandStart[current_band]+band_frame_count),
							(FrameBandStart[current_band]+Max_Frame_Count-1), c_band);

                  Total_Frame_Count+=(Max_Frame_Count-band_frame_count);
						band_frame_count+=(Max_Frame_Count-band_frame_count); 
                }
					 


           } 
#if 0

				else {

                int j,i;
                unsigned short c_frame;
					 unsigned char c_band;
					 Total_Frame_Count=0;

                for (j=0 ; j <= Max_Band ; j++) {
					 	  band_frame_count=0;
                    c_band=*((char *)data+(i*Col)+_BANDS)&0xf;
						  
                    for (i=0 ; i < BandCount[0] ; i++) {
                        c_frame=(((*((char *)data+(j*BandCount[0]*Col)+i*Col+_FRAMES) & 0xFF)<< 8) | 
                                 (*((char *)data+(j*BandCount[0]*Col)+i*Col+_FRAMES+1) & 0xFF));

                        if (c_frame > (FrameBandStart[c_band]+band_frame_count)) {
                            /*Add a frame, gotta add memory too!*/
                            len+=(Col);
                            newbuf=realloc(newbuf,len);

                            Add_Fake_Frame((((unsigned char *)newbuf)+ Total_Frame_Count*Col),
										 j, (FrameBandStart[c_band]+band_frame_count),
                               c_frame-(FrameBandStart[c_band]+band_frame_count), Col);
									 parse_error("Adding frames: %d-%d in Band: %d\n",
										(FrameBandStart[c_band]+band_frame_count),
										(c_frame-1), c_band);
                            new_count+=c_frame-(FrameBandStart[c_band]+new_count);	



                        } else if (c_frame < (FrameBandStart[c_band]+new_count)) {
                            parse_error("Corrupted Frame Sequence...aborting\n");
                            return(NULL);
                        }
						
                        memcpy((((unsigned char *)newbuf)+j*BandCount[0]*Col+(new_count++)*Col),
                               (((unsigned char *)data)+j*BandCount[0]*Col+i*Col),
							   Col);
                    }
                }
            }	
#endif


        } 

		  else {
            munmap(buf,old_len);
            close(fd);
            if (Data_Only){
                return(newVal(BSQ,(Col-_SIGINFO),Lines,1,BYTE,data));
            }
            else {
                return(newVal(BSQ,Col,Lines,1,BYTE,data));
            }
        }

        munmap(buf,old_len);
        close(fd);
        free(data);
        if (Data_Only){
            void *do_buff;
            int count = band_frame_count;
            count*=(Max_Band+1);
            if ((do_buff=malloc(len))==NULL){
                parse_error("Not enough memory for temporary buffer support\n");
                free(newbuf);
                return(NULL);
            }	
            for (i=0;i<Total_Frame_Count;i++){
                memcpy((((unsigned char *)do_buff)+(i*(Col-_SIGINFO))),
						(((unsigned char *)newbuf)+(i*Col)+_DATA),
						(Col-_SIGINFO));
            }
            free(newbuf);
            return(newVal(BSQ,(Col-_SIGINFO),
                          band_frame_count,
                          Max_Band+1,BYTE,do_buff)); 
        }
        return(newVal(BSQ,Col,band_frame_count,Max_Band+1,BYTE,newbuf));
		
    } else {
        int *band = (int *)calloc(32, sizeof(int));
        unsigned int minframe = MAXINT;
        int maxframe = -1;
        i = 0;

        while ((i < len) && !(Done)) {
            c_Band=0;
            c_Frame=0;
            if (Skip_To_Start_Sync(&i,buf,len) < 0) {
                break;
            }
            c_Band=*(buf+i+_BANDS);
            if (c_Band==0xE) /*Last Frame*/
                Done=1;
            c_Frame=(((*(buf+i+_FRAMES) & 0xFF)<< 8) | (*(buf+i+_FRAMES+1) & 0xFF));
            if ((Output=Skip_To_Stop_Sync(&i,buf,len)) < 0) {
                break;
            }
            if (c_Frame > maxframe) maxframe = c_Frame;
            if (c_Frame < minframe) minframe = c_Frame;
            band[c_Band]++;
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

    free(newbuf);
    free(data);
	
    munmap(buf,old_len);
    close(fd);
    return(NULL);

}






/*Older code, moved down here while I tested "newer" code*/

#if 0
                int i;
                int new_count=0;
                for (i=0;i<Lines;i++) {
                    c_band=*((char *)data+(i*Col)+_BANDS)&0xf;
                    c_frame=(((*((char *)data+(i*Col)+_FRAMES) & 0xFF)<< 8) | 
                             (*((char *)data+(i*Col)+_FRAMES+1) & 0xFF));
                    if (last_band != c_band) {
								parse_error("Keeping Band: %d\n",c_band);

                        if (frame_count < Max_Band_Count) {/*Missing ending data*/

                            Add_Fake_Frame((((unsigned char *)newbuf)+new_count*Col),
									(int)c_band, frame_count, (Max_Band_Count-frame_count), Col);

									parse_error("Adding frames: %d-%d in Band: %d\n",
										frame_count,Max_Band_Count, c_band);

                            new_count+=(Max_Band_Count-frame_count)+1;
                        }
                        frame_count=0;
                        last_band=c_band;
                    }
                    if (c_frame > FrameBandStart[c_band]+frame_count){

                        Add_Fake_Frame((((unsigned char *)newbuf)+new_count*Col),
                            (int) c_band, FrameBandStart[c_band]+frame_count,
                            c_frame-(FrameBandStart[c_band]+frame_count), Col);

								parse_error("Adding frames: %d-%d in Band: %d\n",
									(FrameBandStart[c_band]+frame_count),
									(c_frame-1), c_band);

                        new_count+=c_frame-(FrameBandStart[c_band]+frame_count);
                        frame_count+=c_frame-(FrameBandStart[c_band]+frame_count);
                    }
                    else if (c_frame < FrameBandStart[c_band]+frame_count){
                        parse_error("Corrupted Frame Sequence...aborting\n");
                        return(NULL);
                    }

                    if ((new_count*Col+Col) >= len){
                        printf("Extending buffer; newcount=%d; i=%d\n",new_count,i);
                        len+=Col*(new_count-Lines);
                        if((newbuf=realloc(newbuf,len))==NULL){
                            printf("Couldn't extend buffer...you're hosed\n");
                            return(NULL);
                        }
                    }
				
                    memcpy((((unsigned char *)newbuf)+((new_count++)*Col)),
							(((unsigned char *)data)+i*Col),
							Col);
                    frame_count++;
			
                }
            } 

#endif


