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
 * actions() do various filtering actions.   Note that NONE of the various
 *           _it() functions ever return -- they simply finish up what they're
 *           doing, then exit.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#include "magicfilter.h"
#include "rule.h"


/*
 * have_fifo() checks the argument list of a rule to see if it includes
 *             a ${FIFO};  if it does, it creates a fifo, rewrites the
 *             argument list to include the fifo name, and returns the
 *             name of the fifo.
 */
static char *
have_fifo(struct rule *r)
{
    int i;
    char *f, *q, *p, *w;
    char *fifo;


    for (i=1; i < r->argc; i++)
	if ( (f = strstr(r->argv[i], "${FIFO}")) != 0) {
	    if ((fifo = tempnam("/tmp", ".ff")) == 0) {
		perror("with_fifo:tempnam");
		exit(1);
	    }
	    if (mkfifo(fifo, 0600) != 0) {
		perror("with_fifo:mkfifo");
		exit(1);
	    }
	    if ( (p = malloc(strlen(r->argv[i]) + strlen(fifo))) == 0) {
		perror("with_fifo:malloc");
		exit(1);
	    }
	    /* modify the first argument that contains ${FIFO}; any more
	     * ${FIFO}'s will be cheerfully ignored
	     */
	    for (w=p, q=r->argv[i]; q<f; ++q)
		*w++ = *q;
	    for (q=fifo; *q; ++q)
		*w++ = *q;
	    for (q = f+7; *q; ++q)
		*w++ = *q;
	    *w = 0;
	    r->argv[i] = p;
	    return fifo;
	}
    return 0;
} /* have_fifo */


/*
 * copyio() copies input to output, optionally converting \n to \r\n
 * it returns the last character copied, in case the caller needs to
 * do a postscript tweak.
 */
static int
copyio(char *blk, int size, int input, int output, struct rule *r)
{
    char bfr[1024], out[1024+1];
    int idx;
    int odx;
    int lastchar = 0;

    switch (r ? (r->action) : CAT) {
    case TEXT:
    case POSTSCRIPT:
	odx = 0;

	do {
	    if (size > 0)
		lastchar = blk[size-1];
	    for (idx = 0; idx < size; idx++) {
		if (odx >= 1024) {
		    write(output, out, odx);
		    odx = 0;
		}
		if (blk[idx] == '\n')
		    out[odx++] = '\r';
		out[odx++] = blk[idx];
	    }
	    size = read(input, bfr, sizeof bfr);
	    blk = bfr;
	} while (size > 0);

	if (odx > 0)
	    write(1, out, odx);
	break;
    default:
	if (size > 0)
	    write(output, blk, size);
	while ((size = read(input, bfr, sizeof bfr)) > 0)
	    write(output, bfr, size);
	break;
    }
    return lastchar;
} /* copyio */


/*
 * spool() copies the input out to a workfile, and returns its name
 */
static int
spool(char *bfr, int len, char **name)
{
#define TEMPLATE	"/tmp/magicXXXXXX"
    static char tmpfile[sizeof(TEMPLATE) + 1];
    int fd;

    strcpy(tmpfile, TEMPLATE);

    if ( (fd = mkstemp(tmpfile)) == -1) {
	perror("spool::mkstemp");
	exit(1);
    }
    *name = tmpfile;

    copyio(bfr,len,0,fd,0);

    lseek(fd, 0, 0);
    return fd;
} /* spool */


/*
 * execute_it() runs a program, replacing $FILE arguments with
 * the passed-in filename
 */
static int
execute_it(struct rule *r, char *file)
{
    pid_t child;
    int status;

    if ( (child = fork()) < 0) {
	perror("execute_it::fork");
	exit(1);
    }
    if (child == 0) {
	int x;

	for (x=0; x < r->argc; x++)
	    if (strcmp(r->argv[x], "$FILE") == 0)
		r->argv[x] = file;

	trace("execute_it", r);
	execvp(r->argv[0], r->argv);
	perror(r->argv[0]);
	exit(1);
    }
    waitpid(child, &status, 0);
    unlink(file);

    exit( !!(WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0) );
} /* execute_it */


/*
 * pipe_it(): process text and refeed it to magicfilter.
 */
void
pipe_it(char *blk, int size, struct rule *r)
{
    int io[2];
    int sio[2];
    pid_t spawn;
    int status;
    char *file = 0;
    pid_t sysproc;

    if (pipe(io) == -1) {
	perror("pipe::pipe");
	exit(1);
    }

    switch (spawn = fork()) {
    case -1:
	perror("pipe::fork");
	exit(1);

    default:
	/* parent -- run xyzzy against the output of the filter, then exit
	 */
	dup2(io[0], 0);
	close(io[1]);
	level++;
	if (r->hinted)
	    plugh(r->hinted, 0, 0);
	else
	    xyzzy();
	exit(0);	/* be paranoid */

    case 0:
	if (r->action == FPIPE) {
	    /* fpipe spools stdin to a file before processing, so it's easy
	     */
	    int fd;

	    fd = spool(blk, size, &file);
	    dup2(fd, 0);
	    dup2(io[1], 1);
	    close(io[0]);
	    execute_it(r, file);
	}
	else if (size == 0 || (isatty(0) == 0 && lseek(0, 0, 0) != -1)) {
	    /* if we got here via hinting, or our input is a file,
	     * just rewind and fork off the filter
	     */
	    dup2(io[1], 1);
	    close(io[0]);
	    trace("rewind", r);
	    execvp(r->argv[0], r->argv);
	    perror(r->argv[0]);
	    exit(1);
	}
	else {
	    /* we're reading input from a pipe or a terminal, so we
	     * must refork again so we can shovel what we got plus
	     * what we didn't get into the filter.
	     */
	    if (pipe(sio) == -1) {
		perror("pipe::pipe");
		exit(1);
	    }
	    switch (sysproc = fork()) {
	    case -1:
		perror("pipe::fork");
		exit(1);
	    case 0:
		/* child -- execvp() the process
		 */
		close(sio[1]);
		close(io[0]);
		dup2(sio[0], 0);
		dup2(io[1], 1);
		trace("pipe", r);
		execvp(r->argv[0], r->argv);
		perror(r->argv[0]);
		exit(1);
	    default:
		/* parent -- copy blk + stdin -> stdout
		 */
		close(io[0]);
		close(io[1]);
		close(sio[0]);
		copyio(blk, size, 0, sio[1], 0);
		close(sio[1]);
		waitpid(spawn, &status, 0);
		exit( !!(WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0) );
	    }
	}
    }
} /* pipe_it */


/*
 * fifo_cat() forks, reads from a fifo, then deletes the fifo and exits
 */
void
fifo_cat(char *fifo,struct rule *r)
{
    int stat, f;
    pid_t pid;

    if ((pid=fork()) < 0) {
	perror("fifo_cat: fork");
	exit(1);
    }
    if (pid != 0) {
	/* parent; read 'til done, close fifo */
	f = open(fifo, O_RDONLY);

	trace("fifo_cat", r);
	copyio(0, 0, f, 1, 0);
	close(f);
	unlink(fifo);
	waitpid(pid, &stat, 0);
	exit( !!(WIFEXITED(stat) == 0 || WEXITSTATUS(stat) != 0) );
    }
    /* if child, set stdout to stderr, then return to the parent */
    close(1);
    dup2(2,1);
} /* fifo_cat */


/*
 * filter_it() processes text and dumps it to stdout
 */
void
filter_it(char *blk, int size, struct rule *r)
{
    int fd;
    char *file;
    char bfr[1024];
    int io[2];
    pid_t spawn;
    int status;
    char *fifo;

    /* have_fifo() rewrites the ruleset to include the name of a fifo */
    if ( (fifo = have_fifo(r)) != 0) {
	/* fifo_cat forks off a fifo reader, which reads until
	 * eof, then deletes the fifo and exits
	 */
	fifo_cat(fifo,r);
    }

    if (r->action == FFILTER) {
	/* spool and pipe
	 */
	fd = spool(bfr, size, &file);
	dup2(fd, 0);
	execute_it(r, file);
    }
    else if (size == 0 || (isatty(0) == 0 && lseek(0, 0, 0) != -1)) {
	/* if we got here via hinting, or our input is a file,
	 * just rewind and fork off the filter
	 */
	trace("rewind", r);
	execvp(r->argv[0], r->argv);
	perror(r->argv[0]);
	exit(1);
    }
    /* fork and feed all of our input to the command
     */
    if (pipe(io) == -1) {
	perror("filter::pipe");
	exit(1);
    }

    switch (spawn = fork()) {
    case -1:
	perror("filter::spawn");
	exit(1);
    case 0:
	dup2(io[0], 0);
	close(io[1]);
	trace("pipe", r);
	execvp(r->argv[0], r->argv);
	perror(r->argv[0]);
	exit(1);
    default:
	close(io[0]);
	copyio(blk, size, 0, io[1], 0);
	close(io[1]);
	waitpid(spawn, &status, 0);
	exit( !!(WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0) );
    }
} /* filter_it */


/*
 * cat_it() copies input to output, doing various magic to it
 */
void
cat_it(char *blk, int size, struct rule *r)
{
    int lastchar = 0;

    if (r->argc > 0)
	write(1, r->argv[0], strlen(r->argv[0]));

    trace(0, r);
    lastchar = copyio(blk,size,0,1,r);

    if (r->argc > 1)
	write(1, r->argv[1], strlen(r->argv[1]));
    if (r->action == POSTSCRIPT && r->argc <= 1 && lastchar != '\004')
	write(1, "\004", 1);
    exit(0);
} /* cat_it */
