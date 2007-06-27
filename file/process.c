/*
 * file - find type of a file or files - main library interface
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>	/* for open() */
#include <utime.h>
#include <unistd.h>	/* for read() */
#include <errno.h>

#include "file.private.h"
#include "form.h"

/* various global things that are needed one place or another
 * in the file library
 */
int 				/* Global command-line options 		*/
	__lf_debug = 0, 	/* debugging 				*/
	__lf_lflag = 0,		/* follow Symlinks (BSD only) 		*/
	__lf_zflag = 0,		/* follow (uncompress) compressed files */
	__lf_emit_struct = 0;	/* generate magic structure in check	*/

/*
 * __lf_process - process input file
 */
char *
__lf_process(filename)
const char	*filename;
{
	int	fd = 0;
	static  const char stdname[] = "standard input";
	unsigned char	buf[HOWMANY+1];	/* one extra for terminating '\0' */
	struct utimbuf  utbuf;
	struct stat	sb;
	int nbytes = 0;	/* number of bytes read from a datafile */

	__lf_form(0);

	if (__lf_fsmagic(filename, &sb) != 0)
	    return __lf_description();

	/*
	 * first try judging the file based on its filesystem status
	 */
	if (filename) {
	    if ((fd = open(filename, O_RDONLY)) < 0) {
		    /* We can't open it, but we were able to stat it. */
		    if (sb.st_mode & 0002) __lf_form("writeable, ");
		    if (sb.st_mode & 0111) __lf_form("executable, ");
		    __lf_form("can't read `%s' (%s).", filename, strerror(errno));
		    return __lf_description();
	    }
	}


	/*
	 * try looking at the first HOWMANY bytes
	 */
	if ((nbytes = read(fd, (char *)buf, HOWMANY)) == -1) {
		__lf_form("read failed (%s).", strerror(errno));
		return __lf_description();
		/*NOTREACHED*/
	}

	if (nbytes == 0) 
		__lf_form("empty");
	else {
		buf[nbytes++] = '\0';	/* null-terminate it */
		__lf_tryit(buf, nbytes, __lf_zflag);
	}

	if (filename != stdname) {
		/*
		 * Try to restore access, modification times if read it.
		 */
		utbuf.actime = sb.st_atime;
		utbuf.modtime = sb.st_mtime;
		(void) utime(filename, &utbuf); /* don't care if loses */
		(void) close(fd);
	}
	return __lf_description();
}


void
__lf_tryit(buf, nb, zflag)
unsigned char *buf;
int nb, zflag;
{
	/*
	 * Try compression stuff
	 */
	if (!zflag || __lf_zmagic(buf, nb) != 1)
		/*
		 * try tests in /etc/magic (or surrogate magic file)
		 */
		if (__lf_softmagic(buf, nb) != 1)
			/*
			 * try known keywords, check for ascii-ness too.
			 */
			if (__lf_ascmagic(buf, nb) != 1)
			    /*
			     * abandon hope, all ye who remain here
			     */
			    __lf_form("data");
}
