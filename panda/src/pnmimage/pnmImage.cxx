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
//     Function: PNMImage::remix_channels
//       Access: Published
//  Description: Transforms every pixel using the operation
//               (Ro,Go,Bo) = conv.xform_point(Ri,Gi,Bi);
//               Input must be a color image.
////////////////////////////////////////////////////////////////////
void PNMImage::
remix_channels(const LMatrix4f &conv) {
  int nchannels = get_num_channels();
  nassertv((nchannels >= 3) && (nchannels <= 4));
  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      LVector3f inv(get_red(x,y),get_green(x,y),get_blue(x,y));
      LVector3f outv(conv.xform_point(inv));
      set_xel(x,y,outv[0],outv[1],outv[2]);
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
//               whether succesful or not.
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
        RGBColord c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        RGBColord o = get_xel(x, y);
        RGBColord p;
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
        RGBColord c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        RGBColord o = get_xel(x, y);
        RGBColord p;
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
//               s = select_image.get_channel(x, y). Set this image's
//               (x, y) to:
//
//               lt.get_xel(x, y) if s <= threshold, or
//
//               ge.get_xel(x, y) if s > threshold
//
//               Any of select_image, lt, or ge may be the same
//               PNMImge object as this image, or the same as each
//               other; or they may all be different. All images must
//               be the same size.
////////////////////////////////////////////////////////////////////
void PNMImage::
threshold(const PNMImage &select_image, int channel, double threshold,
          const PNMImage &lt, const PNMImage &ge) {
  nassertv(get_x_size() <= select_image.get_x_size() && get_y_size() <= select_image.get_y_size());
  nassertv(get_x_size() <= lt.get_x_size() && get_y_size() <= lt.get_y_size());
  nassertv(get_x_size() <= ge.get_x_size() && get_y_size() <= ge.get_y_size());
  nassertv(channel >= 0 && channel < select_image.get_num_channels());

  xelval threshold_val = (xelval)(select_image.get_maxval() * threshold + 0.5);

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
render_spot(const Colord &fg, const Colord &bg,
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
        Colord c_outer, c_inner, c_a, c_b;
        compute_spot_pixel(c_outer, d2_outer, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_inner, d2_inner, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_a, d2_a, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_b, d2_b, min_radius, max_radius, fg, bg);

        // Now average all four pixels for the antialiased result.
        Colord c;
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
              const Colord &color) {
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
//     Function: PNMImage::operator +=
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               is the sum of the corresponding pixel values
//               in the two given images.
//               Only valid when both images have the same size.
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
//  Description: Returns a new PNMImage in which the provided color
//               is added to each pixel in the provided image.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator += (const Colord &other) {
  size_t array_size = _x_size * _y_size;
  xel addxel (to_val(other.get_x()), to_val(other.get_y()), to_val(other.get_z()));
  xelval addalpha = to_val(other.get_w());

  if (_array != NULL && _alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + addxel.r);
      _array[i].g = clamp_val(_array[i].g + addxel.g);
      _array[i].b = clamp_val(_array[i].b + addxel.b);
      _alpha[i] = clamp_val(_alpha[i] + addalpha);
    }
  } else if (_array != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _array[i].r = clamp_val(_array[i].r + addxel.r);
      _array[i].g = clamp_val(_array[i].g + addxel.g);
      _array[i].b = clamp_val(_array[i].b + addxel.b);
    }
  } else if (_alpha != NULL) {
    for (size_t i = 0; i < array_size; ++i) {
      _alpha[i] = clamp_val(_alpha[i] + addalpha);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator -=
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               from the right image is subtracted from each
//               pixel value from the left image.
//               Only valid when both images have the same size.
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
//  Description: Returns a new PNMImage in which the provided color
//               is subtracted from each pixel in the provided image.
////////////////////////////////////////////////////////////////////
void PNMImage::
operator -= (const Colord &other) {
  (*this) += (-other);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *=
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               from the left image is multiplied by each
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

