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
 * Directory Printing Support
 */
#include "tiffiop.h"
#include <stdio.h>

#ifdef JPEG_SUPPORT
static void
JPEGPrintQTable(FILE* fd, u_char* tab)
{
        int i, j;
        char *sep;

        fputc('\n', fd);
        for (i = 0; i < 8; i++) {
                sep = "    ";
                for (j = 0; j < 8; j++) {
                        fprintf(fd, "%s%2u", sep, tab[8*i+j]);
                        sep = ", ";
                }
                fputc('\n', fd);
        }
}

static void
JPEGPrintCTable(FILE* fd, u_char* tab)
{
        int i, n, count;
        char *sep;

        fprintf(fd, "\n    Bits:");
        count = 0;
        for (i = 0; i < 16; i++) {
                fprintf(fd, " %u", tab[i]);
                count += tab[i];
        }
        n = 0;
        for (; count > 0; count--) {
                if ((n % 8) == 0) {
                        fputc('\n', fd);
                        sep = "    ";
                }
                fprintf(fd, "%s0x%02x", sep, tab[i++]);
                sep = ", ";
                n++;

        }
        if (n % 8)
                fputc('\n', fd);
}
#endif

static const char *photoNames[] = {
    "min-is-white",                             /* PHOTOMETRIC_MINISWHITE */
    "min-is-black",                             /* PHOTOMETRIC_MINISBLACK */
    "RGB color",                                /* PHOTOMETRIC_RGB */
    "palette color (RGB from colormap)",        /* PHOTOMETRIC_PALETTE */
    "transparency mask",                        /* PHOTOMETRIC_MASK */
    "separated",                                /* PHOTOMETRIC_SEPARATED */
    "YCbCr",                                    /* PHOTOMETRIC_YCBCR */
    "7 (0x7)",
    "CIE L*a*b*",                               /* PHOTOMETRIC_CIELAB */
};
#define NPHOTONAMES     (sizeof (photoNames) / sizeof (photoNames[0]))

static const char *orientNames[] = {
    "0 (0x0)",
    "row 0 top, col 0 lhs",                     /* ORIENTATION_TOPLEFT */
    "row 0 top, col 0 rhs",                     /* ORIENTATION_TOPRIGHT */
    "row 0 bottom, col 0 rhs",                  /* ORIENTATION_BOTRIGHT */
    "row 0 bottom, col 0 lhs",                  /* ORIENTATION_BOTLEFT */
    "row 0 lhs, col 0 top",                     /* ORIENTATION_LEFTTOP */
    "row 0 rhs, col 0 top",                     /* ORIENTATION_RIGHTTOP */
    "row 0 rhs, col 0 bottom",                  /* ORIENTATION_RIGHTBOT */
    "row 0 lhs, col 0 bottom",                  /* ORIENTATION_LEFTBOT */
};
#define NORIENTNAMES    (sizeof (orientNames) / sizeof (orientNames[0]))

/*
 * Print the contents of the current directory
 * to the specified stdio file stream.
 */
void
TIFFPrintDirectory(TIFF* tif, FILE* fd, long flags)
{
        register TIFFDirectory *td;
        char *sep;
        int i, j;
        long n;

        fprintf(fd, "TIFF Directory at offset 0x%x\n", tif->tif_diroff);
        td = &tif->tif_dir;
        if (TIFFFieldSet(tif,FIELD_SUBFILETYPE)) {
                fprintf(fd, "  Subfile Type:");
                sep = " ";
                if (td->td_subfiletype & FILETYPE_REDUCEDIMAGE) {
                        fprintf(fd, "%sreduced-resolution image", sep);
                        sep = "/";
                }
                if (td->td_subfiletype & FILETYPE_PAGE) {
                        fprintf(fd, "%smulti-page document", sep);
                        sep = "/";
                }
                if (td->td_subfiletype & FILETYPE_MASK)
                        fprintf(fd, "%stransparency mask", sep);
                fprintf(fd, " (%u = 0x%x)\n",
                    td->td_subfiletype, td->td_subfiletype);
        }
        if (TIFFFieldSet(tif,FIELD_IMAGEDIMENSIONS)) {
                fprintf(fd, "  Image Width: %lu Image Length: %lu",
                    (u_long) td->td_imagewidth, (u_long) td->td_imagelength);
                if (TIFFFieldSet(tif,FIELD_IMAGEDEPTH))
                        fprintf(fd, " Image Depth: %lu",
                            (u_long) td->td_imagedepth);
                fprintf(fd, "\n");
        }
        if (TIFFFieldSet(tif,FIELD_TILEDIMENSIONS)) {
                fprintf(fd, "  Tile Width: %lu Tile Length: %lu",
                    (u_long) td->td_tilewidth, (u_long) td->td_tilelength);
                if (TIFFFieldSet(tif,FIELD_TILEDEPTH))
                        fprintf(fd, " Tile Depth: %lu",
                            (u_long) td->td_tiledepth);
                fprintf(fd, "\n");
        }
        if (TIFFFieldSet(tif,FIELD_RESOLUTION)) {
                fprintf(fd, "  Resolution: %g, %g",
                    td->td_xresolution, td->td_yresolution);
                if (TIFFFieldSet(tif,FIELD_RESOLUTIONUNIT)) {
                        switch (td->td_resolutionunit) {
                        case RESUNIT_NONE:
                                fprintf(fd, " (unitless)");
                                break;
                        case RESUNIT_INCH:
                                fprintf(fd, " pixels/inch");
                                break;
                        case RESUNIT_CENTIMETER:
                                fprintf(fd, " pixels/cm");
                                break;
                        default:
                                fprintf(fd, " (unit %u = 0x%x)",
                                    td->td_resolutionunit,
                                    td->td_resolutionunit);
                                break;
                        }
                }
                fprintf(fd, "\n");
        }
        if (TIFFFieldSet(tif,FIELD_POSITION))
                fprintf(fd, "  Position: %g, %g\n",
                    td->td_xposition, td->td_yposition);
        if (TIFFFieldSet(tif,FIELD_BITSPERSAMPLE))
                fprintf(fd, "  Bits/Sample: %u\n", td->td_bitspersample);
        if (TIFFFieldSet(tif,FIELD_SAMPLEFORMAT)) {
                fprintf(fd, "  Sample Format: ");
                switch (td->td_sampleformat) {
                case SAMPLEFORMAT_VOID:
                        fprintf(fd, "void\n");
                        break;
                case SAMPLEFORMAT_INT:
                        fprintf(fd, "signed integer\n");
                        break;
                case SAMPLEFORMAT_UINT:
                        fprintf(fd, "unsigned integer\n");
                        break;
                case SAMPLEFORMAT_IEEEFP:
                        fprintf(fd, "IEEE floating point\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_sampleformat, td->td_sampleformat);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_COMPRESSION)) {
                fprintf(fd, "  Compression Scheme: ");
                switch (td->td_compression) {
                case COMPRESSION_NONE:
                        fprintf(fd, "none\n");
                        break;
                case COMPRESSION_CCITTRLE:
                        fprintf(fd, "CCITT modified Huffman encoding\n");
                        break;
                case COMPRESSION_CCITTFAX3:
                        fprintf(fd, "CCITT Group 3 facsimile encoding\n");
                        break;
                case COMPRESSION_CCITTFAX4:
                        fprintf(fd, "CCITT Group 4 facsimile encoding\n");
                        break;
                case COMPRESSION_CCITTRLEW:
                        fprintf(fd, "CCITT modified Huffman encoding %s\n",
                            "w/ word alignment");
                        break;
                case COMPRESSION_PACKBITS:
                        fprintf(fd, "Macintosh PackBits encoding\n");
                        break;
                case COMPRESSION_THUNDERSCAN:
                        fprintf(fd, "ThunderScan 4-bit encoding\n");
                        break;
                case COMPRESSION_LZW:
                        fprintf(fd, "Lempel-Ziv & Welch encoding\n");
                        break;
                case COMPRESSION_NEXT:
                        fprintf(fd, "NeXT 2-bit encoding\n");
                        break;
                case COMPRESSION_JPEG:
                        fprintf(fd, "JPEG encoding\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_compression, td->td_compression);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_PHOTOMETRIC)) {
                fprintf(fd, "  Photometric Interpretation: ");
                if (td->td_photometric < NPHOTONAMES)
                        fprintf(fd, "%s\n", photoNames[td->td_photometric]);
                else
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_photometric, td->td_photometric);
        }
        if (TIFFFieldSet(tif,FIELD_EXTRASAMPLES) && td->td_extrasamples) {
                fprintf(fd, "  Extra Samples: %u<", td->td_extrasamples);
                sep = "";
                for (i = 0; i < td->td_extrasamples; i++) {
                        switch (td->td_sampleinfo[i]) {
                        case EXTRASAMPLE_UNSPECIFIED:
                                fprintf(fd, "%sunspecified", sep);
                                break;
                        case EXTRASAMPLE_ASSOCALPHA:
                                fprintf(fd, "%sassoc-alpha", sep);
                                break;
                        case EXTRASAMPLE_UNASSALPHA:
                                fprintf(fd, "%sunassoc-alpha", sep);
                                break;
                        default:
                                fprintf(fd, "%s%u (0x%x)", sep,
                                    td->td_sampleinfo[i], td->td_sampleinfo[i]);
                                break;
                        }
                        sep = ", ";
                }
                fprintf(fd, ">\n");
        }
#ifdef CMYK_SUPPORT
        if (TIFFFieldSet(tif,FIELD_INKSET)) {
                fprintf(fd, "  Ink Set: ");
                switch (td->td_inkset) {
                case INKSET_CMYK:
                        fprintf(fd, "CMYK\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_inkset, td->td_inkset);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_INKNAMES)) {
                char *cp;
                fprintf(fd, "  Ink Names: ");
                i = td->td_samplesperpixel;
                sep = "";
                for (cp = td->td_inknames; i > 0; cp = strchr(cp, '\0')) {
                        fprintf(fd, "%s%s", sep, cp);
                        sep = ", ";
                }
        }
        if (TIFFFieldSet(tif,FIELD_DOTRANGE))
                fprintf(fd, "  Dot Range: %u-%u\n",
                    td->td_dotrange[0], td->td_dotrange[1]);
        if (TIFFFieldSet(tif,FIELD_TARGETPRINTER))
                fprintf(fd, "  Target Printer: %s\n", td->td_targetprinter);
#endif
        if (TIFFFieldSet(tif,FIELD_THRESHHOLDING)) {
                fprintf(fd, "  Thresholding: ");
                switch (td->td_threshholding) {
                case THRESHHOLD_BILEVEL:
                        fprintf(fd, "bilevel art scan\n");
                        break;
                case THRESHHOLD_HALFTONE:
                        fprintf(fd, "halftone or dithered scan\n");
                        break;
                case THRESHHOLD_ERRORDIFFUSE:
                        fprintf(fd, "error diffused\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_threshholding, td->td_threshholding);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_FILLORDER)) {
                fprintf(fd, "  FillOrder: ");
                switch (td->td_fillorder) {
                case FILLORDER_MSB2LSB:
                        fprintf(fd, "msb-to-lsb\n");
                        break;
                case FILLORDER_LSB2MSB:
                        fprintf(fd, "lsb-to-msb\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_fillorder, td->td_fillorder);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_PREDICTOR)) {
                fprintf(fd, "  Predictor: ");
                switch (td->td_predictor) {
                case 1:
                        fprintf(fd, "none\n");
                        break;
                case 2:
                        fprintf(fd, "horizontal differencing\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_predictor, td->td_predictor);
                        break;
                }
        }
#ifdef YCBCR_SUPPORT
        if (TIFFFieldSet(tif,FIELD_YCBCRSUBSAMPLING))
                fprintf(fd, "  YCbCr Subsampling: %u, %u\n",
                    td->td_ycbcrsubsampling[0], td->td_ycbcrsubsampling[1]);
        if (TIFFFieldSet(tif,FIELD_YCBCRPOSITIONING)) {
                fprintf(fd, "  YCbCr Positioning: ");
                switch (td->td_ycbcrpositioning) {
                case YCBCRPOSITION_CENTERED:
                        fprintf(fd, "centered\n");
                        break;
                case YCBCRPOSITION_COSITED:
                        fprintf(fd, "cosited\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_ycbcrpositioning, td->td_ycbcrpositioning);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_YCBCRCOEFFICIENTS))
                fprintf(fd, "  YCbCr Coefficients: %g, %g, %g\n",
                    td->td_ycbcrcoeffs[0],
                    td->td_ycbcrcoeffs[1],
                    td->td_ycbcrcoeffs[2]);
#endif
#ifdef JPEG_SUPPORT
        if (TIFFFieldSet(tif,FIELD_JPEGPROC)) {
                fprintf(fd, "  JPEG Processing Mode: ");
                switch (td->td_jpegproc) {
                case JPEGPROC_BASELINE:
                        fprintf(fd, "baseline sequential algorithm\n");
                        break;
                case JPEGPROC_LOSSLESS:
                        fprintf(fd, "lossless algorithm with Huffman coding\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_jpegproc, td->td_jpegproc);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_JPEGRESTARTINTERVAL)) {
                fprintf(fd, "  JPEG Restart Interval: ");
                if (td->td_jpegrestartinterval)
                        fprintf(fd, "%u\n", td->td_jpegrestartinterval);
                else
                        fprintf(fd, "(no restart markers)\n");
        }
        if (TIFFFieldSet(tif,FIELD_JPEGQTABLES)) {
                fprintf(fd, "  JPEG Quantization Tables: ");
                if (flags & TIFFPRINT_JPEGQTABLES) {
                        for (i = 0; i < td->td_samplesperpixel; i++)
                                JPEGPrintQTable(fd, td->td_qtab[i]);
                } else
                        fprintf(fd, "(present)\n");
        }
        if (TIFFFieldSet(tif,FIELD_JPEGDCTABLES)) {
                fprintf(fd, "  JPEG DC Tables: ");
                if (flags & TIFFPRINT_JPEGDCTABLES) {
                        for (i = 0; i < td->td_samplesperpixel; i++)
                                JPEGPrintCTable(fd, td->td_dctab[i]);
                } else
                        fprintf(fd, "(present)\n");
        }
        if (TIFFFieldSet(tif,FIELD_JPEGACTABLES)) {
                fprintf(fd, "  JPEG AC Tables: ");
                if (flags & TIFFPRINT_JPEGACTABLES) {
                        for (i = 0; i < td->td_samplesperpixel; i++)
                                JPEGPrintCTable(fd, td->td_actab[i]);
                } else
                        fprintf(fd, "(present)\n");
        }
#endif
        if (TIFFFieldSet(tif,FIELD_HALFTONEHINTS))
                fprintf(fd, "  Halftone Hints: light %u dark %u\n",
                    td->td_halftonehints[0], td->td_halftonehints[1]);
        if (TIFFFieldSet(tif,FIELD_ARTIST))
                fprintf(fd, "  Artist: \"%s\"\n", td->td_artist);
        if (TIFFFieldSet(tif,FIELD_DATETIME))
                fprintf(fd, "  Date & Time: \"%s\"\n", td->td_datetime);
        if (TIFFFieldSet(tif,FIELD_HOSTCOMPUTER))
                fprintf(fd, "  Host Computer: \"%s\"\n", td->td_hostcomputer);
        if (TIFFFieldSet(tif,FIELD_SOFTWARE))
                fprintf(fd, "  Software: \"%s\"\n", td->td_software);
        if (TIFFFieldSet(tif,FIELD_DOCUMENTNAME))
                fprintf(fd, "  Document Name: \"%s\"\n", td->td_documentname);
        if (TIFFFieldSet(tif,FIELD_IMAGEDESCRIPTION))
                fprintf(fd, "  Image Description: \"%s\"\n",
                    td->td_imagedescription);
        if (TIFFFieldSet(tif,FIELD_MAKE))
                fprintf(fd, "  Make: \"%s\"\n", td->td_make);
        if (TIFFFieldSet(tif,FIELD_MODEL))
                fprintf(fd, "  Model: \"%s\"\n", td->td_model);
        if (TIFFFieldSet(tif,FIELD_ORIENTATION)) {
                fprintf(fd, "  Orientation: ");
                if (td->td_orientation < NORIENTNAMES)
                        fprintf(fd, "%s\n", orientNames[td->td_orientation]);
                else
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_orientation, td->td_orientation);
        }
        if (TIFFFieldSet(tif,FIELD_SAMPLESPERPIXEL))
                fprintf(fd, "  Samples/Pixel: %u\n", td->td_samplesperpixel);
        if (TIFFFieldSet(tif,FIELD_ROWSPERSTRIP)) {
                fprintf(fd, "  Rows/Strip: ");
                if (td->td_rowsperstrip == 0xffffffffL)
                        fprintf(fd, "(infinite)\n");
                else
                        fprintf(fd, "%lu\n", (u_long) td->td_rowsperstrip);
        }
        if (TIFFFieldSet(tif,FIELD_MINSAMPLEVALUE))
                fprintf(fd, "  Min Sample Value: %u\n", td->td_minsamplevalue);
        if (TIFFFieldSet(tif,FIELD_MAXSAMPLEVALUE))
                fprintf(fd, "  Max Sample Value: %u\n", td->td_maxsamplevalue);
        if (TIFFFieldSet(tif,FIELD_PLANARCONFIG)) {
                fprintf(fd, "  Planar Configuration: ");
                switch (td->td_planarconfig) {
                case PLANARCONFIG_CONTIG:
                        fprintf(fd, "single image plane\n");
                        break;
                case PLANARCONFIG_SEPARATE:
                        fprintf(fd, "separate image planes\n");
                        break;
                default:
                        fprintf(fd, "%u (0x%x)\n",
                            td->td_planarconfig, td->td_planarconfig);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_PAGENAME))
                fprintf(fd, "  Page Name: \"%s\"\n", td->td_pagename);
        if (TIFFFieldSet(tif,FIELD_GROUP3OPTIONS)) {
                fprintf(fd, "  Group 3 Options:");
                sep = " ";
                if (td->td_group3options & GROUP3OPT_2DENCODING)
                        fprintf(fd, "%s2-d encoding", sep), sep = "+";
                if (td->td_group3options & GROUP3OPT_FILLBITS)
                        fprintf(fd, "%sEOL padding", sep), sep = "+";
                if (td->td_group3options & GROUP3OPT_UNCOMPRESSED)
                        fprintf(fd, "%suncompressed data", sep);
                fprintf(fd, " (%lu = 0x%lx)\n",
                    (u_long) td->td_group3options,
                    (u_long) td->td_group3options);
        }
        if (TIFFFieldSet(tif,FIELD_CLEANFAXDATA)) {
                fprintf(fd, "  Fax Data: ");
                switch (td->td_cleanfaxdata) {
                case CLEANFAXDATA_CLEAN:
                        fprintf(fd, "clean\n");
                        break;
                case CLEANFAXDATA_REGENERATED:
                        fprintf(fd, "receiver regenerated\n");
                        break;
                case CLEANFAXDATA_UNCLEAN:
                        fprintf(fd, "uncorrected errors\n");
                        break;
                default:
                        fprintf(fd, "(%u = 0x%x)\n",
                            td->td_cleanfaxdata, td->td_cleanfaxdata);
                        break;
                }
        }
        if (TIFFFieldSet(tif,FIELD_BADFAXLINES))
                fprintf(fd, "  Bad Fax Lines: %lu\n",
                    (u_long) td->td_badfaxlines);
        if (TIFFFieldSet(tif,FIELD_BADFAXRUN))
                fprintf(fd, "  Consecutive Bad Fax Lines: %u\n",
                    td->td_badfaxrun);
        if (TIFFFieldSet(tif,FIELD_GROUP4OPTIONS)) {
                fprintf(fd, "  Group 4 Options:");
                if (td->td_group4options & GROUP4OPT_UNCOMPRESSED)
                        fprintf(fd, "uncompressed data");
                fprintf(fd, " (%lu = 0x%lx)\n",
                    (u_long) td->td_group4options,
                    (u_long) td->td_group4options);
        }
        if (TIFFFieldSet(tif,FIELD_PAGENUMBER))
                fprintf(fd, "  Page Number: %u-%u\n",
                    td->td_pagenumber[0], td->td_pagenumber[1]);
        if (TIFFFieldSet(tif,FIELD_COLORMAP)) {
                fprintf(fd, "  Color Map: ");
                if (flags & TIFFPRINT_COLORMAP) {
                        fprintf(fd, "\n");
                        n = 1L<<td->td_bitspersample;
                        for (i = 0; i < n; i++)
                                fprintf(fd, "   %5d: %5u %5u %5u\n",
                                    i,
                                    td->td_colormap[0][i],
                                    td->td_colormap[1][i],
                                    td->td_colormap[2][i]);
                } else
                        fprintf(fd, "(present)\n");
        }
#ifdef COLORIMETRY_SUPPORT
        if (TIFFFieldSet(tif,FIELD_WHITEPOINT))
                fprintf(fd, "  White Point: %g-%g\n",
                    td->td_whitepoint[0], td->td_whitepoint[1]);
        if (TIFFFieldSet(tif,FIELD_PRIMARYCHROMAS))
                fprintf(fd, "  Primary Chromaticities: %g,%g %g,%g %g,%g\n",
                    td->td_primarychromas[0], td->td_primarychromas[1],
                    td->td_primarychromas[2], td->td_primarychromas[3],
                    td->td_primarychromas[4], td->td_primarychromas[5]);
        if (TIFFFieldSet(tif,FIELD_REFBLACKWHITE)) {
                fprintf(fd, "  Reference Black/White:\n");
                for (i = 0; i < td->td_samplesperpixel; i++)
                        fprintf(fd, "    %2d: %5g %5g\n",
                            i,
                            td->td_refblackwhite[2*i+0],
                            td->td_refblackwhite[2*i+1]);
        }
        if (TIFFFieldSet(tif,FIELD_TRANSFERFUNCTION)) {
                fprintf(fd, "  Transfer Function: ");
                if (flags & TIFFPRINT_CURVES) {
                        fprintf(fd, "\n");
                        n = 1L<<td->td_bitspersample;
                        for (i = 0; i < n; i++) {
                                fprintf(fd, "    %2d: %5u",
                                    i, td->td_transferfunction[0][i]);
                                for (j = 1; j < td->td_samplesperpixel; j++)
                                        fprintf(fd, " %5u",
                                            td->td_transferfunction[j][i]);
                                putc('\n', fd);
                        }
                } else
                        fprintf(fd, "(present)\n");
        }
#endif
        if ((flags & TIFFPRINT_STRIPS) &&
            TIFFFieldSet(tif,FIELD_STRIPOFFSETS)) {
                fprintf(fd, "  %u %s:\n",
                    td->td_nstrips,
                    isTiled(tif) ? "Tiles" : "Strips");
                for (i = 0; i < td->td_nstrips; i++)
                        fprintf(fd, "    %3d: [%8lu, %8lu]\n",
                            i,
                            (u_long) td->td_stripoffset[i],
                            (u_long) td->td_stripbytecount[i]);
        }
}
