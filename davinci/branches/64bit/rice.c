#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ushort unsigned short
#define MAX_BUF_LEN 4096

#define uchar  unsigned char

/*
** These treat the msb bit as bit #1
*/
#define set_bit(w,n)            (w) |= (1 << (7-n))
#define clear_bit(w,n)          (w) &= ~(1 << (7-n))
#define test_bit(w,n)           (((w) & (1 << (7-(n)))) != 0)

/* int test_bit(ushort w, int n) { return((((w) & (1 << (7-n))) != 0)); }  */

int rice_fs(ushort *in, int npts, uchar *out, int start);
void write_zeros(uchar *buf, int len, int x);
void write_bit(uchar *buf, int pos, int value);
int rice_pack(ushort *in, int npts, int bits, uchar *out, int start);
int delta_transform(short *in, int npts, ushort *out);
void delta_transform_inverse(ushort *in, int npts, short *out);


void
write_bit(uchar *buf, int pos, int value)
{
	int byte = pos/8;
	int index= pos%8;

	if (value) set_bit(buf[byte], index);
	else       clear_bit(buf[byte], index);
}

void write_bits(uchar *dst, int pos, ushort src, int len)
{
	while(len) {
		write_bit(dst, pos++, (src >> (len-1)) & 1);
		len--;
	}
}

ushort read_bit(uchar *src, int pos)
{
	return (test_bit(src[pos/8],pos%8));
}

ushort read_bits(uchar *src, int pos, int len)
{
	ushort dst = 0;
	while(len) {
		dst |= read_bit(src,pos++) << (--len);
	}
	return(dst);
}

void
write_zeros(uchar *buf, int len, int x)
{
	int i;
	for (i = 0 ; i < x ; i++) {
		write_bit(buf, len+i, 0);
	}

}

int
rice_auto(short *in, int npts, int bits, uchar *out, int start)
{
	int F0;
	int J = npts;
	int mode = -1;
	int len;
	ushort tmp[MAX_BUF_LEN];
	int hdr;
	int len_size;
	int mode_size;

	F0 = delta_transform(in, npts, tmp);

	if (bits <= 8) {
		hdr = 15;
		len_size = 12;
		mode_size = 3;
	} else {
		hdr = 17;
		len_size = 13;
		mode_size = 4;
	}

		 if (F0 <= 3*J/2)             mode = 0; 	// PSI0()
	else if (F0 <= 5*J/2)             mode = 1; 	// PSI1(0,)
	else if (F0 <= 9*J/2)             mode = 3; 	// PSI1(1,)
	else if (F0 <= 17*J/2)            mode = 4;		// PSI1(2,)
	else if (F0 <= 33*J/2)            mode = 5; 	// PSI1(3,)
	else if (F0 <= 65*J/2)            mode = 6; 	// PSI1(4,)
	else if (F0 <= 129*J/2)           mode = 7; 	// PSI1(5,)
	else if (bits <= 8) 			  mode = 2;		// 8-bit PSI3
	else if (F0 <= 257*J/2)           mode = 8; 	// PSI1(6,)
	else if (F0 <= 513*J/2)           mode = 9; 	// PSI1(7,)
	else if (F0 <= 1025*J/2)          mode = 10; 	// PSI1(8,)
	else if (F0 <= 2049*J/2)          mode = 11;	// PSI1(9,)
	else if (F0 <= 4097*J/2)          mode = 12;	// PSI1(10,)
	else if (F0 <= 8193*J/2)          mode = 13;	// PSI1(11,)
	else if (F0 <= 16385*J/2)         mode = 14;	// PSI1(12,)
	else if (F0 <= 32769*J/2)         mode = 15;	// PSI1(13,)
	else                              mode =  2;    // 16-bit PSI3()

	switch (mode) {
		case 0: len = rice_code(tmp, npts, out, start+hdr); break;
		case 1: len = rice_fs(tmp, npts, out, start+hdr); break;
		case 2: len = rice_pack(tmp, npts, bits, out, start+hdr); break;
		default:
			len = rice_split(tmp, npts, mode-2, out, start+hdr); break;
	}

	len += hdr;
	write_bits(out, start,      len, len_size); start += len_size;
	write_bits(out, start,     mode, mode_size);start += mode_size;

	return(len);
}

/*
** returns number of bytes consumed 
*/
int rice_unauto(uchar *in, int len, int npts, int bits, short *out)
{
	int mode;
	int len1;
	int start = 0;
	char flag;
	int hdr;
	int len_size;
	int mode_size;

	if (bits <= 8) {
		hdr = 15;
		len_size = 12;
		mode_size = 3;
	} else {
		hdr = 17;
		len_size = 13;
		mode_size = 4;
	}
/*
	flag =    read_bits(in, start, 8); start += 8;
	if (flag != 'r') {
		fprintf(stderr, "No rice header\n");
		exit(1);
	}
*/
	len1 = read_bits(in, start, len_size); start += len_size;
	mode = read_bits(in, start, mode_size); start += mode_size;

	/* adjust for header */
	len1 -= hdr;

	switch(mode) {
		case 0: rice_uncode(in, len1, out, start); break;
		case 1: rice_unfs(in, len1, out, start); break;
		case 2: {
			bits = len1 / npts;
			rice_unpack(in, len1, bits, out, start);
			break;
		}
		default: {
			rice_unsplit(in, len1, npts, mode-2, out, start);
			break;
		}
	}
	delta_transform_inverse(out, npts, out);
	return((len1+hdr+7)/8);
}

/* 
** 
*/

int
rice_split(short *in, int npts, int entropy, uchar *out, int start)
{
	int i;
	ushort top[MAX_BUF_LEN];
	ushort bot[MAX_BUF_LEN];
	int len1, len2;


	/*
	** Split things at the entropy bits, and figure out how many bits
	** we really need for the top half, just in case the user lied.
	*/
	for (i = 0 ; i < npts ; i++) {
		top[i] = (in[i] >> entropy);
		bot[i] =  in[i] & ((1 << entropy) -1);
	}

	len1 = rice_pack(bot, npts, entropy, out, start);
	len2 = rice_fs(  top, npts, out,          start+len1);

	// printf("rice_split(%d) = %d\n", entropy, len1+len2);
	return(len1+len2);
}

int
rice_unsplit(uchar *in, int len, int npts, int entropy, short *out, int start) 
{
	int i;
	short top[MAX_BUF_LEN];
	int len1;
	int n1, n2;
	int hdr = 2+4+13;

	len1 = npts*entropy;

	n1 = rice_unpack(in, len1, entropy, out, start);
	n2 = rice_unfs  (in, len-len1,      top, start+len1);

	if (n1 != n2 || n1 != npts) {
		fprintf(stderr, "rice_split_sample: npts doesn't match between parts, %d,%d\n", n1, n2);
		return(0);
	}
	for (i = 0 ; i < n2 ; i++) {
		out[i] |= (top[i] << entropy);
	}

	return(n2);
}

int
rice_split_size(uchar *in)
{
	int start = 0; 						/* number of bits used */
	int len, len1, len2;

	start = 0;
	len1 = read_bits(in, start+6, 13);
	len2 = read_bits(in, start+len1+6, 13);

	return(len1+len2);
}

/**
*** Compute delta transform to make everything >= 0
**/
int
delta_transform(short *in, int npts, ushort *out)
{
	int i, sum = 0;
	for (i = 0 ; i < npts ; i++) {
		if (in[i] < 0) out[i] = -in[i]*2-1;
		else out[i] = in[i]*2;
		sum += out[i]+1;
	}
	return(sum);
}

/**
*** Reverse the delta transform to make everything >= 0
**/
void
delta_transform_inverse(ushort *in, int npts, short *out)
{
	int i, sum = 0;
	for (i = 0 ; i < npts ; i++) {
		if (in[i] & 1) out[i] = -(in[i]+1)/2;
		else out[i] = in[i]/2;
	}
}


int
rice_pack(ushort *in, int npts, int bits, uchar *out, int start)
{
	int i,j;
	int len=0;
	for (i = 0 ; i < npts ; i++) {
		for (j = bits-1 ; j >= 0 ; j--) {
			write_bit(out, start + len++, test_bit(in[i], 7-j));
		}
	}
	// printf("rice_pack(%d) = %d\n", bits, len);
	return(len);
}

int
rice_unpack(uchar *in, int len, int bits, ushort *out, int start)
{
	int i, count = 0;

	for (i = 0 ; i < len ; i+= bits) {
		out[count++] = read_bits(in, i+start, bits);
	}
	return(count);
}

/*
** Convert words to fundamental sequence
**
** returns total size of buffer
*/
int
rice_fs(ushort *in, int npts, uchar *out, int start)
{
	int i;
	int len=0;
	int x;

	for (i = 0 ; i < npts ; i++) {
		x = in[i];
		write_zeros(out, start+len, x);
		write_bit(out, start+len+x, 1);
		len += x+1;
	}

	// printf("rice_fs() = %d\n", len);
	return(len);
}

int
rice_unfs(uchar *in, int len, ushort *out, int start)
{
	int rem = 0, count = 0;
	int i;

	for (i = 0 ; i < len ; i++) {
		if (read_bit(in,start+i) == 0) {
			rem++;
		} else {
			out[count++] = rem;
			rem = 0;
		}
	}
	return(count);
}


uchar rice_codes[] = { 1, 1, 2, 0, 3, 1, 2, 3 };
uchar rice_lens[] =  { 1, 3, 3, 5, 3, 5, 5, 5 };

/*
** Two pass method.  Slow & silly, but correct.
**
** returns total length of buffer (including start offset)
*/
int
rice_code(ushort *in, int npts, uchar *out, int start)
{
	int i;
	int len=0;
	int len2=0;
	int x;
	uchar out2[MAX_BUF_LEN];

	len = rice_fs(in, npts, out2, 0);

    for (i = 0 ; i < len ; i++) {
		/*
		** extract 3 bits and invert them.
		*/
        x = (1-test_bit(out2[i/8],i%8)) << 2; i++;
        x |= (i < len ? (1-test_bit(out2[i/8],i%8)) : 1) << 1; i++;
        x |= (i < len ? (1-test_bit(out2[i/8],i%8)) : 1);

        write_bits(out, start+len2, rice_codes[x], rice_lens[x]);
        len2 += rice_lens[x];
    }
    return(len2);
}

int
rice_uncode(uchar *in, int len, ushort *out, int start)
{
	int i;
	int pos=0;
	int b1, b2, b3;
	int x;
	uchar out2[MAX_BUF_LEN];
	int len2;

	i = 0;
	while (i < len) {
		b1 = read_bit(in,i+start); i++;
		if (b1 == 1) {
			write_bits(out2, pos, ~0x0000, 3); pos += 3;
		} else {
			b2 = read_bits(in, i+start, 2); i+=2;
			switch(b2) {
				case 3: write_bits(out2, pos, ~0x0004, 3); pos += 3; break;
				case 2: write_bits(out2, pos, ~0x0002, 3); pos += 3; break;
				case 1: write_bits(out2, pos, ~0x0001, 3); pos += 3; break;
				case 0: {
					b3 = read_bits(in, i+start, 2); i += 2;
					switch(b3) {
						case 0: write_bits(out2, pos, ~0x0003, 3); pos += 3; break;
						case 1: write_bits(out2, pos, ~0x0005, 3); pos += 3; break;
						case 2: write_bits(out2, pos, ~0x0006, 3); pos += 3; break;
						case 3: write_bits(out2, pos, ~0x0007, 3); pos += 3; break;
					}
				}
				break;
			}
		}
	}
	return(rice_unfs(out2, pos, out, 0));
}

int same(ushort *a, ushort *b, int na, int nb, char *msg)
{
	int i;
	int bad = 0;

	if (na != nb) {
		fprintf(stderr, "%s: number of points doesn't match: %d != %d\n", 
			msg, na, nb);
		return(0);
	}

	for (i = 1 ; i < na ; i++) {
		if (a[i] != b[i]) {
			fprintf(stderr, "%s: sample %d doesn't match: %d != %d\n", 
				msg, i, a[i], b[i]);
			bad++;
		}
	}
	return(bad == 0);
}

void
test_rice_fs(ushort *in, int npts)
{
	/*
	** Test rice fundamental sequence
	*/
	int i;
	uchar out[MAX_BUF_LEN];
	ushort out2[MAX_BUF_LEN];

	int len = rice_fs(in, npts, out, 70);
	int n = rice_unfs(out, len, out2, 70);

	if (same(in, out2, n, npts, "rice_fs")) {
		printf("rice_fs: %d pts (%d bits): passed\n", npts, len);
	}
}

void
test_rice_code(ushort *in, int npts)
{
	/*
	** Test rice code
	*/
	int i;
	uchar out[MAX_BUF_LEN];
	ushort out2[MAX_BUF_LEN];

	int len = rice_code(in, npts, out, 70);
	int n = rice_uncode(out, len, out2, 70);

	if (same(in, out2, n, npts, "rice_code")) {
		printf("rice_code: %d pts (%d bits): passed\n", npts, len);
	}
}

void
test_rice_pack(short *in, int npts, int bits)
{
	/*
	** Test rice pack
	*/
	int i;
	uchar out[MAX_BUF_LEN];
	short out2[MAX_BUF_LEN];

	int len = rice_pack(in, npts, bits, out, 7);
	int n = rice_unpack(out, len, bits, out2, 7);

	if (same(in, out2, n, npts, "rice_pack")) {
		printf("rice_pack: %d pts (%d bits): passed\n", npts, len);
	}
}

void
test_rice_split(short *in, int npts, int entropy, int bits)
{
	/*
	** Test rice pack
	*/
	int i;
	uchar out[MAX_BUF_LEN];
	short out2[MAX_BUF_LEN];

	int len = rice_split(in, npts, entropy, out, 0);
	int n = rice_unsplit(out, len, npts, entropy, out2, 0);

	if (same(in, out2, n, npts, "rice_split")) {
		printf("rice_split: %d pts (%d bits): passed\n", npts, len);
	}
}

void
test_rice_auto(short *in, int npts, int bits)
{
	/*
	** Test rice pack
	*/
	int i;
	uchar out[MAX_BUF_LEN];
	short out2[MAX_BUF_LEN];

	int len = rice_auto(in, npts, bits, out, 0);
	int n = rice_unauto(out, len, npts, bits, out2);

	if (same(in, out2, n, npts, "rice_auto")) {
		printf("rice_auto: %d pts (%d bits): passed\n", npts, len);
	}
}

test_main(int ac, char **av)
{
// short in[] = { 10,10,10,-10,10,10,10,10,1,10,1,10,20,1,10,20,0,1,20,20,20,1,0,1,0,0,0,0,3,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,4,0,1,0,0,7,0,0,0,1,0,10,0,1,0,2,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,0,0,2,0,0,0,1,2,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,2,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,1,0,1,0,0,1,0,0,0,1,0,2,0,0,0,1,2,0,0,3,0,0,0,0,0,0,1,0,0,1,1,0,2,0,0,0,1,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0,2,0,1,0,0,0,0,0,4,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,1,0,10,10,10,10,10,10,0,0,0,1,0,1,0,0,0,0,2,0,0,0,-10,10,20,10,10,20,20,0,10,0,1,0,1,0,0,1,0,10,0,0,10,0,21,10,10,0,10,10,1,19};


	short in[] = { 1,1,1,1,1,1,1};
	/*
	// ushort in[14] = { 0, 0, 0, 0, 0, 2, 0, 0, 2, -5, 0, 0, -1, 0 };
	// ushort in[] = { 0, 0, 0, 1, 0, 0, 0, 2, 0, 0 };
	// short in[] = { 5, 2, -2, -4, -3, 0, 1, 5, 6, 1, -2, -2, 1, 1, 19 };
	*/
	ushort in2[MAX_BUF_LEN];
	int entropy, bits;

	int n = sizeof(in)/2;

	if (ac != 3) {printf("usage: %s entropy bits\n", av[0]); exit(1); }

	entropy = atoi(av[1]);
	bits = atoi(av[2]);

	delta_transform(in, n, in2);
	test_rice_fs(in2, n);
	test_rice_code(in2, n);
	test_rice_pack(in2, n, bits);
	test_rice_split(in2, n, entropy, bits);

	test_rice_auto(in, n, bits);
}

