/* libppm2.c - ppm utility library part 2
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

#include "ppm.h"
#include "libppm.h"

static void putus ARGS((unsigned short n, FILE* file));
static void ppm_writeppmrowplain ARGS((FILE* file, pixel* pixelrow, int cols, pixval maxval));
#ifdef PBMPLUS_RAWBITS
static void ppm_writeppmrowraw ARGS((FILE* file, pixel* pixelrow, int cols, pixval maxval));
#endif /* PBMPLUS_RAWBITS */
#if __STDC__
void
ppm_writeppminit( FILE* file, int cols, int rows, pixval maxval, int forceplain )
#else /*__STDC__*/
void
ppm_writeppminit( file, cols, rows, maxval, forceplain )
    FILE* file;
    int cols, rows;
    pixval maxval;
    int forceplain;
#endif /*__STDC__*/
    {
#ifdef PBMPLUS_RAWBITS
    if ( maxval <= 255 && ! forceplain ) {
	fprintf(
	    file, "%c%c\n%d %d\n%d\n", PPM_MAGIC1, RPPM_MAGIC2,
	    cols, rows, maxval );
#ifdef VMS
        set_outfile_binary();
#endif
        }
    else
	fprintf(
	    file, "%c%c\n%d %d\n%d\n", PPM_MAGIC1, PPM_MAGIC2,
	    cols, rows, maxval );
#else /*PBMPLUS_RAWBITS*/
    fprintf(
	file, "%c%c\n%d %d\n%d\n", PPM_MAGIC1, PPM_MAGIC2,
	cols, rows, maxval );
#endif /*PBMPLUS_RAWBITS*/
    }

static void
putus( n, file )
    unsigned short n;
    FILE* file;
    {
    if ( n >= 10 )
	putus( n / 10, file );
    (void) putc( n % 10 + '0', file );
    }

#ifdef PBMPLUS_RAWBITS
static void
ppm_writeppmrowraw( file, pixelrow, cols, maxval )
    FILE* file;
    pixel* pixelrow;
    int cols;
    pixval maxval;
    {
    register int col;
    register pixel* pP;
    register pixval val;

    for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
	{
	val = PPM_GETR( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "r value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	(void) putc( val, file );
	val = PPM_GETG( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "g value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	(void) putc( val, file );
	val = PPM_GETB( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "b value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	(void) putc( val, file );
        }
    }
#endif /*PBMPLUS_RAWBITS*/

static void
ppm_writeppmrowplain( file, pixelrow, cols, maxval )
    FILE* file;
    pixel* pixelrow;
    int cols;
    pixval maxval;
    {
    register int col, charcount;
    register pixel* pP;
    register pixval val;

    charcount = 0;
    for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
	{
	if ( charcount >= 65 )
	    {
	    (void) putc( '\n', file );
	    charcount = 0;
	    }
	else if ( charcount > 0 )
	    {
	    (void) putc( ' ', file );
	    (void) putc( ' ', file );
	    charcount += 2;
	    }
	val = PPM_GETR( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "r value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	putus( val, file );
	(void) putc( ' ', file );
	val = PPM_GETG( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "g value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	putus( val, file );
	(void) putc( ' ', file );
	val = PPM_GETB( *pP );
#ifdef DEBUG
	if ( val > maxval )
	    pm_error( "b value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
	putus( val, file );
	charcount += 11;
	}
    if ( charcount > 0 )
	(void) putc( '\n', file );
    }

#if __STDC__
void
ppm_writeppmrow( FILE* file, pixel* pixelrow, int cols, pixval maxval, int forceplain )
#else /*__STDC__*/
void
ppm_writeppmrow( file, pixelrow, cols, maxval, forceplain )
    FILE* file;
    pixel* pixelrow;
    int cols;
    pixval maxval;
    int forceplain;
#endif /*__STDC__*/
    {
#ifdef PBMPLUS_RAWBITS
    if ( maxval <= 255 && ! forceplain )
	ppm_writeppmrowraw( file, pixelrow, cols, maxval );
    else
	ppm_writeppmrowplain( file, pixelrow, cols, maxval );
#else /*PBMPLUS_RAWBITS*/
    ppm_writeppmrowplain( file, pixelrow, cols, maxval );
#endif /*PBMPLUS_RAWBITS*/
    }

#if __STDC__
void
ppm_writeppm( FILE* file, pixel** pixels, int cols, int rows, pixval maxval, int forceplain )
#else /*__STDC__*/
void
ppm_writeppm( file, pixels, cols, rows, maxval, forceplain )
    FILE* file;
    pixel** pixels;
    int cols, rows;
    pixval maxval;
    int forceplain;
#endif /*__STDC__*/
    {
    int row;

    ppm_writeppminit( file, cols, rows, maxval, forceplain );

    for ( row = 0; row < rows; ++row )
	ppm_writeppmrow( file, pixels[row], cols, maxval, forceplain );
    }
