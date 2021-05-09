/* See LICENCE file for copyright and licence details */

typedef XftColor Clr;

typedef struct {
	FcPattern *pattern;
	XftFont *xfont;
} Fnt;

typedef struct {
	Drawable drawable;
	Fnt font;
	GC gc;
	Window win, root;
	unsigned int w, h;
	int screen;
	int x, y;
	Display *dpy;
	Clr *scheme;
} Drw;

typedef struct Item {
	unsigned int extw;
	struct Item *next;
	struct Item *prev;
	char *text;
} Item;

Drw	*drw_create(Display *, Window, int, int, int, unsigned int, unsigned int);
void	 drw_drawrect(Drw *, int, int, unsigned int, unsigned int, int);
void	 drw_drawtext(Drw *, char *, unsigned int, Fnt *, Clr *, int, int);
void	 drw_free(Drw *);
int	 drw_getpointer(Drw *, int *, int *);
int	 drw_getpointersel(Drw *, int);
void	 drw_map(Drw *, Window, int , int , unsigned int , unsigned int );
void	 drw_movepointer(Drw *, int, int);
void	 drw_resize(Drw *, int, int, unsigned int, unsigned int);
void	 drw_setfont(Drw *, Fnt);
void	 drw_setscheme(Drw *, Clr *);
