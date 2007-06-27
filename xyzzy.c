/*
 *   Copyright (c) 1999-2000 David Parsons. All rights reserved.
 *   
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. My name may not be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *     
 *  THIS SOFTWARE IS PROVIDED BY DAVID PARSONS ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVID
 *  PARSONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * xyzzy: do the machine incantation for the contents of stdin.
 */

#include "magicfilter.h"
#include <file.h>
#include <string.h>
#include <unistd.h>

char *filetype = 0;

void
xyzzy()
{
    struct rule *r;
    char *ftype;
    char bfr[4096];
    int size;

    /* figure out the file type from the first block
     */
    size = read(0, bfr, sizeof bfr);

    ftype = string_magic(bfr, size);
    if (filetype == 0)
	filetype = strdup(ftype);

    /* see if it matches any of the rules
     */
    for (r = rules; r ; r = r->next)
	if (regexec( &(r->regex), ftype, 0, 0, 0) == 0)
	    plugh(r, bfr, size);

    reject("can't print %s files", ftype);
    exit(1);
}


void
plugh(struct rule *r, char *bfr, int size)
{
    switch (r->action) {
    case FILTER:
    case FFILTER:
	    filter_it(bfr, size, r);
	    break;
    case PIPE:
    case FPIPE:
	    pipe_it(bfr, size, r);
	    break;
    case TEXT:
    case POSTSCRIPT:
    case CAT:
	    cat_it(bfr, size, r);
	    break;
    case REJECT:
	    reject(filetype ? "can't print %s files"
			    : "can't print this file", filetype);
    default:
	    reject("internal processing error (r->action = %d)", r->action);
    }
    exit(1);
}
