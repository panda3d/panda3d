// pnm-image-alias.cc
//
// PNMImage::ReadAlias()
// PNMImage::WriteAlias()

#include <pandabase.h>

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"

// Since Alias image files don't have a magic number, we'll make a little
// sanity check on the size of the image.  If either the width or height is
// larger than this, it must be bogus.
#define INSANE_SIZE 20000

inline unsigned short 
read_ushort(FILE *file) {
  unsigned short x;
  return pm_readbigshort(file, (short *)&x)==0 ? x : 0;
}

inline unsigned char
read_uchar(FILE *file) {
  int x;
  x = getc(file);
  return (x!=EOF) ? (unsigned char)x : 0;
}

inline void
write_ushort(FILE *file, unsigned short x) {
  pm_writebigshort(file, (short)x);
}

inline void
write_uchar(FILE *file, unsigned char x) {
  putc(x, file);
}

PNMReaderAlias::
PNMReaderAlias(FILE *file, int already_read_magic) : PNMReader(file) {
  if (already_read_magic >= 0) {
    cols = already_read_magic;
  } else {
    cols = read_ushort(file);
  }
  rows = read_ushort(file);

  if (cols==0 || rows==0 ||
      cols>INSANE_SIZE || rows>INSANE_SIZE) {
    valid = false;
    return;
  }

  read_ushort(file);
  read_ushort(file);

  int bpp = read_ushort(file);

  switch (bpp) {
  case 8:
    color_type = PNMImage::Grayscale;
    break;

  case 24:
    color_type = PNMImage::Color;
    break;

  default:
    valid = false;
    return;
  }

  maxval = 255;
}

bool PNMReaderAlias::
ReadRow(xel *row, xelval *) {
  int x;
  int num;
  unsigned char red, grn, blu;

  x = 0;
  while (x < cols) {
    num = read_uchar(file);
    if (num==0 || x+num > cols) {
      return false;
    }
    blu = read_uchar(file);
    
    if (color_type == PNMImage::Color) {
      grn = read_uchar(file);
      red = read_uchar(file);
      while (num>0) {
	PPM_ASSIGN(row[x], red, grn, blu);
	x++;
	num--;
      }
    } else {
      while (num>0) {
	PPM_PUTB(row[x], blu);
	x++;
	num--;
      }
    }
  }

  return true;
}

static unsigned char last_red = 0, last_blu = 0, last_grn = 0;
static int num_count = 0;

static void
flush_color(FILE *file) {
  if (num_count>0) {
    write_uchar(file, num_count);
    write_uchar(file, last_blu);
    write_uchar(file, last_grn);
    write_uchar(file, last_red);
    num_count = 0;
  }
}

static void 
write_color(FILE *file, 
	    unsigned char red, unsigned char blu, unsigned char grn) {
  if (red==last_red && blu==last_blu && grn==last_grn && num_count<0377) {
    num_count++;
  } else {
    flush_color(file);
    last_red = red;
    last_grn = grn;
    last_blu = blu;
    num_count = 1;
  }
}

bool PNMWriterAlias::
WriteHeader() {
  write_ushort(file, cols);
  write_ushort(file, rows);

  write_ushort(file, 0);
  write_ushort(file, 0);

  // We'll always write full-color Alias images, even if the source was
  // grayscale.  Many programs don't seem to understand grayscale Alias images.
  write_ushort(file, 24);
  return true;
}

bool PNMWriterAlias::
WriteRow(xel *row_data, xelval *) {
  int x;
  unsigned char red, grn, blu;

  int is_grayscale = PNMImage::IsGrayscale(color_type);

  for (x = 0; x<cols; x++) {
    if (is_grayscale) {
      red = grn = blu = (unsigned char)(255*PPM_GETB(row_data[x])/maxval);
    } else {
      red = (unsigned char)(255*PPM_GETR(row_data[x])/maxval);
      grn = (unsigned char)(255*PPM_GETG(row_data[x])/maxval);
      blu = (unsigned char)(255*PPM_GETB(row_data[x])/maxval);
    }
    
    write_color(file, red, blu, grn);
  }
  flush_color(file);
  
  return true;
}


