/* libpbm4.c - pbm utility library part 4
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

char
pbm_getc( file )
    FILE* file;
    {
    register int ich;
    register char ch;

    ich = getc( file );
    if ( ich == EOF )
	pm_error( "EOF / read error" );
    ch = (char) ich;
    
    if ( ch == '#' )
	{
	do
	    {
	    ich = getc( file );
	    if ( ich == EOF )
		pm_error( "EOF / read error" );
	    ch = (char) ich;
	    }
	while ( ch != '\n' && ch != '\r' );
	}

    return ch;
    }

unsigned char
pbm_getrawbyte( file )
    FILE* file;
    {
    register int iby;

    iby = getc( file );
    if ( iby == EOF )
	pm_error( "EOF / read error" );
    return (unsigned char) iby;
    }

int
pbm_getint( file )
    FILE* file;
    {
    register char ch;
    register int i;

    do
	{
	ch = pbm_getc( file );
	}
    while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' );

    if ( ch < '0' || ch > '9' )
	pm_error( "junk in file where an integer should be" );

    i = 0;
    do
	{
	i = i * 10 + ch - '0';
	ch = pbm_getc( file );
        }
    while ( ch >= '0' && ch <= '9' );

    return i;
    }
