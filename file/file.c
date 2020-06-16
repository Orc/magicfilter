/*
 * file - find type of a file or files - main program.
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
#if 0
static char *moduleid = 
	"@(#)$Id$";
#endif	/* lint */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>	/* for MAXPATHLEN */
#include <malloc.h>

extern int getopt();
extern int optind;
extern char *optarg;

#ifndef __P
# if __STDC__ || __cplusplus
#  define __P(a) a
# else
#  define __P(a) ()
#  define const
# endif
#endif

#include "magic.h"

#ifdef S_IFLNK
# define USAGE  "Usage: %s [-czL] [-f namefile] [-m magicfile] file...\n"
#else
# define USAGE  "Usage: %s [-cz] [-f namefile] [-m magicfile] file...\n"
#endif

#ifndef MAGIC
# define MAGIC "/etc/magic"
#endif

#ifndef MAXPATHLEN
#define	MAXPATHLEN	512
#endif

static void unwrap	__P((char *fn));

static magic_t poof = 0;

/*
 * main - parse arguments and handle options
 */
int
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int didsomefiles = 0, errflg = 0, ret = 0;
	int flags = 0;
	char *magicfile = 0;

	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	while ((c = getopt(argc, argv, "cEdf:Lm:z")) != EOF)
		switch (c) {
		case 'E':
			magic_emit_struct(1);
			/* fall into 'c' */
		case 'c':
			flags |= MAGIC_CHECK;
			break;
		case 'd':
			flags |= MAGIC_DEBUG;
			break;
		case 'f':
			unwrap(optarg);
			++didsomefiles;
			break;
#ifdef S_IFLNK
		case 'L':
			flags |= MAGIC_SYMLINK;
			break;
#endif
		case 'm':
			magicfile = optarg;
			break;
		case 'z':
			flags |= MAGIC_COMPRESS;
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	if (errflg) {
		(void) fprintf(stderr, USAGE, progname);
		exit(2);
	}


	if ( (poof = magic_open(flags)) == 0)
	    exit(1);
	ret = magic_load(poof, magicfile);
	if (flags & MAGIC_CHECK)
	    exit(ret);

	if (optind == argc && !didsomefiles) {
		fprintf(stdout, "%s\n", magic_file(poof, 0));
	}
	else {
		int i, wid, nw;
		for (wid = 0, i = optind; i < argc; i++) {
			nw = strlen(argv[i]);
			if (nw > wid)
				wid = nw;
		}
		for (; optind < argc; optind++)
			fprintf(stdout, "%.*s: %s\n", wid, argv[optind], magic_file(poof, argv[optind]));
	}

	return 0;
}


/*
 * unwrap -- read a file of filenames, do each one.
 */
static void
unwrap(fn)
char *fn;
{
	char buf[MAXPATHLEN];
	FILE *f;
	int wid = 0, cwid;

	if ((f = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "Cannot open `%s' (%s).\n", fn, strerror(errno));
		exit(1);
		/*NOTREACHED*/
	}

	while (fgets(buf, MAXPATHLEN, f) != NULL) {
		cwid = strlen(buf) - 1;
		if (cwid > wid)
			wid = cwid;
	}

	rewind(f);

	while (fgets(buf, MAXPATHLEN, f) != NULL) {
		buf[strlen(buf)-1] = '\0';
		fprintf(stdout, "%.*s: %s\n", wid, buf, magic_file(poof,buf));
	}

	(void) fclose(f);
}
