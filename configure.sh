#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

ac_help='--filterdir=DIR		where to put printer filters (prefix/sbin/printers)
--traditional		build a more traditional sort of magicfilter
--use-local-magic	install a private magic file
--use-fifo		pipe the output from gs through a fifo
--with-lprng		build a stupid filter for use with lprng
--with-papersize=SIZE	set the default paper size for gs/pstext'

LOCAL_AC_OPTIONS='
case X"$1" in
X--filterdir=*)
	    AC_FILTER=`echo "$1" | sed -e 's/^[^=]*=//'`
	    shift 1
	    ;;
X--filterdir)
	    AC_FILTER=$2
	    shift 2
	    ;;
X--traditional)
	    TARGET=traditional
	    shift
	    ;;
*)	    ac_error=1
	    ;;
esac'

# load in the configuration file
#
TARGET=magicfilter
USE_FIFO=T		# default to piping ghostscript via a fifo
. ./configure.inc


# and away we go
#
AC_INIT magicfilter

AC_PROG_CC

AC_SUB filterdir ${AC_FILTER:-$AC_PREFIX/sbin/printers}

if [ "$USE_FIFO" = "T" ]; then
    AC_SUB GSOUT '${FIFO}'
else
    AC_SUB GSOUT '-'
fi

if [ ! "$USE_LOCAL_MAGIC" ]; then
    case $ac_os in
    [Ff]ree[Bb][Ss][Dd])magicpath=/etc:/usr/etc:/usr/share/misc ;;
    *)			magicpath=/etc:/usr/etc: ;;
    esac

    # check to see if the system magic file is moderately recent
    #
    trap "rm -f $$" 1 2 3 9 15
    echo "@PJL JOB" > $$
    F=`file $$ 2>/dev/null | grep -i pjl`
    echo "%PDF-1.1" > $$
    G=`file $$ 2>/dev/null | grep -i pdf`

    if [ "$F" -a "$G" ]; then
	save_AC_PATH=$AC_PATH
	AC_PATH=$magicpath MF_PATH_INCLUDE MAGIC -r magic || USE_LOCAL_MAGIC=T
	AC_PATH=$save_AC_PATH
    else
	LOG "file(1) is too old -- using private magic file"
	USE_LOCAL_MAGIC=T
    fi
else
    LOG "Using private magic file $AC_CONFDIR/mf.magic"
fi
rm -f $$
trap 1 2 3 9 15

if [ "$USE_LOCAL_MAGIC" ]; then
    # if we're using local magic, manually write the substitution
    # information into the config files
    AC_CONFIG MAGIC "$AC_CONFDIR"/mf.magic
    AC_SUB INSTALL_MAGIC "$PROG_INSTALL"
else
    AC_SUB INSTALL_MAGIC ":"
fi
AC_SUB DO_WHAT install-$TARGET

# AC_PROG_LN_S
# AC_PROG_YACC

if [ ! "$WITH_LPRNG" ]; then
    S=`lpq -V 2>/dev/null | head -1 | grep -i lprng`

    if [ "$S" ]; then
	LOG "Found LPRng -- building stupid filters"
	WITH_LPRNG=1
    fi
fi
if [ "$WITH_LPRNG" ]; then
    AC_SUB    "LPD_OPTS" "'s/^LPNG//p'"
    AC_DEFINE "WITH_LPRNG" "1"
else
    AC_SUB    "LPD_OPTS" "'s/^LPR //p'"

    # don't bother to check for options.h unless we're not
    # being built against lprng -- remember that magicfilter
    # for lprng doesn't have any options.
    #
    if AC_CHECK_HEADERS basis/options.h; then
	if LIBS="-lbasis" AC_CHECK_FUNCS x_getopt; then
	    AC_LIBS="$AC_LIBS -lbasis"
	    AC_SUB XGETOPT
	    HAVE_XGETOPT=T
	fi
    fi
    test "$HAVE_XGETOPT" || AC_SUB XGETOPT options.o
fi

AC_CHECK_HEADERS malloc.h || AC_CHECK_HEADERS sys/malloc.h

AC_CHECK_FUNCS basename

MF_PATH_INCLUDE RANLIB ranlib true || AC_CONFIG RANLIB ':'
MF_PATH_INCLUDE M4 m4 || AC_FAIL "magicfilter requires m4"

MF_PATH_INCLUDE GS gs
if MF_PATH_INCLUDE NENSCRIPT nenscript enscript; then
    TLOGN "checking if $CF_NENSCRIPT supports the -q flag..."
    __v=`$CF_NENSCRIPT -q -p - </dev/null`

    if [ \( "$?" -ne 0 \) -o "$__v" ]; then
	TLOG " no"
	AC_SUB NENSCRIPT_QUIET ''
    else
	TLOG " yes"
	AC_SUB NENSCRIPT_QUIET -q
    fi
else
    AC_SUB NENSCRIPT_QUIET ''
    MF_PATH_INCLUDE PSTEXT pstext
fi

paper=$WITH_PAPERSIZE

if [ "$CF_GS" -a "$paper" ]; then
    if ! echo "quit" | $CF_GS -sPAPERSIZE=$paper -dNODISPLAY -; then
	AC_FAIL "$CF_GS cannot write to $paper sized paper"
	unset paper
    fi
fi

if [ "${CF_PSTEXT:-$CF_NENSCRIPT}" -a "$paper" ]; then
    PROG=${CF_PSTEXT:-$CF_NENSCRIPT}
    if ! $PROG -T$paper </dev/null; then
	AC_FAIL "$PROG cannot write to $paper sized paper"
	unset paper
    fi
fi

test "$paper" && AC_DEFINE PAPERSIZE \"$WITH_PAPERSIZE\"

MF_PATH_INCLUDE GZIP gzip gzcat || MF_PATH_INCLUDE ZCAT zcat
MF_PATH_INCLUDE BZIP bzip2
MF_PATH_INCLUDE UNCOMPRESS uncompress
MF_PATH_INCLUDE DVIPS dvips
MF_PATH_INCLUDE PNMTOPS pnmtops
MF_PATH_INCLUDE GIFTOPPM giftopnm giftoppm
MF_PATH_INCLUDE G3TOPBM g3topbm
MF_PATH_INCLUDE DJPEG djpeg
MF_PATH_INCLUDE PNGTOPNM pngtopnm
MF_PATH_INCLUDE SGITOPNM sgitopnm
MF_PATH_INCLUDE TIFFTOPNM tifftopnm
MF_PATH_INCLUDE BMPTOPPM bmptopnm bmptoppm
MF_PATH_INCLUDE RASTTOPNM rasttopnm
MF_PATH_INCLUDE FIG2DEV fig2dev
MF_PATH_INCLUDE ACROREAD acroread
 
# MF_PROG_GNU_ZCAT($ZCAT)

save_AC_PATH=$AC_PATH
AC_PATH=/usr/lib:/usr/sbin:/usr/bin:/bin:/sbin MF_PATH_INCLUDE SENDMAIL sendmail smail mail Mail
AC_PATH=$save_AC_PATH

AC_CHECK_HEADERS memory.h
AC_CHECK_HEADERS paths.h
AC_CHECK_HEADERS stdlib.h
AC_CHECK_HEADERS unistd.h
AC_HEADER_SYS_WAIT

AC_C_CONST
AC_TYPE_PID_T
AC_CHECK_FUNCS dup2
AC_CHECK_FUNCS tmpnam
AC_CHECK_FUNCS waitpid
AC_CHECK_FUNCS wait4
AC_CHECK_FUNCS unsetenv
AC_CHECK_FUNCS setenv
AC_CHECK_FUNCS setlinebuf

AC_SUB MAGICFILTER ${AC_EXEC}/magicfilter
AC_SUB VERSION `test -f VERSION && cat VERSION`

AC_OUTPUT Makefile file/Makefile magicfilter.8templ magicfilter.5 magicfilter.h magicfilter-t.5 magic.m4 contrib/mfconfig.pl args.c

