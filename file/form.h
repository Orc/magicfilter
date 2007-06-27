/*
 * __lf_form() format a buffer for eventual printout or something
 * __lf_description() returns the contents of a __lf_form()ed buffer.
 */
#ifndef _FORM_D
#define _FORM_D

#include "file.private.h"

void __lf_form __P((char *fmt, ...));
char *__lf_description __P((void));

#endif/*_FORM_D*/
