/* Filename: sgi.h
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://etc.cmu.edu/panda3d/docs/license/ .
 *
 * To contact the maintainers of this program write to
 * panda3d-general@lists.sourceforge.net .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
