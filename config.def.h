/* See LICENCE file for copyright and licence details */

static char *lastselfile = "/tmp/91menu";	/* file where the last selected item will be written */

/* appearance */
static unsigned int minwidth = 20;		/* minimal menu width */
static unsigned int borderpx = 1;		/* window border width */
static unsigned int gappx    = 10;		/* left and right side gaps */
static unsigned int padpx    = 2;		/* padding between menu lines */
static unsigned int textpos = CENTER;	/* horizontal text position LEFT/CENTER/RIGTH */

/* mouse */
static unsigned int buttons[]	= { 1, 0, 1 };		/* mouse buttons, 0 untracked, 1: selection only */
static unsigned int grabmouse   = 1;				/* 0 = ignore mouse clicks outside of the window */
static unsigned int buttonevent	= ButtonRelease;	/* mouse event to select an item */



/* font */
static const char *fontname = "GoMono Nerd Font:pixelsize=11:antialias=true;autohint=true";

/* color scheme */
static const char *colornames[COLORNB] = {
	[FG] = "#586e75",	/* foreground */
	[BG] = "#002b36",	/* background */
	[BD] = "#586e75",	/* border */
	[NV] = "#93a1a1",	/* inverted text */
	[SL] = "#073642",	/* selection */
};
