// Filename: pnmImage.cxx
// Created by:  drose (14Jun00)
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

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "config_pnmimage.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clear
//       Access: Public
//  Description: Frees all memory allocated for the image, and clears
//               all its parameters (size, color, type, etc).
////////////////////////////////////////////////////////////////////
void PNMImage::
clear() {
  if (_array != (xel *)NULL) {
    delete[] _array;
    _array = (xel *)NULL;
  }
  if (_alpha != (xelval *)NULL) {
    delete[] _alpha;
    _alpha = (xelval *)NULL;
  }
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _maxval = 255;
  _type = (PNMFileType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clear
//       Access: Public
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
  _type = type;

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
//       Access: Public
//  Description: Makes this image become a copy of the other image.
////////////////////////////////////////////////////////////////////
void PNMImage::
copy_from(const PNMImage &copy) {
  clear();

  if (copy.is_valid()) {
    copy_header_from(copy);

    if (has_alpha()) {
      memcpy(_alpha, copy._alpha, sizeof(xelval) * _y_size * _x_size);
    }
    memcpy(_array, copy._array, sizeof(xel) * _y_size * _x_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::copy_header_from
//       Access: Public
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
//     Function: PNMImage::fill_val
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Reads the indicated image filename.  If type is
//               non-NULL, it is a suggestion for the type of file it
//               is.  Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(const Filename &filename, PNMFileType *type) {
  clear();

  PNMReader *reader = PNMImageHeader::make_reader(filename, type);
  if (reader == (PNMReader *)NULL) {
    return false;
  }
  return read(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::read
//       Access: Public
//  Description: Reads the image data from the indicated stream.  
//
//               The filename is advisory only, and may be used
//               suggest a type if it has a known extension.
//
//               If type is non-NULL, it is a suggestion for the type
//               of file it is.  Returns true if successful, false on
//               error.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(istream &data, const string &filename, PNMFileType *type) {
  clear();

  PNMReader *reader = PNMImageHeader::make_reader
    (&data, false, filename, string(), type);
  if (reader == (PNMReader *)NULL) {
    return false;
  }
  return read(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::read
//       Access: Public
//  Description: This flavor of read() uses an already-existing
//               PNMReader to read the image file.  You can get a
//               reader via the PNMImageHeader::make_reader() methods.
//               This is a good way to examine the header of a file
//               (for instance, to determine its size) before actually
//               reading the entire image.
//
//               The PNMReader is always deleted upon completion,
//               whether succesful or not.
////////////////////////////////////////////////////////////////////
bool PNMImage::
read(PNMReader *reader) {
  clear();

  if (reader == NULL) {
    return false;
  }

  if (!reader->is_valid()) {
    delete reader;
    return false;
  }

  copy_header_from(*reader);

  // We reassign y_size after reading because we might have read a
  // truncated file.
  _y_size = reader->read_data(_array, _alpha);
  delete reader;

  if (_y_size == 0) {
    clear();
    return false;
  } else {
    setup_rc();
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::write
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: This flavor of write() uses an already-existing
//               PNMWriter to write the image file.  You can get a
//               writer via the PNMImageHeader::make_writer() methods.
//
//               The PNMWriter is always deleted upon completion,
//               whether succesful or not.
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
  int result = writer->write_data(_array, _alpha);
  delete writer;

  return (result == _y_size);
}



////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_color_type
//       Access: Public
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
      delete[] _alpha;
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
//       Access: Public
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
//     Function: PNMImage::set_maxval
//       Access: Public
//  Description: Rescales the image to the indicated maxval.
////////////////////////////////////////////////////////////////////
void PNMImage::
set_maxval(xelval maxval) {
  nassertv(maxval > 0.0);

  if (maxval != _maxval) {
    double ratio = maxval / _maxval;

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
//       Access: Public
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
//       Access: Public
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
//     Function: PNMImage::blend
//       Access: Public
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
      RGBColord prev_rgb = get_xel(x, y);
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
//     Function: PNMImage::copy_sub_image
//       Access: Public
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
  if (xfrom < 0) {
    xto += -xfrom;
    xfrom = 0;
  }
  if (yfrom < 0) {
    yto += -yfrom;
    yfrom = 0;
  }

  x_size = (x_size < 0) ?
    copy.get_x_size() :
    min(x_size, copy.get_x_size() - xfrom);
  y_size = (y_size < 0) ?
    copy.get_y_size() :
    min(y_size, copy.get_y_size() - yfrom);

  int xmin = max(0, xto);
  int ymin = max(0, yto);

  int xmax = min(xmin + x_size, get_x_size());
  int ymax = min(ymin + y_size, get_y_size());

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

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::blend_sub_image
//       Access: Public
//  Description: Behaves like copy_sub_image(), except the alpha
//               channel of the copy is used to blend the copy into
//               the destination image, instead of overwriting pixels
//               unconditionally.
//
//               If the copy has no alpha channel, this degenerates
//               into copy_sub_image().
////////////////////////////////////////////////////////////////////
void PNMImage::
blend_sub_image(const PNMImage &copy, int xto, int yto,
                int xfrom, int yfrom, int x_size, int y_size) {
  if (!copy.has_alpha()) {
    copy_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size);
    return;
  }

  if (xfrom < 0) {
    xto += -xfrom;
    xfrom = 0;
  }
  if (yfrom < 0) {
    yto += -yfrom;
    yfrom = 0;
  }

  x_size = (x_size < 0) ?
    copy.get_x_size() :
    min(x_size, copy.get_x_size() - xfrom);
  y_size = (y_size < 0) ?
    copy.get_y_size() :
    min(y_size, copy.get_y_size() - yfrom);

  int xmin = max(0, xto);
  int ymin = max(0, yto);

  int xmax = min(xmin + x_size, get_x_size());
  int ymax = min(ymin + y_size, get_y_size());

  int x, y;
  for (y = ymin; y < ymax; y++) {
    for (x = xmin; x < xmax; x++) {
      blend(x, y, copy.get_xel(x - xmin + xfrom, y - ymin + yfrom),
            copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom));
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImage::setup_rc
//       Access: Public
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
