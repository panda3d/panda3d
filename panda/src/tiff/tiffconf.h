/* $Header$ */
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

#ifndef _TIFFCONF_
#define _TIFFCONF_
/*
 * Library Configuration Definitions.
 *
 * This file defines the default configuration for the library.
 * If the target system does not have make or a way to specify
 * #defines on the command line, this file can be edited to
 * configure the library.  Otherwise, one can override portability
 * and configuration-related definitions from a Makefile or command
 * line by defining FEATURE_SUPPORT and COMPRESSION_SUPPORT (see below).
 */

/*
 * General portability-related defines:
 *
 * HAVE_IEEEFP          define as 0 or 1 according to the floating point
 *                      format suported by the machine
 * BSDTYPES             define this if your system does NOT define the
 *                      usual 4BSD typedefs
 */
#ifndef HAVE_IEEEFP
#define HAVE_IEEEFP     1
#endif

#ifndef FEATURE_SUPPORT
/*
 * Feature support definitions:
 *
 *    MMAP_SUPPORT      enable support for memory mapping read-only files
 *    COLORIMETRY_SUPPORT enable support for 6.0 colorimetry tags
 *    JPEG_SUPPORT      enable support for 6.0 JPEG tags & JPEG algorithms
 *    YCBCR_SUPPORT     enable support for 6.0 YCbCr tags
 *    CMYK_SUPPORT      enable support for 6.0 CMYK tags
 */
#define COLORIMETRY_SUPPORT
#define JPEG_SUPPORT
#define YCBCR_SUPPORT
#define CMYK_SUPPORT
#endif

#ifndef COMPRESSION_SUPPORT
/*
 * Compression support defines:
 *
 *    CCITT_SUPPORT     enable support for CCITT Group 3 & 4 algorithms
 *    PACKBITS_SUPPORT  enable support for Macintosh PackBits algorithm
 *    LZW_SUPPORT       enable support for LZW algorithm
 *    THUNDER_SUPPORT   enable support for ThunderScan 4-bit RLE algorithm
 *    NEXT_SUPPORT      enable support for NeXT 2-bit RLE algorithm
 *    JPEG_SUPPORT      enable support for JPEG DCT algorithm
 */
#define CCITT_SUPPORT
#define PACKBITS_SUPPORT
#define LZW_SUPPORT
#define THUNDER_SUPPORT
#define NEXT_SUPPORT
#endif

/*
 * ``Orthogonal Features''
 *
 * STRIPCHOP_SUPPORT    automatically convert single-strip uncompressed images
 *                      to mutiple strips of ~8Kb (for reducing memory use)
 */
#ifndef STRIPCHOP_SUPPORT
#define STRIPCHOP_SUPPORT               /* enable strip chopping */
#endif
#endif /* _TIFFCONF_ */
