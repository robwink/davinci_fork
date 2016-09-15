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

void write_lsb_int(i64 num, i32 bytes, FILE* file)
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

void write_lsb_uint(u64 num, i32 bytes, FILE* file)
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

void write_lsb_float(float num, FILE* f)
{
	u8 tmp;
	u8* mybyte = (u8*)&num;
#ifdef WORDS_BIGENDIAN
	tmp = mybyte[0];
	mybyte[0] = mybyte[3];
	mybyte[3] = tmp;

	tmp = mybyte[1];
	mybyte[1] = mybyte[2];
	mybyte[2] = tmp;
#endif

	int ret = fwrite(&num, 4, 1, f);
	assert(ret);
}

void write_lsb_double(double num, FILE* f)
{
	u8 tmp;
	u8* mybyte = (u8*)&num;
#ifdef WORDS_BIGENDIAN
	for(int i=0; i<4; i++) {
		tmp = mybyte[i];
		mybyte[i] = mybyte[7-i];
		mybyte[7-i] = tmp;
	}
#endif

	int ret = fwrite(&num, 8, 1, f);
	assert(ret);
}

void write_msb_int(i64 num, i32 bytes, FILE* file)
{
	u8 buf[8];

	u8* mybyte = (u8*)&num;

	for (int i=0; i<bytes; ++i) {
#ifdef WORDS_BIGENDIAN
		buf[i] = mybyte[i];
#else
		buf[i] = mybyte[bytes-1-i];
#endif
	}
	int ret = fwrite(buf, bytes, 1, file);
	assert(ret);
}

void write_msb_uint(u64 num, i32 bytes, FILE* file)
{
	u8 buf[8];

	u8* mybyte = (u8*)&num;

	for (int i=0; i<bytes; ++i) {
#ifdef WORDS_BIGENDIAN
		buf[i] = mybyte[i];
#else
		buf[i] = mybyte[bytes-1-i];
#endif
	}
	int ret = fwrite(buf, bytes, 1, file);
	assert(ret);
}

void write_msb_float(float num, FILE* f)
{
	u8 tmp;
	u8* mybyte = (u8*)&num;
#ifndef WORDS_BIGENDIAN
	tmp = mybyte[0];
	mybyte[0] = mybyte[3];
	mybyte[3] = tmp;

	tmp = mybyte[1];
	mybyte[1] = mybyte[2];
	mybyte[2] = tmp;
#endif

	int ret = fwrite(&num, 4, 1, f);
	assert(ret);
}

void write_msb_double(double num, FILE* f)
{
	u8 tmp;
	u8* mybyte = (u8*)&num;
#ifndef WORDS_BIGENDIAN
	for(int i=0; i<4; i++) {
		tmp = mybyte[i];
		mybyte[i] = mybyte[7-i];
		mybyte[7-i] = tmp;
	}
#endif

	int ret = fwrite(&num, 8, 1, f);
	assert(ret);
}


int main(i32 argc, char** argv)
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

//#unpack("Iu2i3u4r4r8x4U2U3U4R4R8a5", "secondtest.dat", 0)
	i32 i,j;
	for (i=0; i<10; i++) {
		if (i < 9) {
			// MSB u8
			write_msb_int(i, 1, myfile);

			// LSB i16
			write_lsb_uint(i, 2, myfile);

			// LSB 3 byte int
			write_lsb_int(-i, 3, myfile);

			// LSB u32
			write_lsb_uint(i, 4, myfile);

			// LSB float
			write_lsb_float(-i+0.5, myfile);

			// LSB double
			write_lsb_double(-i+0.75, myfile);

			// to test ignore
			fwrite(blah, 4, 1, myfile);

			// MSB u16
			write_msb_uint(i, 2, myfile);

			// MSB 3 byte int
			write_msb_uint(i, 3, myfile);

			// MSB i32
			write_msb_uint(i, 4, myfile);

			// MSB float
			write_msb_float(-i + 0.5, myfile);

			// MSB double
			write_msb_double(-i+0.75, myfile);
		} else {
			// MSB i8
			write_msb_int(-i, 1, myfile);

			// LSB i16
			write_lsb_uint(USHRT_MAX-100, 2, myfile);

			// LSB 3 byte int
			write_lsb_int(SHRT_MAX+1, 3, myfile);

			// LSB u32
			write_lsb_uint(UINT_MAX-100, 4, myfile);

			// LSB float
			write_lsb_float(-i+0.5f, myfile);

			// LSB double
			write_lsb_double(-i + 0.75, myfile);

			//to test ignore
			fwrite(blah, 4, 1, myfile);

			//MSB u16
			write_msb_uint(USHRT_MAX, 2, myfile);

			// MSB 3 byte uint
			write_msb_uint(USHRT_MAX, 3, myfile);

			// MSB u32
			write_msb_uint((u64)INT_MAX+100, 4, myfile);

			// MSB float
			write_msb_float(-i + 0.5f, myfile);

			// MSB double
			write_msb_double(-i + 0.75, myfile);
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
