// Filename: pnmFileTypeBMPWriter.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeBMP.h"
#include "config_pnmimagetypes.h"

#include <pnmImage.h>
#include <pnmWriter.h>

extern "C" {
  #include "bmp.h"
  #include "../pnm/ppmcmap.h"
  #include "../pnm/bitio.h"
}

// Much code in this file is borrowed from Netpbm, specifically ppmtobmp.c.
/*\
 * $Id$
 *
 * ppmtobmp.c - Converts from a PPM file to a Microsoft Windows or OS/2
 * .BMP file.
 *
 * The current implementation is probably not complete, but it works for
 * me.  I welcome feedback.
 *
 * Copyright (C) 1992 by David W. Sanderson.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
 *
 * $Log$
 * Revision 1.3  2001/05/25 15:59:19  drose
 * remove tab characters
 *
 * Revision 1.2  2000/11/09 21:14:02  drose
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2000/10/04 01:14:42  drose
 *
 *
 * Revision 1.9  1992/11/24  19:39:33  dws
 * Added copyright.
 *
 * Revision 1.8  1992/11/17  02:16:52  dws
 * Moved length functions to bmp.h.
 *
 * Revision 1.7  1992/11/11  23:18:16  dws
 * Modified to adjust the bits per pixel to 1, 4, or 8.
 *
 * Revision 1.6  1992/11/11  22:43:39  dws
 * Commented out a superfluous message.
 *
 * Revision 1.5  1992/11/11  05:58:06  dws
 * First version that works.
 *
 * Revision 1.4  1992/11/11  03:40:32  dws
 * Moved calculation of bits per pixel to BMPEncode.
 *
 * Revision 1.3  1992/11/11  03:02:34  dws
 * Added BMPEncode function.
 *
 * Revision 1.2  1992/11/08  01:44:35  dws
 * Added option processing and reading of PPM file.
 *
 * Revision 1.1  1992/11/08  00:46:07  dws
 * Initial revision
\*/

#define MAXCOLORS 256

/*
 * Utilities
 */

static char     er_write[] = "stdout: write error";

/* prototypes */
static void PutByte ARGS((FILE *fp, char v));
static void PutShort ARGS((FILE *fp, short v));
static void PutLong ARGS((FILE *fp, long v));
static int BMPwritefileheader ARGS((FILE *fp, int classv, unsigned long bitcount,
    unsigned long x, unsigned long y));
static int BMPwriteinfoheader ARGS((FILE *fp, int classv, unsigned long bitcount,
    unsigned long x, unsigned long y));
static int BMPwritergb ARGS((FILE *fp, int classv, pixval R, pixval G, pixval B));
static int BMPwritergbtable ARGS((FILE *fp, int classv, int bpp, int colors,
    pixval *R, pixval *G, pixval *B));
static int colorstobpp ARGS((int colors));
static void BMPEncode ARGS((FILE *fp, int classv, int x, int y, pixel **pixels,
    int colors, colorhash_table cht, pixval *R, pixval *G, pixval *B));
static void
PutByte(
        FILE           *fp,
        char            v)
{
        if (putc(v, fp) == EOF)
        {
                pm_error(er_write);
        }
}

static void
PutShort(
        FILE           *fp,
        short           v)
{
        if (pm_writelittleshort(fp, v) == -1)
        {
                pm_error(er_write);
        }
}

static void
PutLong(
        FILE           *fp,
        long            v)
{
        if (pm_writelittlelong(fp, v) == -1)
        {
                pm_error(er_write);
        }
}

/*
 * BMP writing
 */

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritefileheader(
        FILE           *fp,
        int             classv,
        unsigned long   bitcount,
        unsigned long   x,
        unsigned long   y)
{
        PutByte(fp, 'B');
        PutByte(fp, 'M');

        /* cbSize */
        PutLong(fp, (long)BMPlenfile(classv, bitcount, x, y));

        /* xHotSpot */
        PutShort(fp, 0);

        /* yHotSpot */
        PutShort(fp, 0);

        /* offBits */
        PutLong(fp, (long)BMPoffbits(classv, bitcount));

        return 14;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwriteinfoheader(
        FILE           *fp,
        int             classv,
        unsigned long   bitcount,
        unsigned long   x,
        unsigned long   y)
{
        long    cbFix;

        /* cbFix */
        switch (classv)
        {
        case C_WIN:
                cbFix = 40;
                PutLong(fp, cbFix);

                /* cx */
                PutLong(fp, (long)x);
                /* cy */
                PutLong(fp, (long)y);
                /* cPlanes */
                PutShort(fp, 1);
                /* cBitCount */
                PutShort(fp, (short)bitcount);

                /*
                 * We've written 16 bytes so far, need to write 24 more
                 * for the required total of 40.
                 */

                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);


                break;
        case C_OS2:
                cbFix = 12;
                PutLong(fp, cbFix);

                /* cx */
                PutShort(fp, (short)x);
                /* cy */
                PutShort(fp, (short)y);
                /* cPlanes */
                PutShort(fp, 1);
                /* cBitCount */
                PutShort(fp, (short)bitcount);

                break;
        default:
                pm_error(er_internal, "BMPwriteinfoheader");
        }

        return cbFix;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritergb(
        FILE           *fp,
        int             classv,
        pixval          R,
        pixval          G,
        pixval          B)
{
        switch (classv)
        {
        case C_WIN:
                PutByte(fp, B);
                PutByte(fp, G);
                PutByte(fp, R);
                PutByte(fp, 0);
                return 4;
        case C_OS2:
                PutByte(fp, B);
                PutByte(fp, G);
                PutByte(fp, R);
                return 3;
        default:
                pm_error(er_internal, "BMPwritergb");
        }
        return -1;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritergbtable(
        FILE           *fp,
        int             classv,
        int             bpp,
        int             colors,
        pixval         *R,
        pixval         *G,
        pixval         *B)
{
        int             nbyte = 0;
        int             i;
        long            ncolors;

        for (i = 0; i < colors; i++)
        {
                nbyte += BMPwritergb(fp,classv,R[i],G[i],B[i]);
        }

        ncolors = (1 << bpp);

        for (; i < ncolors; i++)
        {
                nbyte += BMPwritergb(fp,classv,0,0,0);
        }

        return nbyte;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwriterow(
        FILE           *fp,
        pixel          *row,
        unsigned long   cx,
        unsigned short  bpp,
        int             indexed,
        colorhash_table cht)
{
        BITSTREAM       b;
        unsigned        nbyte = 0;
        int             rc;
        unsigned        x;

        if (indexed) {
          if ((b = pm_bitinit(fp, "w")) == (BITSTREAM) 0)
            {
              return -1;
            }

          for (x = 0; x < cx; x++, row++)
            {
              if ((rc = pm_bitwrite(b, bpp, ppm_lookupcolor(cht, row))) == -1)
                {
                  return -1;
                }
              nbyte += rc;
            }

          if ((rc = pm_bitfini(b)) == -1)
            {
              return -1;
            }
          nbyte += rc;
        } else {

          for (x = 0; x < cx; x++, row++)
            {
              PutByte(fp, PPM_GETB(*row));
              PutByte(fp, PPM_GETG(*row));
              PutByte(fp, PPM_GETR(*row));
              nbyte += 3;
            }
        }

        /*
         * Make sure we write a multiple of 4 bytes.
         */
        while (nbyte % 4)
        {
                PutByte(fp, 0);
                nbyte++;
        }

        return nbyte;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritebits(
        FILE           *fp,
        unsigned long   cx,
        unsigned long   cy,
        unsigned short  cBitCount,
        pixel         **pixels,
        int             indexed,
        colorhash_table cht)
{
        int             nbyte = 0;
        long            y;

        if(cBitCount > 24)
        {
                pm_error("cannot handle cBitCount: %d"
                         ,cBitCount);
        }

        /*
         * The picture is stored bottom line first, top line last
         */

        for (y = (long)cy - 1; y >= 0; y--)
        {
                int rc;
                rc = BMPwriterow(fp, pixels[y], cx, cBitCount, indexed, cht);

                if(rc == -1)
                {
                        pm_error("couldn't write row %d"
                                 ,y);
                }
                if(rc%4)
                {
                        pm_error("row had bad number of bytes: %d"
                                 ,rc);
                }
                nbyte += rc;
        }

        return nbyte;
}

/*
 * Return the number of bits per pixel required to represent the
 * given number of colors.
 */

static int
colorstobpp(int colors)
{
        int             bpp;

        if (colors < 1)
        {
                pm_error("can't have less than one color");
        }

        if ((bpp = pm_maxvaltobits(colors - 1)) > 8)
        {
                pm_error("can't happen");
        }

        return bpp;
}

/*
 * Write a BMP file of the given classv.
 *
 * Note that we must have 'colors' in order to know exactly how many
 * colors are in the R, G, B, arrays.  Entries beyond those in the
 * arrays are undefined.
 */
static void
BMPEncode(
        FILE           *fp,
        int             classv,
        int             x,
        int             y,
        pixel         **pixels,
        int             colors, /* number of valid entries in R,G,B */
        colorhash_table cht,
        pixval         *R,
        pixval         *G,
        pixval         *B)
{
        int             bpp;    /* bits per pixel */
        unsigned long   nbyte = 0;

        bpp = colorstobpp(colors);

        /*
         * I have found empirically at least one BMP-displaying program
         * that can't deal with (for instance) using 3 bits per pixel.
         * I have seen no programs that can deal with using 3 bits per
         * pixel.  I have seen programs which can deal with 1, 4, and
         * 8 bits per pixel.
         *
         * Based on this, I adjust actual the number of bits per pixel
         * as follows.  If anyone knows better, PLEASE tell me!
         */
        switch(bpp)
        {
        case 2:
        case 3:
                bpp = 4;
                break;
        case 5:
        case 6:
        case 7:
                bpp = 8;
                break;
        }

        pm_message("Using %d bits per pixel", bpp);

        nbyte += BMPwritefileheader(fp, classv, bpp, x, y);
        nbyte += BMPwriteinfoheader(fp, classv, bpp, x, y);
        nbyte += BMPwritergbtable(fp, classv, bpp, colors, R, G, B);

        if(nbyte !=     ( BMPlenfileheader(classv)
                        + BMPleninfoheader(classv)
                        + BMPlenrgbtable(classv, bpp)))
        {
                pm_error(er_internal, "BMPEncode");
        }

        nbyte += BMPwritebits(fp, x, y, bpp, pixels, true, cht);
        if(nbyte != BMPlenfile(classv, bpp, x, y))
        {
                pm_error(er_internal, "BMPEncode");
        }
}

/*
 * Write a BMP file of the given class, with 24 bits per pixel nonindexed.
 */
static void
BMPEncode24(
        FILE           *fp,
        int             classv,
        int             x,
        int             y,
        pixel         **pixels)
{
        unsigned long   nbyte = 0;
        int             bpp = 24;
  
        pm_message("Using %d bits per pixel", bpp);

        nbyte += BMPwritefileheader(fp, classv, bpp, x, y);
        nbyte += BMPwriteinfoheader(fp, classv, bpp, x, y);

        if(nbyte !=     ( BMPlenfileheader(classv)
                        + BMPleninfoheader(classv)))
        {
                pm_error(er_internal, "BMPEncode24");
        }

        nbyte += BMPwritebits(fp, x, y, bpp, pixels, false, colorhash_table());
        if(nbyte != BMPlenfile(classv, bpp, x, y))
        {
                pm_error(er_internal, "BMPEncode24");
        }
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeBMP::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::Writer::write_data
//       Access: Public, Virtual
//  Description: Writes out an entire image all at once, including the
//               header, based on the image data stored in the given
//               _x_size * _y_size array and alpha pointers.  (If the
//               image type has no alpha channel, alpha is ignored.)
//               Returns the number of rows correctly written.
//
//               It is the user's responsibility to fill in the header
//               data via calls to set_x_size(), set_num_channels(),
//               etc., or copy_header_from(), before calling
//               write_data().
//
//               It is important to delete the PNMWriter class after
//               successfully writing the data.  Failing to do this
//               may result in some data not getting flushed!
//
//               Derived classes need not override this if they
//               instead provide supports_streaming() and write_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMFileTypeBMP::Writer::
write_data(xel *array, xelval *) {
  if (_y_size<=0 || _x_size<=0) {
    return 0;
  }

  int             classv = C_WIN;

  int             colors;
  int             i;
  colorhist_vector chv;
  pixval          Red[MAXCOLORS];
  pixval          Green[MAXCOLORS];
  pixval          Blue[MAXCOLORS];
  
  pixel** pixels;
  colorhash_table cht;

#if 0
  {
    char *name;
    switch (classv)
      {
      case C_WIN:
        name = "a Windows";
        break;
      case C_OS2:
        name = "an OS/2";
        break;
      default:
        pm_error(er_internal, "report");
        break;
      }
    pm_message("generating %s BMP file", name);
  }
#endif

  // We need an honest 2-d array of pixels, instead of one big 1-d array.
  pixels = (pixel **)alloca(sizeof(pixel *) * _y_size);
  for (i = 0; i < _y_size; i++) {
    pixels[i] = (pixel *)(array + i * _x_size);
  }

  /* Figure out the colormap. */
  chv = ppm_computecolorhist(pixels, _x_size, _y_size, MAXCOLORS, &colors);
  if (chv == (colorhist_vector) 0) {
    pnmimage_bmp_cat.debug()
      << "too many colors; generating 24-bit BMP file\n";

    BMPEncode24(_file, classv, _x_size, _y_size, pixels);

  } else {
    pnmimage_bmp_cat.debug()
      << colors << " colors found\n";
  
    /*
     * Now turn the ppm colormap into the appropriate BMP colormap.  
     */
    if (_maxval > 255) {
      pnmimage_bmp_cat.debug()
        << "maxval is not 255 - automatically rescaling colors\n";
    }

    for (i = 0; i < colors; ++i) {
      if (_maxval == 255) {
        Red[i] = PPM_GETR(chv[i].color);
        Green[i] = PPM_GETG(chv[i].color);
        Blue[i] = PPM_GETB(chv[i].color);
      } else {
        Red[i] = (pixval) PPM_GETR(chv[i].color) * 255 / _maxval;
        Green[i] = (pixval) PPM_GETG(chv[i].color) * 255 / _maxval;
        Blue[i] = (pixval) PPM_GETB(chv[i].color) * 255 / _maxval;
      }
    }
    
    /* And make a hash table for fast lookup. */
    cht = ppm_colorhisttocolorhash(chv, colors);
    ppm_freecolorhist(chv);
    
    BMPEncode(_file, classv, _x_size, _y_size, pixels, colors, cht,
              Red, Green, Blue);
  }
    
  return _y_size;
}
