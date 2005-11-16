/*
** This file contains headers of memory management wrappers.
*/

#ifndef _MEM_H_
#define _MEM_H_

#include <stddef.h>
#include <sys/types.h>

/*
** Following two functions map a file into current process's memory space.
** These functions hide the difference between the Windows/UNIX implementation
** of memory mapping as well as provide a mechanism to separate out the Windows
** name-space from the main-stream code.
*/

void *
MemMapFile(
	void   *desired_addr, /* mmap-"addr" parameter: desired addr of mapped mem */
	size_t  length,       /* mmap-"len" parameter : memory map size in bytes */
	int     mmap_prot,    /* mmap-"prot" parameter */
	int     mmap_flags,   /* mmap-"flags" parameter */
	char   *file_name,    /* name of the file to map */
	int		open_flags,   /* open-"flags" parameter */
	off_t   file_offset   /* mmap-"off" parameter */
);

void
MemUnMapFile(
	void    *addr,        /* non-NULL address returned by MemMapFile */
	size_t   length       /* length as passed to MemMapFile */
);

#endif /* _MEM_H_ */

