#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
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
 * CCITT Group 3 1-D Modified Huffman
 * Run Length Encoding Compression Support
 */
#include "tiffiop.h"
#include "tif_fax3.h"

TIFFInitCCITTRLE(TIFF* tif)
{
	TIFFInitCCITTFax3(tif);		/* reuse G3 compression */
	tif->tif_preencode = NULL;
	tif->tif_postencode = NULL;
	tif->tif_encoderow = TIFFNoRowEncode;
	tif->tif_encodestrip = TIFFNoStripEncode;
	tif->tif_encodetile = TIFFNoTileEncode;
	tif->tif_close = NULL;
	/*
	 * This magic causes the regular G3 decompression
	 * code to not skip to the EOL mark at the end of
	 * a row, and to flush input data to a byte boundary
	 * at the end of each row.
	 */
	tif->tif_options |= FAX3_NOEOL|FAX3_BYTEALIGN;
	return (1);
}

int
TIFFInitCCITTRLEW(TIFF* tif)
{
	TIFFInitCCITTFax3(tif);		/* reuse G3 compression */
	tif->tif_preencode = NULL;
	tif->tif_postencode = NULL;
	tif->tif_encoderow = TIFFNoRowEncode;
	tif->tif_encodestrip = TIFFNoStripEncode;
	tif->tif_encodetile = TIFFNoTileEncode;
	tif->tif_close = NULL;
	/*
	 * This magic causes the regular G3 decompression
	 * code to not skip to the EOL mark at the end of
	 * a row, and to flush input data to a byte boundary
	 * at the end of each row.
	 */
	tif->tif_options |= FAX3_NOEOL|FAX3_WORDALIGN;
	return (1);
}
