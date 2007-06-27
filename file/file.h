/*
 * interfaces for libfile.a
 */
#ifndef __LIBFILE_D
#define __LIBFILE_D

extern int magic_debug(int);		/* set debug */
extern int magic_zflag(int);		/* set zflag (uncompress) */
extern int magic_followlink(int);	/* follow links or not */
extern int magic_emit_struct(int);	/* have apprentice dump a magic
					   structure suitable for embedding
					   in other programs. */

extern int apprentice(char*,int);	/* parse /etc/magic */

extern char* file_magic(char*);		/* find out what a file is */
extern char* string_magic(char*,int);	/* if this string was a file, what */
					/* would it be? */
extern void *magic;
extern int nmagic;

extern char *progname;

#endif/*__LIBFILE_D*/
