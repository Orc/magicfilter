/*
 * interfaces for libfile.a
 */
#ifndef __LIBMAGIC_D
#define __LIBMAGIC_D

extern int magic_debug(int);		/* set debug */
extern int magic_zflag(int);		/* set zflag (uncompress) */
extern int magic_followlink(int);	/* follow links or not */
extern int magic_emit_struct(int);	/* have apprentice dump a magic
					   structure suitable for embedding
					   in other programs. */

extern int apprentice(char*,int);	/* parse /etc/magic */

typedef int * magic_t;

magic_t magic_open(int);

#define MAGIC_NONE	0x00
#define MAGIC_DEBUG	0x01
#define MAGIC_SYMLINK	0x02
#define MAGIC_COMPRESS	0x04
#define MAGIC_DEVICES	0x08
#define MAGIC_MIME	0x10
#define MAGIC_CONTINUE	0x20
#define MAGIC_CHECK	0x40

extern int magic_load(magic_t, char *);
extern char *magic_buffer(magic_t, char *, int);
extern char *magic_file(magic_t, char*);

#ifndef _FILE_D
  extern void *magic;
  extern int nmagic;
 
  extern char *progname;
#endif

#endif/*__LIBMAGIC_D*/
