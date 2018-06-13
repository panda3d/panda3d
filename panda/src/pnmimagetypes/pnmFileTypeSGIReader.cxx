/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeSGIReader.cxx
 * @author drose
 * @date 2000-06-17
 */

#include "pnmFileTypeSGI.h"

#ifdef HAVE_SGI_RGB

#include "config_pnmimagetypes.h"
#include "sgi.h"

#include "pnmImage.h"
#include "pnmReader.h"

#include "pnotify.h"

using std::istream;
using std::string;

// Much code in this file is borrowed from Netpbm, specifically sgitopnm.c.

/* sgitopnm.c - read an SGI image and and produce a portable anymap
**
** Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**
** Based on the SGI image description v0.9 by Paul Haeberli (paul@sgi.comp)
** Available via ftp from sgi.com:graphics/SGIIMAGESPEC
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** 29Jan94: first version
** 08Feb94: minor bugfix
*/

/* entry in RLE offset table */
typedef PNMFileTypeSGI::Reader::TabEntry TabEntry;

typedef short       ScanElem;
typedef ScanElem *  ScanLine;

/* prototypes */
static unsigned char get_byte ( istream* f );
static long get_big_long (istream *f);
static short get_big_short (istream *f);
static short get_byte_as_short (istream *f);
static int readerr (istream *f);
static void * xmalloc (int bytes);
#define MALLOC(n, type)     (type *)xmalloc((n) * sizeof(type))
static const char * compression_name (char compr);
static void       read_bytes (istream *ifp, int n, char *buf);
static bool read_header(istream *ifp, Header *head, const string &magic_number);
static TabEntry * read_table (istream *ifp, int tablen);
static void       read_channel (istream *ifp, int xsize, int ysize,
                                     int zsize, int bpc, TabEntry *table,
                                     ScanElem *channel_data, long table_start,
                                     int channel, int row);
static void       rle_decompress (ScanElem *src, long srclen, ScanElem *dest, long destlen);

#define WORSTCOMPR(x)   (2*(x) + 2)

#define MAXVAL_BYTE     255
#define MAXVAL_WORD     65535

// This flag shouldn't really be a static global, but it's a little tricky to
// fix and it doesn't do any harm, since it only controls whether an error
// message is repeated.
static bool eof_err = false;


/**
 *
 */
PNMFileTypeSGI::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  eof_err = false;
  table = nullptr;

  if (!read_magic_number(_file, magic_number, 4)) {
    // No magic number.  No image.
    if (pnmimage_sgi_cat.is_debug()) {
      pnmimage_sgi_cat.debug()
        << "RGB file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  Header head;

  if (!::read_header(file, &head, magic_number)) {
    _is_valid = false;
  }

  long pixmax = (head.bpc == 1) ? MAXVAL_BYTE : MAXVAL_WORD;
  if( pixmax > PNM_MAXMAXVAL ) {
    pnmimage_sgi_cat.error()
      << "Cannot read RGB image with maxval of " << pixmax
      << "--largest allowable maxval is currently " << PNM_MAXMAXVAL << "\n";
    _is_valid = false;
    return;
  }

  _maxval = (xelval)pixmax;

  table_start = file->tellg();
  if( head.storage != STORAGE_VERBATIM )
    table = read_table(file, head.ysize * head.zsize);

  _x_size = head.xsize;
  _y_size = head.ysize;
  _num_channels = std::min((int)head.zsize, 4);
  bpc = head.bpc;

  current_row = _y_size - 1;

  if (_is_valid && pnmimage_sgi_cat.is_debug()) {
    head.name[79] = '\0';  /* just to be safe */
    pnmimage_sgi_cat.debug()
      << "Read RGB image:\n"
      << "  raster size " << head.xsize << " x " << head.ysize
      << ", " << head.zsize << " channels\n"
      << "  compression: " << (int)head.storage << " = "
      << compression_name(head.storage) << "\n"
      << "  image name: " << head.name << "\n"
      << "  bpc: " << (int)head.bpc << " dimension: " << head.dimension << "\n"
      << "  pixmin: " << head.pixmin << " pixmax: " << head.pixmax
      << "  colormap: " << head.colormap << "\n";
  }
}

/**
 *
 */
PNMFileTypeSGI::Reader::
~Reader() {
  if (table != nullptr) {
    free(table);
  }
}

/**
 * Returns true if this particular PNMReader supports a streaming interface to
 * reading the data: that is, it is capable of returning the data one row at a
 * time, via repeated calls to read_row().  Returns false if the only way to
 * read from this file is all at once, via read_data().
 */
bool PNMFileTypeSGI::Reader::
supports_read_row() const {
  return true;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 */
bool PNMFileTypeSGI::Reader::
read_row(xel *row_data, xelval *alpha_data, int x_size, int y_size) {
  if (!is_valid()) {
    return false;
  }
  nassertr(current_row >= 0, false);

  ScanElem *red = (ScanElem *)alloca(x_size * sizeof(ScanElem));
  ScanElem *grn = (ScanElem *)alloca(x_size * sizeof(ScanElem));
  ScanElem *blu = (ScanElem *)alloca(x_size * sizeof(ScanElem));
  ScanElem *alpha = (ScanElem *)alloca(x_size * sizeof(ScanElem));

  read_channel(_file, x_size, y_size, _num_channels, bpc, table, red,
               table_start, 0, current_row);

  if (!is_grayscale()) {
    read_channel(_file, x_size, y_size, _num_channels, bpc, table, grn,
                 table_start, 1, current_row);
    read_channel(_file, x_size, y_size, _num_channels, bpc, table, blu,
                 table_start, 2, current_row);
  }

  if (has_alpha()) {
    read_channel(_file, x_size, y_size, _num_channels, bpc, table, alpha,
                 table_start, _num_channels - 1, current_row);
  }

  for (int x = 0; x < x_size; x++) {
    if (is_grayscale()) {
      PPM_PUTB(row_data[x], (xelval)red[x]);
    } else {
      xelval r, g, b;
      r = (xelval)red[x];
      g = (xelval)grn[x];
      b = (xelval)blu[x];
      PPM_ASSIGN(row_data[x], r, g, b);
    }

    if (has_alpha()) {
      alpha_data[x] = (xelval)alpha[x];
    }
  }
  current_row--;
  return true;
}



static bool
read_header(istream *ifp, Header *head, const string &magic_number) {
    nassertr(magic_number.size() == 4, false);
    head->magic =
      ((unsigned char)magic_number[0] << 8) |
      ((unsigned char)magic_number[1]);
    head->storage   = (unsigned char)magic_number[2];
    head->bpc       = (unsigned char)magic_number[3];
    head->dimension = get_big_short(ifp);
    head->xsize     = get_big_short(ifp);
    head->ysize     = get_big_short(ifp);
    head->zsize     = get_big_short(ifp);
    head->pixmin    = get_big_long(ifp);
    head->pixmax    = get_big_long(ifp);
    read_bytes(ifp, 4, head->dummy1);
    read_bytes(ifp, 80, head->name);
    head->colormap  = get_big_long(ifp);
    read_bytes(ifp, 404, head->dummy2);

    if (head->magic != SGI_MAGIC) {
      pnmimage_sgi_cat.error()
        << "Invalid magic number: not an SGI image file.\n";
      return false;
    }

    if (head->storage != 0 && head->storage != 1) {
      pnmimage_sgi_cat.error()
        << "Unknown compression type.\n";
      return false;
    }

    if (head->bpc < 1 || head->bpc > 2) {
      pnmimage_sgi_cat.error()
        << "Illegal precision value " << head->bpc << " (only 1-2 allowed)\n";
      return false;
    }

    // Actually, some old broken SGI image writers put garbage in this field,
    // so just ignore it.
    /*
    if (head->colormap != CMAP_NORMAL) {
      pnmimage_sgi_cat.error()
        << "Unsupported non-normal pixel data (" << head->colormap << ")\n";
      return false;
    }
    */

    /* adjust ysize/zsize to dimension, just to be sure */

    // On reflection, this is a bad idea.  Ignore the number of dimensions,
    // and take the xsizeysizezsize at face value.  The table was written
    // based on these numbers, after all; you can't just change them
    // arbitrarily.

    /*
    switch( head->dimension ) {
    case 1:
      head->ysize = 1;
      break;
    case 2:
      head->zsize = 1;
      break;
    case 3:
      switch( head->zsize ) {
      case 1:
      case 2:
        head->dimension = 2;
        break;
      case 3:
      case 4:
        break;

      default:
        pnmimage_sgi_cat.warning()
          << "Using only first 4 channels of " << head->zsize
          << "-channel image.\n";
        head->zsize = 4;
        break;
      }
      break;
    default:
      pnmimage_sgi_cat.error()
        << "Illegal dimension value " << head->dimension
        << " (only 1-3 allowed)\n";
      return false;
    }
    */

    return true;
}


static TabEntry *
read_table(istream *ifp, int tablen) {
    TabEntry *table;
    int i;

    table = MALLOC(tablen, TabEntry);

    for( i = 0; i < tablen; i++ ) {
        table[i].start = get_big_long(ifp);
    }
    for( i = 0; i < tablen; i++ ) {
        table[i].length = get_big_long(ifp);
    }

    return table;
}



static void
read_channel(istream *ifp,
             int xsize, int ysize, int, int bpc,
             TabEntry *table,
             ScanElem *channel_data, long table_start,
             int channel, int row) {
    ScanElem *temp = nullptr;
    int sgi_index, i;
    long offset, length;

    short (*func)(istream *);
    func = (bpc==1) ? get_byte_as_short : get_big_short;

    if ( table ) {
      temp = (ScanElem *)alloca(WORSTCOMPR(xsize) * sizeof(ScanElem));
    }

    sgi_index = channel * ysize + row;
    if( table ) {
      offset = table[sgi_index].start;
      length = table[sgi_index].length;
      if( bpc == 2 )
        length /= 2;   /* doc says length is in bytes, we are reading words */
      if(!ifp->seekg(offset))
        pm_error("seek error for offset %ld", offset);

      nassertv(length <= WORSTCOMPR(xsize));
      for( i = 0; i < length; i++ )
        temp[i] = (*func)(ifp);

      rle_decompress(temp, length, channel_data, xsize);
    }
    else {
      offset = sgi_index * xsize + table_start;
      if(!ifp->seekg(offset))
        pm_error("seek error for offset %ld", offset);
      for( i = 0; i < xsize; i++ )
        channel_data[i] = (*func)(ifp);
    }
}



static void
rle_decompress(ScanElem *src,
               long srcleft,
               ScanElem *dest,
               long destleft) {
    int count;
    unsigned char el;

    while( srcleft ) {
        el = (unsigned char)(*src++ & 0xff);
        --srcleft;
        count = (int)(el & 0x7f);

        if( count == 0 )
            return;
        if( destleft < count )
            pm_error("RLE error: too much input data (space left %d, need %d)", destleft, count);
        destleft -= count;
        if( el & 0x80 ) {
            if( srcleft < count )
                pm_error("RLE error: not enough data for literal run (data left %d, need %d)", srcleft, count);
            srcleft -= count;
            while( count-- )
                *dest++ = *src++;
        }
        else {
            if( srcleft == 0 )
                pm_error("RLE error: not enough data for replicate run");
            while( count-- )
                *dest++ = *src;
            ++src;
            --srcleft;
        }
    }
    pm_error("RLE error: no terminating 0-byte");
}


/* basic I/O functions, taken from ilbmtoppm.c */

static short
get_big_short(istream *ifp) {
    short s;

    if( pm_readbigshort(ifp, &s) == -1 )
        s = readerr(ifp);

    return s;
}

static long
get_big_long(istream *ifp) {
    long l;

    if( pm_readbiglong(ifp, &l) == -1 )
        l = readerr(ifp);

    return l;
}

static unsigned char
get_byte(istream *ifp) {
    int i;

    i = ifp->get();
    if( i == EOF )
        i = readerr(ifp);

    return (unsigned char) i;
}


static int
readerr(istream *f) {
  if (!eof_err) {
    if (!f->eof()) {
      pnmimage_sgi_cat.warning()
        << "Read error on file.\n";
    } else {
      pnmimage_sgi_cat.warning()
        << "Premature EOF on file.\n";
    }
    eof_err = true;
  }

  return 0;
}


static void
read_bytes(istream *ifp,
           int n,
           char *buf) {
    int r;

    ifp->read(buf, n);
    r = ifp->gcount();
    if( r != n ) {
      readerr(ifp);
      memset(buf+r, 0, n-r);
    }
    Thread::consider_yield();
}


static short
get_byte_as_short(istream *ifp) {
    return (short)get_byte(ifp);
}


static void *
xmalloc(int bytes) {
    void *mem;

    if( bytes == 0 )
        return nullptr;

    mem = malloc(bytes);
    if( mem == nullptr )
        pm_error("out of memory allocating %d bytes", bytes);
    return mem;
}

static const char *
compression_name(char compr) {
    switch( compr ) {
        case STORAGE_VERBATIM:
            return "none";
        case STORAGE_RLE:
            return "RLE";
        default:
            return "unknown";
    }
}

#endif  // HAVE_SGI_RGB
