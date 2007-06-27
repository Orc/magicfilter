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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "magicfilter.h"

#include <basis/options.h>

struct x_option opts[] = {
    { 'c', 'c', 0, 0, "Copy the input to the output without processing" },
    { 'h', 'h', 0, "HOST", "The machine that submitted the job" },
    { 'i', 'i', 0, "INDENT", "page indent ($LPINDENT)" },
    { 'l', 'l', 0, "LENGTH", "lpd length argument (ignored)" },
    { 'n', 'n', 0, "NAME", "The user who submitted the job"},
    { 'w', 'w', 0, "WIDTH", "lpd width argument (ignored)" },
    { 'x', 'x', 0, "WIDTH", "lpd width in pixels (ignored)" },
    { 'y', 'y', 0, "LENGTH", "lpd length in pixels (ignored)" },
    { 'C', 'C', 0, "CLASS", "LPRng classname ($LPCLASS)" },
    { 'F', 'F', 0, "FORMAT", "LPRng format letter ($LPFORMAT)" },
    { 'J', 'J', 0, "JOB", "LPRng jobname ($LPJOB)" },
    { 'K', 'K', 0, "COPIES", "LPRng copy count ($LPCOPIES)" },
    { 'L', 'L', 0, "NAME", "LPRng banner name ($BANNERNAME)" },
    { 'P', 'P', 0, "PRINTER", "LPRng printer name ($PRINTER)" },
    { 'Q', 'Q', 0, "QUEUE", "LPRng queue name ($LPQUEUE)" },
    { 'R', 'R', 0, "INFO",  "LPRng accounting information ($LPACCT)" },
    { 'X', 'X', 0, "OPTIONS", "LPRng options passthrough ($XOPT)" },
    { 'Z', 'Z', 0, "OPTIONS", "LPRng options passthrough ($ZOPT)" },
    { 'd',  0 , "debug", 0, "spit out debugging and trace messages" },
    { '!',  0 , "dump", 0, "Dump the default magicfilter.cf file, then exit" },
    { '@',  0 , "help", 0, "print this help message, then exit" },
} ;
#define NROPTS	(sizeof opts/sizeof opts[0])

#define GETOPT(argc,argv)	x_getopt(argc, argv, NROPTS, opts)
#define OPTIND			x_optind
#define	OPTARG			x_optarg
#define IGNORE			x_opterr = 0

#ifdef HAVE_UNSETENV
#define UNSET(x)	unsetenv(x)
#else
#define	UNSET(x)	putenv(x ## "=")
#endif

#ifdef HAVE_SETENV
#define	SET(x,y)	setenv(x,y,0)
#else
void
SET(char *x,char *y)
{
    char *p = malloc(strlen(x) + strlen(y) + 2);

    sprintf(p, "%s=%s", x, y);

    putenv(p);
}
#endif

#ifdef HAVE_BASENAME
const char *basename(char*);
#endif

int
getoptionsandscript(int argc, char **argv)
{
    char *script = 0;
    int opt;
    int fd;

    progname = "magicfilter";	/* placeholder until the scriptname is 
				 * figured out
				 */

    if (argv[1][0] != '-') {	/* #! hack works.  Goodie */ 
	script = argv[1];
	++argv, --argc;
    }

    /* unset all the LPRng environment variables
     */
    UNSET("LPUSER");
    UNSET("LPHOST");
    UNSET("LPINDENT");
    UNSET("LPCLASS");
    UNSET("LPFORMAT");
    UNSET("LPJOB");
    UNSET("LPCOPIES");
    UNSET("BANNERNAME");
    UNSET("PRINTER");
    UNSET("LPQUEUE");
    UNSET("LPACCT");
    UNSET("ZOPT");

    IGNORE;
    while ( (opt = GETOPT(argc, argv)) != EOF) {
	switch (opt) {
	case 'c':   cflag = 1;
		    break;
	case 'd':   debug++;
		    break;
	case 'n':   SET("LPUSER", OPTARG);
		    break;
	case 'h':   SET("LPHOST", OPTARG);
		    break;
	case 'i':   SET("LPINDENT", OPTARG);
		    break;
	case 'C':   SET("LPCLASS", OPTARG);
		    break;
	case 'F':   SET("LPFORMAT", OPTARG);
		    break;
	case 'J':   SET("LPJOB", OPTARG);
		    break;
	case 'K':   SET("LPCOPIES", OPTARG);
		    break;
	case 'L':   SET("BANNERNAME", OPTARG);
		    break;
	case 'P':   SET("PRINTER", OPTARG);
		    break;
	case 'Q':   SET("LPQUEUE", OPTARG);
		    break;
	case 'R':   SET("LPACCT", OPTARG);
		    break;
	case 'X':   SET("XOPT", OPTARG);
		    break;
	case 'Z':   SET("ZOPT", OPTARG);
		    break;

	case '!':   {   int i;

			for (i=0; m4template[i]; i++)
			    write(1, m4template[i], strlen(m4template[i]));
		    }
		    exit(0);
	case '@':
		    fprintf(stderr, "usage: %s printer-driver [options]\n", progname);
		    showopts(stderr, NROPTS, opts);
		    exit(0);
	}
    }

    if (script == 0)
	if (OPTIND < argc)
	    script = argv[OPTIND];
	else {
	    fprintf(stderr, "%s: no printer driver specified\n", progname);
	    exit(1);
	}

#ifdef HAVE_BASENAME
    progname = basename(script);
#else
    if ( (progname = strrchr(script, '/')) != 0)
	++progname;
    else
	progname = script;
#endif

    fd = open(script, O_RDONLY);

    if (fd < 0) {
	perror(script);
	exit(1);
    }
    return fd;
} /* getoptionsandscript */
