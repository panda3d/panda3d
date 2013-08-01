// Filename: pnmImage.cxx
// Created by:  drose (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "pnmBrush.h"
#include "pfmFile.h"
#include "config_pnmimage.h"
#include "perlinNoise2.h"
#include "stackedPerlinNoise2.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PNMImage::
PNMImage(const Filename &filename, PNMFileType *type) {
  _array = NULL;
  _alpha = NULL;
  _has_read_size = false;

  bool result = read(filename, type);
  if (!result) {
    pnmimage_cat.error()
      << "Could not read image " << filename << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clear
//       Access: Published
//  Description: Frees all memory allocated for the image, and clears
//               all its parameters (size, color, type, etc).
////////////////////////////////////////////////////////////////////
void PNMImage::
clear() {
  if (_array != (xel *)NULL) {
    PANDA_FREE_ARRAY(_array);
    _array = (xel *)NULL;
  }
  if (_alpha != (xelval *)NULL) {
    PANDA_FREE_ARRAY(_alpha);
    _alpha = (xelval *)NULL;
  }
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _maxval = 255;
  _comment.clear();
  _type = (PNMFileType *)NULL;
  _has_read_size = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clear
//       Access: Published
//  Description: This flavor of clear() reinitializes the image to an
//               empty (black) image with the given dimensions.
////////////////////////////////////////////////////////////////////
void PNMImage::
clear(int x_size, int y_size, int num_channels,
      xelval maxval, PNMFileType *type) {
  clear();
  nassertv(num_channels >= 1 && num_channels <= 4);

  _x_size = x_size;
  _y_size = y_size;
  _num_channels = num_channels;
  _maxval = maxval;
  _comment.clear();
  _type = type;
  _has_read_size = false;

  if (has_alpha()) {
    allocate_alpha();
    memset(_alpha, 0, sizeof(xelval) * _y_size * _x_size);
  }

  allocate_array();
  memset(_array, 0, sizeof(xel) * _y_size * _x_size);

  setup_rc();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_from
//       Access: Published
//  Description: Makes this image become a copy of the other image.
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_from(const PNMImage &copy) {
  clear();
  copy_header_from(copy);

  if (copy.is_valid()) {
    if (has_alpha()) {
      memcpy(_alpha, copy._alpha, sizeof(xelval) * _y_size * _x_size);
    }
    memcpy(_array, copy._array, sizeof(xel) * _y_size * _x_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_channel
//       Access: Published
//  Description: Copies a channel from one image into another.
//               Images must be the same size
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_channel(const PNMImage &copy, int src_channel, int dest_channel) {
  // Make sure the channels are in range
  nassertv(src_channel >= 0 && src_channel <= 3);
  nassertv(dest_channel >= 0 && dest_channel <= 3);
  // Make sure that images are valid
  if (!copy.is_valid() || !is_valid()) {
    pnmimage_cat.error() << "One of the images is invalid!\n";
    return;
  }
  // Make sure the images are the same size
  if (_x_size != copy.get_x_size() || _y_size != copy.get_y_size()){
    pnmimage_cat.error() << "Image size doesn't match!\n";
    return;
  }
  // Do the actual copying
  for (int x = 0; x < _x_size; x++) {
    for (int y = 0; y < _y_size; y++) {
      LVecBase4d t = get_xel_a(x, y);
      LVecBase4d o = copy.get_xel_a(x, y);
      t.set_cell(dest_channel,o.get_cell(src_channel));
      set_xel_a(x, y, t);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_header_from
//       Access: Published
//  Description: Copies just the header information into this image.
//               This will blow away any image data stored in the
//               image.  The new image data will be allocated, but
//               left unitialized.
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_header_from(const PNMImageHeader &header) {
  clear();
  PNMImageHeader::operator = (header);

  if (has_alpha()) {
    allocate_alpha();
  }

  allocate_array();
  setup_rc();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::take_from
//       Access: Published
//  Description: Move the contents of the other image into this one,
//               and empty the other image.
////////////////////////////////////////////////////////////////////
void PNMImage::
take_from(PNMImage &orig) {
  clear();
  PNMImageHeader::operator = (orig);
  setup_rc();

  if (has_alpha()) {
    _alpha = orig._alpha;
    orig._alpha = NULL;
  }
  _array = orig._array;
  orig._array = NULL;

  orig.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill_val
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given color.
////////////////////////////////////////////////////////////////////
void PNMImage::
fill_val(xelval red, xelval green, xelval blue) {
  if (is_valid()) {
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        set_xel_val(x, y, red, green, blue);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::alpha_fill_val
//       Access: Published
//  Description: Sets the entire alpha channel to the given level.
////////////////////////////////////////////////////////////////////
void PNMImage::
alpha_fill_val(xelval alpha) {
  if (is_valid()) {
    if (!has_alpha()) {
      add_alpha();
    }

    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        set_alpha_val(x, y, alpha);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::read
//       Access: Published
//  Description: Reads the indicated image filename.  If type is
//               non-NULL, it is a suggestion for the type of file it
//               is.  Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(const Filename &filename, PNMFileType *type, bool report_unknown_type) {
  PNMReader *reader = make_reader(filename, type, report_unknown_type);
  if (reader == (PNMReader *)NULL) {
    clear();
    return false;
  }

  return read(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::read
//       Access: Published
//  Description: Reads the image data from the indicated stream.
//
//               The filename is advisory only, and may be used
//               to suggest a type if it has a known extension.
//
//               If type is non-NULL, it is a suggestion for the type
//               of file it is (and a non-NULL type will override any
//               magic number test or filename extension lookup).
//
//               Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(istream &data, const string &filename, PNMFileType *type,
     bool report_unknown_type) {
  PNMReader *reader = PNMImageHeader::make_reader
    (&data, false, filename, string(), type, report_unknown_type);
  if (reader == (PNMReader *)NULL) {
    clear();
    return false;
  }
  return read(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::read
//       Access: Published
//  Description: This flavor of read() uses an already-existing
//               PNMReader to read the image file.  You can get a
//               reader via the PNMImageHeader::make_reader() methods.
//               This is a good way to examine the header of a file
//               (for instance, to determine its size) before actually
//               reading the entire image.
//
//               The PNMReader is always deleted upon completion,
//               whether successful or not.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(PNMReader *reader) {
  bool has_read_size = _has_read_size;
  int read_x_size = _read_x_size;
  int read_y_size = _read_y_size;

  clear();

  if (reader == NULL) {
    return false;
  }

  if (!reader->is_valid()) {
    delete reader;
    return false;
  }

  if (has_read_size) {
    reader->set_read_size(read_x_size, read_y_size);
  }
  reader->prepare_read();

  copy_header_from(*reader);

  if (reader->is_floating_point()) {
    // Hmm, it's a floating-point file.  Quietly convert it to integer.
    PfmFile pfm;
    if (!reader->read_pfm(pfm)) {
      delete reader;
      return false;
    }
    delete reader;
    return pfm.store(*this);
  }

  // We reassign y_size after reading because we might have read a
  // truncated file.
  _y_size = reader->read_data(_array, _alpha);
  delete reader;

  if (_y_size == 0) {
    clear();
    return false;
  }

  setup_rc();

  if (has_read_size && (_x_size != read_x_size || _y_size != read_y_size)) {
    // The Reader didn't comply with our size request.  Do the sizing
    // explicitly, then.
    PNMImage new_image(read_x_size, read_y_size, get_num_channels(),
                       get_maxval(), get_type());
    new_image.quick_filter_from(*this);
    take_from(new_image);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::write
//       Access: Published
//  Description: Writes the image to the indicated filename.  If type
//               is non-NULL, it is a suggestion for the type of image
//               file to write.
////////////////////////////////////////////////////////////////////
bool PNMImage::
write(const Filename &filename, PNMFileType *type) const {
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = PNMImageHeader::make_writer(filename, type);
  if (writer == (PNMWriter *)NULL) {
    return false;
  }

  return write(writer);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::write
//       Access: Published
//  Description: Writes the image to the indicated ostream.
//
//               The filename is advisory only, and may be used
//               suggest a type if it has a known extension.
//
//               If type is non-NULL, it is a suggestion for the type
//               of image file to write.
////////////////////////////////////////////////////////////////////
bool PNMImage::
write(ostream &data, const string &filename, PNMFileType *type) const {
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = PNMImageHeader::make_writer
    (&data, false, filename, type);
  if (writer == (PNMWriter *)NULL) {
    return false;
  }

  return write(writer);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::write
//       Access: Published
//  Description: This flavor of write() uses an already-existing
//               PNMWriter to write the image file.  You can get a
//               writer via the PNMImageHeader::make_writer() methods.
//
//               The PNMWriter is always deleted upon completion,
//               whether successful or not.
////////////////////////////////////////////////////////////////////
bool PNMImage::
write(PNMWriter *writer) const {
  if (writer == NULL) {
    return false;
  }

  if (!is_valid()) {
    delete writer;
    return false;
  }

  writer->copy_header_from(*this);

  if (!writer->supports_integer()) {
    // Hmm, it's only a floating-point file type.  Convert it from the
    // integer data we have.
    PfmFile pfm;
    if (!pfm.load(*this)) {
      delete writer;
      return false;
    }
    bool success = writer->write_pfm(pfm);
    delete writer;
    return success;
  }

  if (is_grayscale() && !writer->supports_grayscale()) {
    // Copy the gray values to all channels to help out the writer.
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x<get_x_size(); x++) {
        ((PNMImage *)this)->set_xel_val(x, y, get_gray_val(x, y));
      }
    }
  }

  int result = writer->write_data(_array, _alpha);
  delete writer;

  return (result == _y_size);
}



////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_color_type
//       Access: Published
//  Description: Translates the image to or from grayscale, color, or
//               four-color mode.  Grayscale images are converted to
//               full-color images with R, G, B set to the original
//               gray level; color images are converted to grayscale
//               according to the value of Bright().  The alpha
//               channel, if added, is initialized to zero.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_color_type(PNMImage::ColorType color_type) {
  nassertv((int)color_type >= 1 && (int)color_type <= 4);
  if (color_type == get_color_type()) {
    return;
  }

  if (!is_grayscale() && is_grayscale(color_type)) {
    // convert to grayscale from color
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        set_gray(x, y, get_bright(x, y));
      }
    }

  } else if (is_grayscale() && !is_grayscale(color_type)) {
    // convert to color from grayscale
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x<get_x_size(); x++) {
        set_xel_val(x, y, get_gray_val(x, y));
      }
    }
  }

  if (has_alpha() && !has_alpha(color_type)) {
    // discard the alpha channel
    if (_alpha!=NULL) {
      PANDA_FREE_ARRAY(_alpha);
      _alpha = NULL;
    }

  } else if (!has_alpha() && has_alpha(color_type)) {
    // create a new alpha channel
    allocate_alpha();
    memset(_alpha, 0, sizeof(xelval) * (_x_size * _y_size));
  }

  _num_channels = (int)color_type;
  setup_rc();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_grayscale
//       Access: Published
//  Description: Converts the image from RGB to grayscale.  Any alpha
//               channel, if present, is left undisturbed.  The
//               optional rc, gc, bc values represent the relative
//               weights to apply to each channel to convert it to
//               grayscale.
////////////////////////////////////////////////////////////////////
void PNMImage::
make_grayscale(double rc, double gc, double bc) {
  if (is_grayscale()) {
    return;
  }

  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      set_gray(x, y, min(get_bright(x, y, rc, gc, bc), 1.0));
    }
  }

  _num_channels = has_alpha() ? 2 : 1;
  setup_rc();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::reverse_rows
//       Access: Published
//  Description: Performs an in-place reversal of the row (y) data.
////////////////////////////////////////////////////////////////////
void PNMImage::
reverse_rows() {
  if (_array != NULL) {
    xel *new_array = (xel *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xel));
    for (int y = 0; y < _y_size; y++) {
      int new_y = _y_size - 1 - y;
      memcpy(new_array + new_y * _x_size, _array + y * _x_size, _x_size * sizeof(xel));
    }
    PANDA_FREE_ARRAY(_array);
    _array = new_array;
  }

  if (_alpha != NULL) {
    xelval *new_alpha = (xelval *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xelval));
    for (int y = 0; y < _y_size; y++) {
      int new_y = _y_size - 1 - y;
      memcpy(new_alpha + new_y * _x_size, _alpha + y * _x_size, _x_size * sizeof(xelval));
    }
    PANDA_FREE_ARRAY(_alpha);
    _alpha = new_alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::flip
//       Access: Published
//  Description: Reverses, transposes, and/or rotates the image
//               in-place according to the specified parameters.  If
//               flip_x is true, the x axis is reversed; if flip_y is
//               true, the y axis is reversed.  Then, if transpose is
//               true, the x and y axes are exchanged.  These
//               parameters can be used to select any combination of
//               90-degree or 180-degree rotations and flips.
////////////////////////////////////////////////////////////////////
void PNMImage::
flip(bool flip_x, bool flip_y, bool transpose) {
  if (transpose) {
    // Transposed case.  X becomes Y, Y becomes X.
    if (_array != NULL) {
      xel *new_array = (xel *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xel));
      for (int xi = 0; xi < _x_size; ++xi) {
        xel *row = new_array + xi * _y_size;
        int source_xi = !flip_x ? xi : _x_size - 1 - xi;
        for (int yi = 0; yi < _y_size; ++yi) {
          int source_yi = !flip_y ? yi : _y_size - 1 - yi;
          xel *source_row = _array + source_yi * _x_size;
          row[yi] = source_row[source_xi];
        }
      }
      PANDA_FREE_ARRAY(_array);
      _array = new_array;
    }

    if (_alpha != NULL) {
      xelval *new_alpha = (xelval *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xelval));
      for (int xi = 0; xi < _x_size; ++xi) {
        xelval *row = new_alpha + xi * _y_size;
        int source_xi = !flip_x ? xi : _x_size - 1 - xi;
        for (int yi = 0; yi < _y_size; ++yi) {
          int source_yi = !flip_y ? yi : _y_size - 1 - yi;
          xelval *source_row = _alpha + source_yi * _x_size;
          row[yi] = source_row[source_xi];
        }
      }

      PANDA_FREE_ARRAY(_alpha);
      _alpha = new_alpha;
    }

    int t = _x_size;
    _x_size = _y_size;
    _y_size = t;

  } else {
    // Non-transposed.  X is X, Y is Y.
    if (_array != NULL) {
      xel *new_array = (xel *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xel));
      for (int yi = 0; yi < _y_size; ++yi) {
        xel *row = new_array + yi * _x_size;
        int source_yi = !flip_y ? yi : _y_size - 1 - yi;
        xel *source_row = _array + source_yi * _x_size;

        for (int xi = 0; xi < _x_size; ++xi) {
          int source_xi = !flip_x ? xi : _x_size - 1 - xi;
          row[xi] = source_row[source_xi];
        }
      }
      PANDA_FREE_ARRAY(_array);
      _array = new_array;
    }

    if (_alpha != NULL) {
      xelval *new_alpha = (xelval *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xelval));
      for (int yi = 0; yi < _y_size; ++yi) {
        xelval *row = new_alpha + yi * _x_size;
        int source_yi = !flip_y ? yi : _y_size - 1 - yi;
        xelval *source_row = _alpha + source_yi * _x_size;

        for (int xi = 0; xi < _x_size; ++xi) {
          int source_xi = !flip_x ? xi : _x_size - 1 - xi;
          row[xi] = source_row[source_xi];
        }
      }

      PANDA_FREE_ARRAY(_alpha);
      _alpha = new_alpha;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_maxval
//       Access: Published
//  Description: Rescales the image to the indicated maxval.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_maxval(xelval maxval) {
  nassertv(maxval > 0);

  if (maxval != _maxval) {
    double ratio = (double)maxval / (double)_maxval;

    if (is_grayscale()) {
      for (int y = 0; y < get_y_size(); y++) {
        for (int x = 0; x < get_x_size(); x++) {
          set_gray_val(x, y, (xelval)((long)get_gray_val(x, y) * ratio));
        }
      }
    } else {
      for (int y = 0; y < get_y_size(); y++) {
        for (int x = 0; x < get_x_size(); x++) {
          set_red_val(x, y, (xelval)((long)get_red_val(x, y) * ratio));
          set_green_val(x, y, (xelval)((long)get_green_val(x, y) * ratio));
          set_blue_val(x, y, (xelval)((long)get_blue_val(x, y) * ratio));
        }
      }
    }

    if (has_alpha()) {
      for (int y = 0; y < get_y_size(); y++) {
        for (int x = 0; x < get_x_size(); x++) {
          set_alpha_val(x, y,
                        (xelval)((long)get_alpha_val(x, y) * ratio));
        }
      }
    }
    _maxval = maxval;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_channel_val
//       Access: Published
//  Description: Returns the nth component color at the indicated
//               pixel.  The channel index should be in the range
//               0..(get_num_channels()-1).  The channels are ordered B,
//               G, R, A.  This is slightly less optimal than
//               accessing the component values directly by named
//               methods.  The value returned is in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
xelval PNMImage::
get_channel_val(int x, int y, int channel) const {
  switch (channel) {
  case 0:
    return get_blue_val(x, y);

  case 1:
    return (_num_channels == 2) ? get_alpha_val(x, y) : get_green_val(x, y);

  case 2:
    return get_red_val(x, y);

  case 3:
    return get_alpha_val(x, y);

  default:
    pnmimage_cat.error()
      << "Invalid request for channel " << channel << " in "
      << get_num_channels() << "-channel image.\n";
    nassertr(false, 0);
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_channel_val
//       Access: Published
//  Description: Sets the nth component color at the indicated
//               pixel.  The channel index should be in the range
//               0..(get_num_channels()-1).  The channels are ordered B,
//               G, R, A.  This is slightly less optimal than
//               setting the component values directly by named
//               methods.  The value given should be in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_channel_val(int x, int y, int channel, xelval value) {
  switch (channel) {
  case 0:
    set_blue_val(x, y, value);
    break;

  case 1:
    if (_num_channels == 2) {
      set_alpha_val(x, y, value);
    } else {
      set_green_val(x, y, value);
    }
    break;

  case 2:
    set_red_val(x, y, value);
    break;

  case 3:
    set_alpha_val(x, y, value);
    break;

  default:
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_pixel
//       Access: Published
//  Description: Returns the (r, g, b, a) pixel value at the indicated
//               pixel, using a PixelSpec object.
////////////////////////////////////////////////////////////////////
PNMImage::PixelSpec PNMImage::
get_pixel(int x, int y) const {
  switch (_num_channels) {
  case 1:
    return PixelSpec(get_gray_val(x, y));
  case 2:
    return PixelSpec(get_gray_val(x, y), get_alpha_val(x, y));
  case 3:
    return PixelSpec(get_xel_val(x, y));
  case 4:
    return PixelSpec(get_xel_val(x, y), get_alpha_val(x, y));
  }

  return PixelSpec(0);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_pixel
//       Access: Published
//  Description: Sets the (r, g, b, a) pixel value at the indicated
//               pixel, using a PixelSpec object.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_pixel(int x, int y, const PixelSpec &pixel) {
  xel p;
  PPM_ASSIGN(p, pixel._red, pixel._green, pixel._blue);
  set_xel_val(x, y, p);
  if (_alpha != NULL) {
    set_alpha_val(x, y, pixel._alpha);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::blend
//       Access: Published
//  Description: Smoothly blends the indicated pixel value in with
//               whatever was already in the image, based on the given
//               alpha value.  An alpha of 1.0 is fully opaque and
//               completely replaces whatever was there previously;
//               alpha of 0.0 is fully transparent and does nothing.
////////////////////////////////////////////////////////////////////
void PNMImage::
blend(int x, int y, double r, double g, double b, double alpha) {
  if (alpha >= 1.0) {
    // Completely replace the previous color.
    if (has_alpha()) {
      set_alpha(x, y, 1.0);
    }
    set_xel(x, y, r, g, b);

  } else if (alpha > 0.0) {
    // Blend with the previous color.
    double prev_alpha = has_alpha() ? get_alpha(x, y) : 1.0;

    if (prev_alpha == 0.0) {
      // Nothing there previously; replace with this new color.
      set_alpha(x, y, alpha);
      set_xel(x, y, r, g, b);

    } else {
      // Blend the color with the previous color.
      LRGBColord prev_rgb = get_xel(x, y);
      r = r + (1.0 - alpha) * (get_red(x, y) - r);
      g = g + (1.0 - alpha) * (get_green(x, y) - g);
      b = b + (1.0 - alpha) * (get_blue(x, y) - b);
      alpha = prev_alpha + alpha * (1.0 - prev_alpha);

      if (has_alpha()) {
        set_alpha(x, y, alpha);
      }
      set_xel(x, y, r, g, b);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_array
//       Access: Public
//  Description: Replaces the underlying PNMImage array with the
//               indicated pointer.  Know what you are doing!  The new
//               array must be the correct size and must have been
//               allocated via PANDA_MALLOC_ARRAY().  The PNMImage
//               object becomes the owner of this pointer and will
//               eventually free it with PANDA_FREE_ARRAY().  The
//               previous array, if any, will be freed with
//               PANDA_FREE_ARRAY() when this call is made.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_array(xel *array) {
  if (_array != (xel *)NULL) {
    PANDA_FREE_ARRAY(_array);
  }
  _array = array;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_alpha_array
//       Access: Public
//  Description: Replaces the underlying PNMImage alpha array with the
//               indicated pointer.  Know what you are doing!  The new
//               array must be the correct size and must have been
//               allocated via PANDA_MALLOC_ARRAY().  The PNMImage
//               object becomes the owner of this pointer and will
//               eventually free it with PANDA_FREE_ARRAY().  The
//               previous array, if any, will be freed with
//               PANDA_FREE_ARRAY() when this call is made.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_alpha_array(xelval *alpha) {
  if (_alpha != (xelval *)NULL) {
    PANDA_FREE_ARRAY(_alpha);
  }
  _alpha = alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_sub_image
//       Access: Published
//  Description: Copies a rectangular area of another image into a
//               rectangular area of this image.  Both images must
//               already have been initialized.  The upper-left corner
//               of the region in both images is specified, and the
//               size of the area; if the size is omitted, it defaults
//               to the entire other image, or the largest piece that
//               will fit.
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_sub_image(const PNMImage &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval()) {
    // The simple case: no pixel value rescaling is required.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        set_xel_val(x, y, copy.get_xel_val(x - xmin + xfrom, y - ymin + yfrom));
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          set_alpha_val(x, y, copy.get_alpha_val(x - xmin + xfrom, y - ymin + yfrom));
        }
      }
    }

  } else {
    // The harder case: rescale pixel values according to maxval.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        set_xel(x, y, copy.get_xel(x - xmin + xfrom, y - ymin + yfrom));
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          set_alpha(x, y, copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom));
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::blend_sub_image
//       Access: Published
//  Description: Behaves like copy_sub_image(), except the alpha
//               channel of the copy is used to blend the copy into
//               the destination image, instead of overwriting pixels
//               unconditionally.
//
//               If pixel_scale is not 1.0, it specifies an amount to
//               scale each *alpha* value of the source image before
//               applying it to the target image.
//
//               If pixel_scale is 1.0 and the copy has no alpha
//               channel, this degenerates into copy_sub_image().
////////////////////////////////////////////////////////////////////
void PNMImage::
blend_sub_image(const PNMImage &copy, int xto, int yto,
                int xfrom, int yfrom, int x_size, int y_size,
                double pixel_scale) {
  if (!copy.has_alpha() && pixel_scale == 1.0) {
    copy_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size);
    return;
  }

  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  if (copy.has_alpha()) {
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        blend(x, y, copy.get_xel(x - xmin + xfrom, y - ymin + yfrom),
              copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
      }
    }
  } else {
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        blend(x, y, copy.get_xel(x - xmin + xfrom, y - ymin + yfrom),
              pixel_scale);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::add_sub_image
//       Access: Published
//  Description: Behaves like copy_sub_image(), except the copy pixels
//               are added to the pixels of the destination, after
//               scaling by the specified pixel_scale.  Unlike
//               blend_sub_image(), the alpha channel is not treated
//               specially.
////////////////////////////////////////////////////////////////////
void PNMImage::
add_sub_image(const PNMImage &copy, int xto, int yto,
              int xfrom, int yfrom, int x_size, int y_size,
              double pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  if (has_alpha() && copy.has_alpha()) {
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        set_alpha(x, y, get_alpha(x, y) + copy.get_alpha(x, y) * pixel_scale);
      }
    }
  }

  for (y = ymin; y < ymax; y++) {
    for (x = xmin; x < xmax; x++) {
      LRGBColord rgb1 = get_xel(x, y);
      LRGBColord rgb2 = copy.get_xel(x, y);
      set_xel(x, y, 
              rgb1[0] + rgb2[0] * pixel_scale,
              rgb1[1] + rgb2[1] * pixel_scale,
              rgb1[2] + rgb2[2] * pixel_scale);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::mult_sub_image
//       Access: Published
//  Description: Behaves like copy_sub_image(), except the copy pixels
//               are multiplied to the pixels of the destination, after
//               scaling by the specified pixel_scale.  Unlike
//               blend_sub_image(), the alpha channel is not treated
//               specially.
////////////////////////////////////////////////////////////////////
void PNMImage::
mult_sub_image(const PNMImage &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size,
               double pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  if (has_alpha() && copy.has_alpha()) {
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        set_alpha(x, y, get_alpha(x, y) * copy.get_alpha(x, y) * pixel_scale);
      }
    }
  }

  for (y = ymin; y < ymax; y++) {
    for (x = xmin; x < xmax; x++) {
      LRGBColord rgb1 = get_xel(x, y);
      LRGBColord rgb2 = copy.get_xel(x, y);
      set_xel(x, y, 
              rgb1[0] * rgb2[0] * pixel_scale,
              rgb1[1] * rgb2[1] * pixel_scale,
              rgb1[2] * rgb2[2] * pixel_scale);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::darken_sub_image
//       Access: Published
//  Description: Behaves like copy_sub_image(), but the resulting
//               color will be the darker of the source and
//               destination colors at each pixel (and at each R, G,
//               B, A component value).
//
//               If pixel_scale is not 1.0, it specifies an amount to
//               scale each pixel value of the source image before
//               applying it to the target image.  The scale is
//               applied with the center at 1.0: scaling the pixel
//               value smaller brings it closer to 1.0.
////////////////////////////////////////////////////////////////////
void PNMImage::
darken_sub_image(const PNMImage &copy, int xto, int yto,
                 int xfrom, int yfrom, int x_size, int y_size,
                 double pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() && pixel_scale == 1.0) {
    // The simple case: no pixel value rescaling is required.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        xel c = copy.get_xel_val(x - xmin + xfrom, y - ymin + yfrom);
        xel o = get_xel_val(x, y);
        xel p;
        PPM_ASSIGN(p, min(c.r, o.r), min(c.g, o.g), min(c.b, o.b));
        set_xel_val(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          xelval c = copy.get_alpha_val(x - xmin + xfrom, y - ymin + yfrom);
          xelval o = get_alpha_val(x, y);
          set_alpha_val(x, y, min(c, o));
        }
      }
    }

  } else {
    // The harder case: rescale pixel values according to maxval.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        LRGBColord c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        LRGBColord o = get_xel(x, y);
        LRGBColord p;
        p.set(min(1.0 - ((1.0 - c[0]) * pixel_scale), o[0]),
              min(1.0 - ((1.0 - c[1]) * pixel_scale), o[1]),
              min(1.0 - ((1.0 - c[2]) * pixel_scale), o[2]));
        set_xel(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          double c = copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom);
          double o = get_alpha(x, y);
          set_alpha(x, y, min(1.0 - ((1.0 - c) * pixel_scale), o));
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::lighten_sub_image
//       Access: Published
//  Description: Behaves like copy_sub_image(), but the resulting
//               color will be the lighter of the source and
//               destination colors at each pixel (and at each R, G,
//               B, A component value).
//
//               If pixel_scale is not 1.0, it specifies an amount to
//               scale each pixel value of the source image before
//               applying it to the target image.
////////////////////////////////////////////////////////////////////
void PNMImage::
lighten_sub_image(const PNMImage &copy, int xto, int yto,
                  int xfrom, int yfrom, int x_size, int y_size,
                  double pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() && pixel_scale == 1.0) {
    // The simple case: no pixel value rescaling is required.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        xel c = copy.get_xel_val(x - xmin + xfrom, y - ymin + yfrom);
        xel o = get_xel_val(x, y);
        xel p;
        PPM_ASSIGN(p, max(c.r, o.r), max(c.g, o.g), max(c.b, o.b));
        set_xel_val(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          xelval c = copy.get_alpha_val(x - xmin + xfrom, y - ymin + yfrom);
          xelval o = get_alpha_val(x, y);
          set_alpha_val(x, y, max(c, o));
        }
      }
    }

  } else {
    // The harder case: rescale pixel values according to maxval.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        LRGBColord c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        LRGBColord o = get_xel(x, y);
        LRGBColord p;
        p.set(max(c[0] * pixel_scale, o[0]),
              max(c[1] * pixel_scale, o[1]),
              max(c[2] * pixel_scale, o[2]));
        set_xel(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          double c = copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom);
          double o = get_alpha(x, y);
          set_alpha(x, y, max(c * pixel_scale, o));
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::threshold
//       Access: Published
//  Description: Selectively copies each pixel from either one source
//               or another source, depending on the pixel value of
//               the indicated channel of select_image.
//
//               For each pixel (x, y):
//
//               s = select_image.get_channel(x, y, channel). Set this
//               image's (x, y) to:
//
//               lt.get_xel(x, y) if s < threshold, or
//
//               ge.get_xel(x, y) if s >= threshold
//
//               Any of select_image, lt, or ge may be the same
//               PNMImge object as this image, or the same as each
//               other; or they may all be different. All images must
//               be the same size.  As a special case, lt and ge may
//               both be 1x1 images instead of the source image size.
////////////////////////////////////////////////////////////////////
void PNMImage::
threshold(const PNMImage &select_image, int channel, double threshold,
          const PNMImage &lt, const PNMImage &ge) {
  nassertv(get_x_size() <= select_image.get_x_size() && get_y_size() <= select_image.get_y_size());
  nassertv(channel >= 0 && channel < select_image.get_num_channels());

  xelval threshold_val = (xelval)(select_image.get_maxval() * threshold + 0.5);

  if (lt.get_x_size() == 1 && lt.get_y_size() == 1 && 
      ge.get_x_size() == 1 && ge.get_y_size() == 1) {
    // 1x1 source images.
    xel lt_val = lt.get_xel_val(0, 0);
    xelval lt_alpha = 0;
    if (lt.has_alpha()) {
      lt_alpha = lt.get_alpha_val(0, 0);
    }
    if (lt.get_maxval() != get_maxval()) {
      double scale = (double)get_maxval() / (double)lt.get_maxval();
      PPM_ASSIGN(lt_val, 
                 (xelval)(PPM_GETR(lt_val) * scale + 0.5), 
                 (xelval)(PPM_GETG(lt_val) * scale + 0.5), 
                 (xelval)(PPM_GETB(lt_val) * scale + 0.5));
      lt_alpha = (xelval)(lt_alpha * scale + 0.5);
    }

    xel ge_val = ge.get_xel_val(0, 0);
    xelval ge_alpha = 0;
    if (ge.has_alpha()) {
      ge_alpha = ge.get_alpha_val(0, 0);
    }
    if (ge.get_maxval() != get_maxval()) {
      double scale = (double)get_maxval() / (double)ge.get_maxval();
      PPM_ASSIGN(ge_val, 
                 (xelval)(PPM_GETR(ge_val) * scale + 0.5), 
                 (xelval)(PPM_GETG(ge_val) * scale + 0.5), 
                 (xelval)(PPM_GETB(ge_val) * scale + 0.5));
      ge_alpha = (xelval)(ge_alpha * scale + 0.5);
    }

    int x, y;
    
    if (channel == 3) {
      // Further special case: the alpha channel.
      if (has_alpha() && lt.has_alpha() && ge.has_alpha()) {
        // Copy alpha channel too.
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_alpha_val(x, y) < threshold_val) {
              set_xel_val(x, y, lt_val);
              set_alpha_val(x, y, lt_alpha);
            } else {
              set_xel_val(x, y, ge_val);
              set_alpha_val(x, y, ge_alpha);
            }
          }
        }
        
      } else {
        // Don't copy alpha channel.
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_alpha_val(x, y) < threshold_val) {
              set_xel_val(x, y, lt_val);
            } else {
              set_xel_val(x, y, ge_val);
            }
          }
        }
      }
    } else {
      // Any generic channel.
      if (has_alpha() && lt.has_alpha() && ge.has_alpha()) {
        // Copy alpha channel too.
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_channel_val(x, y, channel) < threshold_val) {
              set_xel_val(x, y, lt_val);
              set_alpha_val(x, y, lt_alpha);
            } else {
              set_xel_val(x, y, ge_val);
              set_alpha_val(x, y, ge_alpha);
            }
          }
        }
        
      } else {
        // Don't copy alpha channel.
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_channel_val(x, y, channel) < threshold_val) {
              set_xel_val(x, y, lt_val);
            } else {
              set_xel_val(x, y, ge_val);
            }
          }
        }
      }
    }
    
  } else {
    // Same-sized source images.
    nassertv(get_x_size() <= lt.get_x_size() && get_y_size() <= lt.get_y_size());
    nassertv(get_x_size() <= ge.get_x_size() && get_y_size() <= ge.get_y_size());

    if (get_maxval() == lt.get_maxval() && get_maxval() == ge.get_maxval()) {
      // Simple case: the maxvals are all equal.  Copy by integer value.
      int x, y;

      if (channel == 3) {
        // Further special case: the alpha channel.
        if (has_alpha() && lt.has_alpha() && ge.has_alpha()) {
          // Copy alpha channel too.
          for (y = 0; y < get_y_size(); y++) {
            for (x = 0; x < get_x_size(); x++) {
              if (select_image.get_alpha_val(x, y) < threshold_val) {
                set_xel_val(x, y, lt.get_xel_val(x, y));
                set_alpha_val(x, y, lt.get_alpha_val(x, y));
              } else {
                set_xel_val(x, y, ge.get_xel_val(x, y));
                set_alpha_val(x, y, ge.get_alpha_val(x, y));
              }
            }
          }

        } else {
          // Don't copy alpha channel.
          for (y = 0; y < get_y_size(); y++) {
            for (x = 0; x < get_x_size(); x++) {
              if (select_image.get_alpha_val(x, y) < threshold_val) {
                set_xel_val(x, y, lt.get_xel_val(x, y));
              } else {
                set_xel_val(x, y, ge.get_xel_val(x, y));
              }
            }
          }
        }
      } else {
        // Any generic channel.
        if (has_alpha() && lt.has_alpha() && ge.has_alpha()) {
          // Copy alpha channel too.
          for (y = 0; y < get_y_size(); y++) {
            for (x = 0; x < get_x_size(); x++) {
              if (select_image.get_channel_val(x, y, channel) < threshold_val) {
                set_xel_val(x, y, lt.get_xel_val(x, y));
                set_alpha_val(x, y, lt.get_alpha_val(x, y));
              } else {
                set_xel_val(x, y, ge.get_xel_val(x, y));
                set_alpha_val(x, y, ge.get_alpha_val(x, y));
              }
            }
          }

        } else {
          // Don't copy alpha channel.
          for (y = 0; y < get_y_size(); y++) {
            for (x = 0; x < get_x_size(); x++) {
              if (select_image.get_channel_val(x, y, channel) < threshold_val) {
                set_xel_val(x, y, lt.get_xel_val(x, y));
              } else {
                set_xel_val(x, y, ge.get_xel_val(x, y));
              }
            }
          }
        }
      }

    } else {
      // General case: the maxvals are different.  Copy by floating-point value.
      int x, y;

      if (has_alpha() && lt.has_alpha() && ge.has_alpha()) {
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_channel_val(x, y, channel) < threshold_val) {
              set_xel(x, y, lt.get_xel(x, y));
              set_alpha(x, y, lt.get_alpha(x, y));
            } else {
              set_xel(x, y, ge.get_xel(x, y));
              set_alpha(x, y, ge.get_alpha(x, y));
            }
          }
        }
      } else {
        for (y = 0; y < get_y_size(); y++) {
          for (x = 0; x < get_x_size(); x++) {
            if (select_image.get_channel_val(x, y, channel) < threshold_val) {
              set_xel(x, y, lt.get_xel(x, y));
            } else {
              set_xel(x, y, ge.get_xel(x, y));
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill_distance_inside
//       Access: Published
//  Description: Replaces this image with a grayscale image whose gray
//               channel represents the linear Manhattan distance from
//               the nearest dark pixel in the given mask image, up to
//               the specified radius value (which also becomes the
//               new maxval).  radius may range from 0 to maxmaxval;
//               smaller values will compute faster.  A dark pixel is
//               defined as one whose pixel value is < threshold.
//
//               If shrink_from_border is true, then the mask image is
//               considered to be surrounded by a border of dark
//               pixels; otherwise, the border isn't considered.
//
//               This can be used, in conjunction with threshold, to
//               shrink a mask image inwards by a certain number of
//               pixels.
//
//               The mask image may be the same image as this one, in
//               which case it is destructively modified by this
//               process.
////////////////////////////////////////////////////////////////////
void PNMImage::
fill_distance_inside(const PNMImage &mask, double threshold, int radius, bool shrink_from_border) {
  nassertv(radius <= PNM_MAXMAXVAL);
  PNMImage dist(mask.get_x_size(), mask.get_y_size(), 1, radius);
  dist.fill_val(radius);

  xelval threshold_val = (xelval)(mask.get_maxval() * threshold + 0.5);

  for (int yi = 0; yi < mask.get_y_size(); ++yi) {
    for (int xi = 0; xi < mask.get_x_size(); ++xi) {
      if (mask.get_gray_val(xi, yi) < threshold_val) {
        dist.do_fill_distance(xi, yi, 0);
      }
    }
  }

  if (shrink_from_border) {
    // Also measure from the image border.
    for (int yi = 0; yi < mask.get_y_size(); ++yi) {
      dist.do_fill_distance(0, yi, 1);
      dist.do_fill_distance(mask.get_x_size() - 1, yi, 1);
    }
    for (int xi = 0; xi < mask.get_x_size(); ++xi) {
      dist.do_fill_distance(xi, 0, 1);
      dist.do_fill_distance(xi, mask.get_y_size() - 1, 1);
    }
  }

  take_from(dist);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill_distance_outside
//       Access: Published
//  Description: Replaces this image with a grayscale image whose gray
//               channel represents the linear Manhattan distance from
//               the nearest white pixel in the given mask image, up to
//               the specified radius value (which also becomes the
//               new maxval).  radius may range from 0 to maxmaxval;
//               smaller values will compute faster.  A white pixel is
//               defined as one whose pixel value is >= threshold.
//
//               This can be used, in conjunction with threshold, to
//               grow a mask image outwards by a certain number of
//               pixels.
//
//               The mask image may be the same image as this one, in
//               which case it is destructively modified by this
//               process.
////////////////////////////////////////////////////////////////////
void PNMImage::
fill_distance_outside(const PNMImage &mask, double threshold, int radius) {
  nassertv(radius <= PNM_MAXMAXVAL);
  PNMImage dist(mask.get_x_size(), mask.get_y_size(), 1, radius);
  dist.fill_val(radius);

  xelval threshold_val = (xelval)(mask.get_maxval() * threshold + 0.5);

  for (int yi = 0; yi < mask.get_y_size(); ++yi) {
    for (int xi = 0; xi < mask.get_x_size(); ++xi) {
      if (mask.get_gray_val(xi, yi) >= threshold_val) {
        dist.do_fill_distance(xi, yi, 0);
      }
    }
  }

  take_from(dist);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::rescale
//       Access: Published
//  Description: Rescales the RGB channel values so that any values in
//               the original image between min_val and max_val are
//               expanded to the range 0 .. 1.  Values below min_val
//               are set to 0, and values above max_val are set to 1.
//               Does not affect the alpha channel, if any.
////////////////////////////////////////////////////////////////////
void PNMImage::
rescale(double min_val, double max_val) {
  double scale = max_val - min_val;

  if (_num_channels <= 2) {
    // Grayscale.
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        double val = get_gray(x, y);
        set_gray(x, y, (val - min_val) / scale);
      }
    }
  } else {
    // RGB(A).
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        LRGBColord xel = get_xel(x, y);
        set_xel(x, y, 
                (xel[0] - min_val) / scale,
                (xel[1] - min_val) / scale,
                (xel[2] - min_val) / scale);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_channel
//       Access: Published
//  Description: Copies just a single channel from the source image
//               into a single channel of this image, leaving the
//               remaining channels alone.
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_channel(const PNMImage &copy, int xto, int yto, int cto,
             int xfrom, int yfrom, int cfrom,
             int x_size, int y_size) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval()) {
    // The simple case: no pixel value rescaling is required.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      if (cto == 3 && cfrom == 3) {
        // To alpha channel, from alpha channel.
        for (x = xmin; x < xmax; x++) {
          set_alpha_val(x, y, copy.get_alpha_val(x - xmin + xfrom, y - ymin + yfrom));
        }
      } else if (cto == 3 && cfrom == 0) {
        // To alpha channel, from blue (or gray) channel.
        for (x = xmin; x < xmax; x++) {
          set_alpha_val(x, y, copy.get_blue_val(x - xmin + xfrom, y - ymin + yfrom));
        }
      } else if (cto == 0 && cfrom == 3) {
        // To blue (or gray) channel, from alpha channel.
        for (x = xmin; x < xmax; x++) {
          set_blue_val(x, y, copy.get_alpha_val(x - xmin + xfrom, y - ymin + yfrom));
        }
      } else {
        // Generic channels.
        for (x = xmin; x < xmax; x++) {
          set_channel_val(x, y, cto, copy.get_channel_val(x - xmin + xfrom, y - ymin + yfrom, cfrom));
        }
      }
    }

  } else {
    // The harder case: rescale pixel values according to maxval.
    int x, y;
    for (y = ymin; y < ymax; y++) {
      for (x = xmin; x < xmax; x++) {
        set_channel(x, y, cto, copy.get_channel(x - xmin + xfrom, y - ymin + yfrom, cfrom));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::render_spot
//       Access: Published
//  Description: Renders a solid-color circle, with a fuzzy edge, into
//               the center of the PNMImage.  If the PNMImage is
//               non-square, this actually renders an ellipse.
//
//               The min_radius and max_radius are in the scale 0..1,
//               where 1.0 means the full width of the image.  If
//               min_radius == max_radius, the edge is sharp (but
//               still antialiased); otherwise, the pixels between
//               min_radius and max_radius are smoothly blended
//               between fg and bg colors.
////////////////////////////////////////////////////////////////////
void PNMImage::
render_spot(const LColord &fg, const LColord &bg,
            double min_radius, double max_radius) {
  if (_x_size == 0 || _y_size == 0) {
    return;
  }

  double x_scale = 2.0 / _x_size;
  double y_scale = 2.0 / _y_size;

  // If the width is even, x_center1 == x_center0.  If the width is
  // odd, x_center1 == x_center0 + 1.
  int x_center0 = _x_size / 2;
  int y_center0 = _y_size / 2;
  int x_center1 = (_x_size + 1) / 2;
  int y_center1 = (_y_size + 1) / 2;

  double min_r2 = min_radius * min_radius;
  double max_r2 = max_radius * max_radius;

  for (int yi = 0; yi < y_center1; ++yi) {
    double y = yi * y_scale;
    double y2_inner = y * y;
    double y2_outer = (y + y_scale) * (y + y_scale);
    for (int xi = 0; xi < x_center1; ++xi) {
      double x = xi * x_scale;
      double d2_inner = (x * x + y2_inner);
      double d2_outer = ((x + x_scale) * (x + x_scale) + y2_outer);
      double d2_a = ((x + x_scale) * (x + x_scale) + y2_inner);
      double d2_b = (x * x + y2_outer);

      if ((d2_inner <= min_r2) &&
          (d2_outer <= min_r2) &&
          (d2_a <= min_r2) &&
          (d2_b <= min_r2)) {
        // This pixel is solidly in the center of the spot.
        set_xel_a(x_center1 - 1 - xi, y_center1 - 1 - yi, fg);
        set_xel_a(x_center0 + xi, y_center1 - 1 - yi, fg);
        set_xel_a(x_center1 - 1 - xi, y_center0 + yi, fg);
        set_xel_a(x_center0 + xi, y_center0 + yi, fg);

      } else if ((d2_inner > max_r2) &&
                 (d2_outer > max_r2) &&
                 (d2_a > max_r2) &&
                 (d2_b > max_r2)) {
        // This pixel is solidly outside the spot.
        set_xel_a(x_center1 - 1 - xi, y_center1 - 1 - yi, bg);
        set_xel_a(x_center0 + xi, y_center1 - 1 - yi, bg);
        set_xel_a(x_center1 - 1 - xi, y_center0 + yi, bg);
        set_xel_a(x_center0 + xi, y_center0 + yi, bg);

      } else {
        // This pixel is in a feathered area or along the antialiased edge.
        LColord c_outer, c_inner, c_a, c_b;
        compute_spot_pixel(c_outer, d2_outer, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_inner, d2_inner, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_a, d2_a, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_b, d2_b, min_radius, max_radius, fg, bg);

        // Now average all four pixels for the antialiased result.
        LColord c;
        c = (c_outer + c_inner + c_a + c_b) * 0.25;

        set_xel_a(x_center1 - 1 - xi, y_center1 - 1 - yi, c);
        set_xel_a(x_center0 + xi, y_center1 - 1 - yi, c);
        set_xel_a(x_center1 - 1 - xi, y_center0 + yi, c);
        set_xel_a(x_center0 + xi, y_center0 + yi, c);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::expand_border
//       Access: Published
//  Description: Expands the image by the indicated number of pixels
//               on each edge.  The new pixels are set to the
//               indicated color.
//
//               If any of the values is negative, this actually crops
//               the image.
////////////////////////////////////////////////////////////////////
void PNMImage::
expand_border(int left, int right, int bottom, int top,
              const LColord &color) {
  PNMImage new_image(get_x_size() + left + right,
                     get_y_size() + bottom + top,
                     get_num_channels(), get_maxval(), get_type());
  new_image.fill(color[0], color[1], color[2]);
  if (has_alpha()) {
    new_image.alpha_fill(color[3]);
  }
  new_image.copy_sub_image(*this, left, top);

  take_from(new_image);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::unfiltered_stretch_from
//       Access: Published
//  Description: Resizes from the indicated image into this one by
//               performing a nearest-point sample.
////////////////////////////////////////////////////////////////////
void PNMImage::
unfiltered_stretch_from(const PNMImage &copy) {
  for (int yt = 0; yt < get_y_size(); yt++) {
    int ys = yt * copy.get_y_size() / get_y_size();
    for (int xt = 0; xt < get_x_size(); xt++) {
      int xs = xt * copy.get_x_size() / get_x_size();
      set_xel(xt, yt, copy.get_xel(xs, ys));
    }
  }

  if (has_alpha() && copy.has_alpha()) { 
    for (int yt = 0; yt < get_y_size(); yt++) {
      int ys = yt * copy.get_y_size() / get_y_size();
      for (int xt = 0; xt < get_x_size(); xt++) {
        int xs = xt * copy.get_x_size() / get_x_size();
        set_alpha(xt, yt, copy.get_alpha(xs, ys));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_histogram
//       Access: Published
//  Description: Computes a histogram of the colors used in the
//               image.
////////////////////////////////////////////////////////////////////
void PNMImage::
make_histogram(PNMImage::Histogram &histogram) {
  HistMap hist_map;
  PixelCount pixels;

  int num_pixels = _x_size * _y_size;

  compute_histogram(hist_map, _array, _alpha);

  pixels.reserve(hist_map.size());
  HistMap::const_iterator hi;
  for (hi = hist_map.begin(); hi != hist_map.end(); ++hi) {
    if ((*hi).second <= num_pixels) {
      pixels.push_back(PixelSpecCount((*hi).first, (*hi).second));
    }
  }
  ::sort(pixels.begin(), pixels.end());

  histogram.swap(pixels, hist_map);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::perlin_noise_fill
//       Access: Published
//  Description: Fills the image with a grayscale perlin noise
//               pattern based on the indicated parameters.
//               Uses set_xel to set the grayscale values.
//               The sx and sy parameters are in multiples
//               of the size of this image.
//               See also the PerlinNoise2 class in mathutil.
////////////////////////////////////////////////////////////////////
void PNMImage::
perlin_noise_fill(double sx, double sy, int table_size, unsigned long seed) {
  double x, y;
  double noise;
  PerlinNoise2 perlin (sx * _x_size, sy * _y_size, table_size, seed);
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      noise = perlin.noise(x, y);
      set_xel(x, y, 0.5 * (noise + 1.0));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::perlin_noise_fill
//       Access: Published
//  Description: Variant of perlin_noise_fill that uses an
//               existing StackedPerlinNoise2 object.
////////////////////////////////////////////////////////////////////
void PNMImage::
perlin_noise_fill(StackedPerlinNoise2 &perlin) {
  double x, y;
  double noise;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      noise = perlin.noise(x / (double) _x_size, y / (double) _y_size);
      set_xel(x, y, 0.5 * (noise + 1.0));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::remix_channels
//       Access: Published
//  Description: Transforms every pixel using the operation
//               (Ro,Go,Bo) = conv.xform_point(Ri,Gi,Bi);
//               Input must be a color image.
////////////////////////////////////////////////////////////////////
void PNMImage::
remix_channels(const LMatrix4 &conv) {
  int nchannels = get_num_channels();
  nassertv((nchannels >= 3) && (nchannels <= 4));
  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      LVector3 inv(get_red(x,y), get_green(x,y), get_blue(x,y));
      LVector3 outv(conv.xform_point(inv));
      set_xel(x, y, outv[0], outv[1], outv[2]);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::apply_exponent
//       Access: Published
//  Description: Adjusts each channel of the image by raising the
//               corresponding component value to the indicated
//               exponent, such that L' = L ^ exponent.  For a
//               grayscale image, the blue_exponent value is used for
//               the grayscale value, and red_exponent and
//               green_exponent are unused.
////////////////////////////////////////////////////////////////////
void PNMImage::
apply_exponent(double red_exponent, double green_exponent, double blue_exponent, 
               double alpha_exponent) {
  int num_channels = _num_channels;
  if (has_alpha() && alpha_exponent == 1.0) {
    // If the alpha_exponent is 1, don't bother to apply it.
    --num_channels;
  }

  int x, y;

  if (red_exponent == 1.0 && green_exponent == 1.0 && blue_exponent == 1.0) {
    // If the RGB components are all 1, apply only to the alpha channel.
    switch (num_channels) {
    case 1:
    case 3:
      break;

    case 2:
    case 4:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          double alpha = get_alpha(x, y);
          alpha = cpow(alpha, blue_exponent);
          set_alpha(x, y, alpha);
        }
      }
      break;
    }

  } else {
    // Apply to the color and/or alpha channels.

    switch (num_channels) {
    case 1:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          double gray = get_gray(x, y);
          gray = cpow(gray, blue_exponent);
          set_gray(x, y, gray);
        }
      }
      break;

    case 2:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          double gray = get_gray(x, y);
          gray = cpow(gray, blue_exponent);
          set_gray(x, y, gray);

          double alpha = get_alpha(x, y);
          alpha = cpow(alpha, blue_exponent);
          set_alpha(x, y, alpha);
        }
      }
      break;

    case 3:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          LRGBColord color = get_xel(x, y);
          color[0] = cpow(color[0], red_exponent);
          color[1] = cpow(color[1], green_exponent);
          color[2] = cpow(color[2], blue_exponent);
          set_xel(x, y, color);
        }
      }
      break;

    case 4:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          LColord color = get_xel_a(x, y);
          color[0] = cpow(color[0], red_exponent);
          color[1] = cpow(color[1], green_exponent);
          color[2] = cpow(color[2], blue_exponent);
          color[3] = cpow(color[3], alpha_exponent);
          set_xel_a(x, y, color);
        }
      }
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::setup_rc
//       Access: Private
//  Description: Sets the _default_rc,bc,gc values appropriately
//               according to the color type of the image, so that
//               get_bright() will return a meaningful value for both
//               color and grayscale images.
////////////////////////////////////////////////////////////////////
void PNMImage::
setup_rc() {
  if (is_grayscale()) {
    _default_rc = 0.0;
    _default_gc = 0.0;
    _default_bc = 1.0;
  } else {
    _default_rc = lumin_red;
    _default_gc = lumin_grn;
    _default_bc = lumin_blu;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::do_fill_distance
//       Access: Private
//  Description: Recursively fills in the minimum distance measured
//               from a certain set of points into the gray channel.
////////////////////////////////////////////////////////////////////
void PNMImage::
do_fill_distance(int xi, int yi, int d) {
  if (xi < 0 || xi >= get_x_size() || 
      yi < 0 || yi >= get_y_size()) {
    return;
  }
  if (get_gray_val(xi, yi) <= d) {
    return;
  }
  set_gray_val(xi, yi, d);

  do_fill_distance(xi + 1, yi, d + 1);
  do_fill_distance(xi - 1, yi, d + 1);
  do_fill_distance(xi, yi + 1, d + 1);
  do_fill_distance(xi, yi - 1, d + 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_average_xel
//       Access: Published
//  Description: Returns the average color of all of the pixels
//               in the image.
////////////////////////////////////////////////////////////////////
LRGBColord PNMImage::
get_average_xel() const {
  LRGBColord color (LRGBColord::zero());
  if (_x_size == 0 || _y_size == 0) {
    return color;
  }

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      color += get_xel(x, y);
    }
  }

  color *= 1.0 / (_x_size * _y_size);
  return color;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_average_xel_a
//       Access: Published
//  Description: Returns the average color of all of the pixels
//               in the image, including the alpha channel.
////////////////////////////////////////////////////////////////////
LColord PNMImage::
get_average_xel_a() const {
  LColord color (LColord::zero());
  if (_x_size == 0 || _y_size == 0) {
    return color;
  }

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      color += get_xel_a(x, y);
    }
  }

  color *= 1.0 / (_x_size * _y_size);
  return color;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_average_gray
//       Access: Published
//  Description: Returns the average grayscale component of all of
//               the pixels in the image.
////////////////////////////////////////////////////////////////////
double PNMImage::
get_average_gray() const {
  double color = 0.0;
  if (_x_size == 0 || _y_size == 0) {
    return color;
  }

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      color += get_gray(x, y);
    }
  }

  color *= 1.0 / (_x_size * _y_size);
  return color;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator ~
//       Access: Published
//  Description: Returns a new PNMImage that is the
//               complement of the current PNMImage.
////////////////////////////////////////////////////////////////////
PNMImage PNMImage::
operator ~ () const {
  PNMImage target (*this);
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      target._array[i].r = _maxval - _array[i].r;
      target._array[i].g = _maxval - _array[i].g;
      target._array[i].b = _maxval - _array[i].b;
      target._alpha[i] = _maxval - _alpha[i];
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      target._array[i].r = _maxval - _array[i].r;
      target._array[i].g = _maxval - _array[i].g;
      target._array[i].b = _maxval - _array[i].b;
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      target._alpha[i] = _maxval - _alpha[i];
    }
  }
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator +=
//       Access: Published
//  Description: Sets each pixel value to the sum of the corresponding
//               pixel values in the two given images.  Only valid
//               when both images have the same size.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator += (const PNMImage &other) {
  nassertv(_x_size == other._x_size && _y_size == other._y_size);
  nassertv(_maxval == other._maxval && _maxval == other._maxval);
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + other._array[i].r);
      _array[i].g = clamp_val(_array[i].g + other._array[i].g);
      _array[i].b = clamp_val(_array[i].b + other._array[i].b);
      _alpha[i] = clamp_val(_alpha[i] + other._alpha[i]);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + other._array[i].r);
      _array[i].g = clamp_val(_array[i].g + other._array[i].g);
      _array[i].b = clamp_val(_array[i].b + other._array[i].b);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] + other._alpha[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator +=
//       Access: Published
//  Description: Adds the provided color to each pixel in this image.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator += (const LColord &other) {
  size_t array_size = _x_size * _y_size;
  // Note: don't use to_val here because it clamps values below 0
  int add_r = (int)(other.get_x() * get_maxval() + 0.5);
  int add_g = (int)(other.get_y() * get_maxval() + 0.5);
  int add_b = (int)(other.get_z() * get_maxval() + 0.5);
  int add_a = (int)(other.get_w() * get_maxval() + 0.5);

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + add_r);
      _array[i].g = clamp_val(_array[i].g + add_g);
      _array[i].b = clamp_val(_array[i].b + add_b);
      _alpha[i] = clamp_val(_alpha[i] + add_a);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + add_r);
      _array[i].g = clamp_val(_array[i].g + add_g);
      _array[i].b = clamp_val(_array[i].b + add_b);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] + add_a);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator -=
//       Access: Published
//  Description: Subtracts each pixel from the right image from each
//               pixel value in this image.  Only valid when both
//               images have the same size.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator -= (const PNMImage &other) {
  nassertv(_x_size == other._x_size && _y_size == other._y_size);
  nassertv(_maxval == other._maxval && _maxval == other._maxval);
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r - other._array[i].r);
      _array[i].g = clamp_val(_array[i].g - other._array[i].g);
      _array[i].b = clamp_val(_array[i].b - other._array[i].b);
      _alpha[i] = clamp_val(_alpha[i] - other._alpha[i]);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r - other._array[i].r);
      _array[i].g = clamp_val(_array[i].g - other._array[i].g);
      _array[i].b = clamp_val(_array[i].b - other._array[i].b);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] - other._alpha[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator -=
//       Access: Published
//  Description: Subtracts the provided color from each pixel in this
//               image.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator -= (const LColord &other) {
  (*this) += (-other);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *=
//       Access: Published
//  Description: Multiples each pixel in this image by each
//               pixel value from the right image. Note that the
//               floating-point values in the 0..1 range are
//               multiplied, not in the 0..maxval range.
//               Only valid when both images have the same size.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator *= (const PNMImage &other) {
  nassertv(_x_size == other._x_size && _y_size == other._y_size);
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = to_val(from_val(_array[i].r) * other.from_val(other._array[i].r));
      _array[i].g = to_val(from_val(_array[i].g) * other.from_val(other._array[i].g));
      _array[i].b = to_val(from_val(_array[i].b) * other.from_val(other._array[i].b));
      _alpha[i] = to_val(from_val(_alpha[i]) * other.from_val(other._alpha[i]));
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = to_val(from_val(_array[i].r) * other.from_val(other._array[i].r));
      _array[i].g = to_val(from_val(_array[i].g) * other.from_val(other._array[i].g));
      _array[i].b = to_val(from_val(_array[i].b) * other.from_val(other._array[i].b));
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = to_val(from_val(_alpha[i]) * other.from_val(other._alpha[i]));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *=
//       Access: Published
//  Description: Multiplies every pixel value in the image by
//               a constant floating-point multiplier value.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator *= (double multiplier) {
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r * multiplier);
      _array[i].g = clamp_val(_array[i].g * multiplier);
      _array[i].b = clamp_val(_array[i].b * multiplier);
      _alpha[i] = clamp_val(_alpha[i] * multiplier);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r * multiplier);
      _array[i].g = clamp_val(_array[i].g * multiplier);
      _array[i].b = clamp_val(_array[i].b * multiplier);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] * multiplier);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *=
//       Access: Published
//  Description: Multiples the provided color to each pixel in this
//               image.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator *= (const LColord &other) {
  size_t array_size = _x_size * _y_size;

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r * other[0]);
      _array[i].g = clamp_val(_array[i].g * other[1]);
      _array[i].b = clamp_val(_array[i].b * other[2]);
      _alpha[i] = clamp_val(_alpha[i] * other[3]);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r * other[0]);
      _array[i].g = clamp_val(_array[i].g * other[1]);
      _array[i].b = clamp_val(_array[i].b * other[2]);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] * other[3]);
    }
  }
}

