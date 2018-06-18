/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeTGA.cxx
 * @author drose
 * @date 2001-04-27
 */

// Much code in this file is borrowed from Netpbm, specifically tgatoppm.c and
// ppmtotga.c.

/* tgatoppm.c - read a TrueVision Targa file and write a portable pixmap
**
** Partially based on tga2rast, version 1.0, by Ian MacPhedran.
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/* ppmtotga.c - read a portable pixmap and produce a TrueVision Targa file
**
** Copyright (C) 1989, 1991 by Mark Shand and Jef Poskanzer
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pnmFileTypeTGA.h"

#ifdef HAVE_TGA

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"
#include "pnmimage_base.h"

#include <string.h>

using std::istream;
using std::ostream;
using std::string;

static const char * const extensions_tga[] = {
  "tga"
};
static const int num_extensions_tga = sizeof(extensions_tga) / sizeof(const char *);

TypeHandle PNMFileTypeTGA::_type_handle;


/* Header definition. */
struct ImageHeader {
    unsigned char IDLength;             /* length of Identifier String */
    unsigned char CoMapType;            /* 0 = no map */
    unsigned char ImgType;              /* image type (see below for values) */
    unsigned char Index_lo, Index_hi;   /* index of first color map entry */
    unsigned char Length_lo, Length_hi; /* number of entries in color map */
    unsigned char CoSize;               /* size of color map entry (15,16,24,32) */
    unsigned char X_org_lo, X_org_hi;   /* x origin of image */
    unsigned char Y_org_lo, Y_org_hi;   /* y origin of image */
    unsigned char Width_lo, Width_hi;   /* width of image */
    unsigned char Height_lo, Height_hi; /* height of image */
    unsigned char PixelSize;            /* pixel size (8,16,24,32) */
    unsigned char AttBits;              /* 4 bits, number of attribute bits per pixel */
    unsigned char Rsrvd;                /* 1 bit, reserved */
    unsigned char OrgBit;               /* 1 bit, origin: 0=lower left, 1=upper left */
    unsigned char IntrLve;              /* 2 bits, interleaving flag */
    };

typedef char ImageIDField[256];

/* Max number of colors allowed for colormapped output. */
#define TGA_MAXCOLORS 257

/* Definitions for image types. */
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

/* Definitions for interleave flag. */
#define TGA_IL_None 0
#define TGA_IL_Two 1
#define TGA_IL_Four 2

/**
 *
 */
PNMFileTypeTGA::
PNMFileTypeTGA() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeTGA::
get_name() const {
  return "Targa";
}

/**
 * Returns the number of different possible filename extensions_tga associated
 * with this particular file type.
 */
int PNMFileTypeTGA::
get_num_extensions() const {
  return num_extensions_tga;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeTGA::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_tga, string());
  return extensions_tga[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeTGA::
get_suggested_extension() const {
  return "tga";
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeTGA::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeTGA::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


/**
 *
 */
PNMFileTypeTGA::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  tga_head = new ImageHeader;
  RLE_count = 0;
  RLE_flag = 0;
  ColorMap = nullptr;
  AlphaMap = nullptr;

  Red = 0;
  Grn = 0;
  Blu = 0;
  Alpha = 0;
  l = 0;

    /* Read the Targa file header. */
    readtga( file, tga_head, magic_number );
    /*
        {
        pm_message( "IDLength = %d", (int) tga_head->IDLength );
        pm_message( "CoMapType = %d", (int) tga_head->CoMapType );
        pm_message( "ImgType = %d", (int) tga_head->ImgType );
        pm_message( "Index_lo = %d", (int) tga_head->Index_lo );
        pm_message( "Index_hi = %d", (int) tga_head->Index_hi );
        pm_message( "Length_lo = %d", (int) tga_head->Length_lo );
        pm_message( "Length_hi = %d", (int) tga_head->Length_hi );
        pm_message( "CoSize = %d", (int) tga_head->CoSize );
        pm_message( "X_org_lo = %d", (int) tga_head->X_org_lo );
        pm_message( "X_org_hi = %d", (int) tga_head->X_org_hi );
        pm_message( "Y_org_lo = %d", (int) tga_head->Y_org_lo );
        pm_message( "Y_org_hi = %d", (int) tga_head->Y_org_hi );
        pm_message( "Width_lo = %d", (int) tga_head->Width_lo );
        pm_message( "Width_hi = %d", (int) tga_head->Width_hi );
        pm_message( "Height_lo = %d", (int) tga_head->Height_lo );
        pm_message( "Height_hi = %d", (int) tga_head->Height_hi );
        pm_message( "PixelSize = %d", (int) tga_head->PixelSize );
        pm_message( "AttBits = %d", (int) tga_head->AttBits );
        pm_message( "Rsrvd = %d", (int) tga_head->Rsrvd );
        pm_message( "OrgBit = %d", (int) tga_head->OrgBit );
        pm_message( "IntrLve = %d", (int) tga_head->IntrLve );
        }
    */
    rows = ( (int) tga_head->Height_lo ) + ( (int) tga_head->Height_hi ) * 256;
    cols = ( (int) tga_head->Width_lo ) + ( (int) tga_head->Width_hi ) * 256;

    switch ( tga_head->ImgType )
        {
        case TGA_Map:
        case TGA_RGB:
        case TGA_Mono:
        case TGA_RLEMap:
        case TGA_RLERGB:
        case TGA_RLEMono:
        break;

        default:
        pm_error( "unknown Targa image type %d", tga_head->ImgType );
        }

    int size;

    if ( tga_head->ImgType == TGA_Map ||
         tga_head->ImgType == TGA_RLEMap ||
         tga_head->ImgType == TGA_CompMap ||
         tga_head->ImgType == TGA_CompMap4 )
        { /* Color-mapped image */
        if ( tga_head->CoMapType != 1 )
            pm_error(
                "mapped image (type %d) with color map type != 1",
                tga_head->ImgType );
        mapped = 1;
        /* Figure maxval from CoSize. */
        size = tga_head->CoSize;
        }
    else
        { /* Not colormap, so figure maxval from PixelSize. */
        mapped = 0;
        size = tga_head->PixelSize;
        }

    switch ( size ) {
    case 8:
      _num_channels = 1;
      _maxval = 255;
      break;

    case 24:
      _num_channels = 3;
      _maxval = 255;
      break;

    case 32:
      _num_channels = 4;
      _maxval = 255;
      break;

    case 15:
    case 16:
      _num_channels = 3;
      _maxval = 31;
      break;

    default:
      pm_error("unknown pixel size - %d", size );
    }

    /* If required, read the color map information. */
    if ( tga_head->CoMapType != 0 )
        {
        unsigned int temp1, temp2;
        temp1 = tga_head->Index_lo + tga_head->Index_hi * 256;
        temp2 = tga_head->Length_lo + tga_head->Length_hi * 256;
        int num_colors = temp1 + temp2 + 1;
        nassertv(ColorMap == nullptr && AlphaMap == nullptr);
        ColorMap = (pixel *)PANDA_MALLOC_ARRAY(num_colors * sizeof(pixel));
        AlphaMap = (gray *)PANDA_MALLOC_ARRAY(num_colors * sizeof(gray));
        for ( unsigned int i = temp1; i < ( temp1 + temp2 ); ++i )
            get_map_entry( file, &ColorMap[i], (int) tga_head->CoSize,
                       &AlphaMap[i]);
        }

    /* Check run-length encoding. */
    if ( tga_head->ImgType == TGA_RLEMap ||
         tga_head->ImgType == TGA_RLERGB ||
         tga_head->ImgType == TGA_RLEMono )
        rlencoded = 1;
    else
        rlencoded = 0;

    _x_size = cols;
    _y_size = rows;
    // _num_channels = 3;

}

/**
 *
 */
PNMFileTypeTGA::Reader::
~Reader() {
  delete tga_head;
  if (ColorMap != nullptr) {
    PANDA_FREE_ARRAY(ColorMap);
  }
  if (AlphaMap != nullptr) {
    PANDA_FREE_ARRAY(AlphaMap);
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
int PNMFileTypeTGA::Reader::
read_data(xel *array, xelval *alpha) {
    int truerow = 0;
    int baserow = 0;
    for ( int row = 0; row < rows; ++row )
        {
        int realrow = truerow;
        if ( tga_head->OrgBit == 0 )
            realrow = rows - realrow - 1;

        for ( int col = 0; col < cols; ++col )
            get_pixel( _file, &(array[realrow * cols + col]),
                       (int) tga_head->PixelSize,
                       &(alpha[realrow * cols + col]) );
        if ( tga_head->IntrLve == TGA_IL_Four )
            truerow += 4;
        else if ( tga_head->IntrLve == TGA_IL_Two )
            truerow += 2;
        else
            ++truerow;
        if ( truerow >= rows )
            truerow = ++baserow;
        }

  return rows;
}

/**
 *
 */
PNMFileTypeTGA::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
  tgaHeader = new ImageHeader;
  chv = nullptr;
  cht = nullptr;
  runlength = nullptr;
}

/**
 *
 */
PNMFileTypeTGA::Writer::
~Writer() {
  delete tgaHeader;

  if (chv != nullptr) {
    ppm_freecolorhist(chv);
  }
  if (cht != nullptr) {
    ppm_freecolorhash(cht);
  }
  if (runlength != nullptr) {
    pm_freerow((char *)runlength);
  }
}

/**
 * Writes out an entire image all at once, including the header, based on the
 * image data stored in the given _x_size * _y_size array and alpha pointers.
 * (If the image type has no alpha channel, alpha is ignored.) Returns the
 * number of rows correctly written.
 *
 * It is the user's responsibility to fill in the header data via calls to
 * set_x_size(), set_num_channels(), etc., or copy_header_from(), before
 * calling write_data().
 *
 * It is important to delete the PNMWriter class after successfully writing
 * the data.  Failing to do this may result in some data not getting flushed!
 *
 * Derived classes need not override this if they instead provide
 * supports_streaming() and write_row(), below.
 */
int PNMFileTypeTGA::Writer::
write_data(xel *array, xelval *) {
  // We don't presently support writing 4-channel tga files (since ppmtotga
  // doesn't support this).
  rle_flag = tga_rle;

  int row, col;
  int i;
  pixel* pP;

  cols = _x_size;
  rows = _y_size;

  if (is_grayscale() && tga_grayscale) {
    // We allow grayscale TGA files, and this is a grayscale image, so...
    tgaHeader->ImgType = TGA_Mono;

    // There's no real point in colormapping a grayscale image.
    pnmimage_tga_cat.info()
      << "writing grayscale.\n";

  } else {
    // This will be an RGB image.
    tgaHeader->ImgType = TGA_RGB;

    if (tga_colormap) {
      // It may even be colormapped if there are few enough colors.
      pnmimage_tga_cat.info()
        << "computing colormap...\n";
      chv = ppm_computecolorhist(&array, cols * rows, 1, TGA_MAXCOLORS, &ncolors );
      if ( chv == nullptr ) {
        pnmimage_tga_cat.info()
          << "too many colors, writing RGB.\n";
      } else {
        pnmimage_tga_cat.info()
          << ncolors << " colors found.\n";
        tgaHeader->ImgType = TGA_Map;
      }
    }
  }

  if ( rle_flag )
    {
      switch ( tgaHeader->ImgType )
        {
        case TGA_Mono:
          tgaHeader->ImgType = TGA_RLEMono;
          break;
        case TGA_Map:
          tgaHeader->ImgType = TGA_RLEMap;
          break;
        case TGA_RGB:
          tgaHeader->ImgType = TGA_RLERGB;
          break;
        default:
          pm_error( "can't happen" );
        }
      runlength = (int*) pm_allocrow( cols, sizeof(int) );
    }

  tgaHeader->IDLength = 0;
  tgaHeader->Index_lo = 0;
  tgaHeader->Index_hi = 0;
  if ( tgaHeader->ImgType == TGA_Map || tgaHeader->ImgType == TGA_RLEMap )
    {
      /* Make a hash table for fast color lookup. */
      cht = ppm_colorhisttocolorhash( chv, ncolors );

      tgaHeader->CoMapType = 1;
      tgaHeader->Length_lo = ncolors % 256;
      tgaHeader->Length_hi = ncolors / 256;
      tgaHeader->CoSize = 24;
    }
  else
    {
      tgaHeader->CoMapType = 0;
      tgaHeader->Length_lo = 0;
      tgaHeader->Length_hi = 0;
      tgaHeader->CoSize = 0;
    }
  if ( tgaHeader->ImgType == TGA_RGB || tgaHeader->ImgType == TGA_RLERGB )
    tgaHeader->PixelSize = 24;
  else
    tgaHeader->PixelSize = 8;
  tgaHeader->X_org_lo = tgaHeader->X_org_hi = 0;
  tgaHeader->Y_org_lo = tgaHeader->Y_org_hi = 0;
  tgaHeader->Width_lo = cols % 256;
  tgaHeader->Width_hi = cols / 256;
  tgaHeader->Height_lo = rows % 256;
  tgaHeader->Height_hi = rows / 256;
  tgaHeader->AttBits = 0;
  tgaHeader->Rsrvd = 0;
  tgaHeader->IntrLve = 0;
  tgaHeader->OrgBit = 0;

    /* Write out the Targa header. */
  writetga( tgaHeader, nullptr );

  if ( tgaHeader->ImgType == TGA_Map || tgaHeader->ImgType == TGA_RLEMap )
    {
      /* Write out the Targa colormap. */
      for ( i = 0; i < ncolors; ++i )
        put_map_entry( &chv[i].color, tgaHeader->CoSize, _maxval );
    }

  /* Write out the pixels */
  for ( row = 0; row < rows; ++row )
    {
      int realrow = row;
      if ( tgaHeader->OrgBit == 0 )
        realrow = rows - realrow - 1;
      if ( rle_flag )
        {
          compute_runlengths( cols, &array[realrow * cols], runlength );
          for ( col = 0; col < cols; )
            {
              if ( runlength[col] > 0 )
                {
                  _file->put( 0x80 + runlength[col] - 1 );
                  put_pixel(&(array[realrow * cols + col]),
                            tgaHeader->ImgType, _maxval, cht );
                  col += runlength[col];
                }
              else if ( runlength[col] < 0 )
                {
                  _file->put( -runlength[col] - 1 );
                  for ( i = 0; i < -runlength[col]; ++i )
                    put_pixel(&(array[realrow * cols + (col + i)]),
                              tgaHeader->ImgType, _maxval, cht );
                  col += -runlength[col];
                }
              else
                pm_error( "can't happen" );
            }
        }
      else
        {
          for ( col = 0, pP = &array[realrow * cols]; col < cols; ++col, ++pP )
            put_pixel( pP, tgaHeader->ImgType, _maxval, cht );
        }
    }

  return rows;
}



/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeTGA::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeTGA);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeTGA::
make_PNMFileTypeTGA(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

void PNMFileTypeTGA::Reader::
readtga( istream *ifp, struct ImageHeader *tgaP, const string &magic_number ) {
    unsigned char flags;
    ImageIDField junk;

    if (magic_number.length() > 0) {
      tgaP->IDLength = (unsigned char)magic_number[0];
    } else {
      tgaP->IDLength = getbyte( ifp );
    }
    if (magic_number.length() > 1) {
      tgaP->CoMapType = (unsigned char)magic_number[1];
    } else {
      tgaP->CoMapType = getbyte( ifp );
    }
    if (magic_number.length() > 2) {
      tgaP->ImgType = (unsigned char)magic_number[2];
    } else {
      tgaP->ImgType = getbyte( ifp );
    }
    if (magic_number.length() > 3) {
      tgaP->Index_lo = (unsigned char)magic_number[3];
    } else {
      tgaP->Index_lo = getbyte( ifp );
    }
    tgaP->Index_hi = getbyte( ifp );
    tgaP->Length_lo = getbyte( ifp );
    tgaP->Length_hi = getbyte( ifp );
    tgaP->CoSize = getbyte( ifp );
    tgaP->X_org_lo = getbyte( ifp );
    tgaP->X_org_hi = getbyte( ifp );
    tgaP->Y_org_lo = getbyte( ifp );
    tgaP->Y_org_hi = getbyte( ifp );
    tgaP->Width_lo = getbyte( ifp );
    tgaP->Width_hi = getbyte( ifp );
    tgaP->Height_lo = getbyte( ifp );
    tgaP->Height_hi = getbyte( ifp );
    tgaP->PixelSize = getbyte( ifp );
    flags = getbyte( ifp );
    tgaP->AttBits = flags & 0xf;
    tgaP->Rsrvd = ( flags & 0x10 ) >> 4;
    tgaP->OrgBit = ( flags & 0x20 ) >> 5;
    tgaP->IntrLve = ( flags & 0xc0 ) >> 6;

    if ( tgaP->IDLength != 0 )
        ifp->read(junk, (int) tgaP->IDLength);
    }

void PNMFileTypeTGA::Reader::
get_map_entry( istream *ifp, pixel *Value, int Size, gray *Alpha ) {
    unsigned char j, k;
    unsigned char r = 0, g = 0, b = 0, a = 0;

    /* Read appropriate number of bytes, break into rgb & put in map. */
    switch ( Size )
      {
      case 8:                         /* Grey scale, read and triplicate. */
        r = g = b = getbyte( ifp );
        a = 0;
        break;

      case 16:                        /* 5 bits each of red green and blue. */
      case 15:                        /* Watch for byte order. */
        j = getbyte( ifp );
        k = getbyte( ifp );
        r = ( k & 0x7C ) >> 2;
        g = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
        b = j & 0x1F;
        a = 0;
        break;

      case 32:            /* 8 bits each of blue, green, red, and alpha */
      case 24:                        /* 8 bits each of blue green and red. */
        b = getbyte( ifp );
        g = getbyte( ifp );
        r = getbyte( ifp );
        if ( Size == 32 )
          a = getbyte( ifp );
        else
          a = 0;
        break;

      default:
        pm_error( "unknown colormap pixel size (#2) - %d", Size );
      }
    PPM_ASSIGN( *Value, r, g, b );
    *Alpha = a;
}



void PNMFileTypeTGA::Reader::
get_pixel( istream *ifp, pixel *dest, int Size, gray *alpha_p) {
    unsigned char j, k;

    /* Check if run length encoded. */
    if ( rlencoded )
        {
        if ( RLE_count == 0 )
            { /* Have to restart run. */
            unsigned char i;
            i = getbyte( ifp );
            RLE_flag = ( i & 0x80 );
            if ( RLE_flag == 0 )
                /* Stream of unencoded pixels. */
                RLE_count = i + 1;
            else
                /* Single pixel replicated. */
                RLE_count = i - 127;
            /* Decrement count & get pixel. */
            --RLE_count;
            }
        else
            { /* Have already read count & (at least) first pixel. */
            --RLE_count;
            if ( RLE_flag != 0 )
                /* Replicated pixels. */
                goto PixEncode;
            }
        }
    /* Read appropriate number of bytes, break into RGB. */
    switch ( Size )
        {
        case 8:                         /* Grey scale, read and triplicate. */
        Red = Grn = Blu = l = getbyte( ifp );
    Alpha = 0;
        break;

        case 16:                        /* 5 bits each of red green and blue. */
        case 15:                        /* Watch byte order. */
        j = getbyte( ifp );
        k = getbyte( ifp );
        l = ( (unsigned int) k << 8 ) + j;
        Red = ( k & 0x7C ) >> 2;
        Grn = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
        Blu = j & 0x1F;
    Alpha = 0;
        break;

        case 32:            /* 8 bits each of blue, green, red, and alpha */
        case 24:                        /* 8 bits each of blue, green, and red. */
        Blu = getbyte( ifp );
        Grn = getbyte( ifp );
        Red = getbyte( ifp );
        if ( Size == 32 )
            Alpha = getbyte( ifp );
        else
            Alpha = 0;
        l = 0;
        break;

        default:
        pm_error( "unknown pixel size (#2) - %d", Size );
        }

PixEncode:
    if ( mapped ) {
        *dest = ColorMap[l];
        if (has_alpha()) {
          *alpha_p = AlphaMap[l];
        }
    } else {
        PPM_ASSIGN( *dest, Red, Grn, Blu );
        if (has_alpha()) {
          *alpha_p = Alpha;
        }
    }
    }


unsigned char PNMFileTypeTGA::Reader::
getbyte( istream *ifp ) {
    unsigned char c;

    c = ifp->get();
    if (ifp->fail() || ifp->eof())
        pm_error( "EOF / read error" );

    return c;
    }

void PNMFileTypeTGA::Writer::
writetga( struct ImageHeader *tgaP, char *id )
    {
    unsigned char flags;

    _file->put( tgaP->IDLength );
    _file->put( tgaP->CoMapType );
    _file->put( tgaP->ImgType );
    _file->put( tgaP->Index_lo );
    _file->put( tgaP->Index_hi );
    _file->put( tgaP->Length_lo );
    _file->put( tgaP->Length_hi );
    _file->put( tgaP->CoSize );
    _file->put( tgaP->X_org_lo );
    _file->put( tgaP->X_org_hi );
    _file->put( tgaP->Y_org_lo );
    _file->put( tgaP->Y_org_hi );
    _file->put( tgaP->Width_lo );
    _file->put( tgaP->Width_hi );
    _file->put( tgaP->Height_lo );
    _file->put( tgaP->Height_hi );
    _file->put( tgaP->PixelSize );
    flags = ( tgaP->AttBits & 0xf ) | ( ( tgaP->Rsrvd & 0x1 ) << 4 ) |
            ( ( tgaP->OrgBit & 0x1 ) << 5 ) | ( ( tgaP->OrgBit & 0x3 ) << 6 );
    _file->put( flags );
    if ( tgaP->IDLength )
        _file->write( id, (int) tgaP->IDLength );
    }

void PNMFileTypeTGA::Writer::
put_map_entry( pixel* valueP, int size, pixval maxval )
    {
    int j;
    pixel p;

    switch ( size )
        {
        case 8:                         /* Grey scale. */
        put_mono( valueP, maxval );
        break;

        case 16:                        /* 5 bits each of red green and blue. */
        case 15:                        /* Watch for byte order. */
        PPM_DEPTH( p, *valueP, maxval, 31 );
        j = (int) PPM_GETB( p ) | ( (int) PPM_GETG( p ) << 5 ) |
            ( (int) PPM_GETR( p ) << 10 );
        _file->put( j % 256 );
        _file->put( j / 256 );
        break;

        case 32:
        case 24:                        /* 8 bits each of blue green and red. */
        put_rgb( valueP, maxval );
        break;

        default:
        pm_error( "unknown colormap pixel size (#2) - %d", size );
        }
    }

void PNMFileTypeTGA::Writer::
compute_runlengths( int cols, pixel *pixelrow, int *runlength )
    {
    int col, start;

    /* Initialize all run lengths to 0.  (This is just an error check.) */
    for ( col = 0; col < cols; ++col )
        runlength[col] = 0;

    /* Find runs of identical pixels. */
    for ( col = 0; col < cols; )
        {
        start = col;
        do {
            ++col;
            }
        while ( col < cols &&
                col - start < 128 &&
                PPM_EQUAL( pixelrow[col], pixelrow[start] ) );
        runlength[start] = col - start;
        }

    /* Now look for runs of length-1 runs, and turn them into negative runs. */
    for ( col = 0; col < cols; )
        {
        if ( runlength[col] == 1 )
            {
            start = col;
            while ( col < cols &&
                    col - start < 128 &&
                    runlength[col] == 1 )
                {
                runlength[col] = 0;
                ++col;
                }
            runlength[start] = - ( col - start );
            }
        else
            col += runlength[col];
        }
    }

void PNMFileTypeTGA::Writer::
put_pixel( pixel* pP, int imgtype, pixval maxval, colorhash_table cht )
    {
    switch ( imgtype )
        {
        case TGA_Mono:
        case TGA_RLEMono:
        put_mono( pP, maxval );
        break;
        case TGA_Map:
        case TGA_RLEMap:
        put_map( pP, cht );
        break;
        case TGA_RGB:
        case TGA_RLERGB:
        put_rgb( pP, maxval );
        break;
        default:
        pm_error( "can't happen" );
        }
    }

void PNMFileTypeTGA::Writer::
put_mono( pixel* pP, pixval maxval )
    {
    pixel p;
    PPM_DEPTH( p, *pP, maxval, (pixval) 255 );
    _file->put( PPM_GETB( p ) );
    }

void PNMFileTypeTGA::Writer::
put_map( pixel *pP, colorhash_table cht )
    {
    _file->put( ppm_lookupcolor( cht, pP ) );
    }

void PNMFileTypeTGA::Writer::
put_rgb( pixel* pP, pixval maxval ) {
  pixel p;
  PPM_DEPTH( p, *pP, maxval, (pixval) 255 );
  _file->put( PPM_GETB( p ) );
  if (is_grayscale()) {
    _file->put( PPM_GETB( p ) );
    _file->put( PPM_GETB( p ) );
  } else {
    _file->put( PPM_GETG( p ) );
    _file->put( PPM_GETR( p ) );
  }
}

#endif  // HAVE_TGA
