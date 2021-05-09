/* See LICENCE file for copyright and licence details */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "utf8.h"
#include "util.h"
#include "drw.h"

Drw	*
drw_create(Display *dpy, Window root, int screen, int x, int y, unsigned int w, unsigned int h)
{
	Drw *drw = emalloc(sizeof(Drw));

	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->screen = screen;
	drw->x = 0;
	drw-> y = 0;
	drw->dpy = dpy;
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

void
drw_drawrect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int color)
{
	if (drw == NULL)
		return;
	XSetForeground(drw->dpy, drw->gc, drw->scheme[color].pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
}

void
drw_drawtext(Drw *drw, char *text, unsigned int size, Fnt *font, Clr *color, int x, int y)
{
	char buf[1024];
	long utf8codepoint = 0;
	int charexists = 0;
	int len;
	int utf8strlen, utf8charlen;
	const char *utf8str = NULL;
	XftDraw *xftdrw = NULL;

	xftdrw = XftDrawCreate(drw->dpy, drw->drawable, DefaultVisual(drw->dpy, drw->screen),
	                       DefaultColormap(drw->dpy, drw->screen));
	utf8strlen = 0;
	utf8str = text;
	while(*text) {
		utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
		charexists = charexists || XftCharExists(drw->dpy, font->xfont, utf8codepoint);
		if (charexists) {
			utf8strlen += utf8charlen;
			text += utf8charlen;
		}
		if (!charexists)
			text++;
		else
			charexists = 0;
	}
	if (utf8strlen) {
		len = MIN(utf8strlen, sizeof(buf) - 1);
		if (len) {
			memcpy(buf, utf8str, len);
			buf[len] = '\0';
			XftDrawStringUtf8(xftdrw, color, font->xfont, x, y,
	                     (XftChar8 *) buf, len);
		}
	}
	if (xftdrw)
		XftDrawDestroy(xftdrw);
}


void
drw_free(Drw *drw)
{
	if (drw == NULL)
		return;
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

int
drw_getpointer(Drw *drw, int *x, int *y)
{
	/* dummy variables */
	Window dw;
	unsigned int du;
	int dx, dy;

	if (!XQueryPointer(drw->dpy, drw->root, &dw, &dw, x, y, &dx, &dy, &du))
		return 1;
	return 0;
}

int
drw_getpointersel(Drw *drw, int itemnb)
{
	int i;
	int px, py, x, y;

	if (drw == NULL)
		return -1;
	if (drw_getpointer(drw, &px, &py))
		error("cannot query pointer");
	x = drw->x;
	y = drw->y;
	for (i = 0; i < itemnb; i++) {
		if (x <= px && px < (x + drw->w) &&
		    y <= py && py < (y + drw->h / itemnb))
			return i;
		y += drw->h / itemnb;
	}
	return -1;
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h)
{
	if (drw == NULL)
		return;
	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}

void
drw_movepointer(Drw *drw, int x, int y)
{
	if (drw == NULL)
		return;
	XWarpPointer(drw->dpy, None, drw->root, 0, 0, 0, 0, x, y);
}

void
drw_resize(Drw *drw, int x, int y, unsigned int w, unsigned int h)
{
	if (drw == NULL)
		return;
	drw->w = w;
	drw->h = h;
	drw->x = x;
	drw->y = y;
	if (drw->drawable)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_setfont(Drw *drw, Fnt font)
{
	if (drw != NULL)
		drw->font = font;
}

void
drw_setscheme(Drw *drw, Clr *scheme)
{
	if (drw != NULL)
		drw->scheme = scheme;
}
