// pnm-image-radiance.cc
//
// PNMImage::ReadRadiance() and PNMImage::WriteRadiance().

// Code in this file was initially taken from ra_skel.c, a skeletal Radiance
// picture file conversion program provided on the Radiance WWW server, and
// heavily modified.


/* Copyright (c) 1992 Regents of the University of California */

/*
 *  Skeletal 24-bit image conversion program.  Replace "skel"
 *  in this file with a more appropriate image type identifier.
 *
 *  The Rmakefile entry should look something like this:
 *      ra_skel:        ra_skel.o
 *              cc $(CFLAGS) -o ra_skel ra_skel.o -lrt -lm
 *      ra_skel.o:      ../common/color.h ../common/resolu.h
 *
 *  If you like to do things the hard way, you can link directly
 *  to the object files "color.o colrops.o resolu.o header.o" in
 *  the common subdirectory instead of using the -lrt library.
 */

#include <pandabase.h>

#include <stdio.h>
#include <math.h>
#ifndef WIN32VC
#ifndef PENV_WIN32
#include <alloca.h>
#endif
#endif

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"

extern "C" {
  #include  "color.h"
  #include  "resolu.h"

  void setcolrgam(double);
  int checkheader(FILE *, char *, FILE *);
  int fgetresolu(int *, int *, FILE *);
  int freadcolrs(COLR *, int, FILE *);
  int fwritecolrs(COLR *, unsigned, FILE *);
  void fputformat(char *, FILE *);
  void shiftcolrs(COLR *, int, int);
  void colrs_gambs(COLR *, int);
  void newheader(char *, FILE *);
  void printargs(int, char **, FILE *);
  void gambs_colrs(COLR *, int);
}

double  gamcor = 2.2;                   /* gamma correction */

int  bradj = 0;                         /* brightness adjustment */

static const int COLR_MAX = 255;


PNMReaderRadiance::
PNMReaderRadiance(FILE *file, int already_read_magic) : PNMReader(file) {
  setcolrgam(gamcor);             /* set up gamma correction */

  if (already_read_magic >= 0) {
    ungetc(already_read_magic >> 8, file);
    ungetc(already_read_magic & 0xff, file);
  }

  /* get our header */
  if (checkheader(file, COLRFMT, NULL) < 0 ||
      fgetresolu(&cols, &rows, file) < 0) {
    valid = false;
    return;
  }

  color_type = PNMImage::Color;
  maxval = COLR_MAX;
}

bool PNMReaderRadiance::
ReadRow(xel *row_data, xelval *) {
  COLR *scanin;
  int x;

  scanin = (COLR *)alloca(cols * sizeof(COLR));

  if (freadcolrs(scanin, cols, file) < 0) {
    return false;
  }
  
  if (bradj) {                      /* adjust exposure */
    shiftcolrs(scanin, cols, bradj);
  }
  
  colrs_gambs(scanin, cols);      /* gamma correction */
  
  for (x = 0; x < cols; x++) {
    PPM_ASSIGN(row_data[x], scanin[x][RED], scanin[x][GRN], scanin[x][BLU]);
  }

  return true;
}

bool PNMWriterRadiance::
WriteHeader() {
  setcolrgam(gamcor);             /* set up gamma correction */

  /* put our header */
  newheader("RADIANCE", file);
  fputs("Generated via DRR's pnm-image library\n", file);
  fputformat(COLRFMT, file);
  putc('\n', file);
  fprtresolu(cols, rows, file);

  return true;
}

bool PNMWriterRadiance::
WriteRow(xel *row_data, xelval *) {
  /* convert file */
  int is_grayscale = PNMImage::IsGrayscale(color_type);

  COLR *scanout;
  int x;

  /* allocate scanline */
  scanout = (COLR *)alloca(cols * sizeof(COLR));

  /* convert image */
  for (x = 0; x < cols; x++) {
    if (is_grayscale) {
      scanout[x][RED] =
	scanout[x][GRN] =
	scanout[x][BLU] = (BYTE)((int)COLR_MAX * PPM_GETB(row_data[x]) / maxval);
    } else {
      scanout[x][RED] = (BYTE)((int)COLR_MAX * PPM_GETR(row_data[x]) / maxval);
      scanout[x][GRN] = (BYTE)((int)COLR_MAX * PPM_GETG(row_data[x]) / maxval);
      scanout[x][BLU] = (BYTE)((int)COLR_MAX * PPM_GETB(row_data[x]) / maxval);
    }
  }

  /* undo gamma */
  gambs_colrs(scanout, cols);
  if (bradj)                      /* adjust exposure */
    shiftcolrs(scanout, cols, bradj);
  if (fwritecolrs(scanout, cols, file) < 0)
    return false;

  return true;
}  
