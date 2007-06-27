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
#ifndef __RULE_D
#define __RULE_D

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

enum rule_action { CAT,		/* copy stdin -> stdout */
                   PIPE,	/* stdin -> filter -> more */
		   FPIPE,	/* stdin -> spool, spool -> filter -> more */
		   FILTER,	/* stdin -> filter -> stdout */
				/* or stdin -> filter ->${FIFO}
				     ${FIFO} -> stdout */
		   FFILTER,	/* stdin -> spool, spool -> filter -> stdout */
		   TEXT,	/* copy stdin -> stdout, \n -> \r\n */
		   POSTSCRIPT,	/* TEXT + "^D" */
		   REJECT };	/* go away */

struct rule {
    char *pattern;		/* pattern to match to do this action */
    regex_t regex;		/* compiled */
    char *hint;			/* the format of the resulting stream */
    struct rule *hinted;	/* a pointer to the rule for that stream */
    enum rule_action action;	/* what we want to do (see above) */
    char **argv;		/* arguments to the action */
    int    argc;		/* # of arguments */
    struct rule *next;		/* next rule in the chain */
} ;

extern struct rule *getline(FILE *);
extern struct rule *rules;

#endif/*__RULE_D*/
