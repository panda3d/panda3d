/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeSoftImage.cxx
 * @author drose
 * @date 2000-06-17
 */

#include "pnmFileTypeSoftImage.h"

#ifdef HAVE_SOFTIMAGE_PIC

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

using std::istream;
using std::ostream;
using std::string;

static const float imageVersionNumber = 3.0;
static const int imageCommentLength = 80;
static const char imageComment[imageCommentLength+1] =
  "Written by pnmimage.";

// Values to indicate compresseduncompressed types
#define UNCOMPRESSED 0x00
#define MIXED_RUN_LENGTH 0x02

// Bits to indicate channel type
#define RGB_CHANNEL 0xe0
#define ALPHA_CHANNEL 0x10

// SoftImage magic number: high word, low word
#define SOFTIMAGE_MAGIC1 0x5380
#define SOFTIMAGE_MAGIC2 0xf634

static const char * const extensions_softimage[] = {
  "pic", "soft"
};
static const int num_extensions_softimage = sizeof(extensions_softimage) / sizeof(const char *);

TypeHandle PNMFileTypeSoftImage::_type_handle;

inline float
read_float(istream *file) {
  long l;

  if (pm_readbiglong(file, &l)==0) {
    return *(float *)&l;
  } else {
    return 0.0;
  }
}

inline unsigned short
read_ushort_SI(istream *file) {
  unsigned short x;
  return pm_readbigshort(file, (short *)&x)==0 ? x : 0;
}

inline unsigned char
read_uchar_SI(istream *file) {
  int x;
  x = file->get();
  return (x!=EOF) ? (unsigned char)x : 0;
}

inline void
write_ushort_SI(ostream *file, unsigned short x) {
  pm_writebigshort(file, (short)x);
}

inline void
write_uchar_SI(ostream *file, unsigned char x) {
  file->put(x);
}

inline void
write_float(ostream *file, float x) {
  pm_writebiglong(file, *(long *)&x);
}

static int
read_channel_pkt(istream *file,
                 int &chained, int &size, int &type, int &channel) {
  chained = read_uchar_SI(file);
  size = read_uchar_SI(file);
  type = read_uchar_SI(file);
  channel = read_uchar_SI(file);

  if (file->eof() || file->fail()) {
    return false;
  }

  if (size!=8) {
    pnmimage_soft_cat.error()
      << "Don't know how to interpret " << size << " bits per pixel!\n";
    return false;
  }

  return true;
}

static void
read_rgb(xel *row_data, xelval *, istream *file, int x, int repeat) {
  xelval red, grn, blu;
  red = read_uchar_SI(file);
  grn = read_uchar_SI(file);
  blu = read_uchar_SI(file);

  while (repeat>0) {
    PPM_ASSIGN(row_data[x], red, grn, blu);
    x++;
    repeat--;
  }
}

static void
read_alpha(xel *, xelval *alpha_data, istream *file, int x, int repeat) {
  xelval alpha = read_uchar_SI(file);

  while (repeat>0) {
    alpha_data[x] = alpha;
    x++;
    repeat--;
  }
}

static void
read_rgba(xel *row_data, xelval *alpha_data, istream *file, int x, int repeat) {
  xelval red, grn, blu, alpha;
  red = read_uchar_SI(file);
  grn = read_uchar_SI(file);
  blu = read_uchar_SI(file);
  alpha = read_uchar_SI(file);

  while (repeat>0) {
    PPM_ASSIGN(row_data[x], red, grn, blu);
    alpha_data[x] = alpha;
    x++;
    repeat--;
  }
}


static int
read_scanline(xel *row_data, xelval *alpha_data, int cols, istream *file,
              void (*read_data)(xel *row_data, xelval *alpha_data, istream *file,
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
      num = read_uchar_SI(file);

      if (num<128) {
        // Sequence of non-repeated values.
        num++;
        if (x+num > cols) {
          return false;
        }
        while (num>0) {
          read_data(row_data, alpha_data, file, x, 1);
          if (file->eof() || file->fail()) {
            return false;
          }
          x++;
          num--;
        }
      } else {
        // Sequence of repeated values.
        if (num==128) {
          num = read_ushort_SI(file);
        } else {
          num -= 127;
        }
        if (x+num > cols) {
          return false;
        }
        read_data(row_data, alpha_data, file, x, num);
        if (file->eof() || file->fail()) {
          return false;
        }
        x += num;
      }
    }

    return (x==cols);
  }
}

/**
 *
 */
PNMFileTypeSoftImage::
PNMFileTypeSoftImage() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeSoftImage::
get_name() const {
  return "SoftImage";
}

/**
 * Returns the number of different possible filename extensions_softimage
 * associated with this particular file type.
 */
int PNMFileTypeSoftImage::
get_num_extensions() const {
  return num_extensions_softimage;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeSoftImage::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_softimage, string());
  return extensions_softimage[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeSoftImage::
get_suggested_extension() const {
  return "pic";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeSoftImage::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypeSoftImage::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  int mn =
    ((unsigned char)magic_number[0] << 8) |
    ((unsigned char)magic_number[1]);
  return (mn == SOFTIMAGE_MAGIC1);
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeSoftImage::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeSoftImage::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


/**
 *
 */
PNMFileTypeSoftImage::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  if (!read_magic_number(_file, magic_number, 4)) {
    // No magic number, no image.
    if (pnmimage_soft_cat.is_debug()) {
      pnmimage_soft_cat.debug()
        << "SoftImage image file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  int magic1 =
    ((unsigned char)magic_number[0] << 8) |
    ((unsigned char)magic_number[1]);
  int magic2 =
    ((unsigned char)magic_number[2] << 8) |
    ((unsigned char)magic_number[3]);

  if (magic1 != SOFTIMAGE_MAGIC1 || magic2 != SOFTIMAGE_MAGIC2) {
    _is_valid = false;
    return;
  }

  // skip version number
  read_float(_file);

  // Skip comment
  _file->seekg(imageCommentLength, std::ios::cur);

  char pict_id[4];
  _file->read(pict_id, 4);
  if (_file->gcount() < 4) {
    _is_valid = false;
    return;
  }

  if (memcmp(pict_id, "PICT", 4)!=0) {
    _is_valid = false;
    return;
  }

  _x_size = read_ushort_SI(_file);
  _y_size = read_ushort_SI(_file);

  /* float ratio = */ read_float(_file);
  /* int fields = */ read_ushort_SI(_file);
  read_ushort_SI(_file);

  int chained, size, channel;
  if (!read_channel_pkt(_file, chained, size, rgb_ctype, channel)) {
    _is_valid = false;
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
      if (!read_channel_pkt(_file, chained, size, alpha_ctype, channel)) {
        _is_valid = false;
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
    _num_channels = 3;
    break;

  case rgba:
  case rgb_a:
    _num_channels = 4;
    break;

  default:
    pnmimage_soft_cat.error()
      << "Image is not RGB or RGBA!\n";
    _is_valid = false;
    return;
  }

  if (chained) {
    pnmimage_soft_cat.error()
      << "Unexpected additional channels in image file.\n";
    _is_valid = false;
    return;
  }

  _maxval = 255;

  if (pnmimage_soft_cat.is_debug()) {
    pnmimage_soft_cat.debug()
      << "Reading SoftImage " << *this << "\n";
  }
}


/**
 * Returns true if this particular PNMReader supports a streaming interface to
 * reading the data: that is, it is capable of returning the data one row at a
 * time, via repeated calls to read_row().  Returns false if the only way to
 * read from this file is all at once, via read_data().
 */
bool PNMFileTypeSoftImage::Reader::
supports_read_row() const {
  return true;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 */
bool PNMFileTypeSoftImage::Reader::
read_row(xel *row_data, xelval *alpha_data, int x_size, int) {
  if (!is_valid()) {
    return false;
  }
  switch (soft_color) {
  case rgb:
    if (!read_scanline(row_data, alpha_data, x_size, _file,
                       read_rgb, rgb_ctype)) {
      return false;
    }
    break;

  case rgba:
    if (!read_scanline(row_data, alpha_data, x_size, _file,
                       read_rgba, rgb_ctype)) {
      return false;
    }
    break;

  case rgb_a:
    if (!read_scanline(row_data, alpha_data, x_size, _file,
                       read_rgb, rgb_ctype)) {
      return false;
    }
    if (!read_scanline(row_data, alpha_data, x_size, _file,
                       read_alpha, alpha_ctype)) {
      return false;
    }
    break;

  default:
    break;
  }

  return true;
}


static void
write_channel_pkt(ostream *file,
                 int chained, int size, int type, int channel) {
  write_uchar_SI(file, chained);
  write_uchar_SI(file, size);
  write_uchar_SI(file, type);
  write_uchar_SI(file, channel);
}

static void
write_rgb(xel *row_data, xelval *, ostream *file, int x) {
  write_uchar_SI(file, PPM_GETR(row_data[x]));
  write_uchar_SI(file, PPM_GETG(row_data[x]));
  write_uchar_SI(file, PPM_GETB(row_data[x]));
}

static int
compare_rgb(xel *row_data, xelval *, int x1, int x2) {
  return PPM_EQUAL(row_data[x1], row_data[x2]);
}

static void
write_gray(xel *row_data, xelval *, ostream *file, int x) {
  write_uchar_SI(file, PPM_GETB(row_data[x]));
  write_uchar_SI(file, PPM_GETB(row_data[x]));
  write_uchar_SI(file, PPM_GETB(row_data[x]));
}

static int
compare_gray(xel *row_data, xelval *, int x1, int x2) {
  return (PPM_GETB(row_data[x1])==PPM_GETB(row_data[x2]));
}

static void
write_alpha(xel *, xelval *alpha_data, ostream *file, int x) {
  write_uchar_SI(file, alpha_data[x]);
}

static int
compare_alpha(xel *, xelval *alpha_data, int x1, int x2) {
  return (alpha_data[x1]==alpha_data[x2]);
}

static void
write_diff(xel *row_data, xelval *alpha_data, ostream *file,
           void (*write_data)(xel *row_data, xelval *alpha_data, ostream *file,
                              int x),
           int tox, int length) {
  if (length>0) {
    nassertv(length<=128);

    write_uchar_SI(file, length-1);
    while (length>0) {
      length--;
      write_data(row_data, alpha_data, file, tox-length);
    }
  }
}

static void
write_same(xel *row_data, xelval *alpha_data, ostream *file,
           void (*write_data)(xel *row_data, xelval *alpha_data, ostream *file,
                              int x),
           int tox, int length) {
  if (length==1) {
    write_diff(row_data, alpha_data, file, write_data, tox, length);

  } else if (length>0) {
    if (length<128) {
      write_uchar_SI(file, length+127);
    } else {
      write_uchar_SI(file, 128);
      write_ushort_SI(file, length);
    }
    write_data(row_data, alpha_data, file, tox);
  }
}


static void
write_scanline(xel *row_data, xelval *alpha_data, int cols, ostream *file,
               int (*compare_data)(xel *row_data, xelval *alpha_data,
                                   int x1, int x2),
               void (*write_data)(xel *row_data, xelval *alpha_data,
                                  ostream *file, int x)) {
  int run_length = 0;

  int x = 0;
  int same = true;

  // Go through each value in the scanline, from beginning to end, looking for
  // runs of identical values.
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

      // Mighty unlikely, but we might have just run over the 128-pixel limit.
      if (run_length>128) {
        int excess = run_length - 128;
        write_diff(row_data, alpha_data, file, write_data, cols-excess-1, 128);
        run_length = excess;
      }

      write_diff(row_data, alpha_data, file, write_data, cols-1, run_length);
    }
  }
}

/**
 *
 */
PNMFileTypeSoftImage::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

/**
 * Returns true if this particular PNMWriter supports a streaming interface to
 * writing the data: that is, it is capable of writing the image one row at a
 * time, via repeated calls to write_row().  Returns false if the only way to
 * write from this file is all at once, via write_data().
 */
bool PNMFileTypeSoftImage::Writer::
supports_write_row() const {
  return true;
}

/**
 * If supports_write_row(), above, returns true, this function may be called
 * to write out the image header in preparation to writing out the image data
 * one row at a time.  Returns true if the header is successfully written,
 * false if there is an error.
 *
 * It is the user's responsibility to fill in the header data via calls to
 * set_x_size(), set_num_channels(), etc., or copy_header_from(), before
 * calling write_header().
 */
bool PNMFileTypeSoftImage::Writer::
write_header() {
  write_ushort_SI(_file, SOFTIMAGE_MAGIC1);
  write_ushort_SI(_file, SOFTIMAGE_MAGIC2);
  write_float(_file, imageVersionNumber);

  _file->write(imageComment, imageCommentLength);
  _file->write("PICT", 4);

  write_ushort_SI(_file, _x_size);
  write_ushort_SI(_file, _y_size);

  write_float(_file, 1.0);    // pixel aspect ratio; we don't know.
  write_ushort_SI(_file, 3);     // fields value; we don't really know either.
  write_ushort_SI(_file, 0);     // padding

  // There doesn't seem to be a variation on SoftImage image formats for
  // grayscale images.  We'll write out grayscale as a 3-channel image.

  if (has_alpha()) {
    write_channel_pkt(_file, 1, 8, MIXED_RUN_LENGTH, RGB_CHANNEL);
    write_channel_pkt(_file, 0, 8, MIXED_RUN_LENGTH, ALPHA_CHANNEL);
  } else {
    write_channel_pkt(_file, 0, 8, MIXED_RUN_LENGTH, RGB_CHANNEL);
  }

  return true;
}

/**
 * If supports_write_row(), above, returns true, this function may be called
 * repeatedly to write the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully written, false if there
 * is an error.
 *
 * You must first call write_header() before writing the individual rows.  It
 * is also important to delete the PNMWriter class after successfully writing
 * the last row.  Failing to do this may result in some data not getting
 * flushed!
 */
bool PNMFileTypeSoftImage::Writer::
write_row(xel *row_data, xelval *alpha_data) {
  if (is_grayscale()) {
    write_scanline(row_data, alpha_data, _x_size, _file, compare_gray, write_gray);

  } else {
    write_scanline(row_data, alpha_data, _x_size, _file, compare_rgb, write_rgb);
  }

  if (has_alpha()) {
    write_scanline(row_data, alpha_data, _x_size, _file, compare_alpha, write_alpha);
  }

  return !_file->fail();
}



/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeSoftImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeSoftImage);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeSoftImage::
make_PNMFileTypeSoftImage(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_SOFTIMAGE_PIC
