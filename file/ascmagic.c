/*
 * Ascii magic -- file types that we know based on keywords
 * that can appear anywhere in the file.
 *
 * Copyright (c) Ian F. Darwin, 1987.
 * Written by Ian F. Darwin.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or of the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. The author is not responsible for the consequences of use of this
 *    software, no matter how awful, even if they arise from flaws in it.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Since few users ever read sources,
 *    credits must appear in the documentation.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.  Since few users
 *    ever read sources, credits must appear in the documentation.
 *
 * 4. This notice may not be removed or altered.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "file.private.h"
#include "names.h"
#include "form.h"

#if 0
static char *moduleid = 
	"@(#)ascmagic.c,v 3.0.1.1 1997/08/01 00:38:22 orc Exp";
#endif	/* lint */

			/* an optimisation over plain strcmp() */
#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)

int
__lf_ascmagic(buf, nbytes)
unsigned char *buf;
int nbytes;	/* size actually read */
{
	int i, has_escapes = 0;
	unsigned char *s;
	char nbuf[HOWMANY+1];	/* one extra for terminating '\0' */
	char *token;
	register struct names *p;

	/* these are easy, do them first */

	/*
	 * for troff, look for . + letter + letter or .\";
	 * this must be done to disambiguate tar archives' ./file
	 * and other trash from real troff input.
	 */
	if (*buf == '.') {
		unsigned char *tp = buf + 1;

		while (isascii(*tp) && isspace(*tp))
			++tp;	/* skip leading whitespace */
		if ((isascii(*tp) && (isalnum(*tp) || *tp=='\\') &&
		    isascii(*(tp+1)) && (isalnum(*(tp+1)) || *tp=='"'))) {
			__lf_form("troff or preprocessor input text");
			return 1;
		}
	}
	if ((*buf == 'c' || *buf == 'C') && 
	    isascii(*(buf + 1)) && isspace(*(buf + 1))) {
		__lf_form("fortran program text");
		return 1;
	}

	/* look for tokens from names.h - this is expensive! */
	/* make a copy of the buffer here because strtok() will destroy it */
	s = (unsigned char*) memcpy(nbuf, buf, nbytes);
	/*
	 * The string should already be NULL terminated. However, if we are
	 * going to redo the NULL, then lets do it right.
	 */
	s[nbytes - 1] = '\0';
	has_escapes = (memchr(s, '\033', nbytes) != NULL);
	while ((token = strtok((char*)s, " \t\n\r\f")) != NULL) {
		s = NULL;	/* make strtok() keep on tokin' */
		for (p = names; p < names + NNAMES; p++) {
			if (STREQ(p->name, token)) {
				__lf_form("%s", types[p->type]);
				if (has_escapes)
					__lf_form(" (with escape sequences)"); 
				return 1;
			}
		}
	}

	switch (__lf_is_tar(buf, nbytes)) {
	case 1:
		__lf_form("tar archive");
		return 1;
	case 2:
		__lf_form("POSIX tar archive");
		return 1;
	}

	for (i = 0; i < nbytes; i++) {
		if (!isascii(*(buf+i)))
			return 0;	/* not all ascii */
	}

	/* all else fails, but it is ascii... */
	__lf_form("ascii text");
	if (has_escapes) {
		__lf_form(" (with escape sequences)");
	}
	return 1;
}


