#include "Xfred.h"

void
toggle_state(B,E)
Button B;
XEvent *E;
{
	B->state = ((B->state+1)%B->maxstate);
	(*(B->updateCallback))(B,E);
}

set_state(B,i)
Button B;
int i;
{
	B->state = i;
	(*(B->updateCallback))(B,NULL);
}
