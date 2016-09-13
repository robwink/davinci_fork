#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef unsigned char byte;

int main()
{
	FILE* myfile = fopen("tinytest.txt","wb");

	byte buffer[1] = { 100 };
	fwrite(buffer, 1, 1, myfile);

	fclose(myfile);
	return 0;
}
