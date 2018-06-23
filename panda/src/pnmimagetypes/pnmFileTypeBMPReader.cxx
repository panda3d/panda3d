/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeBMPReader.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "pnmFileTypeBMP.h"

#ifdef HAVE_BMP

#include "config_pnmimagetypes.h"
#include "bmp.h"
#include "pnmbitio.h"

using std::istream;
using std::string;

// Much code in this file is borrowed from Netpbm, specifically bmptoppm.c.
/*
 * bmptoppm.c - Converts from a Microsoft Windows or OS/2 .BMP file to a
 * PPM file.
 *
 * The current implementation is probably not complete, but it works for
 * all the BMP files I have.  I welcome feedback.
 *
 * Copyright (C) 1992 by David W. Sanderson.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
 */

/*
 * Utilities
 */

static int GetByte (istream * fp);
static short GetShort (istream * fp);
static long GetLong (istream * fp);
static void readto (istream *fp, unsigned long *ppos, unsigned long dst);
static void BMPreadfileheader (istream *fp, unsigned long *ppos,
    unsigned long *poffBits);
static void BMPreadinfoheader (istream *fp, unsigned long *ppos,
    unsigned long *pcx, unsigned long *pcy, unsigned short *pcBitCount,
    int *pclassv);
static int BMPreadrgbtable (istream *fp, unsigned long *ppos,
    unsigned short cBitCount, int classv, pixval *R, pixval *G, pixval *B);

static const char *ifname = "BMP";
static char     er_read[] = "%s: read error";
// static char     er_seek[] = "%s: seek error";

static int
GetByte(istream *fp)
{
        int             v;

        if ((v = fp->get()) == EOF)
        {
                pm_error(er_read, ifname);
        }

        return v;
}

static short
GetShort(istream *fp)
{
        short           v;

        if (pm_readlittleshort(fp, &v) == -1)
        {
                pm_error(er_read, ifname);
        }

        return v;
}

static long
GetLong(istream *fp)
{
        long            v;

        if (pm_readlittlelong(fp, &v) == -1)
        {
                pm_error(er_read, ifname);
        }

        return v;
}

/*
 * readto - read as many bytes as necessary to position the
 * file at the desired offset.
 */

static void
readto(istream           *fp,
        unsigned long  *ppos,   /* pointer to number of bytes read from fp */
        unsigned long   dst)
{
        unsigned long   pos;

        if(!fp || !ppos)
                return;

        pos = *ppos;

        if(pos > dst)
                pm_error("%s: internal error in readto()", ifname);

        for(; pos < dst; pos++)
        {
                if (fp->get() == EOF)
                {
                        pm_error(er_read, ifname);
                }
        }

        *ppos = pos;
}


/*
 * BMP reading routines
 */

static void
BMPreadfileheader(
        istream           *fp,
        unsigned long  *ppos,   /* number of bytes read from fp */
        unsigned long  *poffBits)
{
        /*
        unsigned long   cbSize;
        unsigned short  xHotSpot;
        unsigned short  yHotSpot;
        */
        unsigned long   offBits;

        /*
          We've already read the magic number.
        if (GetByte(fp) != 'B')
        {
                pm_error("%s is not a BMP file", ifname);
        }
        if (GetByte(fp) != 'M')
        {
                pm_error("%s is not a BMP file", ifname);
        }
        */

        /* cbSize = */ GetLong(fp);
        /* xHotSpot = */ GetShort(fp);
        /* yHotSpot = */ GetShort(fp);
        offBits = GetLong(fp);

        *poffBits = offBits;

        *ppos += 14;
}

static void
BMPreadinfoheader(
        istream           *fp,
        unsigned long  *ppos,   /* number of bytes read from fp */
        unsigned long  *pcx,
        unsigned long  *pcy,
        unsigned short *pcBitCount,
        int            *pclassv)
{
        unsigned long   cbFix;
        unsigned short  cPlanes = 0;

        unsigned long   cx = 0;
        unsigned long   cy = 0;
        unsigned short  cBitCount = 0;
        int             classv = 0;

        cbFix = GetLong(fp);

        switch (cbFix)
        {
        case 12:
                classv = C_OS2;
                break;
        case 40:  // BITMAPINFOHEADER
                classv = C_WIN;
                break;
        case 52:  // BITMAPV2INFOHEADER
                classv = C_WINV2;
                break;
        case 56:  // BITMAPV3INFOHEADER
                classv = C_WINV3;
                break;
        case 108:  // BITMAPV4HEADER
                classv = C_WINV4;
                break;
        case 124:  // BITMAPV5HEADER
                classv = C_WINV5;
                break;
        default:
                pm_error("%s: unknown cbFix: %d", ifname, cbFix);
                break;
        }

        if (classv == C_OS2) {
                cx = GetShort(fp);
                cy = GetShort(fp);
        } else {
                cx = GetLong(fp);
                cy = GetLong(fp);
        }
        cPlanes = GetShort(fp);
        cBitCount = GetShort(fp);

        /*
         * We've read 16 bytes so far, need to read more
         * for the required total.
         */
        if (classv != C_OS2) {
            for (int i = 0; i < (int)cbFix - 16; i += 4) {
                GetLong(fp);
            }
        }

        if (cPlanes != 1)
        {
                pm_error("%s: don't know how to handle cPlanes = %d"
                         ,ifname
                         ,cPlanes);
        }

        switch (classv)
        {
        case C_WIN:
                pm_message("Windows BMP, %dx%dx%d"
                           ,cx
                           ,cy
                           ,cBitCount);
                break;
        case C_WINV2:
        case C_WINV3:
        case C_WINV4:
        case C_WINV5:
                pm_message("Windows BMP V%d, %dx%dx%d"
                           ,(classv - C_WINV2 + 2)
                           ,cx
                           ,cy
                           ,cBitCount);
                break;
        case C_OS2:
                pm_message("OS/2 BMP, %dx%dx%d"
                           ,cx
                           ,cy
                           ,cBitCount);
                break;
        }

#ifdef DEBUG
        pm_message("cbFix: %d", cbFix);
        pm_message("cx: %d", cx);
        pm_message("cy: %d", cy);
        pm_message("cPlanes: %d", cPlanes);
        pm_message("cBitCount: %d", cBitCount);
#endif

        *pcx = cx;
        *pcy = cy;
        *pcBitCount = cBitCount;
        *pclassv = classv;

        *ppos += cbFix;
}

/*
 * returns the number of bytes read, or -1 on error.
 */
static int
BMPreadrgbtable(
        istream           *fp,
        unsigned long  *ppos,   /* number of bytes read from fp */
        unsigned short  cBitCount,
        int             classv,
        pixval         *R,
        pixval         *G,
        pixval         *B)
{
        int             i;
        int             nbyte = 0;

        long            ncolors = (1 << cBitCount);

        for (i = 0; i < ncolors; i++)
        {
                B[i] = (pixval) GetByte(fp);
                G[i] = (pixval) GetByte(fp);
                R[i] = (pixval) GetByte(fp);
                nbyte += 3;

                if (classv != C_OS2)
                {
                        (void) GetByte(fp);
                        nbyte++;
                }
        }

        *ppos += nbyte;
        return nbyte;
}

/*
 * returns the number of bytes read, or -1 on error.
 */
static int
BMPreadrow(
        istream           *fp,
        unsigned long  *ppos,   /* number of bytes read from fp */
        pixel          *row,
        xelval         *alpha_row,
        unsigned long   cx,
        unsigned short  cBitCount,
        int             indexed,
        pixval         *R,
        pixval         *G,
        pixval         *B)
{
        BITSTREAM       b = nullptr;
        unsigned        nbyte = 0;
        int             rc;
        unsigned        x;

        if (indexed) {
          if ((b = pm_bitinit(fp, "r")) == nullptr)
            {
              return -1;
            }
        }

        for (x = 0; x < cx; x++, row++)
        {
                unsigned long   v;

                if (!indexed) {
                  int r, g, b;
                  b = GetByte(fp);
                  g = GetByte(fp);
                  r = GetByte(fp);
                  if (cBitCount > 24) {
                    *(alpha_row++) = GetByte(fp);
                    ++nbyte;
                  }
                  nbyte += 3;
                  PPM_ASSIGN(*row, r, g, b);
                } else {
                  if ((rc = pm_bitread(b, cBitCount, &v)) == -1)
                    {
                      return -1;
                    }
                  nbyte += rc;

                  PPM_ASSIGN(*row, R[v], G[v], B[v]);
                }
        }

        if (indexed) {
          if ((rc = pm_bitfini(b)) != 0)
            {
              return -1;
            }
        }

        /*
         * Make sure we read a multiple of 4 bytes.
         */
        while (nbyte % 4)
        {
                GetByte(fp);
                nbyte++;
        }

        *ppos += nbyte;
        return nbyte;
}

static void
BMPreadbits(xel *array, xelval *alpha_array,
        istream           *fp,
        unsigned long  *ppos,   /* number of bytes read from fp */
        unsigned long   offBits,
        unsigned long   cx,
        unsigned long   cy,
        unsigned short  cBitCount,
        int             /* classv */,
        int             indexed,
        pixval         *R,
        pixval         *G,
        pixval         *B)
{
        long            y;

        readto(fp, ppos, offBits);

        if(cBitCount > 24 && cBitCount != 32)
        {
                pm_error("%s: cannot handle cBitCount: %d"
                         ,ifname
                         ,cBitCount);
        }

        /*
         * The picture is stored bottom line first, top line last
         */

        for (y = (long)cy - 1; y >= 0; y--)
        {
                int rc;
                rc = BMPreadrow(fp, ppos, array + y*cx, alpha_array + y*cx, cx, cBitCount, indexed, R, G, B);
                if(rc == -1)
                {
                        pm_error("%s: couldn't read row %d"
                                 ,ifname
                                 ,y);
                }
                if(rc%4)
                {
                        pm_error("%s: row had bad number of bytes: %d"
                                 ,ifname
                                 ,rc);
                }
        }

}

/**
 *
 */
PNMFileTypeBMP::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  if (!read_magic_number(_file, magic_number, 2)) {
    // No magic number, no image.
    if (pnmimage_bmp_cat.is_debug()) {
      pnmimage_bmp_cat.debug()
        << "BMP image file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  if (magic_number != string("BM")) {
    pnmimage_bmp_cat.error()
      << "File is not a valid BMP file.\n";
    _is_valid = false;
    return;
  }

  int             rc;

  unsigned long   cx;
  unsigned long   cy;

  pos = 0;

  BMPreadfileheader(file, &pos, &offBits);
  BMPreadinfoheader(file, &pos, &cx, &cy, &cBitCount, &classv);

  if (offBits != BMPoffbits(classv, cBitCount)) {
    pnmimage_bmp_cat.warning()
      << "offBits is " << offBits << ", expected "
      << BMPoffbits(classv, cBitCount) << "\n";
  }

  indexed = false;

  if (cBitCount <= 8) {
    indexed = true;
    rc = BMPreadrgbtable(file, &pos, cBitCount, classv, R, G, B);

    if (rc != (int)BMPlenrgbtable(classv, cBitCount)) {
      pnmimage_bmp_cat.warning()
        << rc << "-byte RGB table, expected "
        << BMPlenrgbtable(classv, cBitCount) << " bytes\n";
    }
  }

  if (cBitCount > 24) {
    _num_channels = 4;
  } else {
    _num_channels = 3;
  }
  _x_size = (int)cx;
  _y_size = (int)cy;
  _maxval = 255;

  if (pnmimage_bmp_cat.is_debug()) {
    pnmimage_bmp_cat.debug()
      << "Reading BMP " << *this << "\n";
  }
}


/**
 * Reads in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * read.
 *
 * Derived classes need not override this if they instead provide
 * supports_read_row() and read_row(), below.
 */
int PNMFileTypeBMP::Reader::
read_data(xel *array, xelval *alpha_array) {
  BMPreadbits(array, alpha_array, _file, &pos, offBits, _x_size, _y_size,
              cBitCount, classv, indexed, R, G, B);

  if (pos != BMPlenfile(classv, cBitCount, _x_size, _y_size)) {
    pnmimage_bmp_cat.warning()
      << "Read " << pos << " bytes, expected to read "
      << BMPlenfile(classv, cBitCount, _x_size, _y_size) << " bytes\n";
  }

  return _y_size;
}

#endif  // HAVE_BMP
