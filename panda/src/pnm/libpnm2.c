/* libpnm2.c - pnm utility library part 2
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

#include "pnm.h"

#include "ppm.h"
#include "libppm.h"

#include "pgm.h"
#include "libpgm.h"

#include "pbm.h"
#include "libpbm.h"

#if __STDC__
void
pnm_writepnminit( FILE* file, int cols, int rows, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
void
pnm_writepnminit( file, cols, rows, maxval, format, forceplain )
    FILE* file;
    int cols, rows, format;
    xelval maxval;
    int forceplain;
#endif /*__STDC__*/
    {
    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	ppm_writeppminit( file, cols, rows, (pixval) maxval, forceplain );
	break;

	case PGM_TYPE:
	pgm_writepgminit( file, cols, rows, (gray) maxval, forceplain );
	break;

	case PBM_TYPE:
	pbm_writepbminit( file, cols, rows, forceplain );
	break;

	default:
	pm_error( "can't happen" );
	}
    }

#if __STDC__
void
pnm_writepnmrow( FILE* file, xel* xelrow, int cols, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
void
pnm_writepnmrow( file, xelrow, cols, maxval, format, forceplain )
    FILE* file;
    xel* xelrow;
    int cols, format;
    xelval maxval;
    int forceplain;
#endif /*__STDC__*/
    {
    register int col;
    register xel* xP;
    gray* grayrow;
    register gray* gP;
    bit* bitrow;
    register bit* bP;

    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	ppm_writeppmrow( file, (pixel*) xelrow, cols, (pixval) maxval, forceplain );
	break;

	case PGM_TYPE:
	grayrow = pgm_allocrow( cols );
	for ( col = 0, gP = grayrow, xP = xelrow; col < cols; ++col, ++gP, ++xP )
	    *gP = PNM_GET1( *xP );
	pgm_writepgmrow( file, grayrow, cols, (gray) maxval, forceplain );
	pgm_freerow( grayrow );
	break;

	case PBM_TYPE:
	bitrow = pbm_allocrow( cols );
	for ( col = 0, bP = bitrow, xP = xelrow; col < cols; ++col, ++bP, ++xP )
	    *bP = PNM_GET1( *xP ) == 0 ? PBM_BLACK : PBM_WHITE;
	pbm_writepbmrow( file, bitrow, cols, forceplain );
	pbm_freerow( bitrow );
	break;

	default:
	pm_error( "can't happen" );
	}
    }

#if __STDC__
void
pnm_writepnm( FILE* file, xel** xels, int cols, int rows, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
void
pnm_writepnm( file, xels, cols, rows, maxval, format, forceplain )
    FILE* file;
    xel** xels;
    xelval maxval;
    int cols, rows, format;
    int forceplain;
#endif /*__STDC__*/
    {
    int row;

    pnm_writepnminit( file, cols, rows, maxval, format, forceplain );

    for ( row = 0; row < rows; ++row )
	pnm_writepnmrow( file, xels[row], cols, maxval, format, forceplain );
    }
