/*
 * fsmagic - magic based on filesystem info - directory, special files, etc.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef	major			/* if `major' not defined in types.h, */
#include <sys/sysmacros.h>	/* try this one. */
#endif
#ifndef	major	/* still not defined? give up, manual intervention needed */
		/* If cc tries to compile this, read and act on it. */
		/* On most systems cpp will discard it automatically */
		Congratulations, you have found a portability bug.
		Please grep /usr/include/sys and edit the above #include 
		to point at the file that defines the "major" macro.
#endif	/*major*/

#include "file.private.h"

#if 0
static char *moduleid = 
	"@(#)$Id$";
#endif	/* lint */

int
__lf_fsmagic(fn, sb)
const char *fn;
struct stat *sb;
{
	int ret = 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
	if (fn == 0)
		ret = fstat(0, sb);
	else
#ifdef	S_IFLNK
	if (!__lf_lflag)
		ret = lstat(fn, sb);
	else
#endif
	ret = stat(fn, sb);	/* don't merge into if; see "ret =" above */

	if (ret) {
		__lf_form("can't stat `%s' (%s).", fn ? fn : "[standard input]", strerror(errno));
		return 1;
	}

	if (sb->st_mode & S_ISUID) __lf_form("setuid ");
	if (sb->st_mode & S_ISGID) __lf_form("setgid ");
	if (sb->st_mode & S_ISVTX) __lf_form("sticky ");
	
	switch (sb->st_mode & S_IFMT) {
	case S_IFDIR:
		__lf_form("directory");
		return 1;
	case S_IFCHR:
		(void) __lf_form("character special (%d/%d)",
			major(sb->st_rdev), minor(sb->st_rdev));
		return 1;
	case S_IFBLK:
		(void) __lf_form("block special (%d/%d)",
			major(sb->st_rdev), minor(sb->st_rdev));
		return 1;
	/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef	S_IFIFO
	case S_IFIFO:
		__lf_form("fifo (named pipe)");
		return 1;
#endif
#ifdef	S_IFLNK
	case S_IFLNK:
		if (fn == 0) {
			__lf_form("symlink to %s", __lf_process(0));
			return 1;
		}
		else {
			char buf[BUFSIZ+4];
			register int nch;
			struct stat tstatbuf;

			if ((nch = readlink(fn, buf, BUFSIZ-1)) <= 0) {
				__lf_form("unreadable symlink (%s).", 
				      strerror(errno));
				return 1;
			}
			buf[nch] = '\0';	/* readlink(2) forgets this */

			/* If broken symlink, say so and quit early. */
			if (*buf == '/') {
			    if (stat(buf, &tstatbuf) < 0) {
				__lf_form( "broken symbolic link to %s", buf);
				return 1;
			    }
			}
			else {
			    char *tmp;
			    char buf2[BUFSIZ+BUFSIZ+4];

			    if ((tmp = strrchr(fn,  '/')) == NULL) {
				tmp = buf; /* in current directory anyway */
			    }
			    else {
				strcpy (buf2, fn);  /* take directory part */
				buf2[tmp-fn+1] = '\0';
				strcat (buf2, buf); /* plus (relative) symlink */
				tmp = buf2;
			    }
			    if (stat(tmp, &tstatbuf) < 0) {
				__lf_form( "broken symbolic link to %s", buf);
				return 1;
			    }
                        }

			/* Otherwise, handle it. */
			if (__lf_lflag) {
				__lf_form("%s", __lf_process(buf));
				return 1;
			} else { /* just print what it points to */
				__lf_form("symbolic link to %s", buf);
			}
		}
		return 1;
#endif
#ifdef	S_IFSOCK
	case S_IFSOCK:
		__lf_form("socket");
		return 1;
#endif
	case S_IFREG:
		break;
	default:
		__lf_form("invalid mode 0%o", sb->st_mode);
		return 1;
	}

	/*
	 * regular file, check next possibility
	 */
	if (sb->st_size == 0) {
		__lf_form("empty");
		return 1;
	}
	return 0;
}

