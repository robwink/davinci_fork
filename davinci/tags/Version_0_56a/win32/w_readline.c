#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
/* The structure used to store a history entry. */
typedef struct _hist_entry {
  char *line;
  char *data;
} HIST_ENTRY;

/* A structure used to pass the current state of the history stuff around. */
typedef struct _hist_state {
  HIST_ENTRY **entries;		/* Pointer to the entries themselves. */
  int offset;			/* The location pointer within this array. */
  int length;			/* Number of elements within this array. */
  int size;			/* Number of slots allocated to this array. */
  int flags;
} HISTORY_STATE;


	HINSTANCE hDLL;               // Handle to DLL
	char *(*readline)(char *);
	void (*add_history)(char *);    // Function pointers
//	void (*initialize_readline)(void);
	HISTORY_STATE *(*history_get_history_state)(void);

int Init_DLL(void)
{
	/*Need to import our dll and hook-up needed functions;
	Functions pointerers need global scope, so they will be declared in parser.h*/
	hDLL = LoadLibrary("readline");
	if (hDLL != NULL){
		readline = (char *(__cdecl *)(char *))GetProcAddress(hDLL,"readline");
		add_history = (void (__cdecl *)(char *))GetProcAddress(hDLL,"add_history");
//		initialize_readline= (void (__cdecl *)(void))GetProcAddress(hDLL,"initialize_readline");
		history_get_history_state = (HISTORY_STATE *(__cdecl *)(void))GetProcAddress(hDLL,"history_get_history_state");
	}
	if (!readline || ! add_history /*|| !initialize_readline*/ || !history_get_history_state)	{
        FreeLibrary(hDLL);       
		return(1);
      
	}
//	initialize_readline();

	return(0);
}

char *w_readline(char * A)
{
	return(readline(A));
}

void w_add_history(char *A)
{
	add_history(A);
}

HISTORY_STATE *w_history_get_history_state(void)
{
	return(history_get_history_state());
}