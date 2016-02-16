/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sgi.h
 */

#ifndef SGI_IMAGE_H
#define SGI_IMAGE_H

typedef struct {
    short           magic;
    char            storage;
    char            bpc;            /* pixel size: 1 = bytes, 2 = shorts */
    unsigned short  dimension;      /* 1 = single row, 2 = B/W, 3 = RGB */
    unsigned short  xsize,          /* width in pixels */
                    ysize,          /* height in pixels */
                    zsize;          /* # of channels; B/W=1, RGB=3, RGBA=4 */
    long            pixmin, pixmax; /* min/max pixel values */
    char            dummy1[4];
    char            name[80];
    long            colormap;
    char            dummy2[404];
} Header;
#define HeaderSize  512

#define SGI_MAGIC           (short)474

#define STORAGE_VERBATIM    0
#define STORAGE_RLE         1

#define CMAP_NORMAL         0
#define CMAP_DITHERED       1   /* not supported */
#define CMAP_SCREEN         2   /* not supported */
#define CMAP_COLORMAP       3   /* not supported */

#endif
