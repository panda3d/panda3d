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
 * Rev 5.0 Lempel-Ziv & Welch Compression Support
 *
 * This code is derived from the compress program whose code is
 * derived from software contributed to Berkeley by James A. Woods,
 * derived from original work by Spencer Thomas and Joseph Orost.
 *
 * The original Berkeley copyright notice appears below in its entirety.
 */
#include "tiffiop.h"
#include <assert.h>
#include <stdio.h>

/*
 * NB: The 5.0 spec describes a different algorithm than Aldus
 *     implements.  Specifically, Aldus does code length transitions
 *     one code earlier than should be done (for real LZW).
 *     Earlier versions of this library implemented the correct
 *     LZW algorithm, but emitted codes in a bit order opposite
 *     to the TIFF spec.  Thus, to maintain compatibility w/ Aldus
 *     we interpret MSB-LSB ordered codes to be images written w/
 *     old versions of this library, but otherwise adhere to the
 *     Aldus "off by one" algorithm.
 *
 * Future revisions to the TIFF spec are expected to "clarify this issue".
 */
#define	LZW_COMPAT		/* include backwards compatibility code */
/*
 * Each strip of data is supposed to be terminated by a CODE_EOI.
 * If the following #define is included, the decoder will also
 * check for end-of-strip w/o seeing this code.  This makes the
 * library more robust, but also slower.
 */
#define	LZW_CHECKEOS		/* include checks for strips w/o EOI code */

#define MAXCODE(n)	((1<<(n))-1)
/*
 * The TIFF spec specifies that encoded bit
 * strings range from 9 to 12 bits.
 */
#define	BITS_MIN	9		/* start with 9 bits */
#define	BITS_MAX	12		/* max of 12 bit strings */
/* predefined codes */
#define	CODE_CLEAR	256		/* code to clear string table */
#define	CODE_EOI	257		/* end-of-information code */
#define CODE_FIRST	258		/* first free code entry */
#define	CODE_MAX	MAXCODE(BITS_MAX)
#define	HSIZE		9001		/* 91% occupancy */
#define	HSHIFT		(13-8)
#ifdef LZW_COMPAT
/* NB: +1024 is for compatibility with old files */
#define	CSIZE		(MAXCODE(BITS_MAX)+1024)
#else
#define	CSIZE		(MAXCODE(BITS_MAX)+1)
#endif

typedef	void (*predictorFunc)(char* data, u_long nbytes, int stride);

/*
 * State block for each open TIFF
 * file using LZW compression/decompression.
 */
typedef	struct {
	predictorFunc hordiff;		/* horizontal differencing method */
	u_long	rowsize;		/* width of tile/strip/row */
	u_short	stride;			/* horizontal diferencing stride */
	u_short	nbits;			/* # of bits/code */
	u_short	maxcode;		/* maximum code for lzw_nbits */
	u_short	free_ent;		/* next free entry in hash table */
	long	nextdata;		/* next bits of i/o */
	long	nextbits;		/* # of valid bits in lzw_nextdata */
} LZWState;

#define	lzw_hordiff	base.hordiff
#define	lzw_rowsize	base.rowsize
#define	lzw_stride	base.stride
#define	lzw_nbits	base.nbits
#define	lzw_maxcode	base.maxcode
#define	lzw_free_ent	base.free_ent
#define	lzw_nextdata	base.nextdata
#define	lzw_nextbits	base.nextbits

/*
 * Decoding-specific state.
 */
typedef struct code_ent {
	struct code_ent *next;
	u_short	length;			/* string len, including this token */
	u_char	value;			/* data value */
	u_char	firstchar;		/* first token of string */
} code_t;

typedef	int (*decodeFunc)(TIFF*, tidata_t, tsize_t, tsample_t);

typedef struct {
	LZWState base;
	long	dec_nbitsmask;		/* lzw_nbits 1 bits, right adjusted */
	long	dec_restart;		/* restart count */
#ifdef LZW_CHECKEOS
	long	dec_bitsleft;		/* available bits in raw data */
#endif
	decodeFunc dec_decode;		/* regular or backwards compatible */
	code_t	*dec_codep;		/* current recognized code */
	code_t	*dec_oldcodep;		/* previously recognized code */
	code_t	*dec_free_entp;		/* next free entry */
	code_t	*dec_maxcodep;		/* max available entry */
	code_t	dec_codetab[CSIZE];
} LZWDecodeState;

/*
 * Encoding-specific state.
 */
typedef struct {
	long	hash;
	long	code;
} hash_t;

typedef struct {
	LZWState base;
	int	enc_oldcode;		/* last code encountered */
	long	enc_checkpoint;		/* point at which to clear table */
#define CHECK_GAP	10000		/* enc_ratio check interval */
	long	enc_ratio;		/* current compression ratio */
	long	enc_incount;		/* (input) data bytes encoded */
	long	enc_outcount;		/* encoded (output) bytes */
	tidata_t enc_rawlimit;		/* bound on tif_rawdata buffer */
	hash_t	enc_hashtab[HSIZE];
} LZWEncodeState;

static	int LZWEncodePredRow(TIFF*, tidata_t, tsize_t, tsample_t);
static	int LZWEncodePredTile(TIFF*, tidata_t, tsize_t, tsample_t);
static	int LZWDecode(TIFF*, tidata_t, tsize_t, tsample_t);
#ifdef LZW_COMPAT
static	int LZWDecodeCompat(TIFF*, tidata_t, tsize_t, tsample_t);
#endif
static	int LZWDecodePredRow(TIFF*, tidata_t, tsize_t, tsample_t);
static	int LZWDecodePredTile(TIFF*, tidata_t, tsize_t, tsample_t);
static	void cl_hash(LZWEncodeState*);

static int
LZWCheckPredictor(TIFF* tif,
    LZWState* sp, predictorFunc pred8bit, predictorFunc pred16bit)
{
	TIFFDirectory *td = &tif->tif_dir;

	sp->hordiff = NULL;
	switch (td->td_predictor) {
	case 1:
		break;
	case 2:
		sp->stride = (td->td_planarconfig == PLANARCONFIG_CONTIG ?
		    td->td_samplesperpixel : 1);
		switch (td->td_bitspersample) {
		case 8:
			sp->hordiff = pred8bit;
			break;
		case 16:
			sp->hordiff = pred16bit;
			break;
		default:
			TIFFError(tif->tif_name,
    "Horizontal differencing \"Predictor\" not supported with %d-bit samples",
			    td->td_bitspersample);
			return (0);
		}
		break;
	default:
		TIFFError(tif->tif_name, "\"Predictor\" value %d not supported",
		    td->td_predictor);
		return (0);
	}
	if (sp->hordiff != NULL) {
		/*
		 * Calculate the scanline/tile-width size in bytes.
		 */
		if (isTiled(tif))
			sp->rowsize = TIFFTileRowSize(tif);
		else
			sp->rowsize = TIFFScanlineSize(tif);
	} else
		sp->rowsize = 0;
	return (1);
}

/*
 * LZW Decoder.
 */

#ifdef LZW_CHECKEOS
/*
 * This check shouldn't be necessary because each
 * strip is suppose to be terminated with CODE_EOI.
 */
#define	NextCode(tif, sp, bp, code, get) {				\
	if ((sp)->dec_bitsleft < nbits) {				\
		TIFFWarning(tif->tif_name,				\
		    "LZWDecode: Strip %d not terminated with EOI code", \
		    tif->tif_curstrip);					\
		code = CODE_EOI;					\
	} else {							\
		get(sp, bp, code);					\
		(sp)->dec_bitsleft -= nbits;				\
	}								\
}
#else
#define	NextCode(tif, sp, bp, code, get) get(sp, bp, code)
#endif

#define REPEAT4(n, op)		\
    switch (n) {		\
    default: { int i; for (i = n-4; i > 0; i--) { op; } } \
    case 4:  op;		\
    case 3:  op;		\
    case 2:  op;		\
    case 1:  op;		\
    case 0:  ;			\
    }
#define XREPEAT4(n, op)		\
    switch (n) {		\
    default: { int i; for (i = n-4; i > 0; i--) { op; } } \
    case 2:  op;		\
    case 1:  op;		\
    case 0:  ;			\
    }

static void
horAcc8(register char* cp, register u_long cc, register int stride)
{
	if (cc > stride) {
		cc -= stride;
		/*
		 * Pipeline the most common cases.
		 */
		if (stride == 3)  {
			u_int cr = cp[0];
			u_int cg = cp[1];
			u_int cb = cp[2];
			do {
				cc -= 3, cp += 3;
				cp[0] = (cr += cp[0]);
				cp[1] = (cg += cp[1]);
				cp[2] = (cb += cp[2]);
			} while ((long)cc > 0);
		} else if (stride == 4)  {
			u_int cr = cp[0];
			u_int cg = cp[1];
			u_int cb = cp[2];
			u_int ca = cp[3];
			do {
				cc -= 4, cp += 4;
				cp[0] = (cr += cp[0]);
				cp[1] = (cg += cp[1]);
				cp[2] = (cb += cp[2]);
				cp[3] = (ca += cp[3]);
			} while ((long)cc > 0);
		} else  {
			do {
				XREPEAT4(stride, cp[stride] += *cp; cp++)
				cc -= stride;
			} while ((long)cc > 0);
		}
	}
}

static void
swabHorAcc16(char* cp, u_long cc, register int stride)
{
	register uint16* wp = (uint16 *)cp;
	register u_long wc = cc / 2;

	if (wc > stride) {
		TIFFSwabArrayOfShort(wp, wc);
		wc -= stride;
		do {
			REPEAT4(stride, wp[stride] += wp[0]; wp++)
			wc -= stride;
		} while (wc > 0);
	}
}

static void
horAcc16(char* cp, u_long cc, register int stride)
{
	register uint16* wp = (uint16 *)cp;
	register u_long wc = cc / 2;

	if (wc > stride) {
		wc -= stride;
		do {
			REPEAT4(stride, wp[stride] += wp[0]; wp++)
			wc -= stride;
		} while (wc > 0);
	}
}

/*
 * Setup state for decoding a strip.
 */
static int
LZWPreDecode(TIFF* tif)
{
	register LZWDecodeState *sp = (LZWDecodeState *)tif->tif_data;

	if (sp == NULL) {
		tif->tif_data = _TIFFmalloc(sizeof (LZWDecodeState));
		if (tif->tif_data == NULL) {
			TIFFError("LZWPreDecode",
			    "No space for LZW state block");
			return (0);
		}
		sp = (LZWDecodeState *)tif->tif_data;
		sp->dec_decode = NULL;
		if (!LZWCheckPredictor(tif, &sp->base, horAcc8, horAcc16))
			return (0);
		if (sp->lzw_hordiff) {
			/*
			 * Override default decoding method with
			 * one that does the predictor stuff.
			 */
			tif->tif_decoderow = LZWDecodePredRow;
			tif->tif_decodestrip = LZWDecodePredTile;
			tif->tif_decodetile = LZWDecodePredTile;
			/*
			 * If the data is horizontally differenced
			 * 16-bit data that requires byte-swapping,
			 * then it must be byte swapped before the
			 * accumulation step.  We do this with a
			 * special-purpose routine and override the
			 * normal post decoding logic that the library
			 * setup when the directory was read.
			 */
			if (tif->tif_flags&TIFF_SWAB) {
				if (sp->lzw_hordiff == horAcc16) {
					sp->lzw_hordiff = swabHorAcc16;
					tif->tif_postdecode = TIFFNoPostDecode;
				} /* else handle 32-bit case... */
			}
		}
		/*
		 * Pre-load the table.
		 */
		{ int code;
		  for (code = 255; code >= 0; code--) {
			sp->dec_codetab[code].value = code;
			sp->dec_codetab[code].firstchar = code;
			sp->dec_codetab[code].length = 1;
			sp->dec_codetab[code].next = NULL;
		  }
		}
	}
	/*
	 * Check for old bit-reversed codes.
	 */
	if (tif->tif_rawdata[0] == 0 && (tif->tif_rawdata[1] & 0x1)) {
#ifdef LZW_COMPAT
		if (!sp->dec_decode) {
			if (sp->lzw_hordiff == NULL) {
				/*
				 * Override default decoding methods with
				 * ones that deal with the old coding.
				 * Otherwise the predictor versions set
				 * above will call the compatibility routines
				 * through the dec_decode method.
				 */
				tif->tif_decoderow = LZWDecodeCompat;
				tif->tif_decodestrip = LZWDecodeCompat;
				tif->tif_decodetile = LZWDecodeCompat;
			}
			TIFFWarning(tif->tif_name,
			    "Old-style LZW codes, convert file");
		}
		sp->lzw_maxcode = MAXCODE(BITS_MIN);
		sp->dec_decode = LZWDecodeCompat;
#else /* !LZW_COMPAT */
		if (!sp->dec_decode) {
			TIFFError(tif->tif_name,
			    "Old-style LZW codes not supported");
			sp->dec_decode = LZWDecode;
		}
		return (0);
#endif/* !LZW_COMPAT */
	} else {
		sp->lzw_maxcode = MAXCODE(BITS_MIN)-1;
		sp->dec_decode = LZWDecode;
	}
	sp->lzw_nbits = BITS_MIN;
	sp->lzw_nextbits = 0;
	sp->lzw_nextdata = 0;

	sp->dec_restart = 0;
	sp->dec_nbitsmask = MAXCODE(BITS_MIN);
#ifdef LZW_CHECKEOS
	sp->dec_bitsleft = tif->tif_rawdatasize << 3;
#endif
	sp->dec_free_entp = sp->dec_codetab + CODE_FIRST;
	sp->dec_oldcodep = &sp->dec_codetab[-1];
	sp->dec_maxcodep = &sp->dec_codetab[sp->dec_nbitsmask-1];
	return (1);
}

/*
 * Decode a "hunk of data".
 */
#define	GetNextCode(sp, bp, code) {				\
	nextdata = (nextdata<<8) | *(bp)++;			\
	nextbits += 8;						\
	if (nextbits < nbits) {					\
		nextdata = (nextdata<<8) | *(bp)++;		\
		nextbits += 8;					\
	}							\
	code = (nextdata >> (nextbits-nbits)) & nbitsmask;	\
	nextbits -= nbits;					\
}

static void
codeLoop(TIFF* tif)
{
	TIFFError(tif->tif_name,
	    "LZWDecode: Bogus encoding, loop in the code table; scanline %d",
	    tif->tif_row);
}

static int
LZWDecode(TIFF* tif, tidata_t op0, tsize_t occ0, tsample_t s)
{
	LZWDecodeState *sp = (LZWDecodeState *)tif->tif_data;
	char *op = (char*) op0;
	long occ = (long) occ0;
	char *tp;
	u_char *bp;
	int code, nbits, nextbits, len;
	long nextdata, nbitsmask;
	code_t *codep, *free_entp, *maxcodep, *oldcodep;

	assert(sp != NULL);
	/*
	 * Restart interrupted output operation.
	 */
	if (sp->dec_restart) {
		int residue;

		codep = sp->dec_codep;
		residue = codep->length - sp->dec_restart;
		if (residue > occ) {
			/*
			 * Residue from previous decode is sufficient
			 * to satisfy decode request.  Skip to the
			 * start of the decoded string, place decoded
			 * values in the output buffer, and return.
			 */
			sp->dec_restart += occ;
			do {
				codep = codep->next;
			} while (--residue > occ && codep);
			if (codep) {
				tp = op + occ;
				do {
					*--tp = codep->value;
					codep = codep->next;
				} while (--occ && codep);
			}
			return (1);
		}
		/*
		 * Residue satisfies only part of the decode request.
		 */
		op += residue, occ -= residue;
		tp = op;
		do {
			int t;
			--tp;
			t = codep->value;
			codep = codep->next;
			*tp = t;
		} while (--residue && codep);
		sp->dec_restart = 0;
	}

	bp = (u_char *)tif->tif_rawcp;
	nbits = sp->lzw_nbits;
	nextdata = sp->lzw_nextdata;
	nextbits = sp->lzw_nextbits;
	nbitsmask = sp->dec_nbitsmask;
	oldcodep = sp->dec_oldcodep;
	free_entp = sp->dec_free_entp;
	maxcodep = sp->dec_maxcodep;

	while (occ > 0) {
		NextCode(tif, sp, bp, code, GetNextCode);
		if (code == CODE_EOI)
			break;
		if (code == CODE_CLEAR) {
			free_entp = sp->dec_codetab + CODE_FIRST;
			nbits = BITS_MIN;
			nbitsmask = MAXCODE(BITS_MIN);
			maxcodep = sp->dec_codetab + nbitsmask-1;
			NextCode(tif, sp, bp, code, GetNextCode);
			if (code == CODE_EOI)
				break;
			*op++ = code, occ--;
			oldcodep = sp->dec_codetab + code;
			continue;
		}
		codep = sp->dec_codetab + code;

		/*
	 	 * Add the new entry to the code table.
	 	 */
		assert(&sp->dec_codetab[0] <= free_entp && free_entp < &sp->dec_codetab[CSIZE]);
		free_entp->next = oldcodep;
		free_entp->firstchar = free_entp->next->firstchar;
		free_entp->length = free_entp->next->length+1;
		free_entp->value = (codep < free_entp) ?
		    codep->firstchar : free_entp->firstchar;
		if (++free_entp > maxcodep) {
			if (++nbits > BITS_MAX)		/* should not happen */
				nbits = BITS_MAX;
			nbitsmask = MAXCODE(nbits);
			maxcodep = sp->dec_codetab + nbitsmask-1;
		}
		oldcodep = codep;
		if (code >= 256) {
			/*
		 	 * Code maps to a string, copy string
			 * value to output (written in reverse).
		 	 */
			if (codep->length > occ) {
				/*
				 * String is too long for decode buffer,
				 * locate portion that will fit, copy to
				 * the decode buffer, and setup restart
				 * logic for the next decoding call.
				 */
				sp->dec_codep = codep;
				do {
					codep = codep->next;
				} while (codep && codep->length > occ);
				if (codep) {
					sp->dec_restart = occ;
					tp = op + occ;
					do  {
						*--tp = codep->value;
						codep = codep->next;
					}  while (--occ && codep);
					if (codep)
						codeLoop(tif);
				}
				break;
			}
			len = codep->length;
			tp = op + len;
			do {
				int t;
				--tp;
				t = codep->value;
				codep = codep->next;
				*tp = t;
			} while (codep && tp > op);
			if (codep) {
			    codeLoop(tif);
			    break;
			}
			op += len, occ -= len;
		} else
			*op++ = code, occ--;
	}

	tif->tif_rawcp = (tidata_t) bp;
	sp->lzw_nbits = nbits;
	sp->lzw_nextdata = nextdata;
	sp->lzw_nextbits = nextbits;
	sp->dec_nbitsmask = nbitsmask;
	sp->dec_oldcodep = oldcodep;
	sp->dec_free_entp = free_entp;
	sp->dec_maxcodep = maxcodep;

	if (occ > 0) {
		TIFFError(tif->tif_name,
		"LZWDecode: Not enough data at scanline %d (short %d bytes)",
		    tif->tif_row, occ);
		return (0);
	}
	return (1);
}

#ifdef LZW_COMPAT
/*
 * Decode a "hunk of data" for old images.
 */
#define	GetNextCodeCompat(sp, bp, code) {			\
	nextdata |= *(bp)++ << nextbits;			\
	nextbits += 8;						\
	if (nextbits < nbits) {					\
		nextdata |= *(bp)++ << nextbits;		\
		nextbits += 8;					\
	}							\
	code = nextdata & nbitsmask;				\
	nextdata >>= nbits;					\
	nextbits -= nbits;					\
}

static int
LZWDecodeCompat(TIFF* tif, tidata_t op0, tsize_t occ0, tsample_t s)
{
	LZWDecodeState *sp = (LZWDecodeState *)tif->tif_data;
	char *op = (char*) op0;
	long occ = (long) occ0;
	char *tp;
	u_char *bp;
	int code, nbits, nextbits;
	long nextdata, nbitsmask;
	code_t *codep, *free_entp, *maxcodep, *oldcodep;

	assert(sp != NULL);
	/*
	 * Restart interrupted output operation.
	 */
	if (sp->dec_restart) {
		int residue;

		codep = sp->dec_codep;
		residue = codep->length - sp->dec_restart;
		if (residue > occ) {
			/*
			 * Residue from previous decode is sufficient
			 * to satisfy decode request.  Skip to the
			 * start of the decoded string, place decoded
			 * values in the output buffer, and return.
			 */
			sp->dec_restart += occ;
			do {
				codep = codep->next;
			} while (--residue > occ);
			tp = op + occ;
			do {
				*--tp = codep->value;
				codep = codep->next;
			} while (--occ);
			return (1);
		}
		/*
		 * Residue satisfies only part of the decode request.
		 */
		op += residue, occ -= residue;
		tp = op;
		do {
			*--tp = codep->value;
			codep = codep->next;
		} while (--residue);
		sp->dec_restart = 0;
	}

	bp = (u_char *)tif->tif_rawcp;
	nbits = sp->lzw_nbits;
	nextdata = sp->lzw_nextdata;
	nextbits = sp->lzw_nextbits;
	nbitsmask = sp->dec_nbitsmask;
	oldcodep = sp->dec_oldcodep;
	free_entp = sp->dec_free_entp;
	maxcodep = sp->dec_maxcodep;

	while (occ > 0) {
		NextCode(tif, sp, bp, code, GetNextCodeCompat);
		if (code == CODE_EOI)
			break;
		if (code == CODE_CLEAR) {
			free_entp = sp->dec_codetab + CODE_FIRST;
			nbits = BITS_MIN;
			nbitsmask = MAXCODE(BITS_MIN);
			maxcodep = sp->dec_codetab + nbitsmask;
			NextCode(tif, sp, bp, code, GetNextCodeCompat);
			if (code == CODE_EOI)
				break;
			*op++ = code, occ--;
			oldcodep = sp->dec_codetab + code;
			continue;
		}
		codep = sp->dec_codetab + code;

		/*
	 	 * Add the new entry to the code table.
	 	 */
		assert(&sp->dec_codetab[0] <= free_entp && free_entp < &sp->dec_codetab[CSIZE]);
		free_entp->next = oldcodep;
		free_entp->firstchar = free_entp->next->firstchar;
		free_entp->length = free_entp->next->length+1;
		free_entp->value = (codep < free_entp) ?
		    codep->firstchar : free_entp->firstchar;
		if (++free_entp > maxcodep) {
			if (++nbits > BITS_MAX)		/* should not happen */
				nbits = BITS_MAX;
			nbitsmask = MAXCODE(nbits);
			maxcodep = sp->dec_codetab + nbitsmask;
		}
		oldcodep = codep;
		if (code >= 256) {
			/*
		 	 * Code maps to a string, copy string
			 * value to output (written in reverse).
		 	 */
			if (codep->length > occ) {
				/*
				 * String is too long for decode buffer,
				 * locate portion that will fit, copy to
				 * the decode buffer, and setup restart
				 * logic for the next decoding call.
				 */
				sp->dec_codep = codep;
				do {
					codep = codep->next;
				} while (codep->length > occ);
				sp->dec_restart = occ;
				tp = op + occ;
				do  {
					*--tp = codep->value;
					codep = codep->next;
				}  while (--occ);
				break;
			}
			op += codep->length, occ -= codep->length;
			tp = op;
			do {
				*--tp = codep->value;
			} while (codep = codep->next);
		} else
			*op++ = code, occ--;
	}

	tif->tif_rawcp = (tidata_t) bp;
	sp->lzw_nbits = nbits;
	sp->lzw_nextdata = nextdata;
	sp->lzw_nextbits = nextbits;
	sp->dec_nbitsmask = nbitsmask;
	sp->dec_oldcodep = oldcodep;
	sp->dec_free_entp = free_entp;
	sp->dec_maxcodep = maxcodep;

	if (occ > 0) {
		TIFFError(tif->tif_name,
	    "LZWDecodeCompat: Not enough data at scanline %d (short %d bytes)",
		    tif->tif_row, occ);
		return (0);
	}
	return (1);
}
#endif /* LZW_COMPAT */

/*
 * Decode a scanline and apply the predictor routine.
 */
static int
LZWDecodePredRow(TIFF* tif, tidata_t op0, tsize_t occ0, tsample_t s)
{
	LZWDecodeState *sp = (LZWDecodeState *)tif->tif_data;

	assert(sp != NULL);
	assert(sp->dec_decode != NULL);
	if ((*sp->dec_decode)(tif, op0, occ0, s)) {
		(*sp->lzw_hordiff)((char *)op0, occ0, sp->lzw_stride);
		return (1);
	} else
		return (0);
}

/*
 * Decode a tile/strip and apply the predictor routine.
 * Note that horizontal differencing must be done on a
 * row-by-row basis.  The width of a "row" has already
 * been calculated at pre-decode time according to the
 * strip/tile dimensions.
 */
static int
LZWDecodePredTile(TIFF* tif, tidata_t op0, tsize_t occ0, tsample_t s)
{
	LZWDecodeState *sp = (LZWDecodeState *)tif->tif_data;
	u_long rowsize;

	assert(sp != NULL);
	assert(sp->dec_decode != NULL);
	if (!(*sp->dec_decode)(tif, op0, occ0, s))
		return (0);
	rowsize = sp->lzw_rowsize;
	assert(rowsize > 0);
	while ((long)occ0 > 0) {
		(*sp->lzw_hordiff)((char*)op0, rowsize, sp->lzw_stride);
		occ0 -= rowsize;
		op0 += rowsize;
	}
	return (1);
}

/*
 * LZW Encoding.
 */

static void
horDiff8(register char* cp, register u_long cc, register int stride)
{
	if (cc > stride) {
		cc -= stride;
		/*
		 * Pipeline the most common cases.
		 */
		if (stride == 3) {
			int r1, g1, b1;
			int r2 = cp[0];
			int g2 = cp[1];
			int b2 = cp[2];
			do {
				r1 = cp[3]; cp[3] = r1-r2; r2 = r1;
				g1 = cp[4]; cp[4] = g1-g2; g2 = g1;
				b1 = cp[5]; cp[5] = b1-b2; b2 = b1;
				cp += 3;
			} while ((long)(cc -= 3) > 0);
		} else if (stride == 4) {
			int r1, g1, b1, a1;
			int r2 = cp[0];
			int g2 = cp[1];
			int b2 = cp[2];
			int a2 = cp[3];
			do {
				r1 = cp[4]; cp[4] = r1-r2; r2 = r1;
				g1 = cp[5]; cp[5] = g1-g2; g2 = g1;
				b1 = cp[6]; cp[6] = b1-b2; b2 = b1;
				a1 = cp[7]; cp[7] = a1-a2; a2 = a1;
				cp += 4;
			} while ((long)(cc -= 4) > 0);
		} else {
			cp += cc - 1;
			do {
				REPEAT4(stride, cp[stride] -= cp[0]; cp--)
			} while ((long)(cc -= stride) > 0);
		}
	}
}

static void
horDiff16(char* cp, u_long cc, register int stride)
{
	register int16 *wp = (int16*)cp;
	register u_long wc = cc/2;

	if (wc > stride) {
		wc -= stride;
		wp += wc - 1;
		do {
			REPEAT4(stride, wp[stride] -= wp[0]; wp--)
			wc -= stride;
		} while (wc > 0);
	}
}

/*
 * Reset encoding state at the start of a strip.
 */
static int
LZWPreEncode(TIFF* tif)
{
	register LZWEncodeState *sp = (LZWEncodeState *)tif->tif_data;

	if (sp == NULL) {
		tif->tif_data = _TIFFmalloc(sizeof (LZWEncodeState));
		if (tif->tif_data == NULL) {
			TIFFError("LZWPreEncode",
			    "No space for LZW state block");
			return (0);
		}
		sp = (LZWEncodeState *)tif->tif_data;
		if (!LZWCheckPredictor(tif, &sp->base, horDiff8, horDiff16))
			return (0);
		if (sp->lzw_hordiff != NULL) {
			tif->tif_encoderow = LZWEncodePredRow;
			tif->tif_encodestrip = LZWEncodePredTile;
			tif->tif_encodetile = LZWEncodePredTile;
		}
	}
	sp->lzw_nbits = BITS_MIN;
	sp->lzw_maxcode = MAXCODE(BITS_MIN);
	sp->lzw_free_ent = CODE_FIRST;
	sp->lzw_nextbits = 0;
	sp->lzw_nextdata = 0;
	sp->enc_checkpoint = CHECK_GAP;
	sp->enc_ratio = 0;
	sp->enc_incount = 0;
	sp->enc_outcount = 0;
	/*
	 * The 4 here insures there is space for 2 max-sized
	 * codes in LZWEncode and LZWPostDecode.
	 */
	sp->enc_rawlimit = tif->tif_rawdata + tif->tif_rawdatasize-1 - 4;
	cl_hash(sp);		/* clear hash table */
	sp->enc_oldcode = -1;	/* generates CODE_CLEAR in LZWEncode */
	return (1);
}

#define	CALCRATIO(sp, rat) {					\
	if (incount > 0x007fffff) { /* NB: shift will overflow */\
		rat = outcount >> 8;				\
		rat = (rat == 0 ? 0x7fffffff : incount/rat);	\
	} else							\
		rat = (incount<<8) / outcount;			\
}
#define	PutNextCode(op, c) {					\
	nextdata = (nextdata << nbits) | c;			\
	nextbits += nbits;					\
	*op++ = nextdata >> (nextbits-8);			\
	nextbits -= 8;						\
	if (nextbits >= 8) {					\
		*op++ = nextdata >> (nextbits-8);		\
		nextbits -= 8;					\
	}							\
	outcount += nbits;					\
}

/*
 * Encode a chunk of pixels.
 *
 * Uses an open addressing double hashing (no chaining) on the 
 * prefix code/next character combination.  We do a variant of
 * Knuth's algorithm D (vol. 3, sec. 6.4) along with G. Knott's
 * relatively-prime secondary probe.  Here, the modular division
 * first probe is gives way to a faster exclusive-or manipulation. 
 * Also do block compression with an adaptive reset, whereby the
 * code table is cleared when the compression ratio decreases,
 * but after the table fills.  The variable-length output codes
 * are re-sized at this point, and a CODE_CLEAR is generated
 * for the decoder. 
 */
static int
LZWEncode(TIFF* tif, tidata_t bp, tsize_t cc, tsample_t s)
{
	register LZWEncodeState *sp = (LZWEncodeState *)tif->tif_data;
	register long fcode;
	register hash_t *hp;
	register int h, c, ent, disp;
	long incount, outcount, checkpoint;
	long nextdata, nextbits;
	int free_ent, maxcode, nbits;
	tidata_t op, limit;

	if (sp == NULL)
		return (0);
	/*
	 * Load local state.
	 */
	incount = sp->enc_incount;
	outcount = sp->enc_outcount;
	checkpoint = sp->enc_checkpoint;
	nextdata = sp->lzw_nextdata;
	nextbits = sp->lzw_nextbits;
	free_ent = sp->lzw_free_ent;
	maxcode = sp->lzw_maxcode;
	nbits = sp->lzw_nbits;
	op = tif->tif_rawcp;
	limit = sp->enc_rawlimit;
	ent = sp->enc_oldcode;

	if (ent == -1 && cc > 0) {
		/*
		 * NB: This is safe because it can only happen
		 *     at the start of a strip where we know there
		 *     is space in the data buffer.
		 */
		PutNextCode(op, CODE_CLEAR);
		ent = *bp++; cc--; incount++;
	}
	while (cc > 0) {
		c = *bp++; cc--; incount++;
		fcode = ((long)c << BITS_MAX) + ent;
		h = (c << HSHIFT) ^ ent;	/* xor hashing */
		hp = &sp->enc_hashtab[h];
		if (hp->hash == fcode) {
			ent = hp->code;
			continue;
		}
		if (hp->hash >= 0) {
			/*
			 * Primary hash failed, check secondary hash.
			 */
			disp = HSIZE - h;
			if (h == 0)
				disp = 1;
			do {
				if ((hp -= disp) < sp->enc_hashtab)
					hp += HSIZE;
				if (hp->hash == fcode) {
					ent = hp->code;
					goto hit;
				}
			} while (hp->hash >= 0);
		}
		/*
		 * New entry, emit code and add to table.
		 */
		/*
		 * Verify there is space in the buffer for the code
		 * and any potential Clear code that might be emitted
		 * below.  The value of limit is setup so that there
		 * are at least 4 bytes free--room for 2 codes.
		 */
		if (op > limit) {
			tif->tif_rawcc = op - tif->tif_rawdata;
			TIFFFlushData1(tif);
			op = tif->tif_rawdata;
		}
		PutNextCode(op, ent);
		ent = c;
		hp->code = free_ent++;
		hp->hash = fcode;
		if (free_ent == CODE_MAX-1) {
			/* table is full, emit clear code and reset */
			cl_hash(sp);
			sp->enc_ratio = 0;
			incount = 0;
			outcount = 0;
			free_ent = CODE_FIRST;
			PutNextCode(op, CODE_CLEAR);
			nbits = BITS_MIN;
			maxcode = MAXCODE(BITS_MIN);
		} else {
			/*
			 * If the next entry is going to be too big for
			 * the code size, then increase it, if possible.
			 */
			if (free_ent > maxcode) {
				nbits++;
				assert(nbits <= BITS_MAX);
				maxcode = MAXCODE(nbits);
			} else if (incount >= checkpoint) {
				long rat;
				/*
				 * Check compression ratio and, if things seem
				 * to be slipping, clear the hash table and
				 * reset state.  The compression ratio is a
				 * 24+8-bit fractional number.
				 */
				checkpoint = incount+CHECK_GAP;
				CALCRATIO(sp, rat);
				if (rat <= sp->enc_ratio) {
					cl_hash(sp);
					sp->enc_ratio = 0;
					incount = 0;
					outcount = 0;
					free_ent = CODE_FIRST;
					PutNextCode(op, CODE_CLEAR);
					nbits = BITS_MIN;
					maxcode = MAXCODE(BITS_MIN);
				} else
					sp->enc_ratio = rat;
			}
		}
	hit:
		;
	}

	/*
	 * Restore global state.
	 */
	sp->enc_incount = incount;
	sp->enc_outcount = outcount;
	sp->enc_checkpoint = checkpoint;
	sp->enc_oldcode = ent;
	sp->lzw_nextdata = nextdata;
	sp->lzw_nextbits = nextbits;
	sp->lzw_free_ent = free_ent;
	sp->lzw_maxcode = maxcode;
	sp->lzw_nbits = nbits;
	tif->tif_rawcp = op;
	return (1);
}

static int
LZWEncodePredRow(TIFF* tif, tidata_t bp, tsize_t cc, tsample_t s)
{
	LZWEncodeState *sp = (LZWEncodeState *)tif->tif_data;

	assert(sp != NULL);
	assert(sp->lzw_hordiff != NULL);
/* XXX horizontal differencing alters user's data XXX */
	(*sp->lzw_hordiff)((char *)bp, cc, sp->lzw_stride);
	return (LZWEncode(tif, bp, cc, s));
}

static int
LZWEncodePredTile(TIFF* tif, tidata_t bp0, tsize_t cc0, tsample_t s)
{
	LZWEncodeState *sp = (LZWEncodeState *)tif->tif_data;
	u_long cc = cc0, rowsize;
	u_char *bp = bp0;

	assert(sp != NULL);
	assert(sp->lzw_hordiff != NULL);
	rowsize = sp->lzw_rowsize;
	assert(rowsize > 0);
	while ((long)cc > 0) {
		(*sp->lzw_hordiff)((char *)bp, rowsize, sp->lzw_stride);
		cc -= rowsize;
		bp += rowsize;
	}
	return (LZWEncode(tif, bp0, cc0, s));
}

/*
 * Finish off an encoded strip by flushing the last
 * string and tacking on an End Of Information code.
 */
static int
LZWPostEncode(TIFF* tif)
{
	register LZWEncodeState *sp = (LZWEncodeState *)tif->tif_data;
	tidata_t op = tif->tif_rawcp;
	long nextbits = sp->lzw_nextbits;
	long nextdata = sp->lzw_nextdata;
	long outcount = sp->enc_outcount;
	int nbits = sp->lzw_nbits;

	if (op > sp->enc_rawlimit) {
		tif->tif_rawcc = op - tif->tif_rawdata;
		TIFFFlushData1(tif);
		op = tif->tif_rawdata;
	}
	if (sp->enc_oldcode != -1) {
		PutNextCode(op, sp->enc_oldcode);
		sp->enc_oldcode = -1;
	}
	PutNextCode(op, CODE_EOI);
	if (nextbits > 0) 
		*op++ = nextdata << (8-nextbits);
	tif->tif_rawcc = op - tif->tif_rawdata;
	return (1);
}

/*
 * Reset encoding hash table.
 */
static void
cl_hash(LZWEncodeState* sp)
{
	register hash_t *hp = &sp->enc_hashtab[HSIZE-1];
	register long i = HSIZE-8;

 	do {
		i -= 8;
		hp[-7].hash = -1;
		hp[-6].hash = -1;
		hp[-5].hash = -1;
		hp[-4].hash = -1;
		hp[-3].hash = -1;
		hp[-2].hash = -1;
		hp[-1].hash = -1;
		hp[ 0].hash = -1;
		hp -= 8;
	} while (i >= 0);
    	for (i += 8; i > 0; i--, hp--)
		hp->hash = -1;
}

static void
LZWCleanup(TIFF* tif)
{
	if (tif->tif_data) {
		_TIFFfree(tif->tif_data);
		tif->tif_data = NULL;
	}
}

int
TIFFInitLZW(TIFF* tif)
{
	tif->tif_predecode = LZWPreDecode;
	tif->tif_decoderow = LZWDecode;
	tif->tif_decodestrip = LZWDecode;
	tif->tif_decodetile = LZWDecode;
	tif->tif_preencode = LZWPreEncode;
	tif->tif_postencode = LZWPostEncode;
	tif->tif_encoderow = LZWEncode;
	tif->tif_encodestrip = LZWEncode;
	tif->tif_encodetile = LZWEncode;
	tif->tif_cleanup = LZWCleanup;
	return (1);
}

/*
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
