/* libpgm2.c - pgm utility library part 2
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pgm.h"
#include "libpgm.h"

static void putus1 ARGS((unsigned short n, FILE* file));
static void pgm_writepgmrowplain ARGS((FILE* file, gray* grayrow, int cols, gray maxval));
#ifdef PBMPLUS_RAWBITS
static void pgm_writepgmrowraw ARGS((FILE* file, gray* grayrow, int cols, gray maxval));
#endif /* PBMPLUS_RAWBITS */
#if __STDC__
void
pgm_writepgminit( FILE* file, int cols, int rows, gray maxval, int forceplain )
#else /*__STDC__*/
void
pgm_writepgminit( file, cols, rows, maxval, forceplain )
    FILE* file;
    int cols, rows;
    gray maxval;
    int forceplain;
#endif /*__STDC__*/
    {
#ifdef PBMPLUS_RAWBITS
    if ( maxval <= 255 && ! forceplain ) {
        fprintf(
            file, "%c%c\n%d %d\n%d\n", PGM_MAGIC1, RPGM_MAGIC2,
            cols, rows, maxval );
#ifdef VMS
        set_outfile_binary();
#endif
        }
    else
        fprintf(
            file, "%c%c\n%d %d\n%d\n", PGM_MAGIC1, PGM_MAGIC2,
            cols, rows, maxval );
#else /*PBMPLUS_RAWBITS*/
    fprintf(
        file, "%c%c\n%d %d\n%d\n", PGM_MAGIC1, PGM_MAGIC2,
        cols, rows, maxval );
#endif /*PBMPLUS_RAWBITS*/
    }

static void
putus1( n, file )
    unsigned short n;
    FILE* file;
    {
    if ( n >= 10 )
        putus1((unsigned short)( n / 10), file );
    (void) putc( n % 10 + '0', file );
    }

#ifdef PBMPLUS_RAWBITS
static void
pgm_writepgmrowraw(FILE* file,gray* grayrow,int cols,gray maxval)
    {
    register int col;
    register gray* gP;

    for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
        {
#ifdef DEBUG
        if ( *gP > maxval )
            pm_error( "value out of bounds (%u > %u)", *gP, maxval );
#endif /*DEBUG*/
        (void) putc( *gP, file );
        }
    }
#endif /*PBMPLUS_RAWBITS*/

static void
pgm_writepgmrowplain(FILE* file,gray* grayrow,int cols,gray maxval)
    {
    register int col, charcount;
    register gray* gP;

    charcount = 0;
    for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
        {
        if ( charcount >= 65 )
            {
            (void) putc( '\n', file );
            charcount = 0;
            }
        else if ( charcount > 0 )
            {
            (void) putc( ' ', file );
            ++charcount;
            }
#ifdef DEBUG
        if ( *gP > maxval )
            pm_error( "value out of bounds (%u > %u)", *gP, maxval );
#endif /*DEBUG*/
/*        putus1( (unsigned long) *gP, file ); */
        putus1( (unsigned short) *gP, file );
        charcount += 3;
        }
    if ( charcount > 0 )
        (void) putc( '\n', file );
    }

#if __STDC__
void
pgm_writepgmrow( FILE* file, gray* grayrow, int cols, gray maxval, int forceplain )
#else /*__STDC__*/
void
pgm_writepgmrow( file, grayrow, cols, maxval, forceplain )
    FILE* file;
    gray* grayrow;
    int cols;
    gray maxval;
    int forceplain;
#endif /*__STDC__*/
    {
#ifdef PBMPLUS_RAWBITS
    if ( maxval <= 255 && ! forceplain )
        pgm_writepgmrowraw( file, grayrow, cols, maxval );
    else
        pgm_writepgmrowplain( file, grayrow, cols, maxval );
#else /*PBMPLUS_RAWBITS*/
    pgm_writepgmrowplain( file, grayrow, cols, maxval );
#endif /*PBMPLUS_RAWBITS*/
    }

#if __STDC__
void
pgm_writepgm( FILE* file, gray** grays, int cols, int rows, gray maxval, int forceplain )
#else /*__STDC__*/
void
pgm_writepgm( file, grays, cols, rows, maxval, forceplain )
    FILE* file;
    gray** grays;
    int cols, rows;
    gray maxval;
    int forceplain;
#endif /*__STDC__*/
    {
    int row;

    pgm_writepgminit( file, cols, rows, maxval, forceplain );

    for ( row = 0; row < rows; ++row )
         pgm_writepgmrow( file, grays[row], cols, maxval, forceplain );
    }
