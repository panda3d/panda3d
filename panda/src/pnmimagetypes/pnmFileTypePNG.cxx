// Filename: pnmFileTypePNG.cxx
// Created by:  drose (16Mar04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pnmFileTypePNG.h"

#ifdef HAVE_PNG

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"
#include "ppmcmap.h"

static const char * const extensions_png[] = {
  "png"
};
static const int num_extensions_png = sizeof(extensions_png) / sizeof(const char *);

TypeHandle PNMFileTypePNG::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePNG::
PNMFileTypePNG() {
  // This constructor may run at static init time, so we use the ->
  // dereferencing convention on the notify category.
  if (pnmimage_png_cat->is_debug()) {
    pnmimage_png_cat->debug()
      << "PNG version " << png_libpng_ver << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNG::
get_name() const {
  return "PNG";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypePNG::
get_num_extensions() const {
  return num_extensions_png;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNG::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_png, string());
  return extensions_png[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNG::
get_suggested_extension() const {
  return "png";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNG::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNG::
matches_magic_number(const string &magic_number) const {
  return png_sig_cmp((png_bytep)magic_number.data(), 0, magic_number.length()) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypePNG::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypePNG::
make_writer(ostream *file, bool owns_file) {
  //  return new Writer(this, file, owns_file);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PNMFileTypePNG::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypePNG);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::make_PNMFileTypePNG
//       Access: Protected, Static
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
//
//               In the case of the PNMFileType objects, since these
//               objects are all shared, we just pull the object from
//               the registry.
////////////////////////////////////////////////////////////////////
TypedWritable *PNMFileTypePNG::
make_PNMFileTypePNG(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_ptr()->get_type_by_handle(get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePNG::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  _png = NULL;
  _info = NULL;
  _is_valid = false;

  _png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                png_error, png_warning);
  if (_png == NULL) {
    return;
  }

  _info = png_create_info_struct(_png);
  if (_info == NULL) {
    png_destroy_read_struct(&_png, NULL, NULL);
    return;
  }

  _is_valid = true;

  if (setjmp(_jmpbuf)) {
    // This is the ANSI C way to handle exceptions.  If setjmp(),
    // above, returns true, it means that libpng detected an exception
    // while executing the code that reads the header info, below.
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

  png_get_IHDR(_png, _info, &width, &height,
               &bit_depth, &color_type, NULL, NULL, NULL);

  pnmimage_png_cat.debug()
    << "width = " << width << " height = " << height << " bit_depth = "
    << bit_depth << " color_type = " << color_type << "\n";

  _x_size = width;
  _y_size = height;
  _maxval = ( 1 << bit_depth ) - 1;

  if (bit_depth < 8) {
    png_set_packing(_png);
  }

  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    pnmimage_png_cat.debug()
      << "PNG_COLOR_TYPE_GRAY\n";
    _num_channels = 1;
    break;

  case PNG_COLOR_TYPE_GRAY_ALPHA:
    pnmimage_png_cat.debug()
      << "PNG_COLOR_TYPE_GRAY_ALPHA\n";
    _num_channels = 2;
    break;

  case PNG_COLOR_TYPE_RGB:
    pnmimage_png_cat.debug()
      << "PNG_COLOR_TYPE_RGB\n";
    _num_channels = 3;
    break;

  case PNG_COLOR_TYPE_RGB_ALPHA:
    pnmimage_png_cat.debug()
      << "PNG_COLOR_TYPE_RGB_ALPHA\n";
    _num_channels = 4;
    break;

  case PNG_COLOR_TYPE_PALETTE:
    pnmimage_png_cat.debug()
      << "PNG_COLOR_TYPE_PALETTE\n";
    png_set_palette_to_rgb(_png);
    _maxval = 255;
    _num_channels = 3;
    return;

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

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePNG::Reader::
~Reader() {
  free_png();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::read_data
//       Access: Public, Virtual
//  Description: Reads in an entire image all at once, storing it in
//               the pre-allocated _x_size * _y_size array and alpha
//               pointers.  (If the image type has no alpha channel,
//               alpha is ignored.)  Returns the number of rows
//               correctly read.
//
//               Derived classes need not override this if they
//               instead provide supports_read_row() and read_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMFileTypePNG::Reader::
read_data(xel *array, xelval *alpha_data) {
  if (!is_valid()) {
    return 0;
  }

  if (setjmp(_jmpbuf)) {
    // This is the ANSI C way to handle exceptions.  If setjmp(),
    // above, returns true, it means that libpng detected an exception
    // while executing the code that reads the image, below.
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
    
  // We need to read a full copy of the image in first, in libpng's
  // 2-d array format, mainly because we keep array and alpha data
  // separately, and there doesn't appear to be good support to get
  // this stuff out row-at-a-time for interlaced files.
  png_bytep *rows = new png_bytep[num_rows];
  int yi;

  for (yi = 0; yi < num_rows; yi++) {
    rows[yi] = new png_byte[row_byte_length];
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
          red = (source[0] << 16) | source[1];
          source += 2;

          green = (source[0] << 16) | source[1];
          source += 2;
        }

        blue = (source[0] << 16) | source[1];
        source += 2;

        if (get_alpha) {
          alpha = (source[0] << 16) | source[1];
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
    delete[] rows[yi];
  }

  delete[] rows;

  png_read_end(_png, NULL);

  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::free_png
//       Access: Private
//  Description: Releases the internal PNG structures and marks the
//               reader invalid.
////////////////////////////////////////////////////////////////////
void PNMFileTypePNG::Reader::
free_png() {
  if (_is_valid) {
    png_destroy_read_struct(&_png, &_info, NULL);
    _is_valid = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::png_read_data
//       Access: Private, Static
//  Description: A callback handler that PNG uses to read data from
//               the iostream.
////////////////////////////////////////////////////////////////////
void PNMFileTypePNG::Reader::
png_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
  Reader *self = (Reader *)png_get_io_ptr(png_ptr);
  self->_file->read((char *)data, length);
  if (length != (png_size_t)self->_file->gcount()) {
    pnmimage_png_cat.error()
      << "Didn't read enough bytes.\n";
    // Is there no way to indicate a read failure to libpng?
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::png_warning
//       Access: Private, Static
//  Description: This is our own warning handler.  It is called by the
//               png library to issue a warning message.
////////////////////////////////////////////////////////////////////
void PNMFileTypePNG::Reader::
png_warning(png_structp, png_const_charp warning_msg) {
  pnmimage_png_cat.warning()
    << warning_msg << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNG::Reader::png_error
//       Access: Private, Static
//  Description: This is our own error handler.  It is called by the
//               png library to issue a fatal error message.
////////////////////////////////////////////////////////////////////
void PNMFileTypePNG::Reader::
png_error(png_structp png_ptr, png_const_charp error_msg) {
  pnmimage_png_cat.error()
    << error_msg << "\n";

  // The PNG library insists we should not return, so instead of
  // returning, we will do a longjmp out of the png code.
  Reader *self = (Reader *)png_get_io_ptr(png_ptr);
  if (self == (Reader *)NULL) {
    // Oops, we haven't got a self pointer yet.  Return anyway and
    // hope we'll be ok.
    pnmimage_png_cat.error()
      << "Returning before opening file.\n";
    return;
  }

  longjmp(self->_jmpbuf, true);
}


#endif  // HAVE_PNG
