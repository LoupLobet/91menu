/* See LICENCE file for copyright and licence details */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#define _POSIX_X_SOURCE 200809L
#define _XOPEN_SOURCE 700

enum { FG, BG, BD, NV, SL, COLORNB };
enum { PRESS, RELEASE };

static void	 cleanup(void);
static Fnt	 createfont(const char *);
static Clr	*createscheme(const char *[], int);
static void	 drawmenu(int);
static int	 fetchlastsel(char *);
static void	 getextw(char *, unsigned int, Fnt *, unsigned int *);
static void	 grabbuttons(void);
static void	 grabfocus(void);
static int	 loadcolor(const char *, Clr *);
static int 	 loadfont(const char *, FcPattern *, Fnt *);
static void	 readstdin(void);
static int	 run(void);
static int	 savelastsel(char *, char *);
static void	 usage(void);
static void	 winsetup(XWindowAttributes *);

static Fnt font;
static XIC xic;
static int itemnb;
static int screen;
static Window win, root;
static Display *dpy;
static Drw *drw;
static Item *items;
static Clr *scheme;

#include "config.h"

int
main(int argc, char *argv[])
{
	XWindowAttributes wa;
	int i, sel;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-g"))
			grabmouse = 1;
		else if (!strcmp(argv[i], "-u")) {
			usage();
			exit(0);
		} else if (!strcmp(argv[i], "-v")) {
			puts("91menu-"VERSION);
			exit(0);
		} else if (i + 1 == argc)
			usage();
		if (!strcmp(argv[i], "-f"))
			lastselfile = argv[++i];
		else if (!strcmp(argv[i], "-bd"))
			colornames[BD] = argv[++i];
		else if (!strcmp(argv[i], "-bg"))
			colornames[BG] = argv[++i];
		else if (!strcmp(argv[i], "-fg"))
			colornames[FG] = argv[++i];
		else if (!strcmp(argv[i], "-nv"))
			colornames[NV] = argv[++i];
		else if (!strcmp(argv[i], "-sl"))
			colornames[SL] = argv[++i];
		else if (!strcmp(argv[i], "-ft"))
			fontname = argv[++i];
		else
			usage();
	}

	if (setlocale(LC_CTYPE, "") == NULL || !XSupportsLocale())
		error("no locale support");
	if ((dpy = XOpenDisplay(NULL)) == NULL)
		error("cannot open display");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	if (!XGetWindowAttributes(dpy, root, &wa))
		error("could not get window attributes: 0x%lx", root);
	drw = drw_create(dpy, root, screen, 0, 0, wa.width, wa.height);
	font = createfont(fontname);
	scheme = createscheme(colornames, COLORNB);
	readstdin();
	winsetup(&wa);

	sel = run();
	cleanup();
	if (sel >= 0) {
		printf("%s\n", items[sel].text);
		if (savelastsel(lastselfile, items[sel].text))
			warning("can't open %s for writing", lastselfile);
	}

	return 0;
}

static void
cleanup(void)
{
	free(scheme);
	drw_free(drw);
	XSync(dpy, False);
	XCloseDisplay(dpy);
}

static Fnt
createfont(const char *fname)
{
	Fnt f;

	if (fname == NULL)
		error("font name: NULL pointer");
	if (loadfont(fname, NULL, &f))
		error("could not load font: %s\n", fname);
	return f;
}

static Clr *
createscheme(const char *cnames[], int cnb)
{
	int i;
	Clr *s = NULL;

	if (cnames == NULL || cnb < 0)
		error("color name: NULL pointer");
	if ((s = calloc(cnb, sizeof(Clr))) == NULL)
		error("could not allocate color scheme");
	for (i = 0; i < cnb; i++) {
		if (loadcolor(cnames[i], &s[i]))
			error("could not load color: %s\n", cnames[i]);
	}
	return s;
}

static void
drawmenu(int nosel)
{
	int i;
	int sel;
	int x, y;

	/* draw background color */
	drw_drawrect(drw, 0, 0, drw->w, drw->h, BG);
	/* draw selected item box */
	if (!nosel && (sel = drw_getpointersel(drw, itemnb)) >= 0)
		drw_drawrect(drw, 0, sel * (drw->h / itemnb), drw->w, drw->h / itemnb, SL);
	for (i = 0; i < itemnb; i++) {
		x = (drw->w - items[i].extw) / 2;
		y = i * (drw->h / itemnb) + font.xfont->ascent + padpx;
		if (i == sel)
			drw_drawtext(drw, items[i].text, strlen(items[i].text), &font,
			             &scheme[NV], x, y);
		else
			drw_drawtext(drw, items[i].text, strlen(items[i].text), &font,
		                 &scheme[FG], x, y);
	}
	drw_map(drw, win, 0, 0, drw->w, drw->h);
}

static int
fetchlastsel(char *file)
{
	char buf[BUFSIZ];
	int i;
	FILE *fp;
	char *p;

	if ((fp = fopen(file, "r")) == NULL)
		return 0;
	if (fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		if ((p = strchr(buf, '\n'))!= NULL)
			*p = '\0';
		for (i = 0; i < itemnb; i++) {
			if (!strcmp(items[i].text, buf))
				return i;
		}
	}
	return 0;
}

static void
getextw(char *str, unsigned int size, Fnt *font, unsigned int *extw)
{
	XGlyphInfo ext;

	if (font->xfont == NULL || str == NULL)
		return;
	XftTextExtentsUtf8(dpy, font->xfont, (XftChar8 *) str, size, &ext);
	if (extw != NULL)
		*extw = ext.xOff;
}

static void
grabbuttons(void)
{
	int i;

	for (i = 1; i - 1 < sizeof(buttons) / sizeof(unsigned int); i++) {
		XGrabButton(dpy, i, AnyModifier, root, True,
		            ButtonPressMask | ButtonReleaseMask,
		            GrabModeAsync, GrabModeAsync, None, None);
	}
}

static void
grabfocus(void)
{
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000  };
	Window focuswin;
	int i, revertwin;

	for (i = 0; i < 100; ++i) {
		XGetInputFocus(dpy, &focuswin, &revertwin);
		if (focuswin == win)
			return;
		XSetInputFocus(dpy, win, RevertToParent, CurrentTime);
		nanosleep(&ts, NULL);
	}
	error("cannot grab focus");
}

static int
loadcolor(const char *cname, Clr *c)
{
	if (cname == NULL || c == NULL)
		return 1;
	if (!XftColorAllocName(dpy, DefaultVisual(dpy, screen),
	    DefaultColormap(dpy, screen), cname, c))
		error("could not allocate color: %s", cname);
	return 0;
}

static int
loadfont(const char *fname, FcPattern *fpattern, Fnt *f)
{
	FcBool iscolorfont;
	FcPattern *pattern = NULL;
	XftFont *xfont = NULL;

	if (fname) {
		if ((xfont = XftFontOpenName(dpy, screen, fname)) == NULL) {
			warning("could not load font from name: %s\n", fname);
			return 1;
		}
		if ((pattern = FcNameParse((FcChar8 *) fname)) == NULL) {
			warning("could not load font from pattern: %s\n", fname);
			return 1;
		}
	} else if (fpattern) {
		if ((xfont = XftFontOpenPattern(dpy, fpattern))) {
			warning("could not load font from pattern\n");
			return 1;
		}
	} else
		error("cannot load unspecified font");
	/* avoid color fonts to be used */
	if (FcPatternGetBool(xfont->pattern, FC_COLOR, 0, &iscolorfont) ==
	   FcResultMatch &&  iscolorfont) {
		XftFontClose(dpy, xfont);
		return 1;
	}
	f->xfont = xfont;
	f->pattern = pattern;
	return 0;
}

static void
readstdin(void)
{
	char buf[BUFSIZ];
	unsigned int extw;
	int i;
	char *p = NULL;

	for (i = 0; fgets(buf, sizeof(buf), stdin); i++) {
		items = erealloc(items, (i + 1) * sizeof(Item));
		if ((p = strchr(buf, '\n'))!= NULL)
			*p = '\0';
		if ((items[i].text = strdup(buf)) == NULL)
			error("strdup");
		extw = 0;
		getextw(items[i].text, strlen(items[i].text), &font, &extw);
		items[i].extw = extw;
	}
	itemnb = i;
}

static int
run(void)
{
	XEvent ev;
	unsigned int button;

	drw_setscheme(drw, scheme);
	drw_setfont(drw, font);
	if (grabmouse)
		grabbuttons();
	XSync(dpy, False);
	while (!XNextEvent(dpy, &ev)) {
		if (XFilterEvent(&ev, win))
			continue;
		switch (ev.type) {
		case ButtonPress:
		case ButtonRelease:
			button = ev.xbutton.button - 1;
			if (button < sizeof(buttons) / sizeof(unsigned int) &&
			    buttons[ev.xbutton.button - 1] && buttonevent == ev.type)
				return drw_getpointersel(drw, itemnb);
			drawmenu(0);
			break;
		case DestroyNotify:
			if (ev.xdestroywindow.window != win)
				break;
			cleanup();
			exit(1);
		case Expose:
			if (ev.xexpose.count == 0)
				drawmenu(0);
			break;
		case FocusIn:
			grabfocus();
			break;
		case LeaveNotify:
			drawmenu(1);
			break;
		case MotionNotify:
			drawmenu(0);
			break;
		case VisibilityNotify:
			if (ev.xvisibility.state != VisibilityUnobscured)
				XRaiseWindow(dpy, win);
			break;
		}
	}
	return -1;
}

static int
savelastsel(char *file, char *sel)
{
	FILE *fp = NULL;

	if ((fp = fopen(file, "w")) == NULL)
		return 1;
	fprintf(fp, "%s", sel);
	fclose(fp);
	return 0;
}

static void
usage(void)
{
	fputs("usage: 91menu [-guv] [-f file] [-bd color] [-bg color] [-fg color]\n"
	      "              [-nv color] [-sl color] [-ft font]\n", stderr);
	exit(1);
}

static void
winsetup(XWindowAttributes *wa)
{
	XSetWindowAttributes swa;
	XClassHint xch = {"91menu", "91menu"};
	XIM xim;
	unsigned int w, h;
	int i, x, y, sel;

	/* compute window width/height */
	w = minwidth;
	for (i = 0; i < itemnb; i++) {
		if (items[i].extw > w)
			w = items[i].extw;
	}
	w += 2 * gappx;
	h = itemnb * (font.xfont->ascent + font.xfont->descent) + itemnb * 2 * padpx;

	/* compute pointer coordinate */
	if (drw_getpointer(drw, &x, &y))
		error("cannot query pointer");

	sel = fetchlastsel(lastselfile);

	/* move the menu to center the first item on the pointer */
	x -= borderpx + w / 2;
	y -= borderpx + h / itemnb / 2 + sel * (h / itemnb);

	/* corrects menu coordinates if the menu isn't completly visible on the screen */
	if (x < 0)
		x  = 0;
	else if (x + w > wa->width)
		x = wa->width - w - borderpx * 2;
	if (y < 0)
		y = 0;
	else if (y + h > wa->height)
		y = wa->height - h - borderpx * 2;
	drw_resize(drw, x + borderpx, y + borderpx, w, h);

	/* move pointer to the center of the first item */
	drw_movepointer(drw, x + borderpx + w / 2, y + borderpx + h / itemnb / 2 + sel * (h / itemnb));

	/* window setup */
	swa.override_redirect = True;
	swa.background_pixel = scheme[BG].pixel;
	swa.event_mask = ButtonPressMask | ButtonReleaseMask | LeaveWindowMask |
	                 PointerMotionMask | ButtonMotionMask | ExposureMask |
	                 StructureNotifyMask;
	win = XCreateWindow(dpy, root, x, y, w, h, borderpx, CopyFromParent,
	                    CopyFromParent, CopyFromParent, CWOverrideRedirect |
	                    CWBackPixel | CWEventMask, &swa);
	XSetWindowBorder(dpy, win, scheme[BD].pixel);
	XSetClassHint(dpy, win, &xch);

	if ((xim = XOpenIM(dpy, NULL, NULL, NULL)) == NULL)
		error("cannot open input device");
	xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
	                XNClientWindow, win, XNFocusWindow, win, NULL);
	XMapRaised(dpy, win);
}
