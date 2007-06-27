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
 * magicfilter-2: magicfilter, reinvented, because the wheel is already there!
 *
 * concatenate the config file and $magic, then pipe them into m4
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "rule.h"
#include "magicfilter.h"

int debug = 0;
char *progname;
int cflag = 0;

#ifndef NOFE
/* fe() concatenates stdin with the m4 template, writes it to stdout.
 */
void
fe(int input, int output)
{
    char blk[1024];
    int size, status, i;
    int fd;

    dup2(input, 0);
    dup2(output, 1);

    while ((size = read(0, blk, sizeof blk)) > 0)
	if (write(1, blk, size) != size) {
	    perror("fe::write");
	    exit(1);
	}

#ifdef PAPERSIZE
    sprintf(blk, "define(papersize,`%s')dnl\n", PAPERSIZE);
    write(1,blk,strlen(blk));
#endif
    if ( (fd = open(MAGICFILTER_CONF, O_RDONLY)) >= 0) {
	while ((size = read(fd, blk, sizeof blk)) > 0)
	    if (write(1, blk, size) != size) {
		perror("fe::write");
		exit(1);
	    }
	close(fd);
    }
    else
	for (i=0; m4template[i]; i++)
	    write(1, m4template[i], strlen(m4template[i]));

    exit(0);
} /* fe */


/* m4() runs m4 on input.
 */
void
m4(int input, int output)
{
    dup2(input, 0);
    dup2(output, 1);

    execlp("m4", "m4", 0);
    perror("m4::execlp");
    exit(1);
} /* m4 */
#endif


struct rule *rules = 0;
struct rule *tail = 0;


/* be() takes a processed patterns file and digests it for printing purposes
 */
void
be(int input, int output)
{
    int rulecount = 0;
    struct rule *r, *h;
    FILE *f;

    if ( (f = fdopen(input, "r")) == 0) {
	perror("be::fdopen");
	exit(1);
    }

    /* read rules
     */
    while ( (r = getline(f)) != 0) {
	rulecount++;
	if (tail) {
	    tail->next = r;
	    tail = r;
	    tail->next = 0;
	}
	else
	    rules = tail = r;
    }

    /* assign hints
     */
    for (r = rules; r; r = r->next)
	if (r->hint)
	    for (h=rules; h; h = h->next)
		if (strcmp(r->hint, h->pattern) == 0) {

		    if (debug > 1)
			fprintf(stderr, " hint /%s/ -> /%s/\n",
					r->pattern, h->pattern);

		    if (h->action == CAT && h->argc == 0) {
			/* if this rule points at a cat rule and that
			 * action isn't passing any additional text,
			 * convert pipe actions to filters and get
			 * rid of the middleman.
			 */
			switch (r->action) {
			case PIPE:	r->action = FILTER;
					break;
			case FPIPE:	r->action = FFILTER;
					break;
			}
		    }

		    r->hinted = h;
		    break;
		}
    fclose(f);
} /* be */


/*
 * magicfilter:  transparently print a variety of file formats
 */
main(int argc, char **argv)
{

    pid_t child;
#ifndef NOFE
    int input[2], output[2];
#endif
    int script;
    int status;
    int x;

    script = getoptionsandscript(argc, argv);

    if (cflag) {
	struct rule dummy;

	memset(&dummy, 0, sizeof dummy);
	dummy.action = CAT;

	cat_it(0, 0, &dummy);
	exit(0);
    }

#ifndef NOFE
    if (pipe(input) == -1 || pipe(output) == -1) {
	perror("pipe");
	exit(1);
    }

    /* split ourselves into 3 pieces:
     *	1) fe, which concatinates stdin and $magic
     *	2) m4, which processes
     *	3) be, which processes the output
     */
    if ((child=fork()) < 0) {
	perror("fork m4");
	exit(1);
    }
    else if (child == 0) {
	if ( (child=fork()) < 0) {
	    perror("fork fe");
	    exit(1);
	}
	if (child == 0) {
	    close(input[0]);
	    close(output[0]);
	    close(output[1]);
	    fe(script,input[1]);
	    exit(0);	/* just in case fe returns */
	}
	close(input[1]);
	close(output[0]);
	m4(input[0], output[1]);
	exit(0);	/* just in case m4 returns */
    }
    close(output[1]);
    close(input[0]);
    close(input[1]);
    be(output[0], 1);

    waitpid(child, &status, 0);
    if (WIFEXITED(status) == 0) {
	fprintf(stderr, "m4 terminated abnormally\n");
	exit(1);
    }
    else if (WEXITSTATUS(status) != 0) {
	fprintf(stderr, "m4 returned status %d\n", WEXITSTATUS(status));
	exit(1);
    }
#else
    be(script, 1);
#endif

    close(script);

    apprentice(PATH_MAGIC, 0);

    umask(077);	/* hide the intermediate files from everyone other than lpd */
    xyzzy();

    exit(0);
} /* magicfilter */
