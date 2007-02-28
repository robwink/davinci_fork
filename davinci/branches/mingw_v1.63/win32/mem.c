/*
**
** CAUTION: As yet the code in this file has been tested for mapping
** files for reading purpose only.
**
**
** This file includes the wrappers for memory management routines.
*/

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32

#include <windows.h>
#include "dos.h"

void *
mmap(
	void *desired_addr,
	size_t len, 
	int mmap_prot, 
	int mmap_flags, 
	HANDLE fd,
	size_t off
)
{
	HANDLE fmh;
	void *base_addr;
	SECURITY_ATTRIBUTES sa;
	DWORD prot;
	
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	
	prot = 0;
	if (mmap_prot & PROT_WRITE){
		if (mmap_flags & MAP_PRIVATE){ prot = PAGE_WRITECOPY; }
		else { prot = PAGE_READWRITE; }
	}
	else if (mmap_prot & PROT_READ){ prot = PAGE_READONLY; }

	if (mmap_prot & PROT_EXEC){ prot |= SEC_IMAGE; }
	if (mmap_flags & MAP_NORESERVE){ prot |= SEC_NOCACHE; }
	/* MAP_FIXED is unhandled */

	fmh = CreateFileMapping(fd, &sa, prot, 0, len, NULL);
	if (fmh == NULL){ return NULL; }

	prot = 0;
	if (mmap_prot & PROT_WRITE){
		if (mmap_flags & MAP_PRIVATE){ prot |= FILE_MAP_COPY; }
		else { prot |= FILE_MAP_WRITE; }
	}
	if (mmap_prot & PROT_READ){
		prot |= FILE_MAP_READ;
	}

	if (mmap_prot & PROT_EXEC && mmap_flags & MAP_PRIVATE){ prot |= FILE_MAP_COPY; }

	base_addr = MapViewOfFileEx(fmh, prot, 0, off, len, desired_addr);

	/* I hope that Windows keeps its own copy */
	CloseHandle(fmh);

	return base_addr;
}

void
munmap(
	void *base_addr,
	size_t len
)
{
	UnmapViewOfFile(base_addr);
}


void *
MemMapFile(
	void   *desired_addr,
	size_t  length,
	int     mmap_prot,
	int     mmap_flags,
	char   *file_name,
	int		open_flags,
	off_t   file_offset
)
{
	DWORD   file_access = 0;
	DWORD	file_share = 0;
	DWORD   file_create_how = 0;
	DWORD   file_attrib = 0;
	LPSECURITY_ATTRIBUTES file_sec = NULL;
	void   *addr = NULL;
	struct  stat sbuf;
	HANDLE  fd;


	/* decode the file access required */
	if (open_flags & (O_CREAT|O_EXCL) == (O_CREAT|O_EXCL)){
		if (stat(file_name, &sbuf) == 0){
			errno = EEXIST;
			return NULL;
		}
		file_access = 0;
	}
	else {
		file_access = 0;
		if (open_flags & O_RDWR){ file_access = GENERIC_READ | GENERIC_WRITE; }
		else if (open_flags & O_WRONLY){ file_access = GENERIC_WRITE; }
		else { /* open_flags & O_RDONLY */ file_access = GENERIC_READ; }
	}

	/* decode the share mode */
	file_share = FILE_SHARE_READ | FILE_SHARE_WRITE;

	/* set the security attirbutes */
	file_sec = NULL;

	/* set the file creation disposition */
	file_create_how = OPEN_EXISTING;
	if (open_flags & O_CREAT){ file_create_how = OPEN_ALWAYS; }
	if (open_flags & O_TRUNC){ file_create_how |= TRUNCATE_EXISTING; }

	/* set the file flags and attirbutes */
	file_attrib = FILE_ATTRIBUTE_NORMAL;


#if 0
	/* believe it or not, this actually opens the file - it does not create it */
	fd = CreateFile(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

	fd = CreateFile(file_name, file_access, file_share, file_sec, file_create_how, file_attrib, NULL);
	if (fd == INVALID_HANDLE_VALUE){
		/*
		fprintf(stderr, "MemMapFile: Open %s failed. ", (fname?fname:"(null)"));
		perror("Reason");
		*/
		return NULL;
	}
	addr = mmap(desired_addr, length, mmap_prot, mmap_flags, fd, file_offset);
	CloseHandle(fd);

	return addr;
}

#else /* _WINDOWS */

#include <sys/mman.h>
#include <unistd.h>

void *
MemMapFile(
	void   *desired_addr,
	size_t  length,
	int     mmap_prot,
	int     mmap_flags,
	char   *file_name,
	int		open_flags,
	off_t   file_offset
)
{
	int     fd;
	void   *addr = NULL;

	fd = open(file_name, open_flags);
	if (fd < 0){
		/*
		fprintf(stderr, "MemMapFile: Open %s failed. ", (fname?fname:"(null)"));
		perror("Reason");
		*/
		return NULL;
	}

	addr = mmap(desired_addr, length, mmap_prot, mmap_flags, fd, file_offset);
	close(fd);

	return addr;
}
#endif /* _WINDOWS */

void
MemUnMapFile(
	void    *addr,     /* non-NULL address returned by MemMapFile */
	size_t   length    /* length as passed to MemMapFile */
)
{
	munmap(addr, length);
}

