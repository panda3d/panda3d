/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeIMG.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "pnmFileTypeIMG.h"

#ifdef HAVE_IMG

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

// Since raw image files don't have a magic number, we'll make a little sanity
// check on the size of the image.  If either the width or height is larger
// than this, it must be bogus.
#define INSANE_SIZE 20000

using std::istream;
using std::ostream;
using std::string;

static const char * const extensions_img[] = {
  "img"
};
static const int num_extensions_img = sizeof(extensions_img) / sizeof(const char *);

TypeHandle PNMFileTypeIMG::_type_handle;

/**
 *
 */
PNMFileTypeIMG::
PNMFileTypeIMG() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeIMG::
get_name() const {
  return "Raw binary RGB";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeIMG::
get_num_extensions() const {
  return num_extensions_img;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeIMG::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_img, string());
  return extensions_img[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeIMG::
get_suggested_extension() const {
  return "img";
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeIMG::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeIMG::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


inline unsigned long
read_ulong(istream *file) {
  unsigned long x;
  return pm_readbiglong(file, (long *)&x)==0 ? x : 0;
}

inline unsigned short
read_ushort_IMG(istream *file) {
  unsigned short x;
  return pm_readbigshort(file, (short *)&x)==0 ? x : 0;
}

inline unsigned char
read_uchar_IMG(istream *file) {
  int x;
  x = file->get();
  return (x!=EOF) ? (unsigned char)x : 0;
}

inline void
write_ulong(ostream *file, unsigned long x) {
  pm_writebiglong(file, (long)x);
}

inline void
write_ushort_IMG(ostream *file, unsigned long x) {
  pm_writebigshort(file, (short)(long)x);
}

inline void
write_uchar_IMG(ostream *file, unsigned char x) {
  file->put(x);
}

/**
 *
 */
PNMFileTypeIMG::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  if (img_header_type == IHT_long) {
    if (!read_magic_number(_file, magic_number, 8)) {
      // Although raw IMG files have no magic number, they may have a pair of
      // ushorts or ulongs at the beginning to indicate the file size.
      if (pnmimage_img_cat.is_debug()) {
        pnmimage_img_cat.debug()
          << "IMG image file appears to be empty.\n";
      }
      _is_valid = false;
      return;
    }

    _x_size =
      ((unsigned char)magic_number[0] << 24) |
      ((unsigned char)magic_number[1] << 16) |
      ((unsigned char)magic_number[2] << 8) |
      ((unsigned char)magic_number[3]);

    _y_size =
      ((unsigned char)magic_number[4] << 24) |
      ((unsigned char)magic_number[5] << 16) |
      ((unsigned char)magic_number[6] << 8) |
      ((unsigned char)magic_number[7]);

  } else if (img_header_type == IHT_short) {
    if (!read_magic_number(_file, magic_number, 4)) {
      if (pnmimage_img_cat.is_debug()) {
        pnmimage_img_cat.debug()
          << "IMG image file appears to be empty.\n";
      }
      _is_valid = false;
      return;
    }

    _x_size =
      ((unsigned char)magic_number[0] << 8) |
      ((unsigned char)magic_number[1]);

    _y_size =
      ((unsigned char)magic_number[2] << 8) |
      ((unsigned char)magic_number[3]);

  } else {
    _x_size = img_size[0];
    _y_size = img_size[1];
  }

  if (_x_size == 0 || _y_size == 0 ||
      _x_size > INSANE_SIZE || _y_size > INSANE_SIZE) {
    _is_valid = false;
    if (img_header_type == IHT_none) {
      pnmimage_img_cat.error()
        << "Must specify img-xsize and img-ysize to load headerless raw files.\n";
    } else {
      pnmimage_img_cat.debug()
        << "IMG file does not have a valid xsize,ysize header.\n";
    }
    return;
  }

  _maxval = 255;
  _num_channels = 3;

  if (pnmimage_img_cat.is_debug()) {
    pnmimage_img_cat.debug()
      << "Reading IMG " << *this << "\n";
  }
}

/**
 * Returns true if this particular PNMReader supports a streaming interface to
 * reading the data: that is, it is capable of returning the data one row at a
 * time, via repeated calls to read_row().  Returns false if the only way to
 * read from this file is all at once, via read_data().
 */
bool PNMFileTypeIMG::Reader::
supports_read_row() const {
  return true;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 */
bool PNMFileTypeIMG::Reader::
read_row(xel *row_data, xelval *, int x_size, int) {
  int x;
  xelval red, grn, blu;
  for (x = 0; x < x_size; x++) {
    red = read_uchar_IMG(_file);
    grn = read_uchar_IMG(_file);
    blu = read_uchar_IMG(_file);

    PPM_ASSIGN(row_data[x], red, grn, blu);
  }

  return true;
}

/**
 *
 */
PNMFileTypeIMG::Writer::
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
bool PNMFileTypeIMG::Writer::
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
bool PNMFileTypeIMG::Writer::
write_header() {
  if (img_header_type == IHT_long) {
    write_ulong(_file, _x_size);
    write_ulong(_file, _y_size);
  } else if (img_header_type == IHT_short) {
    write_ushort_IMG(_file, _x_size);
    write_ushort_IMG(_file, _y_size);
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
bool PNMFileTypeIMG::Writer::
write_row(xel *row_data, xelval *) {
  int x;
  for (x = 0; x < _x_size; x++) {
    write_uchar_IMG(_file, (unsigned char)(255*PPM_GETR(row_data[x])/_maxval));
    write_uchar_IMG(_file, (unsigned char)(255*PPM_GETG(row_data[x])/_maxval));
    write_uchar_IMG(_file, (unsigned char)(255*PPM_GETB(row_data[x])/_maxval));
  }

  return true;
}



/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeIMG::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeIMG);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeIMG::
make_PNMFileTypeIMG(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_IMG
