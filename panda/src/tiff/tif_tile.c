#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * Copyright (c) 1991, 1992 Sam Leffler
 * Copyright (c) 1991, 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library.
 *
 * Tiled Image Support Routines.
 */
#include "tiffiop.h"

/*
 * Compute which tile an (x,y,z,s) value is in.
 */
ttile_t
TIFFComputeTile(TIFF* tif, uint32 x, uint32 y, uint32 z, tsample_t s)
{
        TIFFDirectory *td = &tif->tif_dir;
        u_long dx = td->td_tilewidth;
        u_long dy = td->td_tilelength;
        u_long dz = td->td_tiledepth;
        ttile_t tile = 1;

        if (td->td_imagedepth == 1)
                z = 0;
        if (dx == (u_long) -1)
                dx = td->td_imagewidth;
        if (dy == (u_long) -1)
                dy = td->td_imagelength;
        if (dz == (u_long) -1)
                dz = td->td_imagedepth;
        if (dx != 0 && dy != 0 && dz != 0) {
                u_long xpt = howmany(td->td_imagewidth, dx); 
                u_long ypt = howmany(td->td_imagelength, dy); 
                u_long zpt = howmany(td->td_imagedepth, dz); 

                if (td->td_planarconfig == PLANARCONFIG_SEPARATE) 
                        tile = (xpt*ypt*zpt)*s +
                             (xpt*ypt)*(z/dz) +
                             xpt*(y/dy) +
                             x/dx;
                else
                        tile = (xpt*ypt)*(z/dz) + xpt*(y/dy) + x/dx + s;
        }
        return (tile);
}

/*
 * Check an (x,y,z,s) coordinate
 * against the image bounds.
 */
TIFFCheckTile(TIFF* tif, uint32 x, uint32 y, uint32 z, tsample_t s)
{
        TIFFDirectory *td = &tif->tif_dir;

        if (x >= td->td_imagewidth) {
                TIFFError(tif->tif_name, "Col %ld out of range, max %lu",
                    (long) x, (u_long) td->td_imagewidth);
                return (0);
        }
        if (y >= td->td_imagelength) {
                TIFFError(tif->tif_name, "Row %ld out of range, max %lu",
                    (long) y, (u_long) td->td_imagelength);
                return (0);
        }
        if (z >= td->td_imagedepth) {
                TIFFError(tif->tif_name, "Depth %ld out of range, max %lu",
                    (long) z, (u_long) td->td_imagedepth);
                return (0);
        }
        if (td->td_planarconfig == PLANARCONFIG_SEPARATE &&
            s >= td->td_samplesperpixel) {
                TIFFError(tif->tif_name, "Sample %d out of range, max %u",
                    (int) s, td->td_samplesperpixel);
                return (0);
        }
        return (1);
}

/*
 * Compute how many tiles are in an image.
 */
ttile_t
TIFFNumberOfTiles(TIFF* tif)
{
        TIFFDirectory *td = &tif->tif_dir;
        u_long dx = td->td_tilewidth;
        u_long dy = td->td_tilelength;
        u_long dz = td->td_tiledepth;
        ttile_t ntiles;

        if (dx == (u_long) -1)
                dx = td->td_imagewidth;
        if (dy == (u_long) -1)
                dy = td->td_imagelength;
        if (dz == (u_long) -1)
                dz = td->td_imagedepth;
        ntiles = (dx != 0 && dy != 0 && dz != 0) ?
            (howmany(td->td_imagewidth, dx) * howmany(td->td_imagelength, dy) *
                howmany(td->td_imagedepth, dz)) :
            0;
        if (td->td_planarconfig == PLANARCONFIG_SEPARATE)
                ntiles *= td->td_samplesperpixel;
        return (ntiles);
}

/*
 * Compute the # bytes in each row of a tile.
 */
tsize_t
TIFFTileRowSize(TIFF* tif)
{
        TIFFDirectory *td = &tif->tif_dir;
        tsize_t rowsize;

        if (td->td_tilelength == 0 || td->td_tilewidth == 0)
                return ((tsize_t) 0);
        rowsize = td->td_bitspersample * td->td_tilewidth;
        if (td->td_planarconfig == PLANARCONFIG_CONTIG)
                rowsize *= td->td_samplesperpixel;
        return (howmany(rowsize, 8));
}

/*
 * Compute the # bytes in a variable length, row-aligned tile.
 */
tsize_t
TIFFVTileSize(TIFF* tif, uint32 nrows)
{
        TIFFDirectory *td = &tif->tif_dir;
        tsize_t tilesize;

        if (td->td_tilelength == 0 || td->td_tilewidth == 0 ||
            td->td_tiledepth == 0)
                return ((tsize_t) 0);
#ifdef YCBCR_SUPPORT
        if (td->td_planarconfig == PLANARCONFIG_CONTIG &&
            td->td_photometric == PHOTOMETRIC_YCBCR) {
                /*
                 * Packed YCbCr data contain one Cb+Cr for every
                 * HorizontalSampling*VerticalSampling Y values.
                 * Must also roundup width and height when calculating
                 * since images that are not a multiple of the
                 * horizontal/vertical subsampling area include
                 * YCbCr data for the extended image.
                 */
                tsize_t w =
                    roundup(td->td_tilewidth, td->td_ycbcrsubsampling[0]);
                tsize_t rowsize = howmany(w*td->td_bitspersample, 8);
                tsize_t samplingarea =
                    td->td_ycbcrsubsampling[0]*td->td_ycbcrsubsampling[1];
                nrows = roundup(nrows, td->td_ycbcrsubsampling[1]);
                /* NB: don't need howmany here 'cuz everything is rounded */
                tilesize = nrows*rowsize + 2*(nrows*rowsize / samplingarea);
        } else
#endif
                tilesize = nrows * TIFFTileRowSize(tif);
        return (tilesize * td->td_tiledepth);
}

/*
 * Compute the # bytes in a row-aligned tile.
 */
tsize_t
TIFFTileSize(TIFF* tif)
{
        return (TIFFVTileSize(tif, tif->tif_dir.td_tilelength));
}
