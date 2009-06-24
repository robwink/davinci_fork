#include "parser.h"
#define intswap(i) (i) = ((((i) >> 24) & 0xFF) | (((i) >> 8) & 0xFF00) | \
	       (((i) << 8) &0xFF0000) | (((i) << 24) & 0xFF000000))
char * flip_endian(unsigned char *, unsigned int, unsigned int);
char * var_endian(Var *);

void MSB(unsigned char * data, unsigned int data_elem, unsigned int word_size);
void LSB(unsigned char * data, unsigned int data_elem, unsigned int word_size);
