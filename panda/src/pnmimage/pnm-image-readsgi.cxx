// pnm-image-readsgi.cc
//
// PNMImage::ReadSGI() and supporting functions.



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

#ifndef PENV_WIN32 
  #include <alloca.h>
#endif

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"
#include "../pnm/sgi.h"
#include <notify.h>

/* entry in RLE offset table */
typedef PNMReaderSGI::TabEntry TabEntry;

typedef short       ScanElem;
typedef ScanElem *  ScanLine;

/* prototypes */
static unsigned char get_byte ( FILE* f );
static long get_big_long (FILE *f);
static short get_big_short (FILE *f);
static short get_byte_as_short (FILE *f);
static int readerr (FILE *f);
static void * xmalloc (int bytes);
#define MALLOC(n, type)     (type *)xmalloc((n) * sizeof(type))
//static char * compression_name (char compr);
static void       read_bytes (FILE *ifp, int n, char *buf);
static void read_header(FILE *ifp, Header *head, int already_read_magic);
static TabEntry * read_table (FILE *ifp, int tablen);
static void       read_channel (FILE *ifp, int xsize, int ysize, 
				     int zsize, int bpc, TabEntry *table,
				     ScanElem *channel_data, long table_start,
				     int channel, int row);
static void       rle_decompress (ScanElem *src, long srclen, ScanElem *dest, long destlen);

#define WORSTCOMPR(x)   (2*(x) + 2)

#define MAXVAL_BYTE     255
#define MAXVAL_WORD     65535

static int eof_err = false;

PNMReaderSGI::
PNMReaderSGI(FILE *file, int already_read_magic) : PNMReader(file) {
  eof_err = false;

  table = NULL;
  Header head;

  read_header(file, &head, already_read_magic);

  long pixmax = (head.bpc == 1) ? MAXVAL_BYTE : MAXVAL_WORD;
  if( pixmax > PNM_MAXMAXVAL )
    pm_error("pixel values too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS");

  maxval = (xelval)pixmax;
  
  table_start = ftell(file);
  if( head.storage != STORAGE_VERBATIM )
    table = read_table(file, head.ysize * head.zsize);

  cols = head.xsize;
  rows = head.ysize;
  zsize = head.zsize;
  bpc = head.bpc;

  switch (zsize) {
  case 1:
    color_type = PNMImage::Grayscale;
    break;

  case 2:
    color_type = PNMImage::TwoChannel;
    break;

  case 3:
    color_type = PNMImage::Color;
    break;
    
  case 4:
    color_type = PNMImage::FourChannel;
    break;
    
  default:
    pm_error("Can't happen.");
  }

  current_row = rows-1;
}

bool PNMReaderSGI::
ReadRow(xel *row_data, xelval *alpha_data) {
  nassertr(current_row >= 0, false);

  int is_grayscale = PNMImage::IsGrayscale(color_type);
  int has_alpha = PNMImage::HasAlpha(color_type);
  int col;

  ScanElem *red = (ScanElem *)alloca(cols * sizeof(ScanElem));
  ScanElem *grn = (ScanElem *)alloca(cols * sizeof(ScanElem));
  ScanElem *blu = (ScanElem *)alloca(cols * sizeof(ScanElem));
  ScanElem *alpha = (ScanElem *)alloca(cols * sizeof(ScanElem));

  read_channel(file, cols, rows, zsize, bpc, table, red, table_start, 
	       0, current_row);

  if (!is_grayscale) {
    read_channel(file, cols, rows, zsize, bpc, table, grn, table_start,
		 1, current_row);
    read_channel(file, cols, rows, zsize, bpc, table, blu, table_start,
		 2, current_row);
  }

  if (has_alpha) {
    read_channel(file, cols, rows, zsize, bpc, table, alpha, table_start,
		 zsize-1, current_row);
  }

  for ( col = 0; col < cols; col++ ) {
    if (is_grayscale) {
      PPM_PUTB(row_data[col], (xelval)red[col]);
    } else {
      xelval r, g, b;
      r = (xelval)red[col];
      g = (xelval)grn[col];
      b = (xelval)blu[col];
      PPM_ASSIGN(row_data[col], r, g, b);
    }
    
    if (has_alpha) {
      alpha_data[col] = (xelval)alpha[col];
    }
  }
  current_row--;
  return true;
}

PNMReaderSGI::
~PNMReaderSGI() {
  if (table != NULL) {
    free(table);
  }
}



static void
read_header(FILE *ifp, Header *head, int already_read_magic) {
    if (already_read_magic >= 0) {
      head->magic = (short)already_read_magic;
    } else {
      head->magic     = get_big_short(ifp);
    }

    head->storage   = get_byte(ifp);
    head->bpc       = get_byte(ifp);
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

    if( head->magic != SGI_MAGIC )
        pm_error("bad magic number - not an SGI image");
    if( head->storage != 0 && head->storage != 1 )
        pm_error("unknown compression type");
    if( head->bpc < 1 || head->bpc > 2 )
        pm_error("illegal precision value %d (only 1-2 allowed)", head->bpc );
    if( head->colormap != CMAP_NORMAL )
        pm_message("unsupported non-normal pixel data (%d)",
		   head->colormap);

    /* adjust ysize/zsize to dimension, just to be sure */
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
	pm_message("%d-channel image, using only first 4 channels", head->zsize);
	head->zsize = 4;
	break;
      }
      break;
    default:
      pm_error("illegal dimension value %d (only 1-3 allowed)", head->dimension);
    }

#ifdef DEBUG
    fprintf(stderr, "raster size %dx%d, %d channels\n", head->xsize, head->ysize, head->zsize);
    fprintf(stderr, "compression: %d = %s\n", head->storage, compression_name(head->storage));
    head->name[79] = '\0';  /* just to be safe */
    fprintf(stderr, "Image name: \"%s\"\n", head->name);
    fprintf(stderr, "bpc: %d    dimension: %d    zsize: %d\n", head->bpc, head->dimension, head->zsize);
    fprintf(stderr, "pixmin: %ld    pixmax: %ld    colormap: %ld\n", head->pixmin, head->pixmax, head->colormap);
#endif
}


static TabEntry *
read_table(FILE *ifp, int tablen) {
    TabEntry *table;
    int i;

    table = MALLOC(tablen, TabEntry);

    for( i = 0; i < tablen; i++ )
        table[i].start = get_big_long(ifp);
    for( i = 0; i < tablen; i++ )
        table[i].length = get_big_long(ifp);

    return table;
}



static void
read_channel(FILE *ifp,
	     int xsize, int ysize, int, int bpc,
	     TabEntry *table,
	     ScanElem *channel_data, long table_start,
	     int channel, int row) {
    ScanElem *temp;
    int sgi_index, i;
    long offset, length;

    short (*func)(FILE *);
    func = (bpc==1) ? get_byte_as_short : get_big_short;

    if ( table ) 
      temp = (ScanElem *)alloca(WORSTCOMPR(xsize) * sizeof(ScanElem));

    sgi_index = channel * ysize + row;
    if( table ) {
      offset = table[sgi_index].start;
      length = table[sgi_index].length;
      if( bpc == 2 )
	length /= 2;   /* doc says length is in bytes, we are reading words */
      if( fseek(ifp, offset, SEEK_SET) != 0 )
	pm_error("seek error for offset %ld", offset);
      
      for( i = 0; i < length; i++ )
	temp[i] = (*func)(ifp);
      
      rle_decompress(temp, length, channel_data, xsize);
    }
    else {
      offset = sgi_index * xsize + table_start;
      if( fseek(ifp, offset, SEEK_SET) != 0 )
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
get_big_short(FILE *ifp) {
    short s;

    if( pm_readbigshort(ifp, &s) == -1 )
        s = readerr(ifp);

    return s;
}

static long
get_big_long(FILE *ifp) {
    long l;

    if( pm_readbiglong(ifp, &l) == -1 )
        l = readerr(ifp);

    return l;
}

static unsigned char
get_byte(FILE *ifp) {
    int i;

    i = getc(ifp);
    if( i == EOF )
        i = readerr(ifp);

    return (unsigned char) i;
}


static int
readerr(FILE *f) {
  // This will return only if the error is EOF.
  if( ferror(f) )
    pm_error("read error");

  static FILE *last_eof = NULL;

  if (!eof_err) {
    fprintf(stderr, "Warning: premature EOF on file\n");
    eof_err = true;
  }

  return 0;
}


static void
read_bytes(FILE *ifp,
	   int n,
	   char *buf) {
    int r;

    r = fread((void *)buf, 1, n, ifp);
    if( r != n ) {
      readerr(ifp);
      memset(buf+r, 0, n-r);
    }
}


static short
get_byte_as_short(FILE *ifp) {
    return (short)get_byte(ifp);
}


static void *
xmalloc(int bytes) {
    void *mem;

    if( bytes == 0 )
        return NULL;

    mem = malloc(bytes);
    if( mem == NULL )
        pm_error("out of memory allocating %d bytes", bytes);
    return mem;
}

/*
static char *
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
*/
