/* libppm4.c - ppm utility library part 4
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

static void canonstr ARGS((char* str));
static long rgbnorm ARGS((long rgb, long lmaxval, int n, char* colorname));
static void
canonstr( str )
    char* str;
    {
    while ( *str != '\0' )
        {
        if ( *str == ' ' )
            {
            (void) strcpy( str, &(str[1]) );
            continue;
            }
        if ( isupper( *str ) )
            *str = tolower( *str );
        ++str;
        }
    }

static long
rgbnorm( rgb, lmaxval, n, colorname )
    long rgb, lmaxval;
    int n;
    char* colorname;
    {
    switch ( n )
        {
        case 1:
        if ( lmaxval != 15 )
            rgb = rgb * lmaxval / 15;
        break;
        case 2:
        if ( lmaxval != 255 )
            rgb = rgb * lmaxval / 255;
        break;
        case 3:
        if ( lmaxval != 4095 )
            rgb = rgb * lmaxval / 4095;
        break;
        case 4:
        if ( lmaxval != 65535L )
            rgb = rgb * lmaxval / 65535L;
        break;
        default:
        pm_error( "invalid color specifier - \"%s\"", colorname );
        }
    return rgb;
    }

#if __STDC__
pixel
ppm_parsecolor( char* colorname, pixval maxval )
#else /*__STDC__*/
pixel
ppm_parsecolor( colorname, maxval )
    char* colorname;
    pixval maxval;
#endif /*__STDC__*/
    {
    int hexit[256], i;
    pixel p;
    long lmaxval, r, g, b;
    char* inval = "invalid color specifier - \"%s\"";

    for ( i = 0; i < 256; ++i )
        hexit[i] = 1234567890;
    hexit['0'] = 0;
    hexit['1'] = 1;
    hexit['2'] = 2;
    hexit['3'] = 3;
    hexit['4'] = 4;
    hexit['5'] = 5;
    hexit['6'] = 6;
    hexit['7'] = 7;
    hexit['8'] = 8;
    hexit['9'] = 9;
    hexit['a'] = hexit['A'] = 10;
    hexit['b'] = hexit['B'] = 11;
    hexit['c'] = hexit['C'] = 12;
    hexit['d'] = hexit['D'] = 13;
    hexit['e'] = hexit['E'] = 14;
    hexit['f'] = hexit['F'] = 15;

    lmaxval = maxval;
    if ( strncmp( colorname, "rgb:", 4 ) == 0 )
        {
        /* It's a new-X11-style hexadecimal rgb specifier. */
        char* cp;

        cp = colorname + 4;
        r = g = b = 0;
        for ( i = 0; *cp != '/'; ++i, ++cp )
            r = r * 16 + hexit[*cp];
        r = rgbnorm( r, lmaxval, i, colorname );
        for ( i = 0, ++cp; *cp != '/'; ++i, ++cp )
            g = g * 16 + hexit[*cp];
        g = rgbnorm( g, lmaxval, i, colorname );
        for ( i = 0, ++cp; *cp != '\0'; ++i, ++cp )
            b = b * 16 + hexit[*cp];
        b = rgbnorm( b, lmaxval, i, colorname );
        if ( r < 0 || r > lmaxval || g < 0 || g > lmaxval || b < 0 || b > lmaxval )
            pm_error( inval, colorname );
        }
    else if ( strncmp( colorname, "rgbi:", 5 ) == 0 )
        {
        /* It's a new-X11-style decimal/float rgb specifier. */
        float fr, fg, fb;

        if ( sscanf( colorname, "rgbi:%f/%f/%f", &fr, &fg, &fb ) != 3 )
            pm_error( inval, colorname );
        if ( fr < 0.0 || fr > 1.0 || fg < 0.0 || fg > 1.0 || fb < 0.0 || fb > 1.0 )
            pm_error( "invalid color specifier - \"%s\" - values must be between 0.0 and 1.0", colorname );
        r = fr * lmaxval;
        g = fg * lmaxval;
        b = fb * lmaxval;
        }
    else if ( colorname[0] == '#' )
        {
        /* It's an old-X11-style hexadecimal rgb specifier. */
        switch ( strlen( colorname ) )
            {
            case 4:
            r = hexit[colorname[1]];
            g = hexit[colorname[2]];
            b = hexit[colorname[3]];
            r = rgbnorm( r, lmaxval, 1, colorname );
            g = rgbnorm( g, lmaxval, 1, colorname );
            b = rgbnorm( b, lmaxval, 1, colorname );
            break;

            case 7:
            r = ( hexit[colorname[1]] << 4 ) + hexit[colorname[2]];
            g = ( hexit[colorname[3]] << 4 ) + hexit[colorname[4]];
            b = ( hexit[colorname[5]] << 4 ) + hexit[colorname[6]];
            r = rgbnorm( r, lmaxval, 2, colorname );
            g = rgbnorm( g, lmaxval, 2, colorname );
            b = rgbnorm( b, lmaxval, 2, colorname );
            break;

            case 10:
            r = ( hexit[colorname[1]] << 8 ) + ( hexit[colorname[2]] << 4 ) +
                hexit[colorname[3]];
            g = ( hexit[colorname[4]] << 8 ) + ( hexit[colorname[5]] << 4 ) +
                hexit[colorname[6]];
            b = ( hexit[colorname[7]] << 8 ) + ( hexit[colorname[8]] << 4 ) +
                hexit[colorname[9]];
            r = rgbnorm( r, lmaxval, 3, colorname );
            g = rgbnorm( g, lmaxval, 3, colorname );
            b = rgbnorm( b, lmaxval, 3, colorname );
            break;

            case 13:
            r = ( hexit[colorname[1]] << 12 ) + ( hexit[colorname[2]] << 8 ) +
                ( hexit[colorname[3]] << 4 ) + hexit[colorname[4]];
            g = ( hexit[colorname[5]] << 12 ) + ( hexit[colorname[6]] << 8 ) +
                ( hexit[colorname[7]] << 4 ) + hexit[colorname[8]];
            b = ( hexit[colorname[9]] << 12 ) + ( hexit[colorname[10]] << 8 ) +
                ( hexit[colorname[11]] << 4 ) + hexit[colorname[12]];
            r = rgbnorm( r, lmaxval, 4, colorname );
            g = rgbnorm( g, lmaxval, 4, colorname );
            b = rgbnorm( b, lmaxval, 4, colorname );
            break;

            default:
            pm_error( inval, colorname );
            }
        if ( r < 0 || r > lmaxval || g < 0 || g > lmaxval || b < 0 || b > lmaxval )
            pm_error( inval, colorname );
        }
    else if ( ( colorname[0] >= '0' && colorname[0] <= '9' ) ||
              colorname[0] == '.' )
        {
        /* It's an old-style decimal/float rgb specifier. */
        float fr, fg, fb;

        if ( sscanf( colorname, "%f,%f,%f", &fr, &fg, &fb ) != 3 )
            pm_error( inval, colorname );
        if ( fr < 0.0 || fr > 1.0 || fg < 0.0 || fg > 1.0 || fb < 0.0 || fb > 1.0 )
            pm_error( "invalid color specifier - \"%s\" - values must be between 0.0 and 1.0", colorname );
        r = fr * lmaxval;
        g = fg * lmaxval;
        b = fb * lmaxval;
        }
    else
        {
        /* Must be a name from the X-style rgb file. */
#ifndef RGB_DB
        pm_error( "color names database required - please reconfigure with RGBDEF" );
#else /*RGB_DB*/
        FILE* f;
        char buf1[200], buf2[200];

#ifndef A_RGBENV
        (void) sprintf( buf1, "%s.txt", RGB_DB );
        if ( ( f = fopen( buf1, "r" ) ) == NULL )
            pm_error( "can't open color names database - reconfigure with correct RGBDEF" );
#else /* A_RGBENV */
        char *rgbdef;
        if( (rgbdef = getenv(RGB_DB))==NULL )
            pm_error( "can't get path to color names database - %s not set", RGB_DB );
        if ( ( f = fopen( rgbdef, "r" ) ) == NULL )
            pm_error( "can't open color names database - set %s to correct path", RGB_DB );
#endif /* A_RGBENV */
        canonstr( colorname );
        while ( fgets( buf1, sizeof(buf1), f ) != NULL )
            {
            if ( sscanf( buf1, "%ld %ld %ld %[^\n]", &r, &g, &b, buf2 ) != 4 )
                {
                pm_message(
                    "can't parse color names database line - \"%s\"", buf1 );
                continue;
                }
            canonstr( buf2 );
            if ( strcmp( colorname, buf2 ) == 0 )
                goto gotit;
            }
        (void) fclose( f );
        pm_error( "unknown color - \"%s\"", colorname );

gotit:
        (void) fclose( f );
        /* Rescale from [0..255] if necessary. */
        if ( lmaxval != 255 )
            {
            r = r * lmaxval / 255;
            g = g * lmaxval / 255;
            b = b * lmaxval / 255;
            }
#endif /*RGB_DB*/
        }

    PPM_ASSIGN( p, r, g, b );
    return p;
    }

static char colorname[200];

#if __STDC__
char*
ppm_colorname( pixel* colorP, pixval maxval, int hexok )
#else /*__STDC__*/
char*
ppm_colorname( colorP, maxval, hexok )
    pixel* colorP;
    pixval maxval;
    int hexok;
#endif /*__STDC__*/
    {
    int r, g, b;
#ifdef RGB_DB
    FILE* f;
    char buf[200];
    int this_r, this_g, this_b;
    int best_diff, this_diff;
    char this_colorname[200];
#ifdef A_RGBENV
    char *rgbdef;
#endif /* A_RGBENV */
#endif /*RGB_DB*/

    if ( maxval == 255 )
        {
        r = PPM_GETR( *colorP );
        g = PPM_GETG( *colorP );
        b = PPM_GETB( *colorP );
        }
    else
        {
        r = (int) PPM_GETR( *colorP ) * 255 / (int) maxval;
        g = (int) PPM_GETG( *colorP ) * 255 / (int) maxval;
        b = (int) PPM_GETB( *colorP ) * 255 / (int) maxval;
        }

#ifdef RGB_DB
#ifndef A_RGBENV
    (void) sprintf( buf, "%s.txt", RGB_DB );
    if ( ( f = fopen( buf, "r" ) ) == NULL )
        pm_error( "can't open color names database - reconfigure with correct RGBDEF" );
#else /* A_RGBENV */
    if( (rgbdef = getenv(RGB_DB))==NULL )
        pm_error( "can't get path to color names database - %s not set", RGB_DB );
    if ( ( f = fopen( rgbdef, "r" ) ) == NULL )
        pm_error( "can't open color names database - set %s to correct path", RGB_DB );
#endif /* A_RGBENV */
    best_diff = 32767;
    while ( fgets( buf, sizeof(buf), f ) != NULL )
        {
        if ( sscanf( buf, "%d %d %d %[^\n]", &this_r, &this_g, &this_b,
                     this_colorname ) != 4 )
            {
            pm_message(
                "can't parse color names database line - \"%s\"",
                buf );
            continue;
            }
        this_diff = abs( r - this_r ) + abs( g - this_g ) + abs( b - this_b );
        if ( this_diff < best_diff )
            {
            best_diff = this_diff;
            (void) strcpy( colorname, this_colorname );
            }
        }
    (void) fclose( f );
    if ( best_diff != 32767 && ( best_diff == 0 || ! hexok ) )
        return colorname;
#endif /*RGB_DB*/

    /* Color lookup failed; generate an X11-style hex specifier. */
    if ( ! hexok )
        pm_error(
            "color names database required - please reconfigure with RGBDEF" );
    sprintf( colorname, "#%02x%02x%02x", r, g, b );
    return colorname;
    }
