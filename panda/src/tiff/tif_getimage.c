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
 * TIFF Library
 *
 * Read and return a packed RGBA image.
 */
#include "tiffiop.h"

typedef	u_char RGBvalue;

static	u_short bitspersample;
static	u_short samplesperpixel;
static	u_short photometric;
static	u_short orientation;
static	u_short extrasamples;
static	u_short planarconfig;
/* colormap for pallete images */
static	u_short *redcmap, *greencmap, *bluecmap;
static	int stoponerr;			/* stop on read error */
/* YCbCr support */
static	u_short YCbCrHorizSampling;
static	u_short YCbCrVertSampling;
static	float *YCbCrCoeffs;
static	float *refBlackWhite;

static	u_long **BWmap;
static	u_long **PALmap;

static	int gt(TIFF*, int, int, u_long*);
static	int gtTileContig(TIFF*, u_long*, RGBvalue*, u_long, u_long);
static	int gtTileSeparate(TIFF*, u_long*, RGBvalue*, u_long, u_long);
static	int gtStripContig(TIFF*, u_long*, RGBvalue*, u_long, u_long);
static	int gtStripSeparate(TIFF*, u_long*, RGBvalue*, u_long, u_long);
static	int makebwmap(TIFF*, RGBvalue*);
static	int makecmap(TIFF*, u_short*, u_short*, u_short*);
static	void initYCbCrConversion(void);

int
TIFFReadRGBAImage(TIFF* tif,
    u_long rwidth, u_long rheight, u_long* raster, int stop)
{
	int ok, alpha;
	u_long width, height;
	u_short *sampleinfo;

	TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	switch (bitspersample) {
	case 1: case 2: case 4:
	case 8: case 16:
		break;
	default:
		TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle %d-bit images", bitspersample);
		return (0);
	}
	TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	if (samplesperpixel > 4) {
		TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle images with %d-samples/pixel",
		    samplesperpixel);
		return (0);
	}
	TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES,
	    &extrasamples, &sampleinfo);
	alpha = (extrasamples == 1 && sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA);
	TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planarconfig);
	switch (samplesperpixel - extrasamples) {
	case 3:
		break;
	case 1: case 4:
/* XXX */
		if (!alpha || planarconfig != PLANARCONFIG_CONTIG)
			break;
		/* fall thru... */
	default:
		TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle %d-channel %s images%s",
		    samplesperpixel,
		    planarconfig == PLANARCONFIG_CONTIG ?
			"packed" : "separated",
		    alpha ? " with alpha" : "");
		return (0);
	}
	if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric)) {
		switch (samplesperpixel) {
		case 1:
			photometric = PHOTOMETRIC_MINISBLACK;
			break;
		case 3: case 4:
			photometric = PHOTOMETRIC_RGB;
			break;
		default:
			TIFFError(TIFFFileName(tif),
			    "Missing needed \"PhotometricInterpretation\" tag");
			return (0);
		}
		TIFFError(TIFFFileName(tif),
		    "No \"PhotometricInterpretation\" tag, assuming %s\n",
		    photometric == PHOTOMETRIC_RGB ? "RGB" : "min-is-black");
	}
	switch (photometric) {
	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
	case PHOTOMETRIC_RGB:
	case PHOTOMETRIC_PALETTE:
	case PHOTOMETRIC_YCBCR:
		break;
	case PHOTOMETRIC_SEPARATED: {
		u_short inkset;
		TIFFGetFieldDefaulted(tif, TIFFTAG_INKSET, &inkset);
		if (inkset != INKSET_CMYK) {
			TIFFError(TIFFFileName(tif),
			    "Sorry, can not handle separated image with %s=%d",
			    "InkSet", inkset);
			return (0);
		}
		break;
	}
	default:
		TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle image with %s=%d",
		    "PhotometricInterpretation", photometric);
		return (0);
	}
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	/* XXX verify rwidth and rheight against width and height */
	stoponerr = stop;
	BWmap = NULL;
	PALmap = NULL;
	ok = gt(tif, rwidth, height, raster + (rheight-height)*rwidth);
	if (BWmap)
		_TIFFfree((char *)BWmap);
	if (PALmap)
		_TIFFfree((char *)PALmap);
	return (ok);
}

static int
checkcmap(int n, u_short* r, u_short* g, u_short* b)
{
	while (n-- > 0)
		if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
			return (16);
	return (8);
}

/*
 * Construct a mapping table to convert from the range
 * of the data samples to [0,255] --for display.  This
 * process also handles inverting B&W images when needed.
 */ 
static int
setupMap(TIFF* tif,
    u_short minsamplevalue, u_short maxsamplevalue, RGBvalue** pMap)
{
	register int x, range;
	RGBvalue *Map;

	range = maxsamplevalue - minsamplevalue;
	Map = (RGBvalue *)_TIFFmalloc((range + 1) * sizeof (RGBvalue));
	if (Map == NULL) {
		TIFFError(TIFFFileName(tif),
		    "No space for photometric conversion table");
		return (0);
	}
	if (photometric == PHOTOMETRIC_MINISWHITE) {
		for (x = 0; x <= range; x++)
			Map[x] = ((range - x) * 255) / range;
	} else {
		for (x = 0; x <= range; x++)
			Map[x] = (x * 255) / range;
	}
	if (bitspersample <= 8 &&
	    (photometric == PHOTOMETRIC_MINISBLACK ||
	     photometric == PHOTOMETRIC_MINISWHITE)) {
		/*
		 * Use photometric mapping table to construct
		 * unpacking tables for samples <= 8 bits.
		 */
		if (!makebwmap(tif, Map))
			return (0);
		/* no longer need Map, free it */
		_TIFFfree((char *)Map);
		Map = NULL;
	}
	*pMap = Map;
	return (1);
}

static int
gt(TIFF* tif, int w, int h, u_long* raster)
{
	u_short minsamplevalue, maxsamplevalue;
	RGBvalue *Map;
	int e, ncomps;

	TIFFGetFieldDefaulted(tif, TIFFTAG_MINSAMPLEVALUE, &minsamplevalue);
	TIFFGetFieldDefaulted(tif, TIFFTAG_MAXSAMPLEVALUE, &maxsamplevalue);
	Map = NULL;
	switch (photometric) {
	case PHOTOMETRIC_YCBCR:
		TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRCOEFFICIENTS,
		    &YCbCrCoeffs);
		TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRSUBSAMPLING,
		    &YCbCrHorizSampling, &YCbCrVertSampling);
		TIFFGetFieldDefaulted(tif, TIFFTAG_REFERENCEBLACKWHITE,
		    &refBlackWhite);
		initYCbCrConversion();
		/* fall thru... */
	case PHOTOMETRIC_RGB:
	case PHOTOMETRIC_SEPARATED:
		if (minsamplevalue == 0 && maxsamplevalue == 255)
			break;
		/* fall thru... */
	case PHOTOMETRIC_MINISBLACK:
	case PHOTOMETRIC_MINISWHITE:
		if (!setupMap(tif, minsamplevalue, maxsamplevalue, &Map))
			return (0);
		break;
	case PHOTOMETRIC_PALETTE:
		if (!TIFFGetField(tif, TIFFTAG_COLORMAP,
				  &redcmap, &greencmap, &bluecmap)) {
			TIFFError(TIFFFileName(tif),
			    "Missing required \"Colormap\" tag");
			return (0);
		}
		/*
		 * Convert 16-bit colormap to 8-bit (unless it looks
		 * like an old-style 8-bit colormap).
		 */
		if (checkcmap(1<<bitspersample, redcmap, greencmap, bluecmap) == 16) {
			int i;
			for (i = (1<<bitspersample)-1; i > 0; i--) {
#define	CVT(x)		(((x) * 255) / ((1L<<16)-1))
				redcmap[i] = CVT(redcmap[i]);
				greencmap[i] = CVT(greencmap[i]);
				bluecmap[i] = CVT(bluecmap[i]);
			}
		} else
			TIFFWarning(TIFFFileName(tif), "Assuming 8-bit colormap");
		if (bitspersample <= 8) {
			/*
			 * Use mapping table and colormap to construct
			 * unpacking tables for samples < 8 bits.
			 */
			if (!makecmap(tif, redcmap, greencmap, bluecmap))
				return (0);
		}
		break;
	}
	ncomps = samplesperpixel - extrasamples;
	if (planarconfig == PLANARCONFIG_SEPARATE && ncomps > 1) {
		e = TIFFIsTiled(tif) ?
		    gtTileSeparate(tif, raster, Map, h, w) :
		    gtStripSeparate(tif, raster, Map, h, w);
	} else {
		e = TIFFIsTiled(tif) ? 
		    gtTileContig(tif, raster, Map, h, w) :
		    gtStripContig(tif, raster, Map, h, w);
	}
	if (Map)
		_TIFFfree((char *)Map);
	return (e);
}

static u_long
setorientation(TIFF* tif, u_long h)
{
	u_long y;

	TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orientation);
	switch (orientation) {
	case ORIENTATION_BOTRIGHT:
	case ORIENTATION_RIGHTBOT:	/* XXX */
	case ORIENTATION_LEFTBOT:	/* XXX */
		TIFFWarning(TIFFFileName(tif), "using bottom-left orientation");
		orientation = ORIENTATION_BOTLEFT;
		/* fall thru... */
	case ORIENTATION_BOTLEFT:
		y = 0;
		break;
	case ORIENTATION_TOPRIGHT:
	case ORIENTATION_RIGHTTOP:	/* XXX */
	case ORIENTATION_LEFTTOP:	/* XXX */
	default:
		TIFFWarning(TIFFFileName(tif), "using top-left orientation");
		orientation = ORIENTATION_TOPLEFT;
		/* fall thru... */
	case ORIENTATION_TOPLEFT:
		y = h-1;
		break;
	}
	return (y);
}

typedef void (*tileContigRoutine)
    (u_long*, u_char*, RGBvalue*, u_long, u_long, int, int);
static tileContigRoutine pickTileContigCase(TIFF*, RGBvalue*);

/*
 * Get an tile-organized image that has
 *	PlanarConfiguration contiguous if SamplesPerPixel > 1
 * or
 *	SamplesPerPixel == 1
 */	
static int
gtTileContig(TIFF* tif, u_long* raster, RGBvalue* Map, u_long h, u_long w)
{
	u_long col, row, y;
	u_long tw, th;
	u_char *buf;
	int fromskew, toskew;
	u_int nrow;
	tileContigRoutine put;

	put = pickTileContigCase(tif, Map);
	if (put == 0)
		return (0);
	buf = (u_char *)_TIFFmalloc(TIFFTileSize(tif));
	if (buf == 0) {
		TIFFError(TIFFFileName(tif), "No space for tile buffer");
		return (0);
	}
	TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);
	y = setorientation(tif, h);
	toskew = (orientation == ORIENTATION_TOPLEFT ? -tw + -w : -tw + w);
	for (row = 0; row < h; row += th) {
		nrow = (row + th > h ? h - row : th);
		for (col = 0; col < w; col += tw) {
			if (TIFFReadTile(tif, buf, col, row, 0, 0) < 0 &&
			    stoponerr)
				break;
			if (col + tw > w) {
				/*
				 * Tile is clipped horizontally.  Calculate
				 * visible portion and skewing factors.
				 */
				u_long npix = w - col;
				fromskew = tw - npix;
				(*put)(raster + y*w + col, buf, Map,
				    npix, nrow, fromskew, toskew + fromskew);
			} else
				(*put)(raster + y*w + col, buf, Map,
				    tw, nrow, 0, toskew);
		}
		y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
	}
	_TIFFfree(buf);
	return (1);
}

typedef void (*tileSeparateRoutine)
    (u_long*, u_char*, u_char*, u_char*, RGBvalue*, u_long, u_long, int, int);
static tileSeparateRoutine pickTileSeparateCase(TIFF*, RGBvalue*);

/*
 * Get an tile-organized image that has
 *	 SamplesPerPixel > 1
 *	 PlanarConfiguration separated
 * We assume that all such images are RGB.
 */	
static int
gtTileSeparate(TIFF* tif, u_long* raster, RGBvalue* Map, u_long h, u_long w)
{
	u_long col, row, y;
	u_long tw, th;
	u_char *buf;
	u_char *r, *g, *b;
	u_long tilesize;
	int fromskew, toskew;
	u_int nrow;
	tileSeparateRoutine put;

	put = pickTileSeparateCase(tif, Map);
	if (put == 0)
		return (0);
	tilesize = TIFFTileSize(tif);
	buf = (u_char *)_TIFFmalloc(3*tilesize);
	if (buf == 0) {
		TIFFError(TIFFFileName(tif), "No space for tile buffer");
		return (0);
	}
	r = buf;
	g = r + tilesize;
	b = g + tilesize;
	TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);
	y = setorientation(tif, h);
	toskew = (orientation == ORIENTATION_TOPLEFT ? -tw + -w : -tw + w);
	for (row = 0; row < h; row += th) {
		nrow = (row + th > h ? h - row : th);
		for (col = 0; col < w; col += tw) {
			if (TIFFReadTile(tif, r, col, row,0,0) < 0 && stoponerr)
				break;
			if (TIFFReadTile(tif, g, col, row,0,1) < 0 && stoponerr)
				break;
			if (TIFFReadTile(tif, b, col, row,0,2) < 0 && stoponerr)
				break;
			if (col + tw > w) {
				/*
				 * Tile is clipped horizontally.  Calculate
				 * visible portion and skewing factors.
				 */
				u_long npix = w - col;
				fromskew = tw - npix;
				(*put)(raster + y*w + col, r, g, b, Map,
				    npix, nrow, fromskew, toskew + fromskew);
			} else
				(*put)(raster + y*w + col, r, g, b, Map,
				    tw, nrow, 0, toskew);
		}
		y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
	}
	_TIFFfree(buf);
	return (1);
}

/*
 * Get a strip-organized image that has
 *	PlanarConfiguration contiguous if SamplesPerPixel > 1
 * or
 *	SamplesPerPixel == 1
 */	
static int
gtStripContig(TIFF* tif, u_long* raster, RGBvalue* Map, u_long h, u_long w)
{
	u_long row, y, nrow;
	u_char *buf;
	tileContigRoutine put;
	u_long rowsperstrip;
	u_long imagewidth;
	u_long scanline;
	int fromskew, toskew;

	put = pickTileContigCase(tif, Map);
	if (put == 0)
		return (0);
	buf = (u_char *)_TIFFmalloc(TIFFStripSize(tif));
	if (buf == 0) {
		TIFFError(TIFFFileName(tif), "No space for strip buffer");
		return (0);
	}
	y = setorientation(tif, h);
	toskew = (orientation == ORIENTATION_TOPLEFT ? -w + -w : -w + w);
	TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
	scanline = TIFFScanlineSize(tif);
	fromskew = (w < imagewidth ? imagewidth - w : 0);
	for (row = 0; row < h; row += rowsperstrip) {
		nrow = (row + rowsperstrip > h ? h - row : rowsperstrip);
		if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0),
		    buf, nrow*scanline) < 0 && stoponerr)
			break;
		(*put)(raster + y*w, buf, Map, w, nrow, fromskew, toskew);
		y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
	}
	_TIFFfree(buf);
	return (1);
}

/*
 * Get a strip-organized image with
 *	 SamplesPerPixel > 1
 *	 PlanarConfiguration separated
 * We assume that all such images are RGB.
 */
static int
gtStripSeparate(TIFF* tif, u_long* raster, RGBvalue* Map, u_long h, u_long w)
{
	u_char *buf;
	u_char *r, *g, *b;
	u_long row, y, nrow;
	u_long scanline;
	tileSeparateRoutine put;
	u_long rowsperstrip;
	u_long imagewidth;
	u_long stripsize;
	int fromskew, toskew;

	stripsize = TIFFStripSize(tif);
	r = buf = (u_char *)_TIFFmalloc(3*stripsize);
	if (buf == 0)
		return (0);
	g = r + stripsize;
	b = g + stripsize;
	put = pickTileSeparateCase(tif, Map);
	if (put == 0) {
		TIFFError(TIFFFileName(tif), "Can not handle format");
		return (0);
	}
	y = setorientation(tif, h);
	toskew = (orientation == ORIENTATION_TOPLEFT ? -w + -w : -w + w);
	TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
	scanline = TIFFScanlineSize(tif);
	fromskew = (w < imagewidth ? imagewidth - w : 0);
	for (row = 0; row < h; row += rowsperstrip) {
		nrow = (row + rowsperstrip > h ? h - row : rowsperstrip);
		if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0),
		    r, nrow*scanline) < 0 && stoponerr)
			break;
		if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 1),
		    g, nrow*scanline) < 0 && stoponerr)
			break;
		if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 2),
		    b, nrow*scanline) < 0 && stoponerr)
			break;
		(*put)(raster + y*w, r, g, b, Map, w, nrow, fromskew, toskew);
		y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
	}
	_TIFFfree(buf);
	return (1);
}

#define	PACK(r,g,b)	((u_long)(r)|((u_long)(g)<<8)|((u_long)(b)<<16))

/*
 * Greyscale images with less than 8 bits/sample are handled
 * with a table to avoid lots of shifts and masks.  The table
 * is setup so that put*bwtile (below) can retrieve 8/bitspersample
 * pixel values simply by indexing into the table with one
 * number.
 */
static int
makebwmap(TIFF* tif, RGBvalue* Map)
{
	register int i;
	int nsamples = 8 / bitspersample;
	register u_long *p;

	BWmap = (u_long **)_TIFFmalloc(
	    256*sizeof (u_long *)+(256*nsamples*sizeof(u_long)));
	if (BWmap == NULL) {
		TIFFError(TIFFFileName(tif), "No space for B&W mapping table");
		return (0);
	}
	p = (u_long *)(BWmap + 256);
	for (i = 0; i < 256; i++) {
		BWmap[i] = p;
		switch (bitspersample) {
			register RGBvalue c;
#define	GREY(x)	c = Map[x]; *p++ = PACK(c,c,c);
		case 1:
			GREY(i>>7);
			GREY((i>>6)&1);
			GREY((i>>5)&1);
			GREY((i>>4)&1);
			GREY((i>>3)&1);
			GREY((i>>2)&1);
			GREY((i>>1)&1);
			GREY(i&1);
			break;
		case 2:
			GREY(i>>6);
			GREY((i>>4)&3);
			GREY((i>>2)&3);
			GREY(i&3);
			break;
		case 4:
			GREY(i>>4);
			GREY(i&0xf);
			break;
		case 8:
			GREY(i);
			break;
		}
#undef	GREY
	}
	return (1);
}

/*
 * Palette images with <= 8 bits/sample are handled
 * with a table to avoid lots of shifts and masks.  The table
 * is setup so that put*cmaptile (below) can retrieve 8/bitspersample
 * pixel values simply by indexing into the table with one
 * number.
 */
static int
makecmap(TIFF* tif, u_short* rmap, u_short* gmap, u_short* bmap)
{
	register int i;
	int nsamples = 8 / bitspersample;
	register u_long *p;

	PALmap = (u_long **)_TIFFmalloc(
	    256*sizeof (u_long *)+(256*nsamples*sizeof(u_long)));
	if (PALmap == NULL) {
		TIFFError(TIFFFileName(tif), "No space for Palette mapping table");
		return (0);
	}
	p = (u_long *)(PALmap + 256);
	for (i = 0; i < 256; i++) {
		PALmap[i] = p;
#define	CMAP(x)	\
c = x; *p++ = PACK(rmap[c]&0xff, gmap[c]&0xff, bmap[c]&0xff);
		switch (bitspersample) {
			register RGBvalue c;
		case 1:
			CMAP(i>>7);
			CMAP((i>>6)&1);
			CMAP((i>>5)&1);
			CMAP((i>>4)&1);
			CMAP((i>>3)&1);
			CMAP((i>>2)&1);
			CMAP((i>>1)&1);
			CMAP(i&1);
			break;
		case 2:
			CMAP(i>>6);
			CMAP((i>>4)&3);
			CMAP((i>>2)&3);
			CMAP(i&3);
			break;
		case 4:
			CMAP(i>>4);
			CMAP(i&0xf);
			break;
		case 8:
			CMAP(i);
			break;
		}
#undef CMAP
	}
	return (1);
}

/*
 * The following routines move decoded data returned
 * from the TIFF library into rasters filled with packed
 * ABGR pixels (i.e. suitable for passing to lrecwrite.)
 *
 * The routines have been created according to the most
 * important cases and optimized.  pickTileContigCase and
 * pickTileSeparateCase analyze the parameters and select
 * the appropriate "put" routine to use.
 */
#define	REPEAT8(op)	REPEAT4(op); REPEAT4(op)
#define	REPEAT4(op)	REPEAT2(op); REPEAT2(op)
#define	REPEAT2(op)	op; op
#define	CASE8(x,op)				\
	switch (x) {				\
	case 7: op; case 6: op; case 5: op;	\
	case 4: op; case 3: op; case 2: op;	\
	case 1: op;				\
	}
#define	CASE4(x,op)	switch (x) { case 3: op; case 2: op; case 1: op; }

#define	UNROLL8(w, op1, op2) {		\
	register u_long x;		\
	for (x = w; x >= 8; x -= 8) {	\
		op1;			\
		REPEAT8(op2);		\
	}				\
	if (x > 0) {			\
		op1;			\
		CASE8(x,op2);		\
	}				\
}
#define	UNROLL4(w, op1, op2) {		\
	register u_long x;		\
	for (x = w; x >= 4; x -= 4) {	\
		op1;			\
		REPEAT4(op2);		\
	}				\
	if (x > 0) {			\
		op1;			\
		CASE4(x,op2);		\
	}				\
}
#define	UNROLL2(w, op1, op2) {		\
	register u_long x;		\
	for (x = w; x >= 2; x -= 2) {	\
		op1;			\
		REPEAT2(op2);		\
	}				\
	if (x) {			\
		op1;			\
		op2;			\
	}				\
}
			

#define	SKEW(r,g,b,skew)	{ r += skew; g += skew; b += skew; }

#define	DECLAREContigPutFunc(name) \
static void name(\
    u_long* cp, \
    u_char* pp, \
    RGBvalue* Map, \
    u_long w, u_long h, \
    int fromskew, int toskew \
)

/*
 * 8-bit palette => colormap/RGB
 */
DECLAREContigPutFunc(put8bitcmaptile)
{
	while (h-- > 0) {
		UNROLL8(w, NULL, *cp++ = PALmap[*pp++][0]);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 4-bit palette => colormap/RGB
 */
DECLAREContigPutFunc(put4bitcmaptile)
{
	register u_long *bw;

	fromskew /= 2;
	while (h-- > 0) {
		UNROLL2(w, bw = PALmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 2-bit palette => colormap/RGB
 */
DECLAREContigPutFunc(put2bitcmaptile)
{
	register u_long *bw;

	fromskew /= 4;
	while (h-- > 0) {
		UNROLL4(w, bw = PALmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 1-bit palette => colormap/RGB
 */
DECLAREContigPutFunc(put1bitcmaptile)
{
	register u_long *bw;

	fromskew /= 8;
	while (h-- > 0) {
		UNROLL8(w, bw = PALmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 8-bit greyscale => colormap/RGB
 */
DECLAREContigPutFunc(putgreytile)
{
	while (h-- > 0) {
		register u_long x;
		for (x = w; x-- > 0;)
			*cp++ = BWmap[*pp++][0];
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 1-bit bilevel => colormap/RGB
 */
DECLAREContigPutFunc(put1bitbwtile)
{
	register u_long *bw;

	fromskew /= 8;
	while (h-- > 0) {
		UNROLL8(w, bw = BWmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 2-bit greyscale => colormap/RGB
 */
DECLAREContigPutFunc(put2bitbwtile)
{
	register u_long *bw;

	fromskew /= 4;
	while (h-- > 0) {
		UNROLL4(w, bw = BWmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 4-bit greyscale => colormap/RGB
 */
DECLAREContigPutFunc(put4bitbwtile)
{
	register u_long *bw;

	fromskew /= 2;
	while (h-- > 0) {
		UNROLL2(w, bw = BWmap[*pp++], *cp++ = *bw++);
		cp += toskew;
		pp += fromskew;
	}
}

/*
 * 8-bit packed samples => RGB
 */
DECLAREContigPutFunc(putRGBcontig8bittile)
{
	fromskew *= samplesperpixel;
	if (Map) {
		while (h-- > 0) {
			register u_long x;
			for (x = w; x-- > 0;) {
				*cp++ = PACK(Map[pp[0]], Map[pp[1]], Map[pp[2]]);
				pp += samplesperpixel;
			}
			pp += fromskew;
			cp += toskew;
		}
	} else {
		while (h-- > 0) {
			UNROLL8(w, NULL,
			    *cp++ = PACK(pp[0], pp[1], pp[2]);
			    pp += samplesperpixel);
			cp += toskew;
			pp += fromskew;
		}
	}
}

/*
 * 16-bit packed samples => RGB
 */
DECLAREContigPutFunc(putRGBcontig16bittile)
{
	register u_short *wp = (u_short *)pp;
	register u_int x;

	fromskew *= samplesperpixel;
	if (Map) {
		while (h-- > 0) {
			for (x = w; x-- > 0;) {
				*cp++ = PACK(Map[wp[0]], Map[wp[1]], Map[wp[2]]);
				wp += samplesperpixel;
			}
			cp += toskew;
			wp += fromskew;
		}
	} else {
		while (h-- > 0) {
			for (x = w; x-- > 0;) {
				*cp++ = PACK(wp[0], wp[1], wp[2]);
				wp += samplesperpixel;
			}
			cp += toskew;
			wp += fromskew;
		}
	}
}

/*
 * 8-bit packed CMYK samples => RGB
 *
 * NB: The conversion of CMYK->RGB is *very* crude.
 */
DECLAREContigPutFunc(putRGBcontig8bitCMYKtile)
{
	u_short r, g, b, k;

	fromskew *= samplesperpixel;
	if (Map) {
		while (h-- > 0) {
			register u_long x;
			for (x = w; x-- > 0;) {
				k = 255 - pp[3];
				r = (k*(255-pp[0]))/255;
				g = (k*(255-pp[1]))/255;
				b = (k*(255-pp[2]))/255;
				*cp++ = PACK(Map[r], Map[g], Map[b]);
				pp += samplesperpixel;
			}
			pp += fromskew;
			cp += toskew;
		}
	} else {
		while (h-- > 0) {
			UNROLL8(w, NULL,
			    k = 255 - pp[3];
			    r = (k*(255-pp[0]))/255;
			    g = (k*(255-pp[1]))/255;
			    b = (k*(255-pp[2]))/255;
			    *cp++ = PACK(r, g, b);
			    pp += samplesperpixel);
			cp += toskew;
			pp += fromskew;
		}
	}
}

#define	DECLARESepPutFunc(name) \
static void name(\
    u_long* cp, \
    u_char* r, u_char* g, u_char* b, \
    RGBvalue* Map, \
    u_long w, u_long h, \
    int fromskew, int toskew \
)

/*
 * 8-bit unpacked samples => RGB
 */
DECLARESepPutFunc(putRGBseparate8bittile)
{
	if (Map) {
		while (h-- > 0) {
			register u_long x;
			for (x = w; x > 0; x--)
				*cp++ = PACK(Map[*r++], Map[*g++], Map[*b++]);
			SKEW(r, g, b, fromskew);
			cp += toskew;
		}
	} else {
		while (h-- > 0) {
			UNROLL8(w, NULL, *cp++ = PACK(*r++, *g++, *b++));
			SKEW(r, g, b, fromskew);
			cp += toskew;
		}
	}
}

/*
 * 16-bit unpacked samples => RGB
 */
DECLARESepPutFunc(putRGBseparate16bittile)
{
	register u_short *wr = (u_short *)r;
	register u_short *wg = (u_short *)g;
	register u_short *wb = (u_short *)b;
	register u_long x;

	if (Map) {
		while (h-- > 0) {
			for (x = w; x > 0; x--)
				*cp++ = PACK(Map[*wr++],Map[*wg++],Map[*wb++]);
			SKEW(wr, wg, wb, fromskew);
			cp += toskew;
		}
	} else {
		while (h-- > 0) {
			for (x = 0; x < w; x++)
				*cp++ = PACK(*wr++, *wg++, *wb++);
			SKEW(wr, wg, wb, fromskew);
			cp += toskew;
		}
	}
}

#define	Code2V(c, RB, RW, CR)	((((c)-RB)*(float)CR)/(float)(RW-RB))
#define	CLAMP(f,min,max) \
    (int)((f)+.5 < (min) ? (min) : (f)+.5 > (max) ? (max) : (f)+.5)

#define	LumaRed		YCbCrCoeffs[0]
#define	LumaGreen	YCbCrCoeffs[1]
#define	LumaBlue	YCbCrCoeffs[2]

static	float D1, D2;
static	float D3, D4;

static void
initYCbCrConversion(void)
{
	D1 = 2 - 2*LumaRed;
	D2 = D1*LumaRed / LumaGreen;
	D3 = 2 - 2*LumaBlue;
	D4 = D3*LumaBlue / LumaGreen;
}

static void
putRGBContigYCbCrClump(
	register u_long* cp, register u_char* pp,
	int cw, int ch,
	u_long w,
	int n, int fromskew, int toskew
)
{
	float Cb, Cr;
	int j, k;

	Cb = Code2V(pp[n],   refBlackWhite[2], refBlackWhite[3], 127);
	Cr = Code2V(pp[n+1], refBlackWhite[4], refBlackWhite[5], 127);
	for (j = 0; j < ch; j++) {
		for (k = 0; k < cw; k++) {
			float Y, R, G, B;
			Y = Code2V(*pp++,
			    refBlackWhite[0], refBlackWhite[1], 255);
			R = Y + Cr*D1;
			B = Y + Cb*D3;
			G = Y - Cb*D4 - Cr*D2;
			cp[k] = PACK(CLAMP(R,0,255),
				     CLAMP(G,0,255),
				     CLAMP(B,0,255));
		}
		cp += w+toskew;
		pp += fromskew;
	}
}
#undef LumaBlue
#undef LumaGreen
#undef LumaRed
#undef CLAMP
#undef Code2V

/*
 * 8-bit packed YCbCr samples => RGB
 */
DECLAREContigPutFunc(putcontig8bitYCbCrtile)
{
	u_int Coff = YCbCrVertSampling * YCbCrHorizSampling;
	u_long *tp;
	u_long x;

	/* XXX adjust fromskew */
	while (h >= YCbCrVertSampling) {
		tp = cp;
		for (x = w; x >= YCbCrHorizSampling; x -= YCbCrHorizSampling) {
			putRGBContigYCbCrClump(tp, pp,
			    YCbCrHorizSampling, YCbCrVertSampling,
			    w, Coff, 0, toskew);
			tp += YCbCrHorizSampling;
			pp += Coff+2;
		}
		if (x > 0) {
			putRGBContigYCbCrClump(tp, pp,
			    x, YCbCrVertSampling,
			    w, Coff, YCbCrHorizSampling - x, toskew);
			pp += Coff+2;
		}
		cp += YCbCrVertSampling*(w + toskew);
		pp += fromskew;
		h -= YCbCrVertSampling;
	}
	if (h > 0) {
		tp = cp;
		for (x = w; x >= YCbCrHorizSampling; x -= YCbCrHorizSampling) {
			putRGBContigYCbCrClump(tp, pp, YCbCrHorizSampling, h,
			    w, Coff, 0, toskew);
			tp += YCbCrHorizSampling;
			pp += Coff+2;
		}
		if (x > 0)
			putRGBContigYCbCrClump(tp, pp, x, h,
			    w, Coff, YCbCrHorizSampling - x, toskew);
	}
}

/*
 * Select the appropriate conversion routine for packed data.
 */
static tileContigRoutine
pickTileContigCase(TIFF* tif, RGBvalue* Map)
{
	tileContigRoutine put = 0;

	switch (photometric) {
	case PHOTOMETRIC_RGB:
		if (bitspersample == 8)
			put = putRGBcontig8bittile;
		else
			put = putRGBcontig16bittile;
		break;
	case PHOTOMETRIC_SEPARATED:
		if (bitspersample == 8)
			put = putRGBcontig8bitCMYKtile;
		break;
	case PHOTOMETRIC_PALETTE:
		switch (bitspersample) {
		case 8:	put = put8bitcmaptile; break;
		case 4: put = put4bitcmaptile; break;
		case 2: put = put2bitcmaptile; break;
		case 1: put = put1bitcmaptile; break;
		}
		break;
	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
		switch (bitspersample) {
		case 8:	put = putgreytile; break;
		case 4: put = put4bitbwtile; break;
		case 2: put = put2bitbwtile; break;
		case 1: put = put1bitbwtile; break;
		}
		break;
	case PHOTOMETRIC_YCBCR:
		switch (bitspersample) {
		case 8: put = putcontig8bitYCbCrtile; break;
		}
		break;
	}
	if (put == 0)
		TIFFError(TIFFFileName(tif), "Can not handle format");
	return (put);
}

/*
 * Select the appropriate conversion routine for unpacked data.
 *
 * NB: we assume that unpacked single channel data is directed
 *	 to the "packed routines.
 */
static tileSeparateRoutine
pickTileSeparateCase(TIFF* tif, RGBvalue* Map)
{
	tileSeparateRoutine put = 0;

	switch (photometric) {
	case PHOTOMETRIC_RGB:
		if (bitspersample == 8)
			put = putRGBseparate8bittile;
		else
			put = putRGBseparate16bittile;
		break;
	}
	if (put == 0)
		TIFFError(TIFFFileName(tif), "Can not handle format");
	return (put);
}
