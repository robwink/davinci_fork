/**
 ** XfAddPeerMenu:
 **
 ** Append a peer menu to the end of the list of peers
 **/

XfMenu *
XfAddPeerMenu(parent, str, align, font, pixmap)
	XfMenu *parent;
	char *str;
	int align;
	XFontStruct font;
	Pixmap pixmap;
{
	XfMenu *mptr;
	XfMenu *new = NULL;


	new = calloc(1,sizeof(XfMenu));
	new->status = 0;
	new->align = align;
	new->str = str;
	new->pixmap = pixmap;
	new->child = NULL;
	new->peer = NULL;

	/**
	 ** Put this item at then end of the list of children
	 **/

	if (parent != NULL) {
		while (parent->peer != NULL) {
			parent = parent->peer;
		}
		parent->peer = new;
	}
	return(new);
}
/**
 ** XfAddMenuItem:
 **
 ** Append a child menu to the end of the list of children
 **/

XfMenu *
XfAddMenuItem(parent, str, align, font, pixmap, id)
	XfMenu *parent;
	char *str;
	int align;
	XFontStruct font;
	Pixmap pixmap;
	void *id;
{
	XfMenu *mptr;
	XfMenu *new = NULL;


	new = calloc(1,sizeof(XfMenu));
	new->status = 0;
	new->align = align;
	new->str = str;
	new->pixmap = pixmap;
	new->child = NULL;
	new->peer = NULL;
	new->id = id;

	/**
	 ** Put this item at then end of the list of children
	 **/

	if (parent != NULL) {
		if (parent->child != NULL) {
			mptr = parent->child;
			while (mptr->peer != NULL) {
				mptr = mptr->peer;
			}
			mptr->peer = new;
		} else {
			parent->child = new;
		}
	}
	return(new);
}

void
print_menu(parent, depth)
XfMenu *parent;
int depth;
{
	int i;
	XfMenu *child;

	if (parent->str != NULL) {
		printf("%*s%s\n", depth*4, " ", parent->str);
	} else {
		printf("%*s--------\n", depth*4, " ");
	}
	if (parent->child) print_menu(parent->child, depth+1);
	if (parent->peer)  print_menu(parent->peer, depth);
}


