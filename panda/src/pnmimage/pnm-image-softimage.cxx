// pnm-image-softimage.cc
//
// PNMImage::ReadSoftImage()
// PNMImage::WriteSoftImage()

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmReaderTypes.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"
#include "config_pnmimage.h"
#include <notify.h>

#include <stdio.h>

static const float imageVersionNumber = 3.0;
static const int imageCommentLength = 80;
static const char imageComment[imageCommentLength+1] =
  "Written by DRR's PNMImage library.";



// Values to indicate compressed/uncompressed types
#define UNCOMPRESSED 0x00
#define MIXED_RUN_LENGTH 0x02

// Bits to indicate channel type
#define RGB_CHANNEL 0xe0
#define ALPHA_CHANNEL 0x10

inline float
read_float(FILE *file) {
  long l;

  if (pm_readbiglong(file, &l)==0) {
    return *(float *)&l;
  } else {
    return 0.0;
  }
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
write_ushort(FILE *file, unsigned short x) {
  pm_writebigshort(file, (short)x);
}

inline void
write_uchar(FILE *file, unsigned char x) {
  putc(x, file);
}

inline void
write_float(FILE *file, float x) {
  pm_writebiglong(file, *(long *)&x);
}

static int
read_channel_pkt(FILE *file, 
		 int &chained, int &size, int &type, int &channel) {
  chained = read_uchar(file);
  size = read_uchar(file);
  type = read_uchar(file);
  channel = read_uchar(file);

  if (feof(file)) {
    return false;
  }

  if (size!=8) {
    pnmimage_cat.error()
      << "Don't know how to interpret " << size << " bits per pixel!\n";
    return false;
  }

  return true;
}

static void
read_rgb(xel *row_data, xelval *, FILE *file, int x, int repeat) {
  xelval red, grn, blu;
  red = read_uchar(file);
  grn = read_uchar(file);
  blu = read_uchar(file);

  while (repeat>0) {
    PPM_ASSIGN(row_data[x], red, grn, blu);
    x++;
    repeat--;
  }
}

static void
read_alpha(xel *, xelval *alpha_data, FILE *file, int x, int repeat) {
  xelval alpha = read_uchar(file);

  while (repeat>0) {
    alpha_data[x] = alpha;
    x++;
    repeat--;
  }
}

static void
read_rgba(xel *row_data, xelval *alpha_data, FILE *file, int x, int repeat) {
  xelval red, grn, blu, alpha;
  red = read_uchar(file);
  grn = read_uchar(file);
  blu = read_uchar(file);
  alpha = read_uchar(file);

  while (repeat>0) {
    PPM_ASSIGN(row_data[x], red, grn, blu);
    alpha_data[x] = alpha;
    x++;
    repeat--;
  }
}
    

static int
read_scanline(xel *row_data, xelval *alpha_data, int cols, FILE *file,
	      void (*read_data)(xel *row_data, xelval *alpha_data, FILE *file,
				int x, int repeat),
	      int ctype) {
  if (ctype==UNCOMPRESSED) {
    for (int x = 0; x<cols; x++) {
      read_data(row_data, alpha_data, file, x, 1);
    }
    return true;
  } else {
    int x;
    int num;

    x = 0;
    while (x < cols) {
      num = read_uchar(file);

      if (num<128) {
	// Sequence of non-repeated values.
	num++;
	if (x+num > cols) {
	  return false;
	}
	while (num>0) {
	  read_data(row_data, alpha_data, file, x, 1);
	  if (feof(file)) {
	    return false;
	  }
	  x++;
	  num--;
	}
      } else {
	// Sequence of repeated values.
	if (num==128) {
	  num = read_ushort(file);
	} else {
	  num -= 127;
	}
	if (x+num > cols) {
	  return false;
	}
	read_data(row_data, alpha_data, file, x, num);
	if (feof(file)) {
	  return false;
	}
	x += num;
      }
    }

    return (x==cols);
  }
}
  

PNMReaderSoftImage::
PNMReaderSoftImage(FILE *file, int already_read_magic) : PNMReader(file) {
  unsigned short magic1, magic2;

  magic1 = (already_read_magic >= 0) ? already_read_magic : read_ushort(file);
  if (magic1 != SOFTIMAGE_MAGIC1) {
    valid = false;
    return;
  }

  magic2 = read_ushort(file);
  if (magic2 != SOFTIMAGE_MAGIC2) {
    valid = false;
    return;
  }
  
  // skip version number
  read_float(file);

  // Skip comment
  fseek(file, imageCommentLength, SEEK_CUR);

  char pict_id[4];
  if (fread(pict_id, 1, 4, file) < 4) {
    valid = false;
    return;
  }

  if (memcmp(pict_id, "PICT", 4)!=0) {
    valid = false;
    return;
  }

  cols = read_ushort(file);
  rows = read_ushort(file);

  float ratio = read_float(file);
  int fields = read_ushort(file);
  read_ushort(file);

  int chained, size, channel;
  if (!read_channel_pkt(file, chained, size, rgb_ctype, channel)) {
    valid = false;
    return;
  }

  soft_color = unknown;

  if (channel == (RGB_CHANNEL | ALPHA_CHANNEL)) {
    // Four components in the first part: RGBA.
    soft_color = rgba;

  } else if (channel == RGB_CHANNEL) {
    // Three components in the first part: RGB.
    soft_color = rgb;
    
    if (chained) {
      if (!read_channel_pkt(file, chained, size, alpha_ctype, channel)) {
	valid = false;
	return;
      }
      
      if (channel == ALPHA_CHANNEL) {
	// Alpha component in the second part: RGBA.
	soft_color = rgb_a;
      }
    }
  }

  switch (soft_color) {
  case rgb:
    color_type = PNMImage::Color;
    break;
    
  case rgba:
  case rgb_a:
    color_type = PNMImage::FourChannel;
    break;

  default:
    pnmimage_cat.error()
      << "Image is not RGB or RGBA!\n";
    valid = false;
    return;
  }

  if (chained) {
    pnmimage_cat.error()
      << "Unexpected additional channels in image file.\n";
    valid = false;
    return;
  }

  maxval = 255;
}

bool PNMReaderSoftImage::
ReadRow(xel *row_data, xelval *alpha_data) {
  switch (soft_color) {
  case rgb:
    if (!read_scanline(row_data, alpha_data, cols, file,
		       read_rgb, rgb_ctype)) {
      return false;
    }
    break;
    
  case rgba:
    if (!read_scanline(row_data, alpha_data, cols, file,
		       read_rgba, rgb_ctype)) {
      return false;
    }
    break;
    
  case rgb_a:
    if (!read_scanline(row_data, alpha_data, cols, file,
		       read_rgb, rgb_ctype)) {
      return false;
    }
    if (!read_scanline(row_data, alpha_data, cols, file,
		       read_alpha, alpha_ctype)) {
      return false;
    }
    break;
  }
  
  return true;
}


static void
write_channel_pkt(FILE *file, 
		 int chained, int size, int type, int channel) {
  write_uchar(file, chained);
  write_uchar(file, size);
  write_uchar(file, type);
  write_uchar(file, channel);
}

static void
write_rgb(xel *row_data, xelval *, FILE *file, int x) {
  write_uchar(file, PPM_GETR(row_data[x]));
  write_uchar(file, PPM_GETG(row_data[x]));
  write_uchar(file, PPM_GETB(row_data[x]));
}

static int
compare_rgb(xel *row_data, xelval *, int x1, int x2) {
  return PPM_EQUAL(row_data[x1], row_data[x2]);
}

static void
write_gray(xel *row_data, xelval *, FILE *file, int x) {
  write_uchar(file, PPM_GETB(row_data[x]));
  write_uchar(file, PPM_GETB(row_data[x]));
  write_uchar(file, PPM_GETB(row_data[x]));
}

static int
compare_gray(xel *row_data, xelval *, int x1, int x2) {
  return (PPM_GETB(row_data[x1])==PPM_GETB(row_data[x2]));
}

static void
write_alpha(xel *, xelval *alpha_data, FILE *file, int x) {
  write_uchar(file, alpha_data[x]);
}

static int
compare_alpha(xel *, xelval *alpha_data, int x1, int x2) {
  return (alpha_data[x1]==alpha_data[x2]);
}

static void
write_diff(xel *row_data, xelval *alpha_data, FILE *file,
	   void (*write_data)(xel *row_data, xelval *alpha_data, FILE *file,
			      int x),
	   int tox, int length) {
  if (length>0) {
    nassertv(length<=128);
    
    write_uchar(file, length-1);
    while (length>0) {
      length--;
      write_data(row_data, alpha_data, file, tox-length);
    }
  }
}

static void
write_same(xel *row_data, xelval *alpha_data, FILE *file,
	   void (*write_data)(xel *row_data, xelval *alpha_data, FILE *file,
			      int x),
	   int tox, int length) {
  if (length==1) {
    write_diff(row_data, alpha_data, file, write_data, tox, length);
    
  } else if (length>0) {
    if (length<128) {
      write_uchar(file, length+127);
    } else {
      write_uchar(file, 128);
      write_ushort(file, length);
    } 
    write_data(row_data, alpha_data, file, tox);
  }
}
  

static void
write_scanline(xel *row_data, xelval *alpha_data, int cols, FILE *file,
	       int (*compare_data)(xel *row_data, xelval *alpha_data,
				   int x1, int x2),
	       void (*write_data)(xel *row_data, xelval *alpha_data,
				  FILE *file, int x)) {
  int run_length = 0;

  int x = 0;
  int same = true;

  // Go through each value in the scanline, from beginning to end, looking
  // for runs of identical values.
  while (x < cols) {

    if (same) {

      // We have been scanning past a run of identical values.  In this case,
      // the run is the sequence of values from x-run_length to x-1.

      if (!compare_data(row_data, alpha_data, x, x-run_length)) {
	// Oops, the end of a run.

	if (run_length <= 1) {
	  // If run_length is only 1, no big deal--this is actually the
	  // beginning of a different-valued run.
	  
	  same = false;
	  
	} else {
	  // Write out the old run and begin a new one.  We'll be optimistic
	  // and hope the new run will also represent a sequence of identical
	  // values (until we find otherwise).
	  
	  write_same(row_data, alpha_data, file, write_data, x-1, run_length);
	  same = true;
	  run_length = 0;
	}
      }

    } else {   // !same

      // We have been scanning past a run of different values.  In this case,
      // the run is the sequence of values from x-run_length to x-1.

      if (run_length>128) {
	// We can't have different runs of more than 128 characters.  Close
	// off the old run.
	
	int excess = run_length - 128;
	write_diff(row_data, alpha_data, file, write_data, x-excess-1, 128);
	run_length = excess;
      
      } else if (run_length > 2 &&
		 compare_data(row_data, alpha_data, x, x-1) &&
		 compare_data(row_data, alpha_data, x, x-2)) {

	// If the last three values have been the same, then it's time to
	// begin a new run of similar values.  Close off the old run.
	
	write_diff(row_data, alpha_data, file, write_data, x-3, run_length-2);
	same = true;
	run_length = 2;
      }
    }

    x++;
    run_length++;
  }

  // We made it all the way to the end.  Flush out the last run.

  if (run_length>0) {
    if (same) {
      write_same(row_data, alpha_data, file, write_data, cols-1, run_length);
    } else {

      // Mighty unlikely, but we might have just run over the
      // 128-pixel limit.
      if (run_length>128) {
	int excess = run_length - 128;
	write_diff(row_data, alpha_data, file, write_data, cols-excess-1, 128);
	run_length = excess;
      }

      write_diff(row_data, alpha_data, file, write_data, cols-1, run_length);
    }
  }
}


bool PNMWriterSoftImage::
WriteHeader() {
  write_ushort(file, SOFTIMAGE_MAGIC1);
  write_ushort(file, SOFTIMAGE_MAGIC2);
  write_float(file, imageVersionNumber);

  fwrite(imageComment, 1, imageCommentLength, file);
  fwrite("PICT", 1, 4, file);

  write_ushort(file, cols);
  write_ushort(file, rows);

  write_float(file, 1.0);    // pixel aspect ratio; we don't know.
  write_ushort(file, 3);     // fields value; we don't really know either.
  write_ushort(file, 0);     // padding

  // There doesn't seem to be a variation on SoftImage image formats for
  // grayscale images.  We'll write out grayscale as a 3-channel image.

  switch (color_type) {
  case PNMImage::Grayscale:
  case PNMImage::Color:
    write_channel_pkt(file, 0, 8, MIXED_RUN_LENGTH, RGB_CHANNEL);
    break;

  case PNMImage::FourChannel:
  case PNMImage::TwoChannel:
    write_channel_pkt(file, 1, 8, MIXED_RUN_LENGTH, RGB_CHANNEL);
    write_channel_pkt(file, 0, 8, MIXED_RUN_LENGTH, ALPHA_CHANNEL);
    break;
  }

  return true;
}

bool PNMWriterSoftImage::
WriteRow(xel *row_data, xelval *alpha_data) {
  if (PNMImage::IsGrayscale(color_type)) {
    write_scanline(row_data, alpha_data, cols, file, compare_gray, write_gray);
    
  } else {
    write_scanline(row_data, alpha_data, cols, file, compare_rgb, write_rgb);
  }
  
  if (PNMImage::HasAlpha(color_type)) {
    write_scanline(row_data, alpha_data, cols, file, compare_alpha, write_alpha);
  }
  
  return !ferror(file);
}



