#include <stdlib.h>
#include <stdarg.h>
#include "form.h"

static char *bfr = 0;
static int bfrsize = 0;

#if __STDC__
void
__lf_form(char *fmt, ...)
#else
void
__lf_form(va_alist)
	va_dcl
#endif
{
    va_list ptr;
    char printbfr[200];

#if __STDC__
    va_start(ptr, fmt);
#else
    char *fmt;

    va_start(ptr);
    fmt = va_arg(ptr, const char *);
#endif

    if (fmt == 0) {
	if (bfr)
	    memset(bfr, 0, bfrsize);
    }
    else {
	va_start(ptr, fmt);
	vsprintf(printbfr, fmt, ptr);
	va_end(ptr);
	if (bfrsize == 0) {
	    bfr = malloc(sizeof printbfr);
	    bfr[0] = 0;
	    bfrsize = sizeof printbfr;
	}
	else if (bfrsize < strlen(bfr) + strlen(printbfr)) {
	    bfrsize += strlen(printbfr);
	    bfr = realloc(bfr, bfrsize);
	}
	if (bfr)
	    strcat(bfr, printbfr);
    }
}


char *
__lf_description()
{
    return bfr;
}
