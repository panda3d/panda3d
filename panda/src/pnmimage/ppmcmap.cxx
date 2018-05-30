/* libppm3.c - ppm utility library part 3
**
** Colormap routines.
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppmcmap.h"

#define HASH_SIZE 20023

#ifdef PPM_PACKCOLORS
#define ppm_hashpixel(p) ( ( (int) (p) & 0x7fffffff ) % HASH_SIZE )
#else /*PPM_PACKCOLORS*/
#define ppm_hashpixel(p) ( ( ( (long) PPM_GETR(p) * 33023 + (long) PPM_GETG(p) * 30013 + (long) PPM_GETB(p) * 27011 ) & 0x7fffffff ) % HASH_SIZE )
#endif /*PPM_PACKCOLORS*/

colorhist_vector
ppm_computecolorhist(pixel** pixels,
                     int cols, int rows, int maxcolors,
                     int* colorsP)
    {
    colorhash_table cht;
    colorhist_vector chv;

    cht = ppm_computecolorhash( pixels, cols, rows, maxcolors, colorsP );
    if ( cht == nullptr )
        return nullptr;
    chv = ppm_colorhashtocolorhist( cht, maxcolors );
    ppm_freecolorhash( cht );
    return chv;
    }

void
ppm_addtocolorhist(colorhist_vector chv,
                   int* colorsP,
                   int maxcolors,
                   pixel* colorP,
                   int value, int position)
    {
    int i, j;

    /* Search colorhist for the color. */
    for ( i = 0; i < *colorsP; ++i )
        if ( PPM_EQUAL( chv[i].color, *colorP ) )
            {
            /* Found it - move to new slot. */
            if ( position > i )
                {
                for ( j = i; j < position; ++j )
                    chv[j] = chv[j + 1];
                }
            else if ( position < i )
                {
                for ( j = i; j > position; --j )
                    chv[j] = chv[j - 1];
                }
            chv[position].color = *colorP;
            chv[position].value = value;
            return;
            }
    if ( *colorsP < maxcolors )
        {
        /* Didn't find it, but there's room to add it; so do so. */
        for ( i = *colorsP; i > position; --i )
            chv[i] = chv[i - 1];
        chv[position].color = *colorP;
        chv[position].value = value;
        ++(*colorsP);
        }
    }

colorhash_table
ppm_computecolorhash(pixel** pixels,
                     int cols, int rows, int maxcolors,
                     int* colorsP)
    {
    colorhash_table cht;
    pixel* pP;
    colorhist_list chl;
    int col, row, hash;

    cht = ppm_alloccolorhash( );
    *colorsP = 0;

    /* Go through the entire image, building a hash table of colors. */
    for ( row = 0; row < rows; ++row )
        for ( col = 0, pP = pixels[row]; col < cols; ++col, ++pP )
            {
            hash = ppm_hashpixel( *pP );
            for ( chl = cht[hash]; chl != nullptr; chl = chl->next )
                if ( PPM_EQUAL( chl->ch.color, *pP ) )
                    break;
            if ( chl != nullptr )
                ++(chl->ch.value);
            else
                {
                if ( ++(*colorsP) > maxcolors )
                    {
                    ppm_freecolorhash( cht );
                    return nullptr;
                    }
                chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
                if ( chl == nullptr )
                    pm_error( "out of memory computing hash table" );
                chl->ch.color = *pP;
                chl->ch.value = 1;
                chl->next = cht[hash];
                cht[hash] = chl;
                }
            }
    
    return cht;
    }

colorhash_table
ppm_alloccolorhash( )
    {
    colorhash_table cht;
    int i;

    cht = (colorhash_table) malloc( HASH_SIZE * sizeof(colorhist_list) );
    if ( cht == nullptr )
        pm_error( "out of memory allocating hash table" );

    for ( i = 0; i < HASH_SIZE; ++i )
        cht[i] = nullptr;

    return cht;
    }

int
ppm_addtocolorhash(colorhash_table cht,
                   pixel* colorP,
                   int value)
    {
    int hash;
    colorhist_list chl;

    chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
    if ( chl == nullptr )
        return -1;
    hash = ppm_hashpixel( *colorP );
    chl->ch.color = *colorP;
    chl->ch.value = value;
    chl->next = cht[hash];
    cht[hash] = chl;
    return 0;
    }

colorhist_vector
ppm_colorhashtocolorhist(colorhash_table cht,
                         int maxcolors)
    {
    colorhist_vector chv;
    colorhist_list chl;
    int i, j;

    /* Now collate the hash table into a simple colorhist array. */
    chv = (colorhist_vector) malloc( maxcolors * sizeof(struct colorhist_item) );
    /* (Leave room for expansion by caller.) */
    if ( chv == nullptr )
        pm_error( "out of memory generating histogram" );

    /* Loop through the hash table. */
    j = 0;
    for ( i = 0; i < HASH_SIZE; ++i )
        for ( chl = cht[i]; chl != nullptr; chl = chl->next )
            {
            /* Add the new entry. */
            chv[j] = chl->ch;
            ++j;
            }

    /* All done. */
    return chv;
    }

colorhash_table
ppm_colorhisttocolorhash(colorhist_vector chv,
                         int colors)
    {
    colorhash_table cht;
    int i, hash;
    pixel color;
    colorhist_list chl;

    cht = ppm_alloccolorhash( );

    for ( i = 0; i < colors; ++i )
        {
        color = chv[i].color;
        hash = ppm_hashpixel( color );
        for ( chl = cht[hash]; chl != nullptr; chl = chl->next )
            if ( PPM_EQUAL( chl->ch.color, color ) )
                pm_error(
                    "same color found twice - %d %d %d", PPM_GETR(color),
                    PPM_GETG(color), PPM_GETB(color) );
        chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
        if ( chl == nullptr )
            pm_error( "out of memory" );
        chl->ch.color = color;
        chl->ch.value = i;
        chl->next = cht[hash];
        cht[hash] = chl;
        }

    return cht;
    }

int
ppm_lookupcolor(colorhash_table cht,
                pixel* colorP)
    {
    int hash;
    colorhist_list chl;

    hash = ppm_hashpixel( *colorP );
    for ( chl = cht[hash]; chl != nullptr; chl = chl->next )
        if ( PPM_EQUAL( chl->ch.color, *colorP ) )
            return chl->ch.value;

    return -1;
    }

void
ppm_freecolorhist(colorhist_vector chv)
    {
    free( (char*) chv );
    }

void
ppm_freecolorhash(colorhash_table cht)
    {
    int i;
    colorhist_list chl, chlnext;

    for ( i = 0; i < HASH_SIZE; ++i )
        for ( chl = cht[i]; chl != nullptr; chl = chlnext )
            {
            chlnext = chl->next;
            free( (char*) chl );
            }
    free( (char*) cht );
    }
