/* libpbm2.c - pbm utility library part 2
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

static bit pbm_getbit ARGS((FILE *));
static bit
pbm_getbit( file )
    FILE* file;
    {
    register char ch;

    do
        {
        ch = pbm_getc( file );
        }
    while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' );

    if ( ch != '0' && ch != '1' )
        pm_error( "junk in file where bits should be" );

    return ( ch == '1' ) ? 1 : 0;
    }

int
pbm_readmagicnumber( file )
    FILE* file;
    {
    int ich1, ich2;

    ich1 = getc( file );
    if ( ich1 == EOF )
        pm_error( "EOF / read error reading magic number" );
    ich2 = getc( file );
    if ( ich2 == EOF )
        pm_error( "EOF / read error reading magic number" );
    return ich1 * 256 + ich2;
    }

void
pbm_readpbminitrest( file, colsP, rowsP )
    FILE* file;
    int* colsP;
    int* rowsP;
    {
    /* Read size. */
    *colsP = pbm_getint( file );
    *rowsP = pbm_getint( file );
    }

void
pbm_readpbminit( file, colsP, rowsP, formatP )
    FILE* file;
    int* colsP;
    int* rowsP;
    int* formatP;
    {
    /* Check magic number. */
    *formatP = pbm_readmagicnumber( file );
    switch ( PBM_FORMAT_TYPE(*formatP) )
        {
        case PBM_TYPE:
        pbm_readpbminitrest( file, colsP, rowsP );
        break;

        default:
        pm_error( "bad magic number - not a pbm file" );
        }
    }

void
pbm_readpbmrow( file, bitrow, cols, format )
    FILE* file;
    bit* bitrow;
    int cols, format;
    {
    register int col, bitshift;
    register unsigned char item;
    register bit* bP;

    switch ( format )
        {
        case PBM_FORMAT:
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
            *bP = pbm_getbit( file );
        break;

        case RPBM_FORMAT:
        bitshift = -1;
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
            {
            if ( bitshift == -1 )
                {
                item = pbm_getrawbyte( file );
                bitshift = 7;
                }
            *bP = ( item >> bitshift ) & 1;
            --bitshift;
            }
        break;

        default:
        pm_error( "can't happen" );
        }
    }

bit**
pbm_readpbm( file, colsP, rowsP )
    FILE* file;
    int* colsP;
    int* rowsP;
    {
    register bit** bits;
    int format, row;

    pbm_readpbminit( file, colsP, rowsP, &format );

    bits = pbm_allocarray( *colsP, *rowsP );

    for ( row = 0; row < *rowsP; ++row )
        pbm_readpbmrow( file, bits[row], *colsP, format );

    return bits;
    }
