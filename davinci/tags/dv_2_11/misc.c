#include "parser.h"
/**
 ** Split a buffer into individual words, at any whitespace or separator
 **/

void
split_string(char *buf, int *argc, char ***argv, char *s)
{
    char sep[256] = " \t\n\r";
    int size = 16;
    int count = 0;
    char *ptr = buf;
    char *p;
 
    if (s) strcat(sep, s);

    *argv = (char **)calloc(size, sizeof(char *));
    while((p = strtok(ptr, sep)) != NULL) {
        (*argv)[count++] = p;
        ptr = NULL;
        if (count == size) {
            size *= 2;
            *argv = (char **)my_realloc(*argv, size*sizeof(char *));
        }
    }
    *argc = count;
}

char *
uppercase(char *s)
{
  char *p;
  for (p = s ; p && *p ; p++) {
    if (islower(*p)) *p = *p - 'a' + 'A';
  }
  return(s);
}

char *
lowercase(char *s)
{
  char *p;
  for (p = s ; p && *p ; p++) {
    if (isupper(*p)) *p = *p - 'A' + 'a';
  }
  return(s);
}

char *
ltrim(char *s, char *trim_chars)
{
	int st = 0;
	while(s[st] && strchr(trim_chars, s[st]))
		st++;
	
	if (st > 0)
		strcpy(s, &s[st]);

	return s;
}

char *
rtrim(char *s, char *trim_chars)
{
	int len = strlen(s);

	while(len > 0 && strchr(trim_chars, s[len-1]))
		s[--len] = '\0';
	
	return s;
}

char *
fix_name(const char *input_name)
{
  const char invalid_pfx[] = "__invalid";
  static int invalid_id = 0;
  char *name;
  int len;
  int i;
  int val;
  const char *trim_chars = "\"\t ";

  name = (char *)calloc(1, strlen(input_name)+2);
  strcpy(name, input_name);

  ltrim(name, trim_chars);
  rtrim(name, trim_chars);

  len =  strlen(name);
  if (len < 1){
    name = (char *)calloc(strlen(invalid_pfx)+12, sizeof(char));
    return (sprintf(name, "%s_%d", invalid_pfx, ++invalid_id));
  }

  for(i=0; i<len; i++){
	name[i] = isalnum(name[i])? tolower(name[i]): '_';
  }

  if (isdigit(name[0])){
	for(i=len; i>=0; i--){
		name[i+1] = name[i];
	}
	name[0] = '_';
  }

  return (name);
}

