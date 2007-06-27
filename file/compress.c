/*
 * compress routines:
 *	__lf_zmagic() - returns 0 if not recognized, uncompresses and prints
 *		   information if recognized
 *	uncompress(method, old, n, newch) - uncompress old into new, 
 *					    using method, return sizeof new
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#include "file.private.h"

static struct {
   char *magic;
   int   maglen;
   char *argv[3];
   int	 silent;
} compr[] = {
    { "\037\235", 2, { "uncompress", "-c", NULL }, 0 },
    { "\037\213", 2, { "gzip", "-dq", NULL }, 1 },
    /* 
     * XXX pcat does not work, cause I don't know how to make it read stdin,
     * so we use gzip
     */
    { "\037\036", 2, { "gzip", "-dq", NULL }, 0 },
};

static int ncompr = sizeof(compr) / sizeof(compr[0]);


static int uncompress __P((int, const unsigned char *, unsigned char **, int));

int
__lf_zmagic(buf, nbytes)
unsigned char *buf;
int nbytes;
{
	unsigned char *newbuf;
	int newsize;
	int i;

	for (i = 0; i < ncompr; i++) {
		if (nbytes < compr[i].maglen)
			continue;
		if (memcmp(buf, compr[i].magic,  compr[i].maglen) == 0)
			break;
	}

	if (i == ncompr)
		return 0;

	if ((newsize = uncompress(i, buf, &newbuf, nbytes)) != 0) {
		__lf_tryit(newbuf, newsize, 1);
		free(newbuf);
		__lf_form(" (");
		__lf_tryit(buf, nbytes, 0);
		__lf_form(")");
	}
	return 1;
}


static int
uncompress(method, old, newch, n)
int method;
const unsigned char *old;
unsigned char **newch;
int n;
{
	int fdin[2], fdout[2];

	if (pipe(fdin) == -1 || pipe(fdout) == -1) {
		__lf_form("cannot create pipe (%s),", strerror(errno));
		return EOF;
	}
	switch (fork()) {
	case 0:	/* child */
		(void) close(0);
		(void) dup(fdin[0]);
		(void) close(fdin[0]);
		(void) close(fdin[1]);

		(void) close(1);
		(void) dup(fdout[1]);
		(void) close(fdout[0]);
		(void) close(fdout[1]);
		if (compr[method].silent)
		    (void) close(2);

		execvp(compr[method].argv[0], compr[method].argv);
		__lf_error("could not execute `%s' (%s).\n", 
		      compr[method].argv[0], strerror(errno));
		/*NOTREACHED*/
	case -1:
		__lf_form("could not fork (%s)", strerror(errno));
		return EOF;

	default: /* parent */
		(void) close(fdin[0]);
		(void) close(fdout[1]);
		if (write(fdin[1], old, n) != n) {
			__lf_form("write failed (%s)", strerror(errno));
			return EOF;
		}
		(void) close(fdin[1]);
		if ((*newch = (unsigned char *) malloc(n)) == NULL) {
			__lf_form("out of memory");
			return EOF;
		}
		if ((n = read(fdout[0], *newch, n)) <= 0) {
			free(*newch);
			__lf_form("read failed (%s)", strerror(errno));
			return EOF;
		}
		(void) close(fdout[0]);
		(void) wait(NULL);
		return n;
	}
}


