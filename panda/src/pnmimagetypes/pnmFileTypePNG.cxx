/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypePNG.cxx
 * @author drose
 * @date 2004-03-16
 */

#include "pnmFileTypePNG.h"

#ifdef HAVE_PNG

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"
#include "thread.h"

using std::istream;
using std::ostream;
using std::string;

static const char * const extensions_png[] = {
  "png"
};
static const int num_extensions_png = sizeof(extensions_png) / sizeof(const char *);

TypeHandle PNMFileTypePNG::_type_handle;

static const int png_max_palette = 256;

// This STL comparison functor is used in write_data(), below.  It sorts the
// non-maxval alpha pixels to the front of the list.
class LowAlphaCompare {
public:
  bool operator() (const PNMImageHeader::PixelSpec &a,
                   const PNMImageHeader::PixelSpec &b) {
    if (a._alpha != b._alpha) {
      return a._alpha < b._alpha;
    }
    return a < b;
  }
};

/**
 *
 */
PNMFileTypePNG::
PNMFileTypePNG() {
  // This constructor may run at static init time, so we use the ->
  // dereferencing convention on the notify category.
  if (pnmimage_png_cat->is_debug()) {
    pnmimage_png_cat->debug()
      << "PNG version " << PNG_LIBPNG_VER << "\n";
  }
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypePNG::
get_name() const {
  return "PNG";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypePNG::
get_num_extensions() const {
  return num_extensions_png;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypePNG::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_png, string());
  return extensions_png[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypePNG::
get_suggested_extension() const {
  return "png";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypePNG::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypePNG::
matches_magic_number(const string &magic_number) const {
  return png_sig_cmp((png_bytep)magic_number.data(), 0, magic_number.length()) == 0;
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypePNG::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypePNG::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypePNG::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypePNG);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypePNG::
make_PNMFileTypePNG(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

/**
 *
 */
PNMFileTypePNG::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  _png = nullptr;
  _info = nullptr;
  _is_valid = false;

  _png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                png_error, png_warning);
  if (_png == nullptr) {
    return;
  }

  _info = png_create_info_struct(_png);
  if (_info == nullptr) {
    png_destroy_read_struct(&_png, nullptr, nullptr);
    return;
  }

  _is_valid = true;

  if (setjmp(_jmpbuf)) {
    // This is the ANSI C way to handle exceptions.  If setjmp(), above,
    // returns true, it means that libpng detected an exception while
    // executing the code that reads the header info, below.
    free_png();
    return;
  }

  png_set_read_fn(_png, (void *)this, png_read_data);
  png_set_sig_bytes(_png, magic_number.length());

  png_read_info(_png, _info);

  png_uint_32 width;
  png_uint_32 height;
  int bit_depth;
  int color_type;
  int srgb_intent;
  double gamma;

  png_get_IHDR(_png, _info, &width, &height,
               &bit_depth, &color_type, nullptr, nullptr, nullptr);

  // Look for an sRGB chunk.
  if (png_get_sRGB(_png, _info, &srgb_intent) == PNG_INFO_sRGB) {
    _color_space = CS_sRGB;

  } else if (png_get_gAMA(_png, _info, &gamma) == PNG_INFO_gAMA) {
    // File specifies a gamma.
    if (gamma >= 0.99 && gamma <= 1.01) {
      _color_space = CS_linear;

    } else if (gamma >= 0.44999 && gamma <= 0.455001) {
      // It's probably close enough to sRGB.
      _color_space = CS_sRGB;

    } else {
      pnmimage_png_cat.warning()
        << "Unsupported image gamma " << gamma << ", "
        << "please re-export image as sRGB or linear.\n";
    }
  }

  if (pnmimage_png_cat.is_debug()) {
    pnmimage_png_cat.debug()
      << "width = " << width << " height = " << height << " bit_depth = "
      << bit_depth << " color_type = " << color_type
      << " color_space = " << _color_space << "\n";
  }

  _x_size = width;
  _y_size = height;
  _maxval = (1 << bit_depth) - 1;

  if (bit_depth < 8) {
    png_set_packing(_png);
  }

  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "PNG_COLOR_TYPE_GRAY\n";
    }
    _num_channels = 1;
    break;

  case PNG_COLOR_TYPE_GRAY_ALPHA:
    if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "PNG_COLOR_TYPE_GRAY_ALPHA\n";
    }
    _num_channels = 2;
    break;

  case PNG_COLOR_TYPE_RGB:
    if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "PNG_COLOR_TYPE_RGB\n";
    }
    _num_channels = 3;
    break;

  case PNG_COLOR_TYPE_RGB_ALPHA:
    if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "PNG_COLOR_TYPE_RGB_ALPHA\n";
    }
    _num_channels = 4;
    break;

  case PNG_COLOR_TYPE_PALETTE:
    if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "PNG_COLOR_TYPE_PALETTE\n";
    }
    png_set_palette_to_rgb(_png);
    _maxval = 255;
    _num_channels = 3;
    break;

  default:
    pnmimage_png_cat.error()
      << "Unsupported color type: " << color_type << "\n";
    free_png();
    return;
  }

  if (png_get_valid(_png, _info, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(_png);
    if (!has_alpha()) {
      _num_channels++;
    }
  }

  png_read_update_info(_png, _info);
}

/**
 *
 */
PNMFileTypePNG::Reader::
~Reader() {
  free_png();
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
int PNMFileTypePNG::Reader::
read_data(xel *array, xelval *alpha_data) {
  if (!is_valid()) {
    return 0;
  }

  if (setjmp(_jmpbuf)) {
    // This is the ANSI C way to handle exceptions.  If setjmp(), above,
    // returns true, it means that libpng detected an exception while
    // executing the code that reads the image, below.
    free_png();
    return 0;
  }

  int row_byte_length = _x_size * _num_channels;
  if (_maxval > 255) {
    row_byte_length *= 2;
  }

  int num_rows = _y_size;

  if (pnmimage_png_cat.is_debug()) {
    pnmimage_png_cat.debug()
      << "Allocating " << num_rows << " rows of " << row_byte_length
      << " bytes each.\n";
  }

  // We need to read a full copy of the image in first, in libpng's 2-d array
  // format, mainly because we keep array and alpha data separately, and there
  // doesn't appear to be good support to get this stuff out row-at-a-time for
  // interlaced files.
  png_bytep *rows = (png_bytep *)alloca(num_rows * sizeof(png_bytep));
  int yi;

  png_byte *alloc = (png_byte *)PANDA_MALLOC_ARRAY(row_byte_length * sizeof(png_byte) * num_rows);
  for (yi = 0; yi < num_rows; yi++) {
    rows[yi] = alloc + (row_byte_length * sizeof(png_byte)) * yi;
  }

  png_read_image(_png, rows);

  bool get_color = !is_grayscale();
  bool get_alpha = has_alpha();

  int pi = 0;
  for (yi = 0; yi < num_rows; yi++) {
    png_bytep source = rows[yi];
    for (int xi = 0; xi < _x_size; xi++) {
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 0;

      if (_maxval > 255) {
        if (get_color) {
          red = (source[0] << 8) | source[1];
          source += 2;

          green = (source[0] << 8) | source[1];
          source += 2;
        }

        blue = (source[0] << 8) | source[1];
        source += 2;

        if (get_alpha) {
          alpha = (source[0] << 8) | source[1];
          source += 2;
        }

      } else {
        if (get_color) {
          red = *source;
          source++;

          green = *source;
          source++;
        }

        blue = *source;
        source++;

        if (get_alpha) {
          alpha = *source;
          source++;
        }
      }

      PPM_ASSIGN(array[pi], red, green, blue);
      if (get_alpha) {
        alpha_data[pi] = alpha;
      }
      pi++;
    }

    nassertr(source <= rows[yi] + row_byte_length, yi);
  }

  png_read_end(_png, nullptr);
  PANDA_FREE_ARRAY(alloc);

  return _y_size;
}

/**
 * Releases the internal PNG structures and marks the reader invalid.
 */
void PNMFileTypePNG::Reader::
free_png() {
  if (_is_valid) {
    png_destroy_read_struct(&_png, &_info, nullptr);
    _is_valid = false;
  }
}

/**
 * A callback handler that PNG uses to read data from the iostream.
 */
void PNMFileTypePNG::Reader::
png_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
  Reader *self = (Reader *)png_get_io_ptr(png_ptr);
  self->_file->read((char *)data, length);
  if (length != (png_size_t)self->_file->gcount()) {
    pnmimage_png_cat.error()
      << "Didn't read enough bytes.\n";
    // Is there no way to indicate a read failure to libpng?
  }
  Thread::consider_yield();
}

/**
 * This is our own warning handler.  It is called by the png library to issue
 * a warning message.
 */
void PNMFileTypePNG::Reader::
png_warning(png_structp, png_const_charp warning_msg) {
  pnmimage_png_cat.warning()
    << warning_msg << "\n";
}

/**
 * This is our own error handler.  It is called by the png library to issue a
 * fatal error message.
 */
void PNMFileTypePNG::Reader::
png_error(png_structp png_ptr, png_const_charp error_msg) {
  pnmimage_png_cat.error()
    << error_msg << "\n";

  // The PNG library insists we should not return, so instead of returning, we
  // will do a longjmp out of the png code.
  Reader *self = (Reader *)png_get_io_ptr(png_ptr);
  if (self == nullptr) {
    // Oops, we haven't got a self pointer yet.  Return anyway and hope we'll
    // be ok.
    pnmimage_png_cat.error()
      << "Returning before opening file.\n";
    return;
  }

  longjmp(self->_jmpbuf, true);
}

/**
 *
 */
PNMFileTypePNG::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
  _png = nullptr;
  _info = nullptr;
  _is_valid = false;

  _png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                 png_error, png_warning);
  if (_png == nullptr) {
    return;
  }

  _info = png_create_info_struct(_png);
  if (_info == nullptr) {
    png_destroy_write_struct(&_png, nullptr);
    return;
  }

  _is_valid = true;
}

/**
 *
 */
PNMFileTypePNG::Writer::
~Writer() {
  free_png();
}

/**
 * Writes in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * write.
 *
 * Derived classes need not override this if they instead provide
 * supports_write_row() and write_row(), below.
 */
int PNMFileTypePNG::Writer::
write_data(xel *array, xelval *alpha_data) {
  if (!is_valid()) {
    return 0;
  }

  if (setjmp(_jmpbuf)) {
    // This is the ANSI C way to handle exceptions.  If setjmp(), above,
    // returns true, it means that libpng detected an exception while
    // executing the code that writes the image, below.
    free_png();
    return 0;
  }

  png_set_write_fn(_png, (void *)this, png_write_data, png_flush_data);

  // The compression level corresponds directly to the compression levels for
  // zlib.
  png_set_compression_level(_png, png_compression_level);

  // First, write the header.

  int true_bit_depth = pm_maxvaltobits(_maxval);
  int png_bit_depth = make_png_bit_depth(true_bit_depth);

  png_color_8 sig_bit;
  sig_bit.red = true_bit_depth;
  sig_bit.green = true_bit_depth;
  sig_bit.blue = true_bit_depth;
  sig_bit.gray = true_bit_depth;
  sig_bit.alpha = true_bit_depth;

  int color_type = 0;

  if (!is_grayscale()) {
    color_type |= PNG_COLOR_MASK_COLOR;
  }
  if (has_alpha()) {
    color_type |= PNG_COLOR_MASK_ALPHA;
  }

  // Determine if we should make a palettized image out of this.  In order for
  // this to be possible and effective, we must have no more than 256 unique
  // coloralpha combinations for a color image, and the resulting bitdepth
  // should be smaller than what we would have otherwise.
  Palette palette;
  HistMap palette_lookup;
  png_color png_palette_table[png_max_palette];
  png_byte png_trans[png_max_palette];

  if (png_palette) {
    if (png_bit_depth <= 8) {
      if (compute_palette(palette, array, alpha_data, png_max_palette)) {
        if (pnmimage_png_cat.is_debug()) {
          pnmimage_png_cat.debug()
            << palette.size() << " colors found.\n";
        }

        int palette_bit_depth = make_png_bit_depth(pm_maxvaltobits(palette.size() - 1));

        int total_bits = png_bit_depth;
        if (!is_grayscale()) {
          total_bits *= 3;
        }
        if (has_alpha()) {
          total_bits += png_bit_depth;
        }

        if (palette_bit_depth < total_bits ||
            _maxval != (1 << true_bit_depth) - 1) {
          if (pnmimage_png_cat.is_debug()) {
            pnmimage_png_cat.debug()
              << "palette bit depth of " << palette_bit_depth
              << " improves on bit depth of " << total_bits
              << "; making a palette image.\n";
          }

          color_type = PNG_COLOR_TYPE_PALETTE;

          // Re-sort the palette to put the semitransparent pixels at the
          // beginning.
          sort(palette.begin(), palette.end(), LowAlphaCompare());

          double palette_scale = 255.0 / _maxval;

          int num_alpha = 0;
          for (int i = 0; i < (int)palette.size(); i++) {
            png_palette_table[i].red = (int)(palette[i]._red * palette_scale + 0.5);
            png_palette_table[i].green = (int)(palette[i]._green * palette_scale + 0.5);
            png_palette_table[i].blue = (int)(palette[i]._blue * palette_scale + 0.5);
            png_trans[i] = (int)(palette[i]._alpha * palette_scale + 0.5);
            if (palette[i]._alpha != _maxval) {
              num_alpha = i + 1;
            }

            // Also build a reverse-lookup from color to palette index in the
            // "histogram" structure.
            palette_lookup[palette[i]] = i;
          }

          png_set_PLTE(_png, _info, png_palette_table, palette.size());
          if (has_alpha()) {
            if (pnmimage_png_cat.is_debug()) {
              pnmimage_png_cat.debug()
                << "palette contains " << num_alpha << " transparent entries.\n";
            }
            png_set_tRNS(_png, _info, png_trans, num_alpha, nullptr);
          }
        } else if (pnmimage_png_cat.is_debug()) {
          pnmimage_png_cat.debug()
            << "palette bit depth of " << palette_bit_depth
            << " does not improve on bit depth of " << total_bits
            << "; not making a palette image.\n";
        }

      } else if (pnmimage_png_cat.is_debug()) {
        pnmimage_png_cat.debug()
          << "more than " << png_max_palette
          << " colors found; not making a palette image.\n";
      }
    } else if (pnmimage_png_cat.is_debug()) {
      pnmimage_png_cat.debug()
        << "maxval exceeds 255; not making a palette image.\n";
    }
  } else if (pnmimage_png_cat.is_debug()) {
    pnmimage_png_cat.debug()
      << "palette images are not enabled.\n";
  }

  if (pnmimage_png_cat.is_debug()) {
    pnmimage_png_cat.debug()
      << "width = " << _x_size << " height = " << _y_size
      << " maxval = " << _maxval << " bit_depth = "
      << png_bit_depth << " color_type = " << color_type
      << " color_space = " << _color_space << "\n";
  }

  png_set_IHDR(_png, _info, _x_size, _y_size, png_bit_depth,
               color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  // Set the true bit depth of the image data.
  if (png_bit_depth != true_bit_depth || color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_sBIT(_png, _info, &sig_bit);
  }

  // Set the color space, if we know it.
  switch (_color_space) {
  case CS_linear:
    png_set_gAMA(_png, _info, 1.0);
    // Not sure if we should set cHRM to anything.
    break;

  case CS_sRGB:
    png_set_sRGB_gAMA_and_cHRM(_png, _info, PNG_sRGB_INTENT_RELATIVE);
    break;

  default:
    break;
  }

  png_write_info(_png, _info);


  // Now set up the transformations to write the image data.
  if (png_bit_depth < 8) {
    png_set_packing(_png);
  }

  double val_scale = 1.0;

  if (color_type != PNG_COLOR_TYPE_PALETTE) {
    if (png_bit_depth != true_bit_depth) {
      png_set_shift(_png, &sig_bit);
    }
    // Since this assumes that _maxval is one less than a power of 2, we set
    // val_scale to the appropriate factor in case it is not.
    int png_maxval = (1 << png_bit_depth) - 1;
    val_scale = (double)png_maxval / (double)_maxval;
  }

  int row_byte_length = _x_size * _num_channels;
  if (png_bit_depth > 8) {
    row_byte_length *= 2;
  }

  int num_rows = _y_size;

  if (pnmimage_png_cat.is_debug()) {
    pnmimage_png_cat.debug()
      << "Allocating one row of " << row_byte_length
      << " bytes.\n";
  }

  // When writing, we only need to copy the image out one row at a time,
  // because we don't mess around with writing interlaced files.  If we were
  // writing an interlaced file, we'd have to copy the whole image first.

  png_bytep row = (png_byte *)PANDA_MALLOC_ARRAY(row_byte_length * sizeof(png_byte));

  bool save_color = !is_grayscale();
  bool save_alpha = has_alpha();

  if (val_scale == 1.0) {
    // No scale needed; we're already a power of 2.
    int pi = 0;
    for (int yi = 0; yi < num_rows; yi++) {
      png_bytep dest = row;

      if (color_type == PNG_COLOR_TYPE_PALETTE) {
        for (int xi = 0; xi < _x_size; xi++) {
          int index;

          if (save_color) {
            if (save_alpha) {
              index = palette_lookup[PixelSpec(PPM_GETR(array[pi]), PPM_GETG(array[pi]), PPM_GETB(array[pi]), alpha_data[pi])];
            } else {
              index = palette_lookup[PixelSpec(PPM_GETR(array[pi]), PPM_GETG(array[pi]), PPM_GETB(array[pi]))];
            }
          } else {
            if (save_alpha) {
              index = palette_lookup[PixelSpec(PPM_GETB(array[pi]), alpha_data[pi])];
            } else {
              index = palette_lookup[PixelSpec(PPM_GETB(array[pi]))];
            }
          }

          *dest++ = index;
          pi++;
        }

      } else if (png_bit_depth > 8) {
        for (int xi = 0; xi < _x_size; xi++) {
          if (save_color) {
            xelval red = PPM_GETR(array[pi]);
            *dest++ = (red >> 8) & 0xff;
            *dest++ = red & 0xff;
            xelval green = PPM_GETG(array[pi]);
            *dest++ = (green >> 8) & 0xff;
            *dest++ = green & 0xff;
          }
          xelval blue = PPM_GETB(array[pi]);
          *dest++ = (blue >> 8) & 0xff;
          *dest++ = blue & 0xff;

          if (save_alpha) {
            xelval alpha = alpha_data[pi];
            *dest++ = (alpha >> 8) & 0xff;
            *dest++ = alpha & 0xff;
          }
          pi++;
        }

      } else {
        for (int xi = 0; xi < _x_size; xi++) {
          if (save_color) {
            *dest++ = PPM_GETR(array[pi]);
            *dest++ = PPM_GETG(array[pi]);
          }

          *dest++ = PPM_GETB(array[pi]);

          if (save_alpha) {
            *dest++ = alpha_data[pi];
          }
          pi++;
        }
      }

      nassertr(dest <= row + row_byte_length, yi);
      png_write_row(_png, row);
      Thread::consider_yield();
    }
  } else {
    // Here we might need to scale each component to match the png
    // requirement.
    nassertr(color_type != PNG_COLOR_TYPE_PALETTE, 0);
    int pi = 0;
    for (int yi = 0; yi < num_rows; yi++) {
      png_bytep dest = row;

      if (png_bit_depth > 8) {
        for (int xi = 0; xi < _x_size; xi++) {
          if (save_color) {
            xelval red = (xelval)(PPM_GETR(array[pi]) * val_scale + 0.5);
            *dest++ = (red >> 8) & 0xff;
            *dest++ = red & 0xff;
            xelval green = (xelval)(PPM_GETG(array[pi]) * val_scale + 0.5);
            *dest++ = (green >> 8) & 0xff;
            *dest++ = green & 0xff;
          }
          xelval blue = (xelval)(PPM_GETB(array[pi]) * val_scale + 0.5);
          *dest++ = (blue >> 8) & 0xff;
          *dest++ = blue & 0xff;

          if (save_alpha) {
            xelval alpha = (xelval)(alpha_data[pi] * val_scale + 0.5);
            *dest++ = (alpha >> 8) & 0xff;
            *dest++ = alpha & 0xff;
          }
          pi++;
        }

      } else {
        for (int xi = 0; xi < _x_size; xi++) {
          if (save_color) {
            *dest++ = (xelval)(PPM_GETR(array[pi]) * val_scale + 0.5);
            *dest++ = (xelval)(PPM_GETG(array[pi]) * val_scale + 0.5);
          }

          *dest++ = (xelval)(PPM_GETB(array[pi]) * val_scale + 0.5);

          if (save_alpha) {
            *dest++ = (xelval)(alpha_data[pi] * val_scale + 0.5);
          }
          pi++;
        }
      }

      nassertr(dest <= row + row_byte_length, yi);
      png_write_row(_png, row);
      Thread::consider_yield();
    }
  }
  PANDA_FREE_ARRAY(row);

  png_write_end(_png, nullptr);

  return _y_size;
}

/**
 * Releases the internal PNG structures and marks the writer invalid.
 */
void PNMFileTypePNG::Writer::
free_png() {
  if (_is_valid) {
    png_destroy_write_struct(&_png, &_info);
    _is_valid = false;
  }
}

/**
 * Elevates the indicated bit depth to one of the legal PNG bit depths: 1, 2,
 * 4, 8, or 16.
 */
int PNMFileTypePNG::Writer::
make_png_bit_depth(int bit_depth) {
  switch (bit_depth) {
  case 0:
  case 1:
    return 1;

  case 2:
    return 2;

  case 3:
  case 4:
    return 4;

  case 5:
  case 6:
  case 7:
  case 8:
    return 8;

  default:
    return 16;
  }
}

/**
 * A callback handler that PNG uses to write data to the iostream.
 */
void PNMFileTypePNG::Writer::
png_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
  Writer *self = (Writer *)png_get_io_ptr(png_ptr);
  self->_file->write((char *)data, length);
  if (self->_file->fail()) {
    pnmimage_png_cat.error()
      << "Unable to write to the iostream.\n";
    // Is there no way to indicate a write failure to libpng?
  }
}

/**
 * A callback handler that PNG uses to write data to the iostream.
 */
void PNMFileTypePNG::Writer::
png_flush_data(png_structp png_ptr) {
  Writer *self = (Writer *)png_get_io_ptr(png_ptr);
  self->_file->flush();
}

/**
 * This is our own warning handler.  It is called by the png library to issue
 * a warning message.
 */
void PNMFileTypePNG::Writer::
png_warning(png_structp, png_const_charp warning_msg) {
  pnmimage_png_cat.warning()
    << warning_msg << "\n";
}

/**
 * This is our own error handler.  It is called by the png library to issue a
 * fatal error message.
 */
void PNMFileTypePNG::Writer::
png_error(png_structp png_ptr, png_const_charp error_msg) {
  pnmimage_png_cat.error()
    << error_msg << "\n";

  // The PNG library insists we should not return, so instead of returning, we
  // will do a longjmp out of the png code.
  Writer *self = (Writer *)png_get_io_ptr(png_ptr);
  if (self == nullptr) {
    // Oops, we haven't got a self pointer yet.  Return anyway and hope we'll
    // be ok.
    pnmimage_png_cat.error()
      << "Returning before opening file.\n";
    return;
  }

  longjmp(self->_jmpbuf, true);
}


#endif  // HAVE_PNG
