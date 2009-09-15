#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "isis_obj.h"

static ISISKwdPtr get_keyword_value_pair (FILE *);

#define ISIS_BUFSIZE 1024

static int is_isis(FILE * fh) {
  /* determine whether the file is truly an ISIS file */
  long save_pos;
  char record_buf[ISIS_BUFSIZE];
  int rtn = 0;

  save_pos = ftell(fh);
  rewind(fh);
  if (fgets(record_buf, ISIS_BUFSIZE, fh) == NULL) goto err_exit;
  if (strstr(record_buf, "= SFDU_LABEL\r\n") == NULL) goto err_exit;
  if (strncmp(record_buf, "CCSD3Z", 6)) goto err_exit;
  rtn = 1;

 err_exit:
  fseek(fh, save_pos, SEEK_SET);
  return rtn;
}


static ISISObjPtr parse_isis_file(FILE * fh) {
  ISISKwdPtr currentKwd = NULL, prevKwd = NULL, prevNonKwd = NULL;

  if (!is_isis(fh)) return NULL;
  while (1) {
    currentKwd = get_keyword_value_pair(fh);
    if (currentKwd == NULL) break;
    if (currentKwd->keyword == NULL) {
      if (prevNonKwd != NULL) {
	currentKwd->misc_seq = (prevNonKwd->misc_seq) + 1;
      }
      else {
	currentKwd->misc_seq = 1;
      }
      prevNonKwd = currentKwd;
    }


    prevKwd = currentKwd;
  }
  return NULL;
}

static ISISKwdPtr get_keyword_value_pair (FILE * fh) {
  char record_buf[ISIS_BUFSIZE];
  char * kwd = NULL;
  char * value = NULL;
  char * tester = NULL, * editptr = NULL, * endstr = NULL;
  char * begquote = NULL, * begparen = NULL, termchar;
  ISISKwdPtr rtn_kwd = NULL;

  if (fgets(record_buf, ISIS_BUFSIZE, fh) == NULL) goto err_exit;
  if (record_buf[strlen(record_buf) - 1] != '\n') {
    fprintf(stderr, "ISIS Header Record exceeds buffer size!\n");
    goto err_exit;
  }

  /* algorithm:  find the equal sign to separate keyword value, analyze the
     value for open parens and quotes to see if we need to read follow on
     lines to get the complete value.

     Trivia:  Did you know that C positively SUCKS for handling arbitrary
     string data?
  */

  if ((tester = strchr(record_buf, '=')) == NULL) {
    /* no equals, which means it's a comment line or a spacer.  Just copy
       and set the kwd to NULL */

    if ((value = malloc(strlen(record_buf) + 1)) == NULL) {
      fprintf(stderr, "malloc fails in get_keyword_value_pair!\n");
      goto err_exit;
    }
    strcpy(value, record_buf);
    if ((rtn_kwd = malloc(sizeof(ISISKwd))) == NULL) {
      fprintf(stderr, "malloc fails in get_keyword_value_pair\n");
      goto err_exit;
    }
    rtn_kwd->keyword = NULL; /* null keyword means object is a filler */
    rtn_kwd->misc_seq = 0; /* and it's up to the caller to find the value here */
    rtn_kwd->value = value;
    goto good_exit;
  }

  /* found equal sign, which means we need to dig out the keyword/value */

  /* set trailing blanks to null char */
  for(editptr=(tester-1); (*editptr != ' ' && editptr >= record_buf);
      editptr--)
    *editptr = 0;

  if ((kwd = malloc(strlen(record_buf)+1)) == NULL) {
    fprintf(stderr, "malloc fails in get_keyword_value_pair!\n");
    goto err_exit;
  }

  strcpy(kwd, record_buf);

  for(editptr=(tester+1); (*editptr != ' ' && *editptr != 0); editptr++);
  /* editptr now points at the beginning of the value string, passing any
     and all blanks after the equal sign */
  if ((endstr = strpbrk(editptr, "\r\n")) != NULL) {
    *endstr = 0;
  }
  if ((value = malloc(strlen(editptr) + 1)) == NULL) {
    fprintf(stderr, "malloc fails in get_keyword_value_pair!\n");
    goto err_exit;
  }
  strcpy(value, editptr);
  /* all (or at least the beginning of value) is pointed to by value now.
     We need to check for open parens and quotes to see if the value continues
     on other lines. */
  if ((rtn_kwd = malloc(sizeof(ISISKwd))) == NULL) {
    fprintf(stderr, "malloc fails in get_keyword_value_pair\n");
    goto err_exit;
  }
  rtn_kwd->keyword = kwd;
  rtn_kwd->misc_seq = -1;
  rtn_kwd->value = NULL;

  if ((editptr = strpbrk(value, "\"(")) == NULL) {
    /* the only line.  We're done! */
    rtn_kwd->value = value;
    goto good_exit;
  }
  /* which encloses which? */
  begquote = strchr(value, '"');
  begparen = strchr(value, '(');
  if ((begquote == NULL) || (begquote > begparen)) {
    /* parens first */
    termchar = ')';
  }
  else {
    termchar = '"';
    /* quotes first */
  }
  if ((editptr = strchr(value+1, termchar)) != NULL) {
    /* value terminates on this line. yay! */
    rtn_kwd->value = value;
    goto good_exit;
  }
  /* okay, we need more lines of input from the file to append to the
     value. (Drat) */

  while (1) {
    if (fgets(record_buf, ISIS_BUFSIZE, fh) == NULL) goto err_exit;
    /* advance to first nonblank */
    for(editptr=record_buf; *editptr == ' '; editptr++);
    /* cut the string at the end of whitespace */
    for(endstr=editptr+strlen(editptr)-1;
        (endstr > editptr && (*endstr == ' ' || *endstr == '\r' ||
                              *endstr == '\n' || *endstr == '\t'));
        endstr--) *endstr = 0;
    if ((value = realloc(value, strlen(value)+strlen(editptr)+1)) == NULL) {
      fprintf(stderr, "realloc failed!\n");
      goto err_exit;
    }
    strcpy(value+strlen(value), editptr);
    if (strchr(editptr, termchar) != NULL) /* we're done */ break;
  }
  /* set the Kwd structure pointer to the right valuie pointer */
  rtn_kwd->value = value;
 good_exit:
  return rtn_kwd;

 err_exit:
  if (kwd != NULL) free(kwd);
  if (value != NULL) free(value);
  if (rtn_kwd != NULL) free(rtn_kwd);
  return NULL;


}

static ISISKwdPtr destroy_keyword_pair(ISISKwdPtr kill_me) {
  ISISKwdPtr rtn_next = NULL;
  if (kill_me != NULL) {
    if (kill_me->keyword != NULL) free(kill_me->keyword);
    if (kill_me->value != NULL) free(kill_me->value);
    rtn_next = kill_me->next;
    free(kill_me);
  }
  return rtn_next;
}

/*
         1         2         3         4         5         6         7
12345678901234567890123456789012345678901234567890123456789012345678901234567890
*/
