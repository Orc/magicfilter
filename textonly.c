/***********************************************************************
* textonly.c
*
* Simple program that makes sure that stdin is printable stuff.
* If not, output simple hex dump, otherwise copy to stdout.
* Meant to be a final sanity check pipe printer filter.
* 
* Ed Lim  Fri Sep 24 00:56:56 EDT 1999
*
***********************************************************************/


#include <stdio.h>
#include <unistd.h>


/***********************************************************************
* Prototypes
***********************************************************************/
int scan_input(unsigned char *, int);
void hex_dump(unsigned char *, int, int);
void shove_out(unsigned char *, int);


/* will check this many bytes each iteration to determine printability: */
#define CHUNK_SIZE 512

/* can't be sent to printer */
char nonprint[256] = {
 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/* non-visible , for the hexdump part */
char nonscreen[256] = {
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};


/***********************************************************************
* MAIN
***********************************************************************/
int main() {

    int insize = 0;
    unsigned char inbuf[CHUNK_SIZE];
    int total_read = 0;

    for ( ; ; ) {
        insize = read(0, inbuf, CHUNK_SIZE);
        if ( insize <= 0 )  {
            break;
        } else if ( scan_input(inbuf, insize) ) {
            hex_dump(inbuf, insize, total_read);
            break;
        } else {
            shove_out(inbuf, insize);
        }
        total_read += insize;

    }

    return 0;

}


/***********************************************************************
* scan_input
*
* Scan input buffer for illegal characters.
*
* Returns:
* Number of illegal characters found.
*
************************************************************************/
int scan_input(unsigned char *inbuf, int insize) {

    int bad_chars = 0;
    int i;


    for ( i = 0 ; i < insize ; i++ ) {
        bad_chars += nonprint[inbuf[i]];
    }

    return bad_chars;

}


/***********************************************************************
* hex_dump
*
* This will do some sort of hex dump.
*
***********************************************************************/
void hex_dump(unsigned char *stuff_to_dump, int size, int fake_offset) {

    int line_loc, i, this_line_size;
    int dumped = 0;
    int temp;


    /* some header stuff */
    printf ("\n%s\n%s\n\n",
            "Non-printable characters encountered.  Printing aborted.",
            "Offending characters are prepended by '!'.");


    while ( dumped < size ) {

        /* offset number */
        line_loc = printf("%010x:", fake_offset + dumped);

        /* how many characters for this line? */
        this_line_size = (size - dumped) < 16 ? (size - dumped) : 16;

        /* hex part */
        for (i = 0 ; i < this_line_size ; i++) {
            line_loc += printf("%c%02x", 
                    nonprint[stuff_to_dump[dumped + i]] ?  '!' : ' ', 
                    stuff_to_dump[dumped + i]);
        }

        /* ascii part */
        for (i = line_loc ; i < 62 ; i++) printf(" ");
        for (i = 0 ; i < this_line_size ; i++) {
            printf("%c", nonscreen[stuff_to_dump[dumped + i]] ? 
                    '.' : stuff_to_dump[dumped + i]);
        }

        printf("\n");

        dumped += this_line_size;

    }

    return;

}


/***********************************************************************
* shove_out
*
* Output stuff.
*
***********************************************************************/
void shove_out(unsigned char *stuff_to_shove, int size) {

    int this_shovel, shoved = 0;
    unsigned char *ptr = stuff_to_shove;

    while ( shoved < size ) {
        this_shovel = write(1, ptr, (size - shoved));
        if ( this_shovel < 0 ) exit(-1);
        shoved += this_shovel;
        ptr += this_shovel;
    }

    return;

}


/*
* vim: tabstop=4 shiftwidth=4 autoindent expandtab smartindent nohlsearch
*/
