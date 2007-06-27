/*
 * libe:  library interface to libfile
 *
 * exports the following functions
 *
 * magic_zflag(int x)		-- set zflag to x.
 * magic_debug(int x)		-- set debug to x.
 * magic_followlink(int x)	-- set lflag to x.
 * magic_emit_struct(int x)	-- set emit_struct to x.
 * magic_file(void*,char*)	-- run magic on a file.
 * magic_string(void*,char*,int)-- run magic on an array of characters.
 */

#include "file.private.h"
#include "magic.h"
#include "form.h"

int
magic_zflag(int x)
{
    int tmp = __lf_zflag;

    if (x >= 0)
	__lf_zflag = x;
    return tmp;
}

int
magic_debug(int x)
{
    int tmp = __lf_debug;
    if (x >= 0)
	__lf_debug = x;
    return tmp;
}

int
magic_followlink(int x)
{
    int tmp = __lf_lflag;
    if (x >= 0)
	__lf_lflag = x;
    return tmp;
}

int
magic_emit_struct(int x)
{
    int tmp = __lf_emit_struct;
    if (x >= 0)
	__lf_emit_struct = x;
    return tmp;
}

char *
magic_file(magic_t poof, char *file)
{
    return __lf_process(file);
}


char *
magic_buffer(magic_t poof, char *blk, int size)
{
    __lf_form(0);
    __lf_tryit(blk, size, __lf_zflag);
    return __lf_description();
}


magic_t
magic_open(int flags)
{
    int *ret = malloc(1);

    if (ret == 0)
	return 0;

    if (flags & ~(MAGIC_DEBUG|MAGIC_COMPRESS|MAGIC_SYMLINK|MAGIC_CHECK)) {
	/* unsupported flags */
	return 0;
    }

    *ret = flags;
    if (flags & MAGIC_DEBUG)
	magic_debug(1);
    if (flags & MAGIC_COMPRESS)
	magic_zflag(1);
    if (flags & MAGIC_SYMLINK)
	magic_followlink(1);
    return ret;
}


int
magic_load(magic_t poof, char *file)
{
    return apprentice(file ? file : MAGIC, (*poof) & MAGIC_CHECK);
}

void
magic_close(magic_t poof)
{
    free(poof);
}
