#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

void write_int(i64 num, i32 bytes, FILE* file)
{
	u8 buf[8];

	u8* mybyte = (u8*)&num;

	for (int i=0; i<bytes; ++i) {
#ifdef WORDS_BIGENDIAN
		buf[i] = mybyte[bytes-1-i];
#else
		buf[i] = mybyte[i];
#endif
	}
	int ret = fwrite(buf, bytes, 1, file);
	assert(ret);
}

void write_uint(u64 num, i32 bytes, FILE* file)
{
	u8 buf[8];

	u8* mybyte = (u8*)&num;

	for (int i=0; i<bytes; ++i) {
#ifdef WORDS_BIGENDIAN
		buf[i] = mybyte[bytes-1-i];
#else
		buf[i] = mybyte[i];
#endif
	}
	int ret = fwrite(buf, bytes, 1, file);
	assert(ret);
}


i32 main(i32 argc, char** argv)
{
	FILE* myfile = fopen("secondtest_2.dat","wb");

	//FILE* myfile = fopen("test64bit.dat","wb");


	u8 buf[8];

	i32 tempi32;
	i16 tempshort;
	u32 tmp_u32;
	u16 tmp_u16;

	float tempfloat;
	double tempdouble;

	char *blah = "blah";

	u8* mybyte;
	u8 tempbyte;

	i32 i,j;
	for (i=0; i<10; i++) {
		if (i < 9) {
			// MSB u8
			buf[0] = i;
			fwrite(buf,1,1,myfile);

			// LSB i16
			tmp_u16 = i;
			fwrite(&tmp_u16,2,1,myfile);

			// LSB 3 byte int
			tempi32 = -i;
			mybyte = (u8*)&tempi32;
			buf[0] = mybyte[0];
			buf[1] = mybyte[1];
			buf[2] = mybyte[2];
			fwrite(buf,1,3,myfile);

			// LSB i32
			tmp_u32 = i;
			fwrite(&tmp_u32,4,1,myfile);

			// LSB float
			tempfloat = -i + 0.5;
			fwrite(&tempfloat,4,1,myfile);

			// LSB double
			tempdouble = -i + 0.75;
			fwrite(&tempdouble,8,1,myfile);

			// to test ignore
			fwrite(blah, 4, 1, myfile);

			// MSB i16
			tempshort = (i16)(i);
			mybyte = (u8*)&tempshort;
			tempbyte = mybyte[0];
			mybyte[0] = mybyte[1];
			mybyte[1] = tempbyte;
			fwrite(&tempshort,2,1,myfile);

			// MSB 3 byte int
			tempi32 = (i32)i;
			mybyte = (u8*)&tempi32;
			buf[0] = mybyte[2];
			buf[1] = mybyte[1];
			buf[2] = mybyte[0];
			fwrite(buf,1,3,myfile);

			// MSB i32
			tmp_u32 = i;
			mybyte = (u8*)&tmp_u32;
			for(j=0;j<2;j++) {
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&tmp_u32,4,1,myfile);

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
		} else {
			// MSB u8
			buf[0] = -i;
			fwrite(buf,1,1,myfile);

			// LSB i16
			tmp_u16 = USHRT_MAX - 100;
			fwrite(&tmp_u16,2,1,myfile);


			// LSB 3 byte i32
			tempi32 = SHRT_MAX + 1;
			mybyte = (u8*)&tempi32;
			buf[0] = mybyte[0];
			buf[1] = mybyte[1];
			buf[2] = mybyte[2];
			fwrite(buf,1,3,myfile);

			// LSB i32
			tmp_u32 = UINT_MAX - 100;
			printf("\n%u\n", tmp_u32);
			fwrite(&tmp_u32,4,1,myfile);

			// LSB float
			tempfloat = -i + 0.5;
			fwrite(&tempfloat,4,1,myfile);

			// LSB double
			tempdouble = -i + 0.75;
			fwrite(&tempdouble,8,1,myfile);

			//to test ignore
			fwrite(blah, 4, 1, myfile);

			//MSB i16
			tmp_u16 = USHRT_MAX;
			mybyte = (u8*)&tmp_u16;
			tempbyte = mybyte[0];
			mybyte[0] = mybyte[1];
			mybyte[1] = tempbyte;
			fwrite(&tmp_u16,2,1,myfile);


			// MSB 3 byte i32
			tmp_u32 = USHRT_MAX;
			mybyte = (u8*)&tmp_u32;
			buf[0] = mybyte[2];
			buf[1] = mybyte[1];
			buf[2] = mybyte[0];
			fwrite(buf,1,3,myfile);

			// MSB i32
			tmp_u32 = (u32)INT_MAX + 100;
			printf("%u\n", tmp_u32);
			mybyte = (u8*)&tmp_u32;
			for(j=0;j<2;j++)
			{
				tempbyte = mybyte[j];
				mybyte[j] = mybyte[3-j];
				mybyte[3-j] = tempbyte;
			}
			fwrite(&tmp_u32,4,1,myfile);

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
