// pnm-image-img.cc
//
// PNMImage::ReadImg()
// PNMImage::WriteImg()

#include <pandabase.h>

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"

// Since raw image files don't have a magic number, we'll make a little
// sanity check on the size of the image.  If either the width or height is
// larger than this, it must be bogus.
#define INSANE_SIZE 20000


inline unsigned long 
read_ulong(FILE *file) {
  unsigned long x;
  return pm_readbiglong(file, (long *)&x)==0 ? x : 0;
}

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
write_ulong(FILE *file, unsigned long x) {
  pm_writebiglong(file, (long)x);
}

inline void
write_uchar(FILE *file, unsigned char x) {
  putc(x, file);
}

PNMReaderIMG::
PNMReaderIMG(FILE *file, int already_read_magic) : PNMReader(file) {
  if (already_read_magic>=0) {
    cols = (already_read_magic << 16) | read_ushort(file);
  } else {
    cols = (int)read_ulong(file);
  }

  rows = (int)read_ulong(file);
  color_type = PNMImage::Color;

  if (cols==0 || rows==0 ||
      cols>INSANE_SIZE || rows>INSANE_SIZE) {
    valid = false;
    return;
  }

  maxval = 255;
}

bool PNMReaderIMG::
ReadRow(xel *row, xelval *) {
  int x;
  xelval red, grn, blu;
  for (x = 0; x<cols; x++) {
    red = read_uchar(file);
    grn = read_uchar(file);
    blu = read_uchar(file);
    
    PPM_ASSIGN(row[x], red, grn, blu);
  }

  return true;
}

bool PNMWriterIMG::
WriteHeader() {
  write_ulong(file, cols);
  write_ulong(file, rows);
  return true;
}

bool PNMWriterIMG::
WriteRow(xel *row_data, xelval *) {
  int x;
  for (x = 0; x<cols; x++) {
    write_uchar(file, (unsigned char)(255*PPM_GETR(row_data[x])/maxval));
    write_uchar(file, (unsigned char)(255*PPM_GETG(row_data[x])/maxval));
    write_uchar(file, (unsigned char)(255*PPM_GETB(row_data[x])/maxval));
  }
  
  return true;
}


