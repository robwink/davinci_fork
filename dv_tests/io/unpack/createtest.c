#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

typedef uint8_t u8;

int main(int argc, char** argv)
{
	//FILE* myfile = fopen("secondtest.dat","wb");
	
	FILE* myfile = fopen("test64bit.dat","wb");


	u8 buf[8];

	int tempint;
	short tempshort;
	float tempfloat;
	double tempdouble;
	unsigned int uint;
	unsigned short ushort;
	float ufloat;
	double udouble;
	char *blah = "blah";
	
	u8* mybyte;
	u8 tempbyte;

	int i,j;
	for (i=0; i<10; i++) {
		if (i < 9) {
			// MSB u8
			buf[0] = i;
			fwrite(buf,1,1,myfile);

			// LSB short
			ushort = i;
			fwrite(&ushort,2,1,myfile);

			// LSB 3 u8 int
			tempint = -i;
			mybyte = (u8*)&tempint;
			buf[0] = mybyte[0];
			buf[1] = mybyte[1];
			buf[2] = mybyte[2];
			fwrite(buf,1,3,myfile);

			// LSB int
			uint = i;
			fwrite(&uint,4,1,myfile);

			// LSB float
			tempfloat = -i + 0.5;
			fwrite(&tempfloat,4,1,myfile);
			
			// LSB double
			tempdouble = -i + 0.75;
			fwrite(&tempdouble,8,1,myfile);

			// to test ignore
			fwrite(blah, 4, 1, myfile);

			// MSB short
			tempshort = (short)(i);
			mybyte = (u8*)&tempshort;
			tempbyte = mybyte[0];
			mybyte[0] = mybyte[1];
			mybyte[1] = tempbyte;		
			fwrite(&tempshort,2,1,myfile);

			// MSB 3 byte int
			tempint = (int)i;
			mybyte = (u8*)&tempint;
			buf[0] = mybyte[2];
			buf[1] = mybyte[1];
			buf[2] = mybyte[0];
			fwrite(buf,1,3,myfile);

			// MSB int
			uint = i;
			mybyte = (u8*)&uint;
			for(j=0;j<2;j++) {
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&uint,4,1,myfile);

			// MSB float
			tempfloat = -i + 0.5;
			mybyte = (u8*)&tempfloat;
			for(j=0;j<2;j++)
			{
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&tempfloat,4,1,myfile);
			
			// MSB double
			tempdouble = -i + 0.75;
			mybyte = (u8*)&tempdouble;
			for(j=0;j<4;j++)
			{
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[7-j];
				mybyte[7-j] = tempbyte;
			}
			fwrite(&tempdouble,8,1,myfile);
		}
		else {
			// MSB u8
			buf[0] = -i;
			fwrite(buf,1,1,myfile);

			// LSB short
			ushort = USHRT_MAX - 100;
			fwrite(&ushort,2,1,myfile);


			// LSB 3 u8 int
			tempint = SHRT_MAX + 1;
			mybyte = (u8*)&tempint;
			buf[0] = mybyte[0];
			buf[1] = mybyte[1];
			buf[2] = mybyte[2];
			fwrite(buf,1,3,myfile);

			// LSB int
			uint = UINT_MAX - 100;
			printf("\n%u\n", uint);
			fwrite(&uint,4,1,myfile);

			// LSB float
			tempfloat = -i + 0.5;
			fwrite(&tempfloat,4,1,myfile);
			
			// LSB double
			tempdouble = -i + 0.75;
			fwrite(&tempdouble,8,1,myfile);

			//to test ignore
			fwrite(blah, 4, 1, myfile);

			//MSB short
			ushort = USHRT_MAX;
			mybyte = (u8*)&ushort;
			tempbyte = mybyte[0];
			mybyte[0] = mybyte[1];
			mybyte[1] = tempbyte;
			fwrite(&ushort,2,1,myfile);


			// MSB 3 u8 int
			uint = USHRT_MAX;
			mybyte = (u8*)&uint;
			buf[0] = mybyte[2];
			buf[1] = mybyte[1];
			buf[2] = mybyte[0];
			fwrite(buf,1,3,myfile);

			// MSB int
			uint = INT_MAX + 100;
			printf("%u\n", uint);
			mybyte = (u8*)&uint;
			for(j=0;j<2;j++)
			{
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&uint,4,1,myfile);

			// MSB float
			tempfloat = -i + 0.5;
			mybyte = (u8*)&tempfloat;
			for(j=0;j<2;j++)
			{
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&tempfloat,4,1,myfile);
			
			// MSB double
			tempdouble = -i + 0.75;
			mybyte = (u8*)&tempdouble;
			for (j=0;j<4;j++) {
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[7-j];
				mybyte[7-j] = tempbyte;
			}
			fwrite(&tempdouble,8,1,myfile);
		}

		buf[0] = 'h'; buf[1] = 'e'; buf[2] = 'l'; buf[3] = 'l'; buf[4] = 'o';
		fwrite(buf,5,1,myfile);

/*		for(j=0; j<3; j++) {
			fwrite(buf,5,1,myfile);
			buf[0] = '0' + j;
			fwrite(buf, 1, 1, myfile);
			buf[0] = 'h';
		} */
	}



	return 0;
}
