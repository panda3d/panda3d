/* libpbm3.c - pbm utility library part 3
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"
#include "libpbm.h"

static void pbm_writepbmrowplain ARGS((FILE* file, bit* bitrow, int cols));
#ifdef PBMPLUS_RAWBITS
static void pbm_writepbmrowraw ARGS((FILE* file, bit* bitrow, int cols));
#endif /* PBMPLUS_RAWBITS */
void
pbm_writepbminit( file, cols, rows, forceplain )
    FILE* file;
    int cols, rows;
    int forceplain;
    {
#ifdef PBMPLUS_RAWBITS
    if ( ! forceplain ) {
        fprintf( file, "%c%c\n%d %d\n", PBM_MAGIC1, RPBM_MAGIC2, cols, rows );
#ifdef VMS
        set_outfile_binary();
#endif
        }
    else
        fprintf( file, "%c%c\n%d %d\n", PBM_MAGIC1, PBM_MAGIC2, cols, rows );
#else /*PBMPLUS_RAWBITS*/
    fprintf( file, "%c%c\n%d %d\n", PBM_MAGIC1, PBM_MAGIC2, cols, rows );
#endif /*PBMPLUS_RAWBITS*/
    }

#ifdef PBMPLUS_RAWBITS
static void
pbm_writepbmrowraw( file, bitrow, cols )
    FILE* file;
    bit* bitrow;
    int cols;
    {
    register int col, bitshift;
    register unsigned char item;
    register bit* bP;

    bitshift = 7;
    item = 0;
    for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
        {
        if ( *bP )
            item += 1 << bitshift;
        --bitshift;
        if ( bitshift == -1 )
            {
            (void) putc( item, file );
            bitshift = 7;
            item = 0;
            }
        }
    if ( bitshift != 7 )
        (void) putc( item, file );
    }
#endif /*PBMPLUS_RAWBITS*/

static void
pbm_writepbmrowplain( file, bitrow, cols )
    FILE* file;
    bit* bitrow;
    int cols;
    {
    register int col, charcount;
    register bit* bP;

    charcount = 0;
    for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
        {
        if ( charcount >= 70 )
            {
            (void) putc( '\n', file );
            charcount = 0;
            }
        (void) putc( *bP ? '1' : '0', file );
        ++charcount;
        }
    (void) putc( '\n', file );
    }

void
pbm_writepbmrow( file, bitrow, cols, forceplain )
    FILE* file;
    bit* bitrow;
    int cols;
    int forceplain;
    {
#ifdef PBMPLUS_RAWBITS
    if ( ! forceplain )
        pbm_writepbmrowraw( file, bitrow, cols );
    else
        pbm_writepbmrowplain( file, bitrow, cols );
#else /*PBMPLUS_RAWBITS*/
    pbm_writepbmrowplain( file, bitrow, cols );
#endif /*PBMPLUS_RAWBITS*/
    }

void
pbm_writepbm( file, bits, cols, rows, forceplain )
    FILE* file;
    bit** bits;
    int cols, rows;
    int forceplain;
    {
    int row;

    pbm_writepbminit( file, cols, rows, forceplain );

    for ( row = 0; row < rows; ++row )
        pbm_writepbmrow( file, bits[row], cols, forceplain );
    }
