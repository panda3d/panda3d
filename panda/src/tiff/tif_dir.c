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
 * Directory Tag Get & Set Routines.
 * (and also some miscellaneous stuff)
 */
#include "tiffiop.h"

static void
setString(char** cpp, char* cp)
{
        if (*cpp)
                _TIFFfree(*cpp), *cpp = 0;
        if (cp) {
                size_t len = strlen(cp)+1;
                if (*cpp = _TIFFmalloc(len))
                        memcpy(*cpp, cp, len);
        }
}

static void
setShortArray(uint16** wpp, uint16* wp, long n)
{
        if (*wpp)
                _TIFFfree((char *)*wpp), *wpp = 0;
        n *= sizeof (uint16);
        if (wp && (*wpp = (uint16 *)_TIFFmalloc(n)))
                memcpy(*wpp, wp, n);
}

static void
setLongArray(uint32** wpp, uint32* wp, long n)
{
        if (*wpp)
                _TIFFfree((char *)*wpp), *wpp = 0;
        n *= sizeof (uint32);
        if (wp && (*wpp = (uint32 *)_TIFFmalloc(n)))
                memcpy(*wpp, wp, n);
}

static void
setFloatArray(float** wpp, float* wp, long n)
{
        if (*wpp)
                _TIFFfree((char *)*wpp), *wpp = 0;
        n *= sizeof (float);
        if (wp && (*wpp = (float *)_TIFFmalloc(n)))
                memcpy(*wpp, wp, n);
}

#ifdef JPEG_SUPPORT
/*
 * Install a JPEG Quantization table.
 * Note that we reorder the elements
 * of the array in the zig-zag order
 * that is expected by the compression code
 * and that is to be stored in the file.
 */
static void
setJPEGQTable(u_char*** wpp, u_char** wp, int nc)
{
        static u_char zigzag[64] = {
            0,  1,  5,  6, 14, 15, 27, 28,
            2,  4,  7, 13, 16, 26, 29, 42,
            3,  8, 12, 17, 25, 30, 41, 43,
            9, 11, 18, 24, 31, 40, 44, 53,
           10, 19, 23, 32, 39, 45, 52, 54,
           20, 22, 33, 38, 46, 51, 55, 60,
           21, 34, 37, 47, 50, 56, 59, 61,
           35, 36, 48, 49, 57, 58, 62, 63
        };
        char *tab;
        int i, j;

        if (*wpp)
                _TIFFfree((char *)*wpp), *wpp = 0;
        *wpp = (u_char **)
            _TIFFmalloc(nc * (sizeof (u_char *) + 64*sizeof (u_char)));
        tab = (((char *)*wpp) + nc*sizeof (u_char *));
        for (i = 0; i < nc; i++) {
                (*wpp)[i] = (u_char *)tab;
                for (j = 0; j < 64; j++)
                        tab[zigzag[j]] = wp[i][j];
                tab += 64*sizeof (u_char);
        }
}

/*
 * Install a JPEG Coefficient table.
 */
static void
setJPEGCTable(u_char*** cpp, u_char** cp, int nc)
{
        u_char *tab;
        int i, j, nw;

        if (*cpp)
                _TIFFfree(*cpp), *cpp = 0;
        /*
         * Calculate the size of the table by counting
         * the number of codes specified in the bits array.
         */
        nw = 0;
        for (i = 0; i < nc; i++) {
                nw += 16;               /* 16 bytes for bits array */
                for (j = 0; j < 16; j++)/* sum up count of codes */
                        nw += cp[i][j];
        }
        *cpp = (u_char **)_TIFFmalloc(nc*sizeof (u_char *) + nw);
        tab = ((u_char *)*cpp) + nc*sizeof (u_char *);
        /*
         * Setup internal array and copy user data.
         */
        for (i = 0; i < nc; i++) {
                (*cpp)[i] = tab;
                for (nw = 16, j = 0; j < 16; j++)
                        nw += cp[i][j];
                memcpy(tab, cp[i], nw);
                tab += nw;
        }
}
#endif

/*
 * Install extra samples information.
 */
static int
setExtraSamples(TIFFDirectory* td, va_list ap, int* v)
{
        uint16* va;
        int i;

        *v = va_arg(ap, int);
        if (*v > td->td_samplesperpixel)
                return (0);
        va = va_arg(ap, uint16*);
        if (va == NULL)                 /* typically missing param */
                return (0);
        for (i = 0; i < *v; i++)
                if (va[i] > EXTRASAMPLE_UNASSALPHA)
                        return (0);
        td->td_extrasamples = *v;
        setShortArray(&td->td_sampleinfo, va, td->td_extrasamples);
        return (1);
}

static int
TIFFSetField1(TIFF* tif, ttag_t tag, va_list ap)
{
        TIFFDirectory *td = &tif->tif_dir;
        int status = 1;
        uint32 v32;
        int i, v;

        switch (tag) {
        case TIFFTAG_SUBFILETYPE:
                td->td_subfiletype = va_arg(ap, uint32);
                break;
        case TIFFTAG_IMAGEWIDTH:
                td->td_imagewidth = va_arg(ap, uint32);
                break;
        case TIFFTAG_IMAGELENGTH:
                td->td_imagelength = va_arg(ap, uint32);
                break;
        case TIFFTAG_BITSPERSAMPLE:
                td->td_bitspersample = va_arg(ap, int);
                /*
                 * If the data require post-decoding processing
                 * to byte-swap samples, set it up here.  Note
                 * that since tags are required to be ordered,
                 * compression code can override this behaviour
                 * in the setup method if it wants to roll the
                 * post decoding work in with its normal work.
                 */
                if (tif->tif_flags & TIFF_SWAB) {
                        if (td->td_bitspersample == 16)
                                tif->tif_postdecode = TIFFSwab16BitData;
                        else if (td->td_bitspersample == 32)
                                tif->tif_postdecode = TIFFSwab32BitData;
                }
                break;
        case TIFFTAG_COMPRESSION:
                v = va_arg(ap, int) & 0xffff;
                /*
                 * If we're changing the compression scheme,
                 * the notify the previous module so that it
                 * can cleanup any state it's setup.
                 */
                if (TIFFFieldSet(tif, FIELD_COMPRESSION)) {
                        if (td->td_compression == v)
                                break;
                        if (tif->tif_cleanup)
                                (*tif->tif_cleanup)(tif);
                }
                /*
                 * Setup new compression routine state.
                 */
                if (status = TIFFSetCompressionScheme(tif, v))
                        td->td_compression = v;
                break;
        case TIFFTAG_PHOTOMETRIC:
                td->td_photometric = va_arg(ap, int);
                break;
        case TIFFTAG_THRESHHOLDING:
                td->td_threshholding = va_arg(ap, int);
                break;
        case TIFFTAG_FILLORDER:
                v = va_arg(ap, int);
                if (v != FILLORDER_LSB2MSB && v != FILLORDER_MSB2LSB)
                        goto badvalue;
                td->td_fillorder = v;
                break;
        case TIFFTAG_DOCUMENTNAME:
                setString(&td->td_documentname, va_arg(ap, char *));
                break;
        case TIFFTAG_ARTIST:
                setString(&td->td_artist, va_arg(ap, char *));
                break;
        case TIFFTAG_DATETIME:
                setString(&td->td_datetime, va_arg(ap, char *));
                break;
        case TIFFTAG_HOSTCOMPUTER:
                setString(&td->td_hostcomputer, va_arg(ap, char *));
                break;
        case TIFFTAG_IMAGEDESCRIPTION:
                setString(&td->td_imagedescription, va_arg(ap, char *));
                break;
        case TIFFTAG_MAKE:
                setString(&td->td_make, va_arg(ap, char *));
                break;
        case TIFFTAG_MODEL:
                setString(&td->td_model, va_arg(ap, char *));
                break;
        case TIFFTAG_SOFTWARE:
                setString(&td->td_software, va_arg(ap, char *));
                break;
        case TIFFTAG_ORIENTATION:
                v = va_arg(ap, int);
                if (v < ORIENTATION_TOPLEFT || ORIENTATION_LEFTBOT < v) {
                        TIFFWarning(tif->tif_name,
                            "Bad value %ld for \"%s\" tag ignored",
                            v, TIFFFieldWithTag(tag)->field_name);
                } else
                        td->td_orientation = v;
                break;
        case TIFFTAG_SAMPLESPERPIXEL:
                /* XXX should cross check -- e.g. if pallette, then 1 */
                v = va_arg(ap, int);
                if (v == 0)
                        goto badvalue;
                td->td_samplesperpixel = v;
                break;
        case TIFFTAG_ROWSPERSTRIP:
                v32 = va_arg(ap, uint32);
                if (v32 == 0)
                        goto badvalue32;
                td->td_rowsperstrip = v32;
                if (!TIFFFieldSet(tif, FIELD_TILEDIMENSIONS)) {
                        td->td_tilelength = v32;
                        td->td_tilewidth = td->td_imagewidth;
                }
                break;
        case TIFFTAG_MINSAMPLEVALUE:
                td->td_minsamplevalue = va_arg(ap, int) & 0xffff;
                break;
        case TIFFTAG_MAXSAMPLEVALUE:
                td->td_maxsamplevalue = va_arg(ap, int) & 0xffff;
                break;
        case TIFFTAG_XRESOLUTION:
                td->td_xresolution = va_arg(ap, dblparam_t);
                break;
        case TIFFTAG_YRESOLUTION:
                td->td_yresolution = va_arg(ap, dblparam_t);
                break;
        case TIFFTAG_PLANARCONFIG:
                v = va_arg(ap, int);
                if (v != PLANARCONFIG_CONTIG && v != PLANARCONFIG_SEPARATE)
                        goto badvalue;
                td->td_planarconfig = v;
                break;
        case TIFFTAG_PAGENAME:
                setString(&td->td_pagename, va_arg(ap, char *));
                break;
        case TIFFTAG_XPOSITION:
                td->td_xposition = va_arg(ap, dblparam_t);
                break;
        case TIFFTAG_YPOSITION:
                td->td_yposition = va_arg(ap, dblparam_t);
                break;
        case TIFFTAG_GROUP3OPTIONS:
                td->td_group3options = va_arg(ap, uint32);
                break;
        case TIFFTAG_GROUP4OPTIONS:
                td->td_group4options = va_arg(ap, uint32);
                break;
        case TIFFTAG_RESOLUTIONUNIT:
                v = va_arg(ap, int);
                if (v < RESUNIT_NONE || RESUNIT_CENTIMETER < v)
                        goto badvalue;
                td->td_resolutionunit = v;
                break;
        case TIFFTAG_PAGENUMBER:
                td->td_pagenumber[0] = va_arg(ap, int);
                td->td_pagenumber[1] = va_arg(ap, int);
                break;
        case TIFFTAG_HALFTONEHINTS:
                td->td_halftonehints[0] = va_arg(ap, int);
                td->td_halftonehints[1] = va_arg(ap, int);
                break;
        case TIFFTAG_COLORMAP:
                v = 1L<<td->td_bitspersample;
                setShortArray(&td->td_colormap[0], va_arg(ap, uint16*), v);
                setShortArray(&td->td_colormap[1], va_arg(ap, uint16*), v);
                setShortArray(&td->td_colormap[2], va_arg(ap, uint16*), v);
                break;
        case TIFFTAG_PREDICTOR:
                td->td_predictor = va_arg(ap, int);
                break;
        case TIFFTAG_EXTRASAMPLES:
                if (!setExtraSamples(td, ap, &v))
                        goto badvalue;
                break;
        case TIFFTAG_MATTEING:
                td->td_extrasamples = (va_arg(ap, int) != 0);
                if (td->td_extrasamples) {
                        uint16 sv = EXTRASAMPLE_ASSOCALPHA;
                        setShortArray(&td->td_sampleinfo, &sv, 1);
                }
                break;
        case TIFFTAG_BADFAXLINES:
                td->td_badfaxlines = va_arg(ap, uint32);
                break;
        case TIFFTAG_CLEANFAXDATA:
                td->td_cleanfaxdata = va_arg(ap, int);
                break;
        case TIFFTAG_CONSECUTIVEBADFAXLINES:
                td->td_badfaxrun = va_arg(ap, uint32);
                break;
        case TIFFTAG_TILEWIDTH:
                v32 = va_arg(ap, uint32);
                if (v32 % 16)
                        goto badvalue32;
                td->td_tilewidth = v32;
                tif->tif_flags |= TIFF_ISTILED;
                break;
        case TIFFTAG_TILELENGTH:
                v32 = va_arg(ap, uint32);
                if (v32 % 16)
                        goto badvalue32;
                td->td_tilelength = v32;
                tif->tif_flags |= TIFF_ISTILED;
                break;
        case TIFFTAG_TILEDEPTH:
                v32 = va_arg(ap, uint32);
                if (v == 0)
                        goto badvalue32;
                td->td_tiledepth = v32;
                break;
        case TIFFTAG_DATATYPE:
        case TIFFTAG_SAMPLEFORMAT:
                v = va_arg(ap, int);
                if (tag == TIFFTAG_DATATYPE && v == 0)
                        v = SAMPLEFORMAT_VOID;
                if (v < SAMPLEFORMAT_UINT || SAMPLEFORMAT_VOID < v)
                        goto badvalue;
                td->td_sampleformat = v;
                break;
        case TIFFTAG_IMAGEDEPTH:
                td->td_imagedepth = va_arg(ap, uint32);
                break;
#ifdef YCBCR_SUPPORT
        case TIFFTAG_YCBCRCOEFFICIENTS:
                setFloatArray(&td->td_ycbcrcoeffs, va_arg(ap, float *), 3);
                break;
        case TIFFTAG_YCBCRPOSITIONING:
                td->td_ycbcrpositioning = va_arg(ap, int);
                break;
        case TIFFTAG_YCBCRSUBSAMPLING:
                td->td_ycbcrsubsampling[0] = va_arg(ap, int);
                td->td_ycbcrsubsampling[1] = va_arg(ap, int);
                break;
#endif
#ifdef JPEG_SUPPORT
        case TIFFTAG_JPEGPROC:
                td->td_jpegproc = va_arg(ap, int);
                break;
        case TIFFTAG_JPEGRESTARTINTERVAL:
                td->td_jpegrestartinterval = va_arg(ap, int);
                break;
        case TIFFTAG_JPEGQTABLES:
                setJPEGQTable(&td->td_qtab, va_arg(ap, u_char **),
                    td->td_samplesperpixel);
                break;
        case TIFFTAG_JPEGDCTABLES:
                setJPEGCTable(&td->td_dctab, va_arg(ap, u_char **),
                    td->td_samplesperpixel);
                break;
        case TIFFTAG_JPEGACTABLES:
                setJPEGCTable(&td->td_actab, va_arg(ap, u_char **),
                    td->td_samplesperpixel);
                break;
#endif
#ifdef COLORIMETRY_SUPPORT
        case TIFFTAG_WHITEPOINT:
                setFloatArray(&td->td_whitepoint, va_arg(ap, float *), 2);
                break;
        case TIFFTAG_PRIMARYCHROMATICITIES:
                setFloatArray(&td->td_primarychromas, va_arg(ap, float *), 6);
                break;
        case TIFFTAG_TRANSFERFUNCTION:
                v = (td->td_samplesperpixel - td->td_extrasamples) > 1 ? 3 : 1;
                for (i = 0; i < v; i++)
                        setShortArray(&td->td_transferfunction[i],
                            va_arg(ap, uint16*), 1L<<td->td_bitspersample);
                break;
        case TIFFTAG_REFERENCEBLACKWHITE:
                /* XXX should check for null range */
                setFloatArray(&td->td_refblackwhite, va_arg(ap, float *), 6);
                break;
#endif
#ifdef CMYK_SUPPORT
        case TIFFTAG_INKSET:
                td->td_inkset = va_arg(ap, int);
                break;
        case TIFFTAG_DOTRANGE:
                /* XXX should check for null range */
                td->td_dotrange[0] = va_arg(ap, int);
                td->td_dotrange[1] = va_arg(ap, int);
                break;
        case TIFFTAG_INKNAMES:
                setString(&td->td_inknames, va_arg(ap, char *));
                break;
        case TIFFTAG_TARGETPRINTER:
                setString(&td->td_targetprinter, va_arg(ap, char *));
                break;
#endif
        default:
                TIFFError(tif->tif_name,
                    "Internal error, tag value botch, tag \"%s\"",
                    TIFFFieldWithTag(tag)->field_name);
                status = 0;
                break;
        }
        if (status) {
                TIFFSetFieldBit(tif, TIFFFieldWithTag(tag)->field_bit);
                tif->tif_flags |= TIFF_DIRTYDIRECT;
        }
        va_end(ap);
        return (status);
badvalue:
        TIFFError(tif->tif_name, "%d: Bad value for \"%s\"", v,
            TIFFFieldWithTag(tag)->field_name);
        va_end(ap);
        return (0);
badvalue32:
        TIFFError(tif->tif_name, "%ld: Bad value for \"%s\"", v32,
            TIFFFieldWithTag(tag)->field_name);
        va_end(ap);
        return (0);
}

/*
 * Return 1/0 according to whether or not
 * it is permissible to set the tag's value.
 * Note that we allow ImageLength to be changed
 * so that we can append and extend to images.
 * Any other tag may not be altered once writing
 * has commenced, unless its value has no effect
 * on the format of the data that is written.
 */
static int
OkToChangeTag(TIFF* tif, ttag_t tag)
{
        if (tag != TIFFTAG_IMAGELENGTH &&
            (tif->tif_flags & TIFF_BEENWRITING)) {
                const TIFFFieldInfo *fip = TIFFFindFieldInfo(tag, TIFF_ANY);
                /*
                 * Consult info table to see if tag can be changed
                 * after we've started writing.  We only allow changes
                 * to those tags that don't/shouldn't affect the
                 * compression and/or format of the data.
                 */
                if (fip && !fip->field_oktochange)
                        return (0);
        }
        return (1);
}

/*
 * Record the value of a field in the
 * internal directory structure.  The
 * field will be written to the file
 * when/if the directory structure is
 * updated.
 */
int
TIFFSetField(TIFF* tif, ttag_t tag, ...)
{
        int status = 0;

        if (OkToChangeTag(tif, tag)) {
                va_list ap;

                va_start(ap, tag);
                status = TIFFSetField1(tif, tag, ap);
                va_end(ap);
        } else {
                const TIFFFieldInfo *fip = TIFFFindFieldInfo(tag, TIFF_ANY);
                if (fip)
                        TIFFError("TIFFSetField",
                            "%s: Cannot modify tag \"%s\" while writing",
                            tif->tif_name, fip->field_name);
        }
        return (status);
}

/*
 * Like TIFFSetField, but taking a varargs
 * parameter list.  This routine is useful
 * for building higher-level interfaces on
 * top of the library.
 */
int
TIFFVSetField(TIFF* tif, ttag_t tag, va_list ap)
{
        int status = 0;

        if (!OkToChangeTag(tif, tag)) {
                const TIFFFieldInfo *fip = TIFFFindFieldInfo(tag, TIFF_ANY);
                if (fip)
                        TIFFError("TIFFVSetField",
                            "%s: Cannot modify tag \"%s\" while writing",
                            tif->tif_name, fip->field_name);
        } else
                status = TIFFSetField1(tif, tag, ap);
        return (status);
}

static void
TIFFGetField1(TIFFDirectory* td, ttag_t tag, va_list ap)
{
        switch (tag) {
        case TIFFTAG_SUBFILETYPE:
                *va_arg(ap, uint32*) = td->td_subfiletype;
                break;
        case TIFFTAG_IMAGEWIDTH:
                *va_arg(ap, uint32*) = td->td_imagewidth;
                break;
        case TIFFTAG_IMAGELENGTH:
                *va_arg(ap, uint32*) = td->td_imagelength;
                break;
        case TIFFTAG_BITSPERSAMPLE:
                *va_arg(ap, uint16*) = td->td_bitspersample;
                break;
        case TIFFTAG_COMPRESSION:
                *va_arg(ap, uint16*) = td->td_compression;
                break;
        case TIFFTAG_PHOTOMETRIC:
                *va_arg(ap, uint16*) = td->td_photometric;
                break;
        case TIFFTAG_THRESHHOLDING:
                *va_arg(ap, uint16*) = td->td_threshholding;
                break;
        case TIFFTAG_FILLORDER:
                *va_arg(ap, uint16*) = td->td_fillorder;
                break;
        case TIFFTAG_DOCUMENTNAME:
                *va_arg(ap, char **) = td->td_documentname;
                break;
        case TIFFTAG_ARTIST:
                *va_arg(ap, char **) = td->td_artist;
                break;
        case TIFFTAG_DATETIME:
                *va_arg(ap, char **) = td->td_datetime;
                break;
        case TIFFTAG_HOSTCOMPUTER:
                *va_arg(ap, char **) = td->td_hostcomputer;
                break;
        case TIFFTAG_IMAGEDESCRIPTION:
                *va_arg(ap, char **) = td->td_imagedescription;
                break;
        case TIFFTAG_MAKE:
                *va_arg(ap, char **) = td->td_make;
                break;
        case TIFFTAG_MODEL:
                *va_arg(ap, char **) = td->td_model;
                break;
        case TIFFTAG_SOFTWARE:
                *va_arg(ap, char **) = td->td_software;
                break;
        case TIFFTAG_ORIENTATION:
                *va_arg(ap, uint16*) = td->td_orientation;
                break;
        case TIFFTAG_SAMPLESPERPIXEL:
                *va_arg(ap, uint16*) = td->td_samplesperpixel;
                break;
        case TIFFTAG_ROWSPERSTRIP:
                *va_arg(ap, uint32*) = td->td_rowsperstrip;
                break;
        case TIFFTAG_MINSAMPLEVALUE:
                *va_arg(ap, uint16*) = td->td_minsamplevalue;
                break;
        case TIFFTAG_MAXSAMPLEVALUE:
                *va_arg(ap, uint16*) = td->td_maxsamplevalue;
                break;
        case TIFFTAG_XRESOLUTION:
                *va_arg(ap, float *) = td->td_xresolution;
                break;
        case TIFFTAG_YRESOLUTION:
                *va_arg(ap, float *) = td->td_yresolution;
                break;
        case TIFFTAG_PLANARCONFIG:
                *va_arg(ap, uint16*) = td->td_planarconfig;
                break;
        case TIFFTAG_XPOSITION:
                *va_arg(ap, float *) = td->td_xposition;
                break;
        case TIFFTAG_YPOSITION:
                *va_arg(ap, float *) = td->td_yposition;
                break;
        case TIFFTAG_PAGENAME:
                *va_arg(ap, char **) = td->td_pagename;
                break;
        case TIFFTAG_GROUP3OPTIONS:
                *va_arg(ap, uint32*) = td->td_group3options;
                break;
        case TIFFTAG_GROUP4OPTIONS:
                *va_arg(ap, uint32*) = td->td_group4options;
                break;
        case TIFFTAG_RESOLUTIONUNIT:
                *va_arg(ap, uint16*) = td->td_resolutionunit;
                break;
        case TIFFTAG_PAGENUMBER:
                *va_arg(ap, uint16*) = td->td_pagenumber[0];
                *va_arg(ap, uint16*) = td->td_pagenumber[1];
                break;
        case TIFFTAG_HALFTONEHINTS:
                *va_arg(ap, uint16*) = td->td_halftonehints[0];
                *va_arg(ap, uint16*) = td->td_halftonehints[1];
                break;
        case TIFFTAG_COLORMAP:
                *va_arg(ap, uint16**) = td->td_colormap[0];
                *va_arg(ap, uint16**) = td->td_colormap[1];
                *va_arg(ap, uint16**) = td->td_colormap[2];
                break;
        case TIFFTAG_PREDICTOR:
                *va_arg(ap, uint16*) = td->td_predictor;
                break;
        case TIFFTAG_STRIPOFFSETS:
        case TIFFTAG_TILEOFFSETS:
                *va_arg(ap, uint32**) = td->td_stripoffset;
                break;
        case TIFFTAG_STRIPBYTECOUNTS:
        case TIFFTAG_TILEBYTECOUNTS:
                *va_arg(ap, uint32**) = td->td_stripbytecount;
                break;
        case TIFFTAG_MATTEING:
                *va_arg(ap, uint16*) =
                    (td->td_extrasamples == 1 &&
                     td->td_sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA);
                break;
        case TIFFTAG_EXTRASAMPLES:
                *va_arg(ap, uint16*) = td->td_extrasamples;
                *va_arg(ap, uint16**) = td->td_sampleinfo;
                break;
        case TIFFTAG_BADFAXLINES:
                *va_arg(ap, uint32*) = td->td_badfaxlines;
                break;
        case TIFFTAG_CLEANFAXDATA:
                *va_arg(ap, uint16*) = td->td_cleanfaxdata;
                break;
        case TIFFTAG_CONSECUTIVEBADFAXLINES:
                *va_arg(ap, uint32*) = td->td_badfaxrun;
                break;
        case TIFFTAG_TILEWIDTH:
                *va_arg(ap, uint32*) = td->td_tilewidth;
                break;
        case TIFFTAG_TILELENGTH:
                *va_arg(ap, uint32*) = td->td_tilelength;
                break;
        case TIFFTAG_TILEDEPTH:
                *va_arg(ap, uint32*) = td->td_tiledepth;
                break;
        case TIFFTAG_DATATYPE:
                *va_arg(ap, uint16*) =
                    (td->td_sampleformat == SAMPLEFORMAT_VOID ?
                        0 : td->td_sampleformat);
                break;
        case TIFFTAG_SAMPLEFORMAT:
                *va_arg(ap, uint16*) = td->td_sampleformat;
                break;
        case TIFFTAG_IMAGEDEPTH:
                *va_arg(ap, uint32*) = td->td_imagedepth;
                break;
#ifdef YCBCR_SUPPORT
        case TIFFTAG_YCBCRCOEFFICIENTS:
                *va_arg(ap, float **) = td->td_ycbcrcoeffs;
                break;
        case TIFFTAG_YCBCRPOSITIONING:
                *va_arg(ap, uint16*) = td->td_ycbcrpositioning;
                break;
        case TIFFTAG_YCBCRSUBSAMPLING:
                *va_arg(ap, uint16*) = td->td_ycbcrsubsampling[0];
                *va_arg(ap, uint16*) = td->td_ycbcrsubsampling[1];
                break;
#endif
#ifdef JPEG_SUPPORT
        case TIFFTAG_JPEGPROC:
                *va_arg(ap, uint16*) = td->td_jpegproc;
                break;
        case TIFFTAG_JPEGRESTARTINTERVAL:
                *va_arg(ap, uint16*) = td->td_jpegrestartinterval;
                break;
        case TIFFTAG_JPEGQTABLES:
                *va_arg(ap, u_char ***) = td->td_qtab;
                break;
        case TIFFTAG_JPEGDCTABLES:
                *va_arg(ap, u_char ***) = td->td_dctab;
                break;
        case TIFFTAG_JPEGACTABLES:
                *va_arg(ap, u_char ***) = td->td_actab;
                break;
#endif
#ifdef COLORIMETRY_SUPPORT
        case TIFFTAG_WHITEPOINT:
                *va_arg(ap, float **) = td->td_whitepoint;
                break;
        case TIFFTAG_PRIMARYCHROMATICITIES:
                *va_arg(ap, float **) = td->td_primarychromas;
                break;
        case TIFFTAG_TRANSFERFUNCTION:
                *va_arg(ap, uint16**) = td->td_transferfunction[0];
                if (td->td_samplesperpixel - td->td_extrasamples > 1) {
                        *va_arg(ap, uint16**) = td->td_transferfunction[1];
                        *va_arg(ap, uint16**) = td->td_transferfunction[2];
                }
                break;
        case TIFFTAG_REFERENCEBLACKWHITE:
                *va_arg(ap, float **) = td->td_refblackwhite;
                break;
#endif
#ifdef CMYK_SUPPORT
        case TIFFTAG_INKSET:
                *va_arg(ap, uint16*) = td->td_inkset;
                break;
        case TIFFTAG_DOTRANGE:
                *va_arg(ap, uint16*) = td->td_dotrange[0];
                *va_arg(ap, uint16*) = td->td_dotrange[1];
                break;
        case TIFFTAG_INKNAMES:
                *va_arg(ap, char **) = td->td_inknames;
                break;
        case TIFFTAG_TARGETPRINTER:
                *va_arg(ap, char **) = td->td_targetprinter;
                break;
#endif
        default:
                TIFFError("TIFFGetField1",
                    "Internal error, no value returned for tag \"%s\"",
                    TIFFFieldWithTag(tag)->field_name);
                break;
        }
        va_end(ap);
}

/*
 * Return the value of a field in the
 * internal directory structure.
 */
int
TIFFGetField(TIFF* tif, ttag_t tag, ...)
{
        const TIFFFieldInfo *fip = TIFFFindFieldInfo(tag, TIFF_ANY);

        if (fip) {
                u_short bit = fip->field_bit;
                if (bit != FIELD_IGNORE && TIFFFieldSet(tif, bit)) {
                        va_list ap;
                        va_start(ap, tag);
                        TIFFGetField1(&tif->tif_dir, tag, ap);
                        va_end(ap);
                        return (1);
                }
        } else
                TIFFError("TIFFGetField", "Unknown field, tag 0x%x", tag);
        return (0);
}

/*
 * Like TIFFGetField, but taking a varargs
 * parameter list.  This routine is useful
 * for building higher-level interfaces on
 * top of the library.
 */
int
TIFFVGetField(TIFF* tif, ttag_t tag, va_list ap)
{
        const TIFFFieldInfo *fip = TIFFFindFieldInfo(tag, TIFF_ANY);

        if (fip) {
                u_short bit = fip->field_bit;
                if (bit != FIELD_IGNORE && TIFFFieldSet(tif, bit)) {
                        TIFFGetField1(&tif->tif_dir, tag, ap);
                        return (1);
                }
        } else
                TIFFError("TIFFGetField", "Unknown field, tag 0x%x", tag);
        return (0);
}

/*
 * Internal interface to TIFFGetField...
 */
void
_TIFFgetfield(TIFFDirectory* td, ttag_t tag, ...)
{
        va_list ap;
        va_start(ap, tag);
        TIFFGetField1(td, tag, ap);
        va_end(ap);
}

#define CleanupField(member) {          \
    if (td->member) {                   \
        _TIFFfree((char *)td->member);  \
        td->member = 0;                 \
    }                                   \
}

/*
 * Release storage associated with a directory.
 */
void
TIFFFreeDirectory(TIFF* tif)
{
        register TIFFDirectory *td = &tif->tif_dir;

        CleanupField(td_colormap[0]);
        CleanupField(td_colormap[1]);
        CleanupField(td_colormap[2]);
        CleanupField(td_documentname);
        CleanupField(td_artist);
        CleanupField(td_datetime);
        CleanupField(td_hostcomputer);
        CleanupField(td_imagedescription);
        CleanupField(td_make);
        CleanupField(td_model);
        CleanupField(td_software);
        CleanupField(td_pagename);
        CleanupField(td_sampleinfo);
#ifdef YCBCR_SUPPORT
        CleanupField(td_ycbcrcoeffs);
#endif
#ifdef JPEG_SUPPORT
        CleanupField(td_qtab);
        CleanupField(td_dctab);
        CleanupField(td_actab);
#endif
#ifdef CMYK_SUPPORT
        CleanupField(td_inknames);
        CleanupField(td_targetprinter);
#endif
#ifdef COLORIMETRY_SUPPORT
        CleanupField(td_whitepoint);
        CleanupField(td_primarychromas);
        CleanupField(td_refblackwhite);
        CleanupField(td_transferfunction[0]);
        CleanupField(td_transferfunction[1]);
        CleanupField(td_transferfunction[2]);
#endif
        CleanupField(td_stripoffset);
        CleanupField(td_stripbytecount);
}
#undef CleanupField

/*
 * Setup a default directory structure.
 */
int
TIFFDefaultDirectory(TIFF* tif)
{
        register TIFFDirectory *td = &tif->tif_dir;

        memset(td, 0, sizeof (*td));
        td->td_fillorder = FILLORDER_MSB2LSB;
        td->td_bitspersample = 1;
        td->td_threshholding = THRESHHOLD_BILEVEL;
        td->td_orientation = ORIENTATION_TOPLEFT;
        td->td_samplesperpixel = 1;
        td->td_predictor = 1;
        td->td_rowsperstrip = 0xffffffff;
        td->td_tilewidth = 0xffffffff;
        td->td_tilelength = 0xffffffff;
        td->td_tiledepth = 1;
        td->td_resolutionunit = RESUNIT_INCH;
        td->td_sampleformat = SAMPLEFORMAT_VOID;
        td->td_imagedepth = 1;
#ifdef YCBCR_SUPPORT
        td->td_ycbcrsubsampling[0] = 2;
        td->td_ycbcrsubsampling[1] = 2;
        td->td_ycbcrpositioning = YCBCRPOSITION_CENTERED;
#endif
#ifdef CMYK_SUPPORT
        td->td_inkset = INKSET_CMYK;
#endif
        (void) TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        /*
         * NB: The directory is marked dirty as a result of setting
         * up the default compression scheme.  However, this really
         * isn't correct -- we want TIFF_DIRTYDIRECT to be set only
         * if the user does something.  We could just do the setup
         * by hand, but it seems better to use the normal mechanism
         * (i.e. TIFFSetField).
         */
        tif->tif_flags &= ~TIFF_DIRTYDIRECT;
        return (1);
}

/*
 * Set the n-th directory as the current directory.
 * NB: Directories are numbered starting at 0.
 */
int
TIFFSetDirectory(TIFF* tif, tdir_t dirn)
{
        static const char module[] = "TIFFSetDirectory";
        uint16 dircount;
        uint32 nextdir;
        tdir_t n;

        nextdir = tif->tif_header.tiff_diroff;
        for (n = dirn; n > 0 && nextdir != 0; n--) {
                if (!SeekOK(tif, nextdir) ||
                    !ReadOK(tif, &dircount, sizeof (uint16))) {
                        TIFFError(module, "%s: Error fetching directory count",
                            tif->tif_name);
                        return (0);
                }
                if (tif->tif_flags & TIFF_SWAB)
                        TIFFSwabShort(&dircount);
                TIFFSeekFile(tif, dircount*sizeof (TIFFDirEntry), L_INCR);
                if (!ReadOK(tif, &nextdir, sizeof (uint32))) {
                        TIFFError(module, "%s: Error fetching directory link",
                            tif->tif_name);
                        return (0);
                }
                if (tif->tif_flags & TIFF_SWAB)
                        TIFFSwabLong(&nextdir);
        }
        tif->tif_nextdiroff = nextdir;
        /*
         * Set curdir to the actual directory index.  The
         * -1 is because TIFFReadDirectory will increment
         * tif_curdir after successfully reading the directory.
         */
        tif->tif_curdir = (dirn - n) - 1;
        return (TIFFReadDirectory(tif));
}

/*
 * Return an indication of whether or not we are
 * at the last directory in the file.
 */
int
TIFFLastDirectory(TIFF* tif)
{
        return (tif->tif_nextdiroff == 0);
}
