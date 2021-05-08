/* See LICENCE file for copyright and licence details */

/* appearance */
static unsigned int minwidth = 20;	/* minimal menu width */
static unsigned int borderpx = 2;	/* window border width */
static unsigned int gappx    = 10;	/* left and right side gaps */
static unsigned int padpx    = 2;	/* padding between menu lines */

/* controls */
static unsigned int selbuttons[]  = { 1, 0, 1 };	/* mouse buttons to select item */
static unsigned int selevent      = ButtonPress;	/* mouse event to select an item */

static char *lastselfile = "/tmp/91menu";	/* file where the last selected item will be written */

/* font */
static const char *fontname = "GoMono Nerd Font:pixelsize=13:antialias=true;autohint=true";

/* color scheme */
static const char *colornames[COLORNB] = {
	[FG] = "#000000",	/* foreground */
	[BG] = "#ebffed",	/* background */
	[BD] = "#9bc99e",	/* border */
	[NV] = "#ebffed",	/* inverted text */
	[SL] = "#477e45",	/* selection */
};
