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

	printf("%lu\n", num);
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
	//FILE* myfile = fopen("secondtest_2.dat","wb");

	//rowlen = 124
	//unpack("U1U3U5U6U7U8u1u3u5u6u7u8x4I1I3I5I6I7I8i1i3i5i6i7i8", "test64bit.dat", 0)
	FILE* myfile = fopen("test64bit.dat","wb");


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

	i32 i,neg_i;
	for (i=0; i<10; i++) {
		neg_i = -i;
		if (i < 9) {
			write_msb_uint(i, 1, myfile);
			write_msb_uint(i, 3, myfile);
			write_msb_uint(i, 5, myfile);
			write_msb_uint(i, 6, myfile);
			write_msb_uint(i, 7, myfile);
			write_msb_uint(i, 8, myfile);

			write_lsb_uint(i, 1, myfile);
			write_lsb_uint(i, 3, myfile);
			write_lsb_uint(i, 5, myfile);
			write_lsb_uint(i, 6, myfile);
			write_lsb_uint(i, 7, myfile);
			write_lsb_uint(i, 8, myfile);

			fwrite(blah, 4, 1, myfile);

			write_msb_int(neg_i, 1, myfile);
			write_msb_int(neg_i, 3, myfile);
			write_msb_int(neg_i, 5, myfile);
			write_msb_int(neg_i, 6, myfile);
			write_msb_int(neg_i, 7, myfile);
			write_msb_int(neg_i, 8, myfile);

			write_lsb_int(neg_i, 1, myfile);
			write_lsb_int(neg_i, 3, myfile);
			write_lsb_int(neg_i, 5, myfile);
			write_lsb_int(neg_i, 6, myfile);
			write_lsb_int(neg_i, 7, myfile);
			write_lsb_int(neg_i, 8, myfile);
		} else {
			write_msb_uint(255, 1, myfile);

			// 16777215
			write_msb_uint(UINT16_MAX << 8 | 0xff, 3, myfile);
			// 1099511627775
			write_msb_uint((u64)UINT32_MAX << 8 | 0xff, 5, myfile);
			// 281474976710655
			write_msb_uint((u64)UINT32_MAX << 16 | 0xffff, 6, myfile);
			// 72057594037927935
			write_msb_uint((u64)UINT32_MAX << 24 | 0xffffff, 7, myfile);
			// 18446744073709551615
			write_msb_uint(UINT64_MAX, 8, myfile);

			write_lsb_uint(255, 1, myfile);

			write_lsb_uint(UINT16_MAX << 8 | 0xff, 3, myfile);

			/*
			printf("%lu\n", (u64)UINT32_MAX << 8 | 0xff);
			printf("%lu\n", (u64)UINT32_MAX << 16 | 0xffff);
			printf("%lu\n", (u64)UINT32_MAX << 24 | 0xffffff);
			*/

			write_lsb_uint((u64)UINT32_MAX << 8 | 0xff, 5, myfile);
			write_lsb_uint((u64)UINT32_MAX << 16 | 0xffff, 6, myfile);
			write_lsb_uint((u64)UINT32_MAX << 24 | 0xffffff, 7, myfile);
			write_lsb_uint(UINT64_MAX, 8, myfile);

			fwrite(blah, 4, 1, myfile);

			write_msb_int(-128, 1, myfile);

			// Because C does integer operations in plain int by default which is 32 bit
			// I'd have to cast (i64)1 to prevent overflow.  Easier to just use a i64 variable
			i64 o = 1;

			// -8,388,608
			write_msb_int(o << 23, 3, myfile);
			// -549,755,813,888
			write_msb_int(o << 39, 5, myfile);
			// -140,737,488,355,328
			write_msb_int(o << 47, 6, myfile);
			// -36028797018963968
			write_msb_int(o << 55, 7, myfile);
			// -9223372036854775808 (2^63 in traditional 2's complement but spec says
			// just has to be <= -2^63 + 1
			write_msb_int(INT64_MIN, 8, myfile);

			write_lsb_int(-128, 1, myfile);
			write_lsb_int(o << 23, 3, myfile);
			write_lsb_int(o << 39, 5, myfile);
			write_lsb_int(o << 47, 6, myfile);
			write_lsb_int(o << 55, 7, myfile);
			write_lsb_int(INT64_MIN, 8, myfile);
		}
	}



	return 0;
}
