#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * Copyright (c) 1991, 1992 Sam Leffler
 * Copyright (c) 1991, 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * Program to construct Group 3 & Group 4 decoding tables.
 *
 * This code is derived from code by Michael P. Marking.  In
 * particular, the algorithms to generate the null_mode and
 * horiz_mode state tables are his.  See the comments below
 * for more information.
 *
 * BEGIN (from the original source)
 LEGAL
 *      Copyright 1989, 1990 Michael P. Marking, Post Office Box 8039,
 *      Scottsdale, Arizona 85252-8039. All rights reserved.
 *
 *      License is granted by the copyright holder to distribute and use this
 *      code without payment of royalties or the necessity of notification as
 *      long as this notice (all the text under "LEGAL") is included.
 *
 *      Reference: $Id$
 *
 *      This program is offered without any warranty of any kind. It includes
 *      no warranty of merchantability or fitness for any purpose. Testing and
 *      suitability for any use are the sole responsibility of the user.
 *
 INFORMATION
 *      Although there is no support offered with this program, the author will
 *      endeavor to correct errors. Updates will also be made available from
 *      time to time.
 *
 *      Contact: Michael P. Marking, Post Office Box 8039, Scottsdale, Arizona
 *      85252-8039 USA. Replies are not guaranteed to be swift. Beginning
 *      July 1990, e-mail may be sent to uunet!ipel!marking.
 *
 *      Also beginning in July 1990, this code will be archived at the
 *      ipel!phoenix BBS in file g3g4.zoo. The 24-hour telephone number
 *      for 300/1200/2400 is (602)274-0462. When logging in, specify user
 *      "public", system "bbs", and password "public".
 *
 *      This code is also available from the C Users Group in volume 317.
 *
 * END (from the original source)
 */
#include <stdio.h>
#include <stdlib.h>
#include "tiffcomp.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define WHITE   0
#define BLACK   1

/*
 * G3 2D and G4 decoding modes.  Note that
 * the vertical modes are ordered so that
 * (mode - MODE_VERT_V0) gives the vertical
 * adjustment for the b1 parameter.
 */
#define MODE_NULL       0
#define MODE_PASS       1
#define MODE_HORIZ      2
#define MODE_VERT_VL3   3
#define MODE_VERT_VL2   4
#define MODE_VERT_VL1   5
#define MODE_VERT_V0    6
#define MODE_VERT_VR1   7
#define MODE_VERT_VR2   8
#define MODE_VERT_VR3   9
#define MODE_UNCOMP     10
#define MODE_ERROR      11
#define MODE_ERROR_1    12

unsigned long
append_0(unsigned long prefix)
{
    return (prefix + (1L<<16));
}

unsigned long
append_1(unsigned long prefix)
{
    static unsigned short prefix_bit[16] = {
        0x8000, 0x4000, 0x2000, 0x1000,
        0x0800, 0x0400, 0x0200, 0x0100,
        0x0080, 0x0040, 0x0020, 0x0010,
        0x0008, 0x0004, 0x0002, 0x0001
    };
    unsigned char len = (prefix >> 16) & 0xf;
    return (append_0(prefix) + prefix_bit[len]);
}

#define G3CODES
#include "t4.h"

short
search_table(unsigned long prefix, const tableentry* tab, int n)
{
    unsigned short len = (prefix >> 16) & 0xf;
    unsigned short code = (prefix & 0xffff) >> (16 - len);

    while (n-- > 0) {
        if (tab->length == len && tab->code == code)
            return ((short) tab->runlen);
        tab++;
    }
    return (G3CODE_INCOMP);
}

#define NCODES(a)       (sizeof (a) / sizeof (a[0]))
short
white_run_length(unsigned long prefix)
{
    return (search_table(prefix, TIFFFaxWhiteCodes, NCODES(TIFFFaxWhiteCodes)));
}

short
black_run_length(unsigned long prefix)
{
    return (search_table(prefix, TIFFFaxBlackCodes, NCODES(TIFFFaxBlackCodes)));
}
#undef NCODES

#define MAX_NULLPREFIX  200     /* max # of null-mode prefixes */
typedef unsigned char NullModeTable[MAX_NULLPREFIX][256];
#define MAX_HORIZPREFIX 250     /* max # of incomplete 1-D prefixes */
typedef unsigned char HorizModeTable[MAX_HORIZPREFIX][256];

  /* the bit string corresponding to this row of the decoding table */
long    null_mode_prefix[MAX_NULLPREFIX];
NullModeTable null_mode;                /* MODE_*, indexed by bit and byte */
NullModeTable null_mode_next_state;     /* next row of decoding tables to use */
  /* number of prefixes or rows in the G4 decoding tables */
short   null_mode_prefix_count = 0;

/*
 * 2D uncompressed mode codes.  Note
 * that two groups of codes are arranged
 * so that the decoder can caluclate the
 * length of the run by subtracting the
 * code from a known base value.
 */
#define UNCOMP_INCOMP   0
/* runs of [0]*1 */
#define UNCOMP_RUN0     1
#define UNCOMP_RUN1     2
#define UNCOMP_RUN2     3
#define UNCOMP_RUN3     4
#define UNCOMP_RUN4     5
#define UNCOMP_RUN5     6
#define UNCOMP_RUN6     7
/* runs of [0]* w/ terminating color */
#define UNCOMP_TRUN0    8
#define UNCOMP_TRUN1    9
#define UNCOMP_TRUN2    10
#define UNCOMP_TRUN3    11
#define UNCOMP_TRUN4    12
/* special code for unexpected EOF */
#define UNCOMP_EOF      13
/* invalid code encountered */
#define UNCOMP_INVALID  14

long    uncomp_mode_prefix[MAX_NULLPREFIX];
NullModeTable uncomp_mode;
NullModeTable uncomp_mode_next_state;
short   uncomp_mode_prefix_count = 0;

/*
 * Decoding action values for horiz_mode.
 */
#define ACT_INCOMP      0               /* incompletely decoded code */
#define ACT_INVALID     1               /* invalide code */
#define ACT_WRUNT       2               /* terminating white run code */
#define ACT_WRUN        65              /* non-terminating white run code */
#define ACT_BRUNT       106             /* terminating black run code */
#define ACT_BRUN        169             /* non-terminating black run code */
#define ACT_EOL         210             /* end-of-line code */
HorizModeTable horiz_mode;

short
horiz_mode_code_black(short runlen)
{
    return (runlen < 64 ? runlen + ACT_BRUNT : (runlen / 64) + ACT_BRUN);
}

short
horiz_mode_code_white(short runlen)
{
    return (runlen < 64 ? runlen + ACT_WRUNT : (runlen / 64) + ACT_WRUN);
}

/*
 * If the corresponding horiz_mode entry is ACT_INCOMP
 * this entry is a row number for decoding the next byte;
 * otherwise, it is the bit number with which to continue
 * decoding the next codeword.
 */
HorizModeTable horiz_mode_next_state;
                /* prefixes corresponding to the rows of the decoding table */
long    horiz_mode_prefix[MAX_HORIZPREFIX];
                /* color of next run, BLACK or WHITE */
char    horiz_mode_color[MAX_HORIZPREFIX];
short   horiz_mode_prefix_count = 0;

static  unsigned char bit_mask[8] =
    { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void    build_null_mode_tables(void);
short   find_horiz_mode_prefix(long, char);
short   find_null_mode_prefix(long);
short   null_mode_type(long);
void    build_horiz_mode_tables(void);
short   horiz_mode_code_black(short);
short   horiz_mode_code_white(short);
void    build_uncomp_mode_tables(void);
void    write_tables(FILE*);

int     verbose = FALSE;
char    *storage_class = "";
int     packoutput = TRUE;

void
main(int argc, char** argv)
{
    while (argc > 1 && argv[1][0] == '-') {
        if (strcmp(argv[1], "-v") == 0) {
            verbose = TRUE;
            argc--, argv++;
        } else if (strcmp(argv[1], "-c") == 0) {
            storage_class = "const ";
            argc--, argv++;
        } else if (strcmp(argv[1], "-p") == 0) {
            packoutput = FALSE;
            argc--, argv++;
        }
    }
    build_null_mode_tables();           /* null mode decoding tables */
    if (verbose) {
        fprintf(stderr, "%d null mode prefixes defined\n",
            (int) null_mode_prefix_count);
        fprintf(stderr, "building uncompressed mode scripts...\n");
    }
    build_uncomp_mode_tables();         /* uncompressed mode decoding tables */
    if (verbose) {
        fprintf(stderr, "%d uncompressed mode prefixes defined\n",
            (int) uncomp_mode_prefix_count);
        fprintf(stderr, "building 1D scripts...\n");
    }
    build_horiz_mode_tables();          /* 1D decoding tables */
    if (verbose)
        fprintf(stderr, "%d incomplete prefixes defined\n",
            (int) horiz_mode_prefix_count);
    write_tables(stdout);
    exit(0);
}

void
write_null_mode_table(FILE* fd, NullModeTable table, char* name)
{
    int i, j, lastNonZero;
    char* outersep;
    char* sep;

    fprintf(fd, "%su_char\t%s[%d][256] = {", storage_class,
        name, (int) null_mode_prefix_count);
    outersep = "";
    if (!packoutput) {
        for (i = 0; i < null_mode_prefix_count; i++) {
            fprintf(fd, "%s\n/* prefix %d */ {\n", outersep, i);
            sep = "    ";
            for (j = 0; j < 256; j++) {
                fprintf(fd, "%s%2d", sep, (int) table[i][j]);
                if (((j+1) % 16) == 0) {
                    fprintf(fd, ", /* %3d-%3d */\n", j-15, j);
                    sep = "    ";
                } else
                    sep = ",";
            }
            fprintf(fd, "}");
            outersep = ",";
        }
    } else {
        for (i = 0; i < null_mode_prefix_count; i++) {
            fprintf(fd, "%s{\n", outersep);
            for (j = 255; j > 0; j--)
                if (table[i][j] != 0)
                    break;
            sep = "";
            lastNonZero = j;
            for (j = 0; j <= lastNonZero; j++) {
                fprintf(fd, "%s%d", sep, (int) table[i][j]);
                if (((j+1) % 24) == 0)
                    putc('\n', fd);
                sep = ",";
            }
            fprintf(fd, "}");
            outersep = ",";
        }
    }
    fprintf(fd, "\n};\n");
}

void
write_horiz_mode_table(FILE* fd, HorizModeTable table, char* name)
{
    int i, j, lastNonZero;
    char* outersep;
    char* sep;

    fprintf(fd, "%s u_char\t%s[%d][256] = {", storage_class,
        name, (int) horiz_mode_prefix_count);
    outersep = "";
    if (!packoutput) {
        for (i = 0; i < horiz_mode_prefix_count; i++) {
            fprintf(fd, "%s\n/* prefix %d */ {\n", outersep, i);
            sep = "    ";
            for (j = 0; j < 256; j++) {
                fprintf(fd, "%s%3d", sep, (int) table[i][j]);
                if (((j+1) % 14) == 0) {
                    fprintf(fd, ", /* %3d-%3d */\n", j-13, j);
                    sep = "    ";
                } else
                    sep = ",";
            }
            fprintf(fd, "\n}");
            outersep = ",";
        }
    } else {
        outersep = "";
        for (i = 0; i < horiz_mode_prefix_count; i++) {
            fprintf(fd, "%s{\n", outersep);
            for (j = 255; j > 0; j--)
                if (table[i][j] != 0)
                    break;
            sep = "";
            lastNonZero = j;
            for (j = 0; j <= lastNonZero; j++) {
                fprintf(fd, "%s%d", sep, (int) table[i][j]);
                if (((j+1) % 24) == 0)
                    putc('\n', fd);
                sep = ",";
            }
            fprintf(fd, "}");
            outersep = ",";
        }
    }
    fprintf(fd, "\n};\n");
}

void
write_define(FILE* fd, char* name, int value, char* comment)
{
    fprintf(fd, "#define\t%s\t%d", name, value);
    if (!packoutput && comment)
        fprintf(fd, "\t/* %s */", comment);
    fprintf(fd, "\n");
}

void
write_preamble(FILE* fd)
{
    fprintf(fd, "%s\n",
"/* DO NOT EDIT THIS FILE, IT WAS AUTOMATICALLY CREATED BY mkg3state */");
    write_define(fd, "ACT_INCOMP", ACT_INCOMP, "incompletely decoded code");
    write_define(fd, "ACT_INVALID", ACT_INVALID, "invalide code");
    write_define(fd, "ACT_WRUNT", ACT_WRUNT, "terminating white run code");
    write_define(fd, "ACT_WRUN", ACT_WRUN, "non-terminating white run code");
    write_define(fd, "ACT_BRUNT", ACT_BRUNT, "terminating black run code");
    write_define(fd, "ACT_BRUN", ACT_BRUN, "non-terminating black run code");
    write_define(fd, "ACT_EOL", ACT_EOL, "end-of-line code");
    fprintf(fd, "\n");
    fprintf(fd, "/* modes that the decoder can be in */\n");
    write_define(fd, "MODE_NULL", MODE_NULL, NULL);
    write_define(fd, "MODE_PASS", MODE_PASS, NULL);
    write_define(fd, "MODE_HORIZ", MODE_HORIZ, NULL);
    write_define(fd, "MODE_VERT_V0", MODE_VERT_V0, NULL);
    write_define(fd, "MODE_VERT_VR1", MODE_VERT_VR1, NULL);
    write_define(fd, "MODE_VERT_VR2", MODE_VERT_VR2, NULL);
    write_define(fd, "MODE_VERT_VR3", MODE_VERT_VR3, NULL);
    write_define(fd, "MODE_VERT_VL1", MODE_VERT_VL1, NULL);
    write_define(fd, "MODE_VERT_VL2", MODE_VERT_VL2, NULL);
    write_define(fd, "MODE_VERT_VL3", MODE_VERT_VL3, NULL);
    write_define(fd, "MODE_UNCOMP", MODE_UNCOMP, NULL);
    write_define(fd, "MODE_ERROR", MODE_ERROR, NULL);
    write_define(fd, "MODE_ERROR_1", MODE_ERROR_1, NULL);
    fprintf(fd, "\n");
    fprintf(fd, "#define\tRUNLENGTH(ix) (TIFFFaxWhiteCodes[ix].runlen)\n");
    fprintf(fd, "\n");
    write_define(fd, "UNCOMP_INCOMP", UNCOMP_INCOMP, NULL);
    fprintf(fd, "/* runs of [0]*1 */\n");
    write_define(fd, "UNCOMP_RUN0", UNCOMP_RUN0, NULL);
    write_define(fd, "UNCOMP_RUN1", UNCOMP_RUN1, NULL);
    write_define(fd, "UNCOMP_RUN2", UNCOMP_RUN2, NULL);
    write_define(fd, "UNCOMP_RUN3", UNCOMP_RUN3, NULL);
    write_define(fd, "UNCOMP_RUN4", UNCOMP_RUN4, NULL);
    write_define(fd, "UNCOMP_RUN5", UNCOMP_RUN5, NULL);
    write_define(fd, "UNCOMP_RUN6", UNCOMP_RUN6, NULL);
    fprintf(fd, "/* runs of [0]* w/ terminating color */\n");
    write_define(fd, "UNCOMP_TRUN0", UNCOMP_TRUN0, NULL);
    write_define(fd, "UNCOMP_TRUN1", UNCOMP_TRUN1, NULL);
    write_define(fd, "UNCOMP_TRUN2", UNCOMP_TRUN2, NULL);
    write_define(fd, "UNCOMP_TRUN3", UNCOMP_TRUN3, NULL);
    write_define(fd, "UNCOMP_TRUN4", UNCOMP_TRUN4, NULL);
    fprintf(fd, "/* special code for unexpected EOF */\n");
    write_define(fd, "UNCOMP_EOF", UNCOMP_EOF, NULL);
    fprintf(fd, "/* invalid code encountered */\n");
    write_define(fd, "UNCOMP_INVALID", UNCOMP_INVALID, NULL);
    fprintf(fd, "/* codes >= terminate uncompress mode */\n");
    fprintf(fd, "#define\tUNCOMP_EXIT   UNCOMP_TRUN0\n");
    fprintf(fd, "\n");
}

void
extern_table(FILE* fd, char* name)
{
    fprintf(fd, "extern\t%su_char %s[][256];\n", storage_class, name);
}

void
write_tables(FILE* fd)
{
    write_preamble(fd);
    fprintf(fd, "#ifdef G3STATES\n");
    write_null_mode_table(fd, null_mode, "TIFFFax2DMode");
    write_null_mode_table(fd, null_mode_next_state, "TIFFFax2DNextState");
    write_null_mode_table(fd, uncomp_mode, "TIFFFaxUncompAction");
    write_null_mode_table(fd, uncomp_mode_next_state, "TIFFFaxUncompNextState");
    write_horiz_mode_table(fd, horiz_mode, "TIFFFax1DAction");
    write_horiz_mode_table(fd, horiz_mode_next_state, "TIFFFax1DNextState");
    fprintf(fd, "#else\n");
    extern_table(fd, "TIFFFax2DMode");
    extern_table(fd, "TIFFFax2DNextState");
    extern_table(fd, "TIFFFaxUncompAction");
    extern_table(fd, "TIFFFaxUncompNextState");
    extern_table(fd, "TIFFFax1DAction");
    extern_table(fd, "TIFFFax1DNextState");
    fprintf(fd, "#endif\n");
}

short
find_null_mode_prefix(long prefix)
{
    short j1;

    if (prefix == 0L)
        return (0);
    for (j1 = 8; j1 < null_mode_prefix_count; j1++)
        if (prefix == null_mode_prefix[j1])
                return (j1);
    if (null_mode_prefix_count == MAX_NULLPREFIX) {
        fprintf(stderr, "ERROR: null mode prefix table overflow\n");
        exit(1);
    }
    if (verbose)
        fprintf(stderr, "adding null mode prefix[%d] 0x%lx\n",
            (int) null_mode_prefix_count, prefix);
    null_mode_prefix[null_mode_prefix_count++] = prefix;
    return (null_mode_prefix_count-1);
}

short
find_horiz_mode_prefix(long prefix, char color)
{
    short j1;

    for (j1 = 0; j1 < horiz_mode_prefix_count; j1++)
        if (prefix == horiz_mode_prefix[j1] && horiz_mode_color[j1] == color)
            return (j1);
    /*
     * It wasn't found, so add it to the tables, but first, is there room?
     */
    if (horiz_mode_prefix_count == MAX_HORIZPREFIX) {
        fprintf(stderr, "ERROR: 1D prefix table overflow\n");
        exit(1);
    }
    /* OK, there's room... */
    if (verbose)
        fprintf(stderr, "\nhoriz mode prefix %d, color %c = 0x%lx ",
            (int) horiz_mode_prefix_count, "WB"[color], prefix);
    horiz_mode_prefix[horiz_mode_prefix_count] = prefix;
    horiz_mode_color[horiz_mode_prefix_count] = color;
    horiz_mode_prefix_count++;
    return (horiz_mode_prefix_count - 1);
}

short
find_uncomp_mode_prefix(long prefix)
{
    short j1;

    if (prefix == 0L)
        return (0);
    for (j1 = 8; j1 < uncomp_mode_prefix_count; j1++)
        if (prefix == uncomp_mode_prefix[j1])
                return (j1);
    if (uncomp_mode_prefix_count == MAX_NULLPREFIX) {
        fprintf(stderr, "ERROR: uncomp mode prefix table overflow\n");
        exit(1);
    }
    if (verbose)
        fprintf(stderr, "adding uncomp mode prefix[%d] 0x%lx\n",
            (int) uncomp_mode_prefix_count, prefix);
    uncomp_mode_prefix[uncomp_mode_prefix_count++] = prefix;
    return (uncomp_mode_prefix_count-1);
}

short
null_mode_type(long prefix)
{
    switch (prefix) {
    case 0x18000L: return (MODE_VERT_V0);       /* 1 */
    case 0x36000L: return (MODE_VERT_VR1);      /* 011 */
    case 0x34000L: return (MODE_VERT_VL1);      /* 010 */
    case 0x32000L: return (MODE_HORIZ);         /* 001 */
    case 0x41000L: return (MODE_PASS);          /* 0001 */
    case 0x60C00L: return (MODE_VERT_VR2);      /* 0000 11 */
    case 0x60800L: return (MODE_VERT_VL2);      /* 0000 10 */
    case 0x70600L: return (MODE_VERT_VR3);      /* 0000 011 */
    case 0x70400L: return (MODE_VERT_VL3);      /* 0000 010 */
    case 0x80200L: return (MODE_ERROR);         /* 0000 0010 */
    case 0x90300L: return (MODE_ERROR);         /* 0000 0011 0 */
    case 0xA0380L: return (MODE_ERROR);         /* 0000 0011 10 */
    case 0xA03C0L: return (MODE_UNCOMP);        /* 0000 0011 11 */
    /*
     * Under the assumption that there are no
     * errors in the file, then this bit string
     * can only be the beginning of an EOL code.
     */
    case 0x70000L: return (MODE_ERROR_1);       /* 0000 000 */
    }
    return (-1);
}

short
uncomp_mode_type(long prefix)
{
    short code;
    short len;
    switch (prefix) {
    case 0x18000L: return (UNCOMP_RUN1);        /* 1 */
    case 0x24000L: return (UNCOMP_RUN2);        /* 01 */
    case 0x32000L: return (UNCOMP_RUN3);        /* 001 */
    case 0x41000L: return (UNCOMP_RUN4);        /* 0001 */
    case 0x50800L: return (UNCOMP_RUN5);        /* 0000 1 */
    case 0x60400L: return (UNCOMP_RUN6);        /* 0000 01 */
    case 0x70200L: return (UNCOMP_TRUN0);       /* 0000 001 */
    case 0x80100L: return (UNCOMP_TRUN1);       /* 0000 0001 */
    case 0x90080L: return (UNCOMP_TRUN2);       /* 0000 0000 1 */
    case 0xA0040L: return (UNCOMP_TRUN3);       /* 0000 0000 01 */
    case 0xB0020L: return (UNCOMP_TRUN4);       /* 0000 0000 001 */
    }
    code = prefix & 0xffffL;
    len = (prefix >> 16) & 0xf;
    return ((code || len > 10) ? UNCOMP_INVALID : -1);
}

#define BASESTATE(b)    ((unsigned char) ((b) & 0x7))

void
build_null_mode_tables(void)
{
    short prefix;

    /*
     * Note: the first eight entries correspond to
     * a null prefix and starting bit numbers 0-7.
     */
    null_mode_prefix_count = 8;
    for (prefix = 0; prefix < null_mode_prefix_count; prefix++) {
        short byte;
        for (byte = 0; byte < 256; byte++) {
            short firstbit;
            short bit;
            long curprefix;
            char found_code = FALSE;

            if (prefix < 8) {
                curprefix = 0L;
                firstbit = prefix;
            } else {
                curprefix = null_mode_prefix[prefix];
                firstbit = 0;
            }
            for (bit = firstbit; bit < 8 && !found_code; bit++) {
                short mode;

                if (bit_mask[bit] & byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                switch (mode = null_mode_type(curprefix)) {
                case MODE_PASS:
                case MODE_HORIZ:
                case MODE_VERT_V0:
                case MODE_VERT_VR1:
                case MODE_VERT_VR2:
                case MODE_VERT_VR3:
                case MODE_VERT_VL1:
                case MODE_VERT_VL2:
                case MODE_VERT_VL3:
                case MODE_UNCOMP:
                case MODE_ERROR:
                case MODE_ERROR_1:
                    /*
                     * NOTE: if the bit number is 8, then the table
                     * entry will be zero, which indicates a new byte
                     * is to be fetched during the decoding process
                     */
                    found_code = TRUE;
                    null_mode[prefix][byte] = (unsigned char) mode;
                    null_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                    break;
                }
            }
            if (!found_code) {
                null_mode_next_state[prefix][byte] = (unsigned char)
                    find_null_mode_prefix(curprefix);
                /*
                 * This indicates to the decoder that
                 * no valid code has yet been identified.
                 */
                null_mode[prefix][byte] = MODE_NULL;
            }
        }
    }
}

void
build_horiz_mode_tables(void)
{
    unsigned short byte;
    short prefix;

    /*
     * The first 8 are for white,
     * the second 8 are for black,
     * beginning with bits 0-7.
     */
    horiz_mode_prefix_count = 16;
    for (prefix = 0; prefix < horiz_mode_prefix_count; prefix++)
        for (byte = 0; byte < 256; byte++) {
            short bits_digested = 0;
            short bit;
            short firstbit;
            char color;
            unsigned long curprefix;

            if (prefix < 8) {
                color = WHITE;
                curprefix = 0L;
                firstbit = prefix;
            } else if (prefix < 16) {
                color = BLACK;
                curprefix = 0L;
                firstbit = prefix - 8;
            } else {
                color = horiz_mode_color[prefix];
                curprefix = horiz_mode_prefix[prefix];
                firstbit = 0;
            }
            for (bit = firstbit; bit < 8 && !bits_digested; bit++) {
                if (bit_mask[bit] & byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                /*
                 * The following conversion allows for arbitrary strings of
                 * zeroes to precede the end-of-line code 0000 0000 0001.
                 * It assumes no errors in the data, and is based on
                 * the assumption that the code replaced (12 consecutive
                 * zeroes) can only be "legally" encountered before the
                 * end-of-line code.  This assumption is valid only for
                 * a Group 3 image; the combination will never occur
                 * in horizontal mode in a proper Group 4 image.
                 */
                if (curprefix == 0xC0000L)
                    curprefix = 0xB0000L;
                if (color == WHITE) {
                    short runlength = white_run_length(curprefix);

                    if (runlength == G3CODE_INVALID) {
                        horiz_mode[prefix][byte] = (unsigned char) ACT_INVALID;
                        horiz_mode_next_state[prefix][byte] = (unsigned char) bit;
                        bits_digested = bit + 1;
                    } else if (runlength == G3CODE_EOL) { /* Group 3 only */
                        horiz_mode[prefix][byte] = (unsigned char) ACT_EOL;
                        horiz_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                    } else if (runlength != G3CODE_INCOMP) {
                        horiz_mode[prefix][byte] = (unsigned char)
                            horiz_mode_code_white(runlength);
                        horiz_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                    }
                } else {                /* color == BLACK */
                    short runlength = black_run_length(curprefix);

                    if (runlength == G3CODE_INVALID) {
                        horiz_mode[prefix][byte] = (unsigned char) ACT_INVALID;
                        horiz_mode_next_state[prefix][byte] = (unsigned char) (bit+8);
                        bits_digested = bit + 1;
                    } else if (runlength == G3CODE_EOL) { /* Group 3 only */
                        horiz_mode[prefix][byte] = (unsigned char) ACT_EOL;
                        horiz_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                    } else if (runlength != G3CODE_INCOMP) {
                        horiz_mode[prefix][byte] = (unsigned char)
                            horiz_mode_code_black(runlength);
                        horiz_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                    }
                }
            }
            if (!bits_digested) {       /* no codewords after examining byte */
                horiz_mode[prefix][byte] = (unsigned char) ACT_INCOMP;
                horiz_mode_next_state[prefix][byte] = (unsigned char)
                    find_horiz_mode_prefix(curprefix, color);
            }
        }
}

void
build_uncomp_mode_tables(void)
{
    short prefix;

    /*
     * Note: the first eight entries correspond to
     * a null prefix and starting bit numbers 0-7.
     */
    uncomp_mode_prefix_count = 8;
    for (prefix = 0; prefix < uncomp_mode_prefix_count; prefix++) {
        short byte;
        for (byte = 0; byte < 256; byte++) {
            short firstbit;
            short bit;
            long curprefix;
            char found_code = FALSE;

            if (prefix < 8) {
                curprefix = 0L;
                firstbit = prefix;
            } else {
                curprefix = uncomp_mode_prefix[prefix];
                firstbit = 0;
            }
            for (bit = firstbit; bit < 8 && !found_code; bit++) {
                short mode;

                if (bit_mask[bit] & byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                mode = uncomp_mode_type(curprefix);
                if (mode != -1) {
                    /*
                     * NOTE: if the bit number is 8, then the table
                     * entry will be zero, which indicates a new byte
                     * is to be fetched during the decoding process
                     */
                    found_code = TRUE;
                    uncomp_mode[prefix][byte] = (unsigned char) mode;
                    uncomp_mode_next_state[prefix][byte] = BASESTATE(bit+1);
                    break;
                }
            }
            if (!found_code) {
                uncomp_mode_next_state[prefix][byte] = (unsigned char)
                    find_uncomp_mode_prefix(curprefix);
                /*
                 * This indicates to the decoder that
                 * no valid code has yet been identified.
                 */
                uncomp_mode[prefix][byte] = UNCOMP_INCOMP;
            }
        }
    }
}
