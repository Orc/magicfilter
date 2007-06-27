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
#ifndef	lint
static char *moduleid = 
	"@(#)$Id$";
#endif	/* lint */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>	/* for MAXPATHLEN */

extern int getopt();
extern int optind;
extern char *optarg;

#include "file.h"

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

/*
 * main - parse arguments and handle options
 */
int
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int check = 0, didsomefiles = 0, errflg = 0, ret = 0;
	char *magicfile = MAGIC;

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
			++check;
			break;
		case 'd':
			magic_debug(1);
			break;
		case 'f':
			unwrap(optarg);
			++didsomefiles;
			break;
#ifdef S_IFLNK
		case 'L':
			magic_followlink(1);
			break;
#endif
		case 'm':
			magicfile = optarg;
			break;
		case 'z':
			magic_zflag(1);
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

	ret = apprentice(magicfile, check);
	if (check)
		exit(ret);

	if (optind == argc && !didsomefiles) {
		fprintf(stdout, "%s\n", file_magic(0));
	}
	else {
		int i, wid, nw;
		for (wid = 0, i = optind; i < argc; i++) {
			nw = strlen(argv[i]);
			if (nw > wid)
				wid = nw;
		}
		for (; optind < argc; optind++)
			fprintf(stdout, "%.*s: %s\n", wid, argv[optind], file_magic(argv[optind]));
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
		fprintf(stdout, "%.*s: %s\n", wid, buf, file_magic(buf));
	}

	(void) fclose(f);
}
