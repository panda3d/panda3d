/* $Header$ */

/*
 * Copyright (c) 1990, 1991, 1992 Sam Leffler
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

#ifndef _FAX3_
#define	_FAX3_
/*
 * CCITT Group 3 compression/decompression definitions.
 */
#define	FAX3_CLASSF	TIFF_OPT0	/* use Class F protocol */
/* the following are for use by Compression=2, 32771, and 4 (T.6) algorithms */
#define	FAX3_NOEOL	TIFF_OPT1	/* no EOL code at end of row */
#define	FAX3_BYTEALIGN	TIFF_OPT2	/* force byte alignment at end of row */
#define	FAX3_WORDALIGN	TIFF_OPT3	/* force word alignment at end of row */

/*
 * Compression+decompression state blocks are
 * derived from this ``base state'' block.
 */
typedef struct {
	short	data;			/* current i/o byte */
	short	bit;			/* current i/o bit in byte */
	short	white;			/* value of the color ``white'' */
	u_long	rowbytes;		/* XXX maybe should be a long? */
	u_long	rowpixels;		/* XXX maybe should be a long? */
	enum { 				/* decoding/encoding mode */
	    G3_1D,			/* basic 1-d mode */
	    G3_2D			/* optional 2-d mode */
	} tag;
	const u_char *bitmap;		/* bit reversal table */
	u_char	*refline;		/* reference line for 2d decoding */
} Fax3BaseState;

/* these routines are used by Group 4 (T.6) */
extern	int Fax3Decode2DRow(TIFF*, u_char*, int);
extern	int Fax3Encode2DRow(TIFF*, u_char*, u_char*, int);
extern	void Fax3PutBits(TIFF*, u_int, u_int);
extern	void Fax3PutEOL(TIFF*);
extern	int TIFFInitCCITTFax3(TIFF*);

#define	Fax3FlushBits(tif, sp) {			\
	if ((tif)->tif_rawcc >= (tif)->tif_rawdatasize)	\
		(void) TIFFFlushData1(tif);		\
	*(tif)->tif_rawcp++ = (sp)->bitmap[(sp)->data];	\
	(tif)->tif_rawcc++;				\
	(sp)->data = 0;					\
	(sp)->bit = 8;					\
}
#endif /* _FAX3_ */
