#include <stdio.h>
#include <sys/types.h>
#include <malloc.h>
#include <io.h>

void *mmap(void *x0, size_t len, int x1, int x2, int fd, size_t off)
{
	char *buf;

    lseek(fd, off, SEEK_SET);
	if ((buf = (char *)malloc(len)) == NULL) {
		fprintf(stderr, "Unable to allocate memory for mmap.\n");
		exit(1);
	}
	read(fd, buf, len);
	return(buf);
}

void munmap(void *buf,int len)
{
	free(buf);
}
