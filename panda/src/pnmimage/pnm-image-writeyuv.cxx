// pnm-image-writeyuv.cc
//
// PNMImage::WriteYUV() and supporting functions.



// Much code in this file is borrowed from Netpbm, specifically ppmtoyuv.c.
/* ppmtoyuv.c - convert a portable pixmap into an Abekas YUV file
**
** by Marc Boucher
** Internet: marc@PostImage.cxxOM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69.
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/



#include "pnmImage.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"


bool PNMWriterYUV::
WriteHeader() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }

  yuvbuf = (unsigned char *) pm_allocrow( cols, 2 );

  return true;
}

bool PNMWriterYUV::
WriteRow(xel *row_data, xelval *) {
  int             col;
  unsigned long   y1, y2=0, u=0, v=0, u0=0, u1, u2, v0=0, v1, v2;
  static const int max_byte = 255;

  unsigned char *yuvptr;
    
  for (col = 0, yuvptr=yuvbuf; col < cols; col += 2) {
    pixval r, g, b;
      
    /* first pixel gives Y and 0.5 of chroma */
    r = (pixval)(max_byte * PPM_GETR(row_data[col])/maxval);
    g = (pixval)(max_byte * PPM_GETG(row_data[col])/maxval);
    b = (pixval)(max_byte * PPM_GETB(row_data[col])/maxval);
    
    y1 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y2);
    u1 = -4853 * r - 9530 * g + 14383 * b;
    v1 = 14386 * r - 12046 * g - 2340 * b;
    
    /* second pixel just yields a Y and 0.25 U, 0.25 V */
    r = (pixval)(max_byte * PPM_GETR(row_data[col])/maxval);
    g = (pixval)(max_byte * PPM_GETG(row_data[col])/maxval);
    b = (pixval)(max_byte * PPM_GETB(row_data[col])/maxval);
    
    y2 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y1);
    u2 = -2426 * r - 4765 * g + 7191 * b;
    v2 = 7193 * r - 6023 * g - 1170 * b;
    
    /* filter the chroma */
    u = u0 + u1 + u2 + (0xffff & u);
    v = v0 + v1 + v2 + (0xffff & v);
    
    u0 = u2;
    v0 = v2;
    
    *yuvptr++ = (unsigned char)((u >> 16) + 128);
    *yuvptr++ = (unsigned char)((y1 >> 16) + 16);
    *yuvptr++ = (unsigned char)((v >> 16) + 128);
    *yuvptr++ = (unsigned char)((y2 >> 16) + 16);
  }
  fwrite(yuvbuf, cols*2, 1, file);

  return true;
}

PNMWriterYUV::
~PNMWriterYUV() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }
}
