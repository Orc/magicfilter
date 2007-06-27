/*
 * libe:  library interface to libfile
 *
 * exports the following functions
 *
 * magic_zflag(int x)		-- set zflag to x.
 * magic_debug(int x)		-- set debug to x.
 * magic_followlink(int x)	-- set lflag to x.
 * magic_emit_struct(int x)	-- set emit_struct to x.
 * file_magic(char*)		-- run magic on a file.
 * string_magic(char*,int)	-- run magic on an array of characters.
 */

#include "file.private.h"
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
file_magic(char *file)
{
    return __lf_process(file);
}


char *
string_magic(char *blk, int size)
{
    __lf_form(0);
    __lf_tryit(blk, size, __lf_zflag);
    return __lf_description();
}
