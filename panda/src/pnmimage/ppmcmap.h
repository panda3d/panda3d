/* ppmcmap.h - header file for colormap routines in libppm
*/

/* Color histogram stuff. */

#ifndef PPMCMAP_H
#define PPMCMAP_H

#include "pandabase.h"
#include "pnmimage_base.h"

typedef struct colorhist_item* colorhist_vector;
struct colorhist_item
    {
    pixel color;
    int value;
    };

typedef struct colorhist_list_item* colorhist_list;
struct colorhist_list_item
    {
    struct colorhist_item ch;
    colorhist_list next;
    };

EXPCL_PANDA_PNMIMAGE colorhist_vector ppm_computecolorhist( pixel** pixels, int cols, int rows, int maxcolors, int* colorsP );
/* Returns a colorhist *colorsP long (with space allocated for maxcolors. */

EXPCL_PANDA_PNMIMAGE void ppm_addtocolorhist ( colorhist_vector chv, int* colorsP, int maxcolors, pixel* colorP, int value, int position );

EXPCL_PANDA_PNMIMAGE void ppm_freecolorhist( colorhist_vector chv );


/* Color hash table stuff. */

typedef colorhist_list* colorhash_table;

EXPCL_PANDA_PNMIMAGE colorhash_table ppm_computecolorhash ( pixel** pixels, int cols, int rows, int maxcolors, int* colorsP );

EXPCL_PANDA_PNMIMAGE int
ppm_lookupcolor( colorhash_table cht, pixel* colorP );

EXPCL_PANDA_PNMIMAGE colorhist_vector ppm_colorhashtocolorhist ( colorhash_table cht, int maxcolors );
EXPCL_PANDA_PNMIMAGE colorhash_table ppm_colorhisttocolorhash( colorhist_vector chv, int colors );

EXPCL_PANDA_PNMIMAGE int ppm_addtocolorhash ( colorhash_table cht, pixel* colorP, int value );
/* Returns -1 on failure. */

EXPCL_PANDA_PNMIMAGE colorhash_table ppm_alloccolorhash ( void );

EXPCL_PANDA_PNMIMAGE void ppm_freecolorhash( colorhash_table cht );

#endif
