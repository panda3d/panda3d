// pnm-image-readyuv.cc
//
// PNMImage::ReadYUV() and supporting functions.



// Much code in this file is borrowed from Netpbm, specifically yuvtoppm.c.
/* yuvtoppm.c - convert Abekas YUV bytes into a portable pixmap
**
** by Marc Boucher
** Internet: marc@PostImage.cxxOM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69
**
** Uses integer arithmetic rather than floating point for better performance
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/



#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"

#include <notify.h>

// YUV format doesn't include an image size specification, so the image size
// must be specified externally.  The defaults here are likely candidates,
// since this is the Abekas native size; the ysize is automatically adjusted
// down to account for a short file.
int yuv_xsize = 720;
int yuv_ysize = 486;

/* x must be signed for the following to work correctly */
#define limit(x) (xelval)(((x>0xffffff)?0xff0000:((x<=0xffff)?0:x&0xff0000))>>16)

PNMReaderYUV::
PNMReaderYUV(FILE *file, int already_read_magic) : PNMReader(file) {
  yuvbuf = NULL;

  if (already_read_magic >= 0) {
    ungetc(already_read_magic >> 8, file);
    ungetc(already_read_magic & 0xff, file);
  }

  cols = yuv_xsize;
  rows = yuv_ysize;
  color_type = PNMImage::Color;

  if (cols <= 0 || rows <= 0) {
    valid = false;
    return;
  }

  nassertv(255 <= PGM_MAXMAXVAL);

  yuvbuf = (long *) pm_allocrow(cols, 2);

  maxval = 255;
}

bool PNMReaderYUV::
ReadRow(xel *row_data, xelval *) {
  long tmp, y, u, v, y1, r, g, b, *yuvptr;
  int col;

  if (fread(yuvbuf, cols * 2, 1, file) != 1) {
    // Short file--perhaps it's just a field instead of a full frame.
    // Since the YUV format does not include a length designation, we'll
    // have to assume this is not a problem and just truncate here.
    return false;
  }

  for (col = 0, yuvptr = yuvbuf; col < cols; col += 2) {
    tmp = *yuvptr++;
    u = (0xff & (tmp >> 24)) - 128;
    y = ((0xff & (tmp >> 16)) - 16);
    if (y < 0) y = 0;
    
    v = (0xff & (tmp >> 8)) - 128;
    y1 = ((0xff & tmp) - 16);
    if (y1 < 0) y1 = 0;
    
    r = 104635 * v;
    g = -25690 * u + -53294 * v;
    b = 132278 * u;
    
    y*=76310; y1*=76310;
    
    PPM_ASSIGN(row_data[col], limit(r+y), limit(g+y), limit(b+y));
    PPM_ASSIGN(row_data[col+1], limit(r+y1), limit(g+y1), limit(b+y1));
  }
  return true;
}

PNMReaderYUV::
~PNMReaderYUV() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }
}


