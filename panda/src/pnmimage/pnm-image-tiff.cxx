// pnm-image-readtiff.cc
//
// PNMImage::ReadTIFF() and supporting functions.



// Much code in this file is borrowed from Netpbm, specifically tifftopnm.c
// and pnmtotiff.c.
/*
** tifftopnm.c - converts a Tagged Image File to a portable anymap
**
** Derived by Jef Poskanzer from tif2ras.c, which is:
**
** Copyright (c) 1990 by Sun Microsystems, Inc.
**
** Author: Patrick J. Naughton
** naughton@wind.sun.com
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted,
** provided that the above copyright notice appear in all copies and that
** both that copyright notice and this permission notice appear in
** supporting documentation.
**
** This file is provided AS IS with no warranties of any kind.  The author
** shall have no liability with respect to the infringement of copyrights,
** trade secrets or any patents by this file or any part thereof.  In no
** event will the author be liable for any lost revenue or profits or
** other special, indirect and consequential damages.
*/

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
#include "../pnm/sgi.h"
#include "../pnm/ppmcmap.h"
#include "../tiff/tiff.h"
#include "../tiff/tiffio.h"
}

// true to display image information on load; false otherwise.
int tiff_headerdump = false;

// These are configurable parameters to specify TIFF details on output.
// See tools/drr/../pnm/libtiff/tiff.h or type man pnmtotiff for a better
// explanation of options.

unsigned short tiff_compression = COMPRESSION_LZW;
/* One of:
   COMPRESSION_NONE
   COMPRESSION_CCITTRLE
   COMPRESSION_CCITTFAX3
   COMPRESSION_CCITTFAX4
   COMPRESSION_LZW		
   COMPRESSION_JPEG		
   COMPRESSION_NEXT		
   COMPRESSION_CCITTRLEW	
   COMPRESSION_PACKBITS	
   COMPRESSION_THUNDERSCAN	
   */

long tiff_g3options = 0;
/* One or more of:
   GROUP3OPT_2DENCODING
   GROUP3OPT_FILLBITS

   meaningful when tiff_compression == COMPRESSION_CCITTFAX3.
   */

unsigned short tiff_fillorder = FILLORDER_MSB2LSB;
/* One of:
   FILLORDER_MSB2LSB
   FILLORDER_LSB2MSB
   */

short tiff_predictor = 0;
/* 0, 1, or 2;  meaningful when tiff_compression == COMPRESSION_LZW. */


long tiff_rowsperstrip = 0;
/* 0 or any positive number */

#define MAXCOLORS 1024
#ifndef PHOTOMETRIC_DEPTH
#define PHOTOMETRIC_DEPTH 32768
#endif

static tsize_t
StdioReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
  return ((tsize_t)fread((void *)buf, 1, (size_t) size, (FILE *)fd));
}

static tsize_t
StdioWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
  return ((tsize_t)fwrite((void *)buf, 1, (size_t) size, (FILE *)fd));
}

static toff_t
StdioSeekProc(thandle_t fd, off_t off, int whence)
{
  fseek((FILE *)fd, (long)off, whence);
  return (toff_t)ftell((FILE *)fd);
}

static int
StdioCloseProc(thandle_t)
{
  // We don't actually close the file; we'll leave that up to our calling
  // procedure.
  return true;
}

static toff_t
StdioSizeProc(thandle_t fd)
{
  fseek((FILE *)fd, 0, SEEK_END);
  return (toff_t)ftell((FILE *)fd);
}

static int
StdioMapProc(thandle_t, tdata_t*, toff_t*)
{
  return (0);
}

static void
StdioUnmapProc(thandle_t, tdata_t, toff_t)
{
}


PNMReaderTIFF::
PNMReaderTIFF(FILE *file, int already_read_magic) : PNMReader(file) {
  int grayscale;
  int numcolors;
  int i;
  unsigned short* redcolormap;
  unsigned short* greencolormap;
  unsigned short* bluecolormap;

  if (already_read_magic >= 0) {
    ungetc(already_read_magic >> 8, file);
    ungetc(already_read_magic & 0xff, file);
  }

  tif = TIFFClientOpen("TIFF file", "r",
		       (thandle_t) file,
		       StdioReadProc, StdioWriteProc,
		       (TIFFSeekProc)StdioSeekProc, 
		       StdioCloseProc, StdioSizeProc,
		       StdioMapProc, StdioUnmapProc);

  if ( tif == NULL )
    pm_error( "error opening TIFF file" );

  if ( tiff_headerdump )
    TIFFPrintDirectory( tif, stderr, TIFFPRINT_NONE );

  if ( ! TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &bps ) )
    bps = 1;
  if ( ! TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &spp ) )
    spp = 1;
  if ( ! TIFFGetField( tif, TIFFTAG_PHOTOMETRIC, &photomet ) )
    pm_error( "error getting photometric" );

  switch ( spp )
    {
    case 1:
      color_type = PNMImage::Grayscale;
      break;
	  
    case 3:
      color_type = PNMImage::Color;
      break;
	  
    case 4:
      color_type = PNMImage::FourChannel;
      break;

    default:
      pm_error(
	       "can only handle 1-channel gray scale or 1- or 3-channel color" );
    }

  (void) TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &cols );
  (void) TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &rows );

  if ( tiff_headerdump )
    {
      pm_message( "%dx%dx%d image", cols, rows, bps * spp );
      pm_message( "%d bits/sample, %d samples/pixel", bps, spp );
    }

  maxval = ( 1 << bps ) - 1;
  if ( maxval == 1 && spp == 1 )
    {
      if ( tiff_headerdump )
	pm_message("monochrome" );
      grayscale = 1;
    }
  else
    {
      switch ( photomet )
	{
	case PHOTOMETRIC_MINISBLACK:
	  if ( tiff_headerdump )
	    pm_message( "%d graylevels (min=black)", maxval + 1 );
	  grayscale = 1;
	  break;

	case PHOTOMETRIC_MINISWHITE:
	  if ( tiff_headerdump )
	    pm_message( "%d graylevels (min=white)", maxval + 1 );
	  grayscale = 1;
	  break;

	case PHOTOMETRIC_PALETTE:
	  if ( tiff_headerdump )
	    pm_message( "colormapped" );
	  if ( ! TIFFGetField( tif, TIFFTAG_COLORMAP, &redcolormap, &greencolormap, &bluecolormap ) )
	    pm_error( "error getting colormaps" );
	  numcolors = maxval + 1;
	  if ( numcolors > MAXCOLORS )
	    pm_error( "too many colors" );
	  maxval = PNM_MAXMAXVAL;
	  grayscale = 0;
	  for ( i = 0; i < numcolors; ++i )
	    {
	      xelval r, g, b;
	      r = (xelval)(maxval * (double)(redcolormap[i] / 65535.0));
	      g = (xelval)(maxval * (double)(greencolormap[i] / 65535.0));
	      b = (xelval)(maxval * (double)(bluecolormap[i] / 65535.0));
	      PPM_ASSIGN( colormap[i], r, g, b );
	    }
	  break;

	case PHOTOMETRIC_RGB:
	  if ( tiff_headerdump )
	    pm_message( "truecolor" );
	  grayscale = 0;
	  break;

	case PHOTOMETRIC_MASK:
	  pm_error( "don't know how to handle PHOTOMETRIC_MASK" );

	case PHOTOMETRIC_DEPTH:
	  pm_error( "don't know how to handle PHOTOMETRIC_DEPTH" );

	default:
	  pm_error( "unknown photometric: %d", photomet );
	}
    }
  if ( maxval > PNM_MAXMAXVAL )
    pm_error(
	     "bits/sample is too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS" );


  if ( grayscale ) {
    color_type = PNMImage::Grayscale;
  } else if (color_type == PNMImage::Grayscale) {
    color_type = PNMImage::Color;
  }

  current_row = 0;
}

#define NEXTSAMPLE \
  { \
    if ( bitsleft == 0 ) \
    { \
      ++inP; \
      bitsleft = 8; \
    } \
    bitsleft -= bps; \
    sample = ( *inP >> bitsleft ) & maxval; \
  }

bool PNMReaderTIFF::
ReadRow(xel *row_data, xelval *alpha_data) {
  unsigned char *buf = (unsigned char*) alloca((size_t)TIFFScanlineSize(tif));
  int col;
  unsigned char sample;

  if ( TIFFReadScanline( tif, buf, current_row, 0 ) < 0 )
    pm_error( "bad data read on line %d", current_row );
  unsigned char *inP = buf;
  int bitsleft = 8;

  switch ( photomet ) {
  case PHOTOMETRIC_MINISBLACK:
    for ( col = 0; col < cols; ++col )
      {
	NEXTSAMPLE;
	PPM_PUTB(row_data[col], sample);
      }
    break;
    
  case PHOTOMETRIC_MINISWHITE:
    for ( col = 0; col < cols; ++col )
      {
	NEXTSAMPLE;
	sample = maxval - sample;
	PPM_PUTB(row_data[col], sample);
      }
    break;
    
  case PHOTOMETRIC_PALETTE:
    for ( col = 0; col < cols; ++col )
      {
	NEXTSAMPLE;
	row_data[col] = colormap[sample];
      }
    break;
    
  case PHOTOMETRIC_RGB:
    for ( col = 0; col < cols; ++col ) {
      xelval r, g, b;
      
      NEXTSAMPLE;
      r = sample;
      NEXTSAMPLE;
      g = sample;
      NEXTSAMPLE;
      b = sample;
      PPM_ASSIGN(row_data[col], r, g, b);
      if ( spp == 4 ) {
	NEXTSAMPLE;  // Alpha channel
	alpha_data[col] = sample;
      }  
    }
    break;
    
  default:
    pm_error( "unknown photometric: %d", photomet );
  }
  
  current_row++;
  return true;
}

PNMReaderTIFF::
~PNMReaderTIFF() {
  TIFFClose( tif );
  //  owns_file = false;
}


int PNMWriterTIFF::
WriteData(xel *array, xelval *alpha) {
  colorhist_vector chv;
  colorhash_table cht;
  unsigned short red[MAXCOLORS], grn[MAXCOLORS], blu[MAXCOLORS];
  int row, colors, i;
  register int col;
  int grayscale;
  struct tiff * tif;
  short photometric;
  short samplesperpixel;
  short bitspersample;
  int bytesperrow;
  unsigned char* buf;
  unsigned char* tP;

    /* Check for grayscale. */
  int is_grayscale = PNMImage::IsGrayscale(color_type);

  switch ( color_type ) {
  case PNMImage::Color:
    pm_message( "computing colormap..." );

    // This call is a bit of fakery to convert our proper 2-d array of
    // xels to an indirect 2-d array of pixels.  We make it look like a
    // single row of cols * rows pixels.
    chv = ppm_computecolorhist( (pixel **)&array, cols * rows, 1,
				MAXCOLORS, &colors );
    if ( chv == (colorhist_vector) 0 ) {
      pm_message("Too many colors - proceeding to write a 24-bit RGB file." );
      pm_message("If you want an 8-bit palette file, try doing a 'ppmquant %d'.",
		 MAXCOLORS );
      grayscale = 0;
    } else {
      pm_message( "%d colors found", colors );
      grayscale = 1;
      for ( i = 0; i < colors; ++i ) {
	register xelval r, g, b;
	
	r = PPM_GETR( chv[i].color );
	g = PPM_GETG( chv[i].color );
	b = PPM_GETB( chv[i].color );
	if ( r != g || g != b ) {
	  grayscale = 0;
	  break;
	}
      }
    }
    break;

  case PNMImage::FourChannel:
    chv = (colorhist_vector) 0;
    grayscale = 0;
    break;

  case PNMImage::TwoChannel:  // We don't yet support two-channel output for TIFF's.
  case PNMImage::Grayscale:
    chv = (colorhist_vector) 0;
    grayscale = 1;
    break;
  }

  /* Open output file. */
  tif = TIFFClientOpen("TIFF file", "w",
		       (thandle_t) file,
		       StdioReadProc, StdioWriteProc,
		       (TIFFSeekProc)StdioSeekProc, 
		       StdioCloseProc, StdioSizeProc,
		       StdioMapProc, StdioUnmapProc);
  if ( tif == NULL )
    return 0;
  
  /* Figure out TIFF parameters. */
  switch ( color_type ) {
  case PNMImage::Color:
  case PNMImage::FourChannel:
    if ( chv == (colorhist_vector) 0 ) {
      samplesperpixel = (color_type==PNMImage::FourChannel ? 4 : 3);
      bitspersample = 8;
      photometric = PHOTOMETRIC_RGB;
      bytesperrow = cols * samplesperpixel;
    } else if ( grayscale ) {
      samplesperpixel = 1;
      bitspersample = pm_maxvaltobits( maxval );
      photometric = PHOTOMETRIC_MINISBLACK;
      bytesperrow = ( cols + i - 1 ) / i;
    } else {
      samplesperpixel = 1;
      bitspersample = 8;
      photometric = PHOTOMETRIC_PALETTE;
      bytesperrow = cols;
    }
    break;

  case PNMImage::Grayscale:
  case PNMImage::TwoChannel:
    samplesperpixel = 1;
    bitspersample = pm_maxvaltobits( maxval );
    photometric = PHOTOMETRIC_MINISBLACK;
    i = 8 / bitspersample;
    bytesperrow = ( cols + i - 1 ) / i;
    break;
  }

  if ( tiff_rowsperstrip == 0 )
    tiff_rowsperstrip = ( 8 * 1024 ) / bytesperrow;
  buf = (unsigned char*) malloc( bytesperrow );
  if ( buf == (unsigned char*) 0 )
    pm_error( "can't allocate memory for row buffer" );

  /* Set TIFF parameters. */
  TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, cols );
  TIFFSetField( tif, TIFFTAG_IMAGELENGTH, rows );
  TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, bitspersample );
  TIFFSetField( tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( tif, TIFFTAG_COMPRESSION, tiff_compression );
  if ( tiff_compression == COMPRESSION_CCITTFAX3 && tiff_g3options != 0 )
    TIFFSetField( tif, TIFFTAG_GROUP3OPTIONS, tiff_g3options );
  if ( tiff_compression == COMPRESSION_LZW && tiff_predictor != 0 )
    TIFFSetField( tif, TIFFTAG_PREDICTOR, tiff_predictor );
  TIFFSetField( tif, TIFFTAG_PHOTOMETRIC, photometric );
  TIFFSetField( tif, TIFFTAG_FILLORDER, tiff_fillorder );
  //TIFFSetField( tif, TIFFTAG_DOCUMENTNAME, "TIFF Image File");
  TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, 
		"Generated via DRR's pnm-image library\n" );
  TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel );
  TIFFSetField( tif, TIFFTAG_ROWSPERSTRIP, tiff_rowsperstrip );
  /* TIFFSetField( tif, TIFFTAG_STRIPBYTECOUNTS, rows / tiff_rowsperstrip ); */
  TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  
  if ( chv == (colorhist_vector) 0 ) {
    cht = (colorhash_table) 0;
  } else {
    /* Make TIFF colormap. */
    for ( i = 0; i < colors; ++i ) {
      red[i] = (unsigned short) (PPM_GETR( chv[i].color ) * 65535L / maxval);
      grn[i] = (unsigned short) (PPM_GETG( chv[i].color ) * 65535L / maxval);
      blu[i] = (unsigned short) (PPM_GETB( chv[i].color ) * 65535L / maxval);
    }
    TIFFSetField( tif, TIFFTAG_COLORMAP, red, grn, blu );
    
    /* Convert color vector to color hash table, for fast lookup. */
    cht = ppm_colorhisttocolorhash( chv, colors );
    ppm_freecolorhist( chv );
  }

  /* Now write the TIFF data. */
  for ( row = 0; row < rows; ++row ) {
    xel *row_data = array + row*cols;
    xelval *alpha_data = alpha + row*cols;

    if ( !is_grayscale && ! grayscale ) {
      if ( cht == (colorhash_table) 0 ) {
	tP = buf;
	for ( col = 0; col < cols; ++col ) {
	  *tP++ = (unsigned char)(255 * PPM_GETR(row_data[col]) / maxval);
	  *tP++ = (unsigned char)(255 * PPM_GETG(row_data[col]) / maxval);
	  *tP++ = (unsigned char)(255 * PPM_GETB(row_data[col]) / maxval);
	  if (samplesperpixel==4) {
	    *tP++ = (unsigned char)(255 * alpha_data[col] / maxval);
	  }
	}
      } else {
	tP = buf;
	for ( col = 0; col < cols; ++col ) {
	  register int s;
	    
	  s = ppm_lookupcolor( cht, (pixel *)(&row_data[col]) );
	  if ( s == -1 )
	    pm_error("color not found?!?  row=%d col=%d", row, col);
	  *tP++ = (unsigned char) s;
	}
      }
    } else {
      register xelval bigger_maxval;
      register int bitshift;
      register unsigned char byte;
      register xelval s;
      
      bigger_maxval = pm_bitstomaxval( bitspersample );
      bitshift = 8 - bitspersample;
      byte = 0;
      tP = buf;
      for ( col = 0; col < cols; ++col ) {
	s = PPM_GETB(row_data[col]);
	if ( maxval != bigger_maxval )
	  s = (xelval)((long) s * bigger_maxval / maxval);
	byte |= s << bitshift;
	bitshift -= bitspersample;
	if ( bitshift < 0 ) {
	  *tP++ = byte;
	  bitshift = 8 - bitspersample;
	  byte = 0;
	}
      }
      if ( bitshift != 8 - bitspersample )
	*tP++ = byte;
    }

    if ( TIFFWriteScanline( tif, buf, row, 0 ) < 0 )
      pm_error( "failed a scanline write on row %d", row );
  }
  TIFFFlushData( tif );
  TIFFClose( tif );
  //  owns_file = false;

  return rows;
}
