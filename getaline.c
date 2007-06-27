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
#include <stdio.h>
#include <malloc.h>

int
getaline(FILE *input, char **bfr, int *buflen, int *lineno)
{
    int idx;
    int c;

    for (idx = 0; (c = getc(input)) != EOF; idx++) {
	if (idx == (*buflen) ) {
	    if ( ((*bfr) = realloc( (*bfr), (*buflen) *= 2)) == 0) {
		perror("getaline::realloc");
		exit(1);
	    }
	}
	if (c == '\n') {
	    (*lineno)++;
	    if (idx == 0 || (*bfr)[idx-1] != '\\')
		break;
	    /* bail on newline, unless the previous character was a \
	     */
	    idx -= 2;
	}
	else
	    (*bfr)[idx] = c;
    }

    if (c == EOF)	/* die if we ran out of file in the middle of a line */
	return EOF;
    (*bfr)[idx] = 0;
    return idx;
} /* getaline */
