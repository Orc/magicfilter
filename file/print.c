/*
 * print.c - debugging printout routines
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
#include <errno.h>
#include <string.h>
#if __STDC__
# include <stdarg.h>
#else
# include <varargs.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "file.private.h"

#if 0
static char *moduleid =
	"@(#)$Id$";
#endif  /* lint */

#define SZOF(a)	(sizeof(a) / sizeof(a[0]))

void
__lf_mdump(m)
struct magic *m;
{
	static char *typ[] = {   "invalid", "byte", "short", "invalid",
				 "long", "string", "date", "beshort",
				 "belong", "bedate", "leshort", "lelong",
				 "ledate" };
	int i, n;

	if (__lf_emit_struct) {
	    printf("    {");
	    printf(" %d", m->flag);
	    printf(", %d", m->cont_level);
	    printf(", { %#o, %ldL }", (unsigned char)(m->in.type),
						m->in.offset);
	    printf(", %ldL", m->offset);
	    printf(", %#o", (unsigned char)(m->reln));
	    printf(", %d", m->type);
	    printf(", %u", (unsigned char)m->vallen);

	    for (n=MAXstring; n>0; )
		if (m->value.s[n-1])
		    break;
		else
		    --n;
	    printf(", \"");
	    for (i=0; i<n; i++)
		printf(isprint(m->value.s[i]) ? "%c" : "\\%o",
				   (unsigned char)(m->value.s[i]));
	    printf("\"");

	    printf(", %luL", m->mask);
	    printf(", %#o,", (unsigned char)(m->nospflag));
	    printf("\"%s\" },\n", m->desc);
	    return;
	}

	(void) fputc('[', stderr);
	(void) fprintf(stderr, ">>>>>>>> %d" + 8 - (m->cont_level & 7),
		       m->offset);

	if (m->flag & INDIR)
		(void) fprintf(stderr, "(%s,%ld),",
			       (m->in.type >= 0 && m->in.type < SZOF(typ)) ? 
					typ[(unsigned char) m->in.type] :
					"*bad*",
			       (long)(m->in.offset));

	(void) fprintf(stderr, " %s%s", (m->flag & UNSIGNED) ? "u" : "",
		       (m->type >= 0 && m->type < SZOF(typ)) ? 
				typ[(unsigned char) m->type] : 
				"*bad*");
	if (m->mask != ~0L)
		(void) fprintf(stderr, " & %.8lx", (long)(m->mask));

	(void) fprintf(stderr, ",%c", m->reln);

	if (m->reln != 'x') {
	    switch (m->type) {
	    case BYTE:
	    case SHORT:
	    case LONG:
	    case LESHORT:
	    case LELONG:
	    case BESHORT:
	    case BELONG:
		    (void) fprintf(stderr, "%ld", m->value.l);
		    break;
	    case STRING:
		    __lf_showstr(stderr, m->value.s, -1);
		    break;
	    case DATE:
	    case LEDATE:
	    case BEDATE:
		    {
			    char *rt, *pp = ctime((time_t*) &m->value.l);
			    if ((rt = strchr(pp, '\n')) != NULL)
				    *rt = '\0';
			    (void) fprintf(stderr, "%s,", pp);
			    if (rt)
				    *rt = '\n';
		    }
		    break;
	    default:
		    (void) fputs("*bad*", stderr);
		    break;
	    }
	}
	(void) fprintf(stderr, ",\"%s\"]\n", m->desc);
}

/*
 * __lf_ckfputs - fputs, but with error checking
 * __lf_ckfprintf - fprintf, but with error checking
 */
void
__lf_ckfputs(str, fil) 	
    const char *str;
    FILE *fil;
{
	if (fputs(str,fil) == EOF)
		__lf_error("write failed.\n");
}

/*VARARGS*/
void
#if __STDC__
__lf_ckfprintf(FILE *f, const char *fmt, ...)
#else
__lf_ckfprintf(va_alist)
	va_dcl
#endif
{
	va_list va;
#if __STDC__
	va_start(va, fmt);
#else
	FILE *f;
	const char *fmt;
	va_start(va);
	f = va_arg(va, FILE *);
	fmt = va_arg(va, const char *);
#endif
	(void) vfprintf(f, fmt, va);
	if (ferror(f))
		__lf_error("write failed.\n");
	va_end(va);
}

/*
 * __lf_error - print best error message possible and exit
 */
/*VARARGS*/
void
#if __STDC__
__lf_error(const char *f, ...)
#else
__lf_error(va_alist)
	va_dcl
#endif
{
	va_list va;
#if __STDC__
	va_start(va, f);
#else
	const char *f;
	va_start(va);
	f = va_arg(va, const char *);
#endif
	if (progname != NULL) 
		(void) fprintf(stderr, "%s: ", progname);
	(void) vfprintf(stderr, f, va);
	va_end(va);
	exit(1);
}
