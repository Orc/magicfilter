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
 * getline() picks up a magicfilter rule line and passes it back to magicfilter
 *
 * rules are:
 *
 *	/pattern/	action[/hint/]	[arg {arg ...}]
 */

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "rule.h"
#include "magicfilter.h"

static int lineno = 0;	/* line number in the input stream */
static int ruleno = 0;	/* how manu rules have we crossed? */

static struct {
    char *text;
    enum rule_action action;
} mappings[] = { 
    { "cat",           CAT },
    { "pipe",          PIPE },
    { "fpipe",         FPIPE },
    { "filter",        FILTER },
    { "ffilter",       FFILTER },
    { "text",          TEXT },
    { "postscript",    POSTSCRIPT },
    { "reject",        REJECT },
};
#define NRMAPPINGS	(sizeof mappings/sizeof mappings[0])

/*
 * action_p() returns a string describing the action
 */
char *
action_p(enum rule_action action)
{
    int x;

    for (x = 0; x < NRMAPPINGS; x++)
	if (mappings[x].action == action)
	    return mappings[x].text;

    return "unknown";
} /* action_p */


/*
 * getarg() picks an space-delimited argument off the input stream
 */
static char *
getarg(char **p)
{
    char *ret, *r;
    char quot;

    /* skip whitespace; bail if we fall off the end of the string
     */
    while (**p && isspace(**p))
	++(*p);

    if (**p == 0)
	return 0;

    ret = r = *p;
    while (**p && !isspace(**p)) {
	if (**p == '"' || **p == '\'') {
	    quot = *(*p)++;

	    while (**p != quot) {
		if (**p == 0) {
		    fprintf(stderr,
			    "getarg: unterminated string at %d\n", lineno);
		    exit(1);
		}
		if (**p == '\\' && (*p)[1])
		    (*p)++;
		*r++ = *(*p)++;
	    }
	    (*p)++;
	}
	else {
	    if (**p == '\\' && (*p)[1]) {
		int ct;
		char c;

		(*p)++;
		switch (**p) {
		    case 'r':	*r = '\r';	break;
		    case 'n':	*r = '\n';	break;
		    case 't':	*r = '\t';	break;
		    case 'e':	*r = '\033';	break;
		    case 'v':	*r = '\v';	break;	/* vtab */
		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
				for (ct = 0; isdigit(**p); ++(*p))
				    ct = (ct << 3) + (**p - '0');
				*r = ct;
				break;
		    default:	*r = **p;	break;
		}
		(*p)++;
		r++;
	    }
	    else {
		if (r < *p)
		    *r = **p;
		r++, (*p)++;
	    }
	}
    }
    if (**p)
	(*p)++;
    *r = 0;

    return ret;
} /* getarg */


/*
 * vis() shows a string in strictly human-readable form
 */
void
vis(FILE *output, unsigned char *s)
{
    char *pat;
    for ( ; *s ; ++s) {
	switch (*s) {
	    case '\r':	fputs("\\r", output);	continue;
	    case '\n':	fputs("\\n", output);	continue;
	    case '\t':	fputs("\\t", output);	continue;
	    case '\v':	fputs("\\v", output);	continue;
	    case 033 :	fputs("\\e", output);	continue;
	}
	if (isprint(*s))
	    fputc(*s, output);
	else
	    fprintf(output, "\\%03o", *s);
    }
} /* vis */


/*
 * getline() gets a rule off the input stream
 */
struct rule *
getline(FILE *input)
{
    static char *bfr = 0;	/* a buffer for reading lines into */
    static int buflen = 0;	/* size of that buffer */

    struct rule *r;		/* the rule we're looking at */

    int idx, err;		/* current char in line, error return flag */
    char *p, *q, *hint;		/* pointers for disassembling the line */
    int c;

    if (buflen == 0) {
	/* initialize the line buffer the first time in
	 */
	if ( (bfr = malloc(buflen=80)) == 0) {
	    perror("getline::malloc");
	    exit(1);
	}
    }

    if ( (r = calloc(1, sizeof *r)) == 0) {
	perror("getline::calloc");
	exit(1);
    }

/* read in a line of arbitrary size.
 */
again:
    if ( (idx = getaline(input, &bfr, &buflen, &lineno)) == EOF)
	return 0;

/* advance to the first token or comment
 */
    for (p = bfr; isspace(*p) && *p != '#' && *p; ++p)
	;

/* nothing of interest on this line ?
 */
    if (*p == '#' || *p == 0)
	goto again;

/* line starts with /pattern/ -- scan and compile it.
 */
    if (*p != '/') {
	fprintf(stderr, "malformed pattern (%s) at line %d\n",  p, lineno);
	goto again;
    }
    q = ++p;
    while (*p && *p != '/') {
	if (*p == '\\' && p[1])
	    ++p;
	++p;
    }
    if (*p != '/') {
	fprintf(stderr, "malformed pattern (%s) at line %d\n", p, lineno);
	goto again;
    }
    *p++ = 0;

    if ( (err = regcomp( &(r->regex), q, REG_EXTENDED|REG_ICASE)) != 0) {
	regerror(err, &(r->regex), bfr, buflen);
	fprintf(stderr, "%s", bfr);
	goto again;
    }

    if ( (r->pattern = strdup(q)) == 0) {
	perror("getline::strdup");
	exit(1);
    }

/* continues with an action and an optional output hint.  Pick them up
 * and check the action out (the hints are linked after the whole pattern
 * file has been read)
 */
    while (*p && isspace(*p))
	++p;

    if (*p == 0) {
	regfree( &(r->regex) );
	fprintf(stderr, "missing action at line %d\n", lineno);
	goto again;
    }

    hint = 0;
    for (q = p; *p && isalnum(*p); ++p)
	;

    if (*p == '/') {
	*p++ = 0;
	for (hint = p; *p && !isspace(*p); ++p)
	    ;
	if (p[-1] != '/') {
	    regfree( &(r->regex) );
	    fprintf(stderr, "malformed hint at line %d\n", lineno);
	    goto again;
	}
	p[-1] = 0;
    }
    else if (*p)
	*p++ = 0;

    for (idx = 0; idx < NRMAPPINGS; idx++)
	if (strcasecmp(q, mappings[idx].text) == 0) {
	    r->action = mappings[idx].action;
	    break;
	}

    if (idx == NRMAPPINGS) {
	regfree( &(r->regex) );
	fprintf(stderr, "unknown action at line %d\n", lineno);
	goto again;
    }

    if (hint)
	if ( (r->hint = strdup(hint)) == 0) {
	    perror("getline::strdup");
	    exit(1);
	}

/* finally any text associated with the action
 */
    while (*p && isspace(*p))
	++p;

    r->argv = malloc(1);
    while (q = getarg(&p)) {
	r->argv = realloc(r->argv, (3 + r->argc) * sizeof r->argv[0]);
	if (r->argv == 0) {
	    perror("getline::realloc");
	    exit(1);
	}
	if ( (r->argv[r->argc++] = strdup(q)) == 0) {
	    perror("getline::strdup");
	    exit(1);
	}
    }
    r->argv[r->argc] = 0;

    if (debug > 2) {
	int x;

	fprintf(stderr, "rule %d:\n"
			"        /%s/ -> %s",
			    ruleno, r->pattern, action_p(r->action));
	if (r->hint)
	    fprintf(stderr, "/%s/", r->hint);
	fputc('\n', stderr);

	for (x = 0; x < r->argc; x++) {
	    fprintf(stderr, "        arg %d is `", x);
	    vis(stderr, r->argv[x]);
	    fprintf(stderr, "'\n");
	}
    }
    ++ruleno;
    return r;
} /* getline */
