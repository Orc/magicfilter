#define	FILE_VERSION_MAJOR	3
#define	patchlevel		14

/*
 * Patchlevel file for Ian Darwin's MAGIC command.
 * $Id$
 *
 * $Log$
 * Revision 1.3  2000/02/25 18:28:20  orc
 * move around some more of the bits of libfile -- move magwarn() into
 * apprentice.c so it doesn't have to be exported.  Also move magicfile
 * and lineno there so THEY don't have to be exported, and clean up
 * commentary to recover from the global __lf_ prefixing of unexported
 * symbols.
 *
 * Revision 1.2  2000/02/25 17:10:07  orc
 * Clean up the published interface, so that all exported things are declared
 * in file.h and defined in either libe.c or progname.c.
 * progname.c:  placeholder ``char *progname'' for programs that don't define it.
 * libe.c: magic_*() set functions, *_magic() examining functions.
 * file.private.h: used to be file.h
 *
 * Revision 1.1.1.1  2000/02/25 16:18:25  orc
 * Magicfilter-2;  a reinvention of magicfilter.
 *
 * Revision 3.0.1.1  1997/08/01 00:38:22  orc
 * Moving the ENGINE version control down to California
 *
 * Revision 1.1.1.1  1997/06/12  17:33:01  orc
 * The new av engine, moved around a bit
 *
 * Revision 1.2  1997/03/03  04:03:07  orc
 * Lots of changes.
 *
 * Revision 1.1  1996/07/24 19:58:50  orc
 * File identification library, derived from the file(1) written by Ian Darwin,
 * and later extended by John Gilmore, Geoff Collyer, Rob McMahon, Guy Harris,
 * Christos Zoulas, and others.  It is copyright Ian Darwin, but may be used
 * as long as he is credited for the original program and it is clearly noted
 * that this is a derived work and he's not responsible for it eating any
 * sensitive information.
 *
 * Revision 1.14  1994/05/03  17:58:23  christos
 * changes from mycroft@gnu.ai.mit.edu (Charles Hannum) for unsigned
 *
 * Revision 1.13  1994/01/21  01:27:01  christos
 * Fixed null termination bug from Don Seeley at BSDI in ascmagic.c
 *
 * Revision 1.12  1993/10/27  20:59:05  christos
 * Changed -z flag to understand gzip format too.
 * Moved builtin compression detection to a table, and move
 * the compress magic entry out of the source.
 * Made printing of numbers unsigned, and added the mask to it.
 * Changed the buffer size to 8k, because gzip will refuse to
 * unzip just a few bytes.
 *
 * Revision 1.11  1993/09/24  18:49:06  christos
 * Fixed small bug in softmagic.c introduced by
 * copying the data to be examined out of the input
 * buffer. Changed the Makefile to use sed to create
 * the correct man pages.
 *
 * Revision 1.10  1993/09/23  21:56:23  christos
 * Passed purify. Fixed indirections. Fixed byte order printing.
 * Fixed segmentation faults caused by referencing past the end
 * of the magic buffer. Fixed bus errors caused by referencing
 * unaligned shorts or longs.
 *
 * Revision 1.9  1993/03/24  14:23:40  ian
 * Batch of minor changes from several contributors.
 *
 * Revision 1.8  93/02/19  15:01:26  ian
 * Numerous changes from Guy Harris too numerous to mention but including
 * byte-order independance, fixing "old-style masking", etc. etc. A bugfix
 * for broken symlinks from martin@@d255s004.zfe.siemens.de.
 * 
 * Revision 1.7  93/01/05  14:57:27  ian
 * Couple of nits picked by Christos (again, thanks).
 * 
 * Revision 1.6  93/01/05  13:51:09  ian
 * Lotsa work on the Magic directory.
 * 
 * Revision 1.5  92/09/14  14:54:51  ian
 * Fix a tiny null-pointer bug in previous fix for tar archive + uncompress.
 * 
 */

