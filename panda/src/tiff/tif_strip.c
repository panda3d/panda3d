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
 * Strip-organized Image Support Routines.
 */
#include "tiffiop.h"

/*
 * Compute which strip a (row,sample) value is in.
 */
tstrip_t
TIFFComputeStrip(TIFF* tif, uint32 row, tsample_t sample)
{
        TIFFDirectory *td = &tif->tif_dir;
        tstrip_t strip;

        strip = row / td->td_rowsperstrip;
        if (td->td_planarconfig == PLANARCONFIG_SEPARATE) {
                if (sample >= td->td_samplesperpixel) {
                        TIFFError(tif->tif_name,
                            "%u: Sample out of range, max %u",
                            sample, td->td_samplesperpixel);
                        return ((tstrip_t) 0);
                }
                strip += sample*td->td_stripsperimage;
        }
        return (strip);
}

/*
 * Compute how many strips are in an image.
 */
tstrip_t
TIFFNumberOfStrips(TIFF* tif)
{
        TIFFDirectory *td = &tif->tif_dir;
        tstrip_t nstrips;

        nstrips = (td->td_rowsperstrip == 0xffffffff ?
             (td->td_imagelength != 0 ? 1 : 0) :
             howmany(td->td_imagelength, td->td_rowsperstrip));
        if (td->td_planarconfig == PLANARCONFIG_SEPARATE)
                nstrips *= td->td_samplesperpixel;
        return (nstrips);
}

/*
 * Compute the # bytes in a variable height, row-aligned strip.
 */
tsize_t
TIFFVStripSize(TIFF* tif, uint32 nrows)
{
        TIFFDirectory *td = &tif->tif_dir;

        if (nrows == (uint32) -1)
                nrows = td->td_imagelength;
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
                    roundup(td->td_imagewidth, td->td_ycbcrsubsampling[0]);
                tsize_t scanline = howmany(w*td->td_bitspersample, 8);
                tsize_t samplingarea =
                    td->td_ycbcrsubsampling[0]*td->td_ycbcrsubsampling[1];
                nrows = roundup(nrows, td->td_ycbcrsubsampling[1]);
                /* NB: don't need howmany here 'cuz everything is rounded */
                return (nrows*scanline + 2*(nrows*scanline / samplingarea));
        } else
#endif
                return (nrows * TIFFScanlineSize(tif));
}

/*
 * Compute the # bytes in a (row-aligned) strip.
 */
tsize_t
TIFFStripSize(TIFF* tif)
{
        return (TIFFVStripSize(tif, tif->tif_dir.td_rowsperstrip));
}
