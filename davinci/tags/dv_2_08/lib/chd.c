/***********************************************************************/
/*                                                                     */
/* This program segment was modified from:                             */
/*  chd.c  - an emacs style directory browser with filename completion */
/*  Marc S. Majka - UBC Laboratory for Computational Vision            */
/*                                                                     */
/***********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


int dn = 0;
char **dirs, *strrchr(), *strchr();

sort_dirs(buf)
char	*buf;
{
	DIR *cwd;
	struct dirent *dent;
	int i, j;
	char *p, c, d;
	char str[256];

/* append and remove the necessary /'s */

/* if doesn't start with "/", put . as directory */

	if (buf[0] != '/') {
		sprintf(str,"./%s",buf);
	} else {
		strcpy(str,buf);
	}

/* remove trailing filename (assuming there is one) */

	p = strrchr(str,'/');
	if (p) *p = '\0';
	if (str[0] == '\0') strcpy(str,"/");

/* Make sure path to here is valid */

	cwd = opendir(str);
	if (cwd == NULL) {
		/**
		 ** no such directory.  Try moving up 1 directory, to look for error 
		 **/
		strcpy(buf,str);
		return(sort_dirs(buf));
	}
	closedir(cwd);

/* read directory, save a list of directory entires. */

	dn = get_sorted_dir(str, &dirs);
	return(strlen(str));
}

get_sorted_dir(path, ds)
char *path;
char ***ds;
{
	DIR *cwd;
	struct dirent *dent;
	int size=64;
	char **dirs;
	int dn = 0;
	int i,j;

	cwd = opendir(path);
	if (cwd == NULL) return(-1);

	dirs = (char **)calloc(size,sizeof(char *));

	while ((dent = readdir(cwd)) != NULL) {
		if (!strcmp(dent->d_name,"..")) continue;
		if (!strcmp(dent->d_name,".")) continue;

		if (dn >= size-2) {
			size *= 2;
			dirs = (char **)realloc(dirs, size * sizeof(char *));
		}
		/* Insert directory name in sorted list */

		for (i = 0; i < dn && strcmp(dent->d_name, dirs[i]) > 0; i++)
			;
		for (j = dn; j >= i; j--) 
			dirs[j+1] = dirs[j];
		dirs[i] = (char *)calloc(1,strlen(dent->d_name) + 1);
		strcpy(dirs[i], dent->d_name);
		dn++;
	}
	closedir(cwd);

	dirs = (char **)realloc(dirs, dn * sizeof(char *));
	*ds = dirs;
	return(dn);
}

complete_match(str)
char *str;
{
	char *dir, *p, c;
	int i, j, done;
	int first, last;

	p = strrchr(str, '/');
	if (p == NULL) {
		dir = str;
	} else {
		dir = p+1;
	}

	last = dn-1;
	first =0;
	for (i = 1 ; i <= strlen(dir) ; i++) {
		while((first < last) && (strncmp(dir, dirs[first], i) > 0)) {
			first++;
		}
		while(last > first && (strncmp(dir, dirs[last], i) < 0)) {
			last--;
		}
	}

	if (last == first) {
		if (!strncmp(dir, dirs[last], strlen(dir))) {
			strcat(str, dirs[last]+strlen(dir));
			return(-1);
		} else {
			i = 0; 
			while((i < strlen(dir)) && (dir[i] == dirs[first][i])) i++;
			return(dir - str + i);
		}
	} else {
		i = strlen(dir);
		while(dirs[first][i] == dirs[last][i]) {
			dir[i] =  dirs[first][i];
			i++;
		}
		dir[i] = '\0';
		return(strlen(str));
	}
}



complete_dir(str)
char *str;
{
	int i,j;
	j = sort_dirs(str);
	if (j < 0) return(0);
	if (j == 0) return(-1);
	i = complete_match(str);

	for (j = 0; j < dn; j++) 
		free(dirs[j]);
	dn = 0;

	return(i);
}

/*

	i = complete_dir(buf);
	i = -1, good
	i >= 0, beep.
	i > 0, locate to that point 

*/
