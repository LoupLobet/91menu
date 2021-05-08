/* See LICENCE file for copyright and licence details */
/* this piece of software come from dmenu : http://tools.suckless.org/dmenu/ */

#define UTF_SIZ     4

long	 utf8decodebyte(const char, int *);
int	 utf8validate(long *, int);
int	 utf8decode(const char *, long *, int);
