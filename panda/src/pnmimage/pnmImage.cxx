/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmImage.cxx
 * @author drose
 * @date 2000-06-14
 */

#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "pnmBrush.h"
#include "pfmFile.h"
#include "config_pnmimage.h"
#include "perlinNoise2.h"
#include "stackedPerlinNoise2.h"
#include <algorithm>

using std::max;
using std::min;

/**
 *
 */
PNMImage::
PNMImage(const Filename &filename, PNMFileType *type) {
  _array = nullptr;
  _alpha = nullptr;
  _has_read_size = false;

  bool result = read(filename, type);
  if (!result) {
    pnmimage_cat.error()
      << "Could not read image " << filename << "\n";
  }
}

/**
 * Frees all memory allocated for the image, and clears all its parameters
 * (size, color, type, etc).
 */
void PNMImage::
clear() {
  if (_array != nullptr) {
    PANDA_FREE_ARRAY(_array);
    _array = nullptr;
  }
  if (_alpha != nullptr) {
    PANDA_FREE_ARRAY(_alpha);
    _alpha = nullptr;
  }
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _maxval = 255;
  _inv_maxval = 1.0 / 255.0;
  _color_space = CS_linear;
  _comment.clear();
  _type = nullptr;
  _has_read_size = false;
  _xel_encoding = XE_generic;
}

/**
 * This flavor of clear() reinitializes the image to an empty (black) image
 * with the given dimensions.
 */
void PNMImage::
clear(int x_size, int y_size, int num_channels,
      xelval maxval, PNMFileType *type, ColorSpace color_space) {
  clear();
  nassertv(num_channels >= 1 && num_channels <= 4);
  nassertv(color_space != CS_unspecified);

  _x_size = x_size;
  _y_size = y_size;
  _num_channels = num_channels;
  _maxval = maxval;
  _color_space = color_space;
  _comment.clear();
  _type = type;
  _has_read_size = false;

  if (has_alpha()) {
    allocate_alpha();
    memset(_alpha, 0, sizeof(xelval) * _y_size * _x_size);
  }

  allocate_array();
  memset(_array, 0, sizeof(xel) * _y_size * _x_size);

  setup_encoding();
  setup_rc();
}

/**
 * Makes this image become a copy of the other image.
 */
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

/**
 * Copies a channel from one image into another.  Images must be the same size
 */
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
      LColorf t = get_xel_a(x, y);
      LColorf o = copy.get_xel_a(x, y);
      t.set_cell(dest_channel, o.get_cell(src_channel));
      set_xel_a(x, y, t);
    }
  }
}

/**
 * Copies some subset of the bits of the specified channel from one image into
 * some subset of the bits of the specified channel in another image.  Images
 * must be the same size.
 *
 * If right_shift is negative, it means a left shift.
 */
void PNMImage::
copy_channel_bits(const PNMImage &copy, int src_channel, int dest_channel, xelval src_mask, int right_shift) {
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

  // Do the actual copying.
  if (right_shift >= 0) {
    xelval dest_mask = ~(src_mask >> right_shift);
    for (int x = 0; x < _x_size; x++) {
      for (int y = 0; y < _y_size; y++) {
        xelval src = copy.get_channel_val(x, y, src_channel);
        xelval dest = get_channel_val(x, y, dest_channel);
        dest = (dest & dest_mask) | ((src & src_mask) >> right_shift);
        set_channel_val(x, y, dest_channel, dest);
      }
    }
  } else {
    int left_shift = -right_shift;
    xelval dest_mask = ~(src_mask << left_shift);
    for (int x = 0; x < _x_size; x++) {
      for (int y = 0; y < _y_size; y++) {
        xelval src = copy.get_channel_val(x, y, src_channel);
        xelval dest = get_channel_val(x, y, dest_channel);
        dest = (dest & dest_mask) | ((src & src_mask) << left_shift);
        set_channel_val(x, y, dest_channel, dest);
      }
    }
  }
}

/**
 * Copies just the header information into this image.  This will blow away
 * any image data stored in the image.  The new image data will be allocated,
 * but left unitialized.
 */
void PNMImage::
copy_header_from(const PNMImageHeader &header) {
  clear();
  PNMImageHeader::operator = (header);

  if (_maxval == 0) {
    _inv_maxval = 0.0f;
  } else {
    _inv_maxval = 1.0f / (float)_maxval;
  }

  if (has_alpha()) {
    allocate_alpha();
  }

  allocate_array();
  setup_encoding();
  setup_rc();
}

/**
 * Move the contents of the other image into this one, and empty the other
 * image.
 */
void PNMImage::
take_from(PNMImage &orig) {
  clear();
  PNMImageHeader::operator = (orig);
  setup_encoding();
  setup_rc();

  if (has_alpha()) {
    _alpha = orig._alpha;
    orig._alpha = nullptr;
  }
  _array = orig._array;
  orig._array = nullptr;

  orig.clear();
}

/**
 * Sets the entire image (except the alpha channel) to the given color.
 */
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

/**
 * Sets the entire alpha channel to the given level.
 */
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

/**
 * Reads the indicated image filename.  If type is non-NULL, it is a
 * suggestion for the type of file it is.  Returns true if successful, false
 * on error.
 */
bool PNMImage::
read(const Filename &filename, PNMFileType *type, bool report_unknown_type) {
  PNMReader *reader = make_reader(filename, type, report_unknown_type);
  if (reader == nullptr) {
    clear();
    return false;
  }

  return read(reader);
}

/**
 * Reads the image data from the indicated stream.
 *
 * The filename is advisory only, and may be used to suggest a type if it has
 * a known extension.
 *
 * If type is non-NULL, it is a suggestion for the type of file it is (and a
 * non-NULL type will override any magic number test or filename extension
 * lookup).
 *
 * Returns true if successful, false on error.
 */
bool PNMImage::
read(std::istream &data, const std::string &filename, PNMFileType *type,
     bool report_unknown_type) {
  PNMReader *reader = PNMImageHeader::make_reader
    (&data, false, filename, std::string(), type, report_unknown_type);
  if (reader == nullptr) {
    clear();
    return false;
  }
  return read(reader);
}

/**
 * This flavor of read() uses an already-existing PNMReader to read the image
 * file.  You can get a reader via the PNMImageHeader::make_reader() methods.
 * This is a good way to examine the header of a file (for instance, to
 * determine its size) before actually reading the entire image.
 *
 * The PNMReader is always deleted upon completion, whether successful or not.
 */
bool PNMImage::
read(PNMReader *reader) {
  bool has_read_size = _has_read_size;
  int read_x_size = _read_x_size;
  int read_y_size = _read_y_size;

  clear();

  if (reader == nullptr) {
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

  // We reassign y_size after reading because we might have read a truncated
  // file.
  _y_size = reader->read_data(_array, _alpha);
  delete reader;

  if (_y_size == 0) {
    clear();
    return false;
  }

  setup_encoding();
  setup_rc();

  if (has_read_size && (_x_size != read_x_size || _y_size != read_y_size)) {
    // The Reader didn't comply with our size request.  Do the sizing
    // explicitly, then.
    PNMImage new_image(read_x_size, read_y_size, get_num_channels(),
                       get_maxval(), get_type(), get_color_space());
    new_image.quick_filter_from(*this);
    take_from(new_image);
  }

  return true;
}

/**
 * Writes the image to the indicated filename.  If type is non-NULL, it is a
 * suggestion for the type of image file to write.
 */
bool PNMImage::
write(const Filename &filename, PNMFileType *type) const {
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = PNMImageHeader::make_writer(filename, type);
  if (writer == nullptr) {
    return false;
  }

  return write(writer);
}

/**
 * Writes the image to the indicated ostream.
 *
 * The filename is advisory only, and may be used suggest a type if it has a
 * known extension.
 *
 * If type is non-NULL, it is a suggestion for the type of image file to
 * write.
 */
bool PNMImage::
write(std::ostream &data, const std::string &filename, PNMFileType *type) const {
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = PNMImageHeader::make_writer
    (&data, false, filename, type);
  if (writer == nullptr) {
    return false;
  }

  return write(writer);
}

/**
 * This flavor of write() uses an already-existing PNMWriter to write the
 * image file.  You can get a writer via the PNMImageHeader::make_writer()
 * methods.
 *
 * The PNMWriter is always deleted upon completion, whether successful or not.
 */
bool PNMImage::
write(PNMWriter *writer) const {
  if (writer == nullptr) {
    return false;
  }

  if (!is_valid()) {
    delete writer;
    return false;
  }

  writer->copy_header_from(*this);

  if (!writer->supports_integer()) {
    // Hmm, it's only a floating-point file type.  Convert it from the integer
    // data we have.
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

/**
 * Translates the image to or from grayscale, color, or four-color mode.
 * Grayscale images are converted to full-color images with R, G, B set to the
 * original gray level; color images are converted to grayscale according to
 * the value of Bright().  The alpha channel, if added, is initialized to
 * zero.
 */
void PNMImage::
set_color_type(PNMImage::ColorType color_type) {
  nassertv((int)color_type >= 1 && (int)color_type <= 4);
  if (color_type == get_color_type()) {
    return;
  }

  if (!is_grayscale() && is_grayscale(color_type)) {
    // Convert to grayscale from color
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        set_gray(x, y, get_bright(x, y));
      }
    }

  } else if (is_grayscale() && !is_grayscale(color_type)) {
    // Convert to color from grayscale
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        set_xel_val(x, y, get_gray_val(x, y));
      }
    }
  }

  if (has_alpha() && !has_alpha(color_type)) {
    // Discard the alpha channel
    if (_alpha != nullptr) {
      PANDA_FREE_ARRAY(_alpha);
      _alpha = nullptr;
    }

  } else if (!has_alpha() && has_alpha(color_type)) {
    // Create a new alpha channel
    allocate_alpha();
    memset(_alpha, 0, sizeof(xelval) * (_x_size * _y_size));
  }

  _num_channels = (int)color_type;
  setup_encoding();
  setup_rc();
}

/**
 * Converts the colors in the image to the indicated color space.  This may be
 * a lossy operation, in particular when going from sRGB to linear.  The alpha
 * channel remains untouched.
 *
 * Note that, because functions like get_xel() and set_xel() work on
 * linearized floating-point values, this conversion won't affect those values
 * (aside from some minor discrepancies due to storage precision).  It does
 * affect the values used by get_xel_val() and set_xel_val(), though, since
 * those operate on encoded colors.
 *
 * Some color spaces, particularly scRGB, may enforce the use of a particular
 * maxval setting.
 */
void PNMImage::
set_color_space(ColorSpace color_space) {
  nassertv(color_space != CS_unspecified);

  if (color_space == _color_space) {
    return;
  }

  if (_array != nullptr) {
    size_t array_size = _x_size * _y_size;

    // Note: only convert RGB, since alpha channel is always linear.
    switch (color_space) {
    case CS_linear:
      if (_maxval == 255 && _color_space == CS_sRGB) {
        for (size_t i = 0; i < array_size; ++i) {
          xel &col = _array[i];
          col.r = decode_sRGB_uchar((unsigned char) col.r);
          col.g = decode_sRGB_uchar((unsigned char) col.g);
          col.b = decode_sRGB_uchar((unsigned char) col.b);
        }
      } else {
        for (int x = 0; x < _x_size; ++x) {
          for (int y = 0; y < _y_size; ++y) {
            LRGBColorf scaled = get_xel(x, y) * _maxval + 0.5f;
            xel &col = row(y)[x];
            col.r = clamp_val((int)scaled[0]);
            col.g = clamp_val((int)scaled[1]);
            col.b = clamp_val((int)scaled[2]);
          }
        }
      }
      break;

    case CS_sRGB:
      if (_maxval == 255 && _color_space == CS_linear) {
        for (size_t i = 0; i < array_size; ++i) {
          xel &col = _array[i];
          col.r = encode_sRGB_uchar((unsigned char) col.r);
          col.g = encode_sRGB_uchar((unsigned char) col.g);
          col.b = encode_sRGB_uchar((unsigned char) col.b);
        }
      } else {
        for (int x = 0; x < _x_size; ++x) {
          for (int y = 0; y < _y_size; ++y) {
            xel &col = row(y)[x];
            encode_sRGB_uchar(get_xel_a(x, y), col);
          }
        }
      }
      break;

    case CS_scRGB:
      for (int x = 0; x < _x_size; ++x) {
        for (int y = 0; y < _y_size; ++y) {
          LRGBColorf scaled = get_xel(x, y) * 8192.f + 4096.5f;
          xel &col = row(y)[x];
          col.r = min(max(0, (int)scaled[0]), 65535);
          col.g = min(max(0, (int)scaled[1]), 65535);
          col.b = min(max(0, (int)scaled[2]), 65535);
        }
      }
      _maxval = 65535;
      break;

    default:
      nassert_raise("invalid color space");
      return;
    }

    // Initialize the new encoding settings.
    _color_space = color_space;
    setup_encoding();
  }
}

/**
 * Converts the image from RGB to grayscale.  Any alpha channel, if present,
 * is left undisturbed.  The optional rc, gc, bc values represent the relative
 * weights to apply to each channel to convert it to grayscale.
 */
void PNMImage::
make_grayscale(float rc, float gc, float bc) {
  if (is_grayscale()) {
    return;
  }

  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      set_gray(x, y, min(get_bright(x, y, rc, gc, bc), 1.0f));
    }
  }

  _num_channels = has_alpha() ? 2 : 1;
  setup_encoding();
  setup_rc();
}

/**
 * Converts an image in-place to its "premultiplied" form, where, for every
 * pixel in the image, the red, green, and blue components are multiplied by
 * that pixel's alpha value.
 *
 * This does not modify any alpha values.
 */
void PNMImage::
premultiply_alpha() {
  if (!has_alpha()) {
    return;
  }

  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      float alpha = get_alpha(x, y);
      float r = get_red(x, y) * alpha;
      float g = get_green(x, y) * alpha;
      float b = get_blue(x, y) * alpha;
      set_xel(x, y, r, g, b);
    }
  }
}

/**
 * Converts an image in-place to its "straight alpha" form (presumably from a
 * "premultiplied" form), where, for every pixel in the image, the red, green,
 * and blue components are divided by that pixel's alpha value.
 *
 * This does not modify any alpha values.
 */
void PNMImage::
unpremultiply_alpha() {
  if (!has_alpha()) {
    return;
  }

  for (int y = 0; y < get_y_size(); y++) {
    for (int x = 0; x < get_x_size(); x++) {
      float alpha = get_alpha(x, y);
      if (alpha > 0) {
        float r = get_red(x, y) / alpha;
        float g = get_green(x, y) / alpha;
        float b = get_blue(x, y) / alpha;
        set_xel(x, y, r, g, b);
      }
    }
  }
}

/**
 * Performs an in-place reversal of the row (y) data.
 */
void PNMImage::
reverse_rows() {
  if (_array != nullptr) {
    xel *new_array = (xel *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xel));
    for (int y = 0; y < _y_size; y++) {
      int new_y = _y_size - 1 - y;
      memcpy(new_array + new_y * _x_size, _array + y * _x_size, _x_size * sizeof(xel));
    }
    PANDA_FREE_ARRAY(_array);
    _array = new_array;
  }

  if (_alpha != nullptr) {
    xelval *new_alpha = (xelval *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xelval));
    for (int y = 0; y < _y_size; y++) {
      int new_y = _y_size - 1 - y;
      memcpy(new_alpha + new_y * _x_size, _alpha + y * _x_size, _x_size * sizeof(xelval));
    }
    PANDA_FREE_ARRAY(_alpha);
    _alpha = new_alpha;
  }
}

/**
 * Reverses, transposes, and/or rotates the image in-place according to the
 * specified parameters.  If flip_x is true, the x axis is reversed; if flip_y
 * is true, the y axis is reversed.  Then, if transpose is true, the x and y
 * axes are exchanged.  These parameters can be used to select any combination
 * of 90-degree or 180-degree rotations and flips.
 */
void PNMImage::
flip(bool flip_x, bool flip_y, bool transpose) {
  if (transpose) {
    // Transposed case.  X becomes Y, Y becomes X.
    if (_array != nullptr) {
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

    if (_alpha != nullptr) {
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
    if (_array != nullptr) {
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

    if (_alpha != nullptr) {
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

/**
 * Rescales the image to the indicated maxval.
 */
void PNMImage::
set_maxval(xelval maxval) {
  nassertv(maxval > 0);

  if (maxval != _maxval) {
    float ratio = (float)maxval / (float)_maxval;

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
    setup_encoding();
  }
}

/**
 * Returns the nth component color at the indicated pixel.  The channel index
 * should be in the range 0..(get_num_channels()-1).  The channels are ordered
 * B, G, R, A.  This is slightly less optimal than accessing the component
 * values directly by named methods.  The value returned is in the range
 * 0..maxval.
 */
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
    nassert_raise("unexpected channel count");
    return 0;
  }
}

/**
 * Sets the nth component color at the indicated pixel.  The channel index
 * should be in the range 0..(get_num_channels()-1).  The channels are ordered
 * B, G, R, A.  This is slightly less optimal than setting the component
 * values directly by named methods.  The value given should be in the range
 * 0..maxval.
 */
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
    nassert_raise("unexpected channel count");
    break;
  }
}

/**
 * Returns the nth component color at the indicated pixel.  The channel index
 * should be in the range 0..(get_num_channels()-1).  The channels are ordered
 * B, G, R, A.  This is slightly less optimal than accessing the component
 * values directly by named methods.  The value returned is a float in the
 * range 0..1.
 */
float PNMImage::
get_channel(int x, int y, int channel) const {
  switch (channel) {
  case 0:
    return get_blue(x, y);

  case 1:
    return (_num_channels == 2) ? get_alpha(x, y) : get_green(x, y);

  case 2:
    return get_red(x, y);

  case 3:
    return get_alpha(x, y);

  default:
    pnmimage_cat.error()
      << "Invalid request for channel " << channel << " in "
      << get_num_channels() << "-channel image.\n";
    nassert_raise("unexpected channel count");
    return 0;
  }
}

/**
 * Sets the nth component color at the indicated pixel.  The channel index
 * should be in the range 0..(get_num_channels()-1).  The channels are ordered
 * B, G, R, A.  This is slightly less optimal than setting the component
 * values directly by named methods.  The value given should be a float in the
 * range 0..1.
 */
void PNMImage::
set_channel(int x, int y, int channel, float value) {
  switch (channel) {
  case 0:
    set_blue(x, y, value);
    break;

  case 1:
    if (_num_channels == 2) {
      set_alpha(x, y, value);
    } else {
      set_green(x, y, value);
    }
    break;

  case 2:
    set_red(x, y, value);
    break;

  case 3:
    set_alpha(x, y, value);
    break;

  default:
    nassert_raise("unexpected channel count");
    break;
  }
}

/**
 * Returns the (r, g, b, a) pixel value at the indicated pixel, using a
 * PixelSpec object.
 */
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

/**
 * Sets the (r, g, b, a) pixel value at the indicated pixel, using a PixelSpec
 * object.
 */
void PNMImage::
set_pixel(int x, int y, const PixelSpec &pixel) {
  xel p;
  PPM_ASSIGN(p, pixel._red, pixel._green, pixel._blue);
  set_xel_val(x, y, p);
  if (_alpha != nullptr) {
    set_alpha_val(x, y, pixel._alpha);
  }
}

/**
 * Smoothly blends the indicated pixel value in with whatever was already in
 * the image, based on the given alpha value.  An alpha of 1.0 is fully opaque
 * and completely replaces whatever was there previously; alpha of 0.0 is
 * fully transparent and does nothing.
 */
void PNMImage::
blend(int x, int y, float r, float g, float b, float alpha) {
  if (alpha >= 1.0) {
    // Completely replace the previous color.
    if (has_alpha()) {
      set_alpha(x, y, 1.0);
    }
    set_xel(x, y, r, g, b);

  } else if (alpha > 0.0f) {
    // Blend with the previous color.
    float prev_alpha = has_alpha() ? get_alpha(x, y) : 1.0f;

    if (prev_alpha == 0.0f) {
      // Nothing there previously; replace with this new color.
      set_alpha(x, y, alpha);
      set_xel(x, y, r, g, b);

    } else {
      // Blend the color with the previous color.
      LRGBColorf prev_rgb = get_xel(x, y);
      r = r + (1.0f - alpha) * (prev_rgb[0] - r);
      g = g + (1.0f - alpha) * (prev_rgb[1] - g);
      b = b + (1.0f - alpha) * (prev_rgb[2] - b);
      alpha = prev_alpha + alpha * (1.0f - prev_alpha);

      if (has_alpha()) {
        set_alpha(x, y, alpha);
      }
      set_xel(x, y, r, g, b);
    }
  }
}

/**
 * Replaces the underlying PNMImage array with the indicated pointer.  Know
 * what you are doing!  The new array must be the correct size and must have
 * been allocated via PANDA_MALLOC_ARRAY().  The PNMImage object becomes the
 * owner of this pointer and will eventually free it with PANDA_FREE_ARRAY().
 * The previous array, if any, will be freed with PANDA_FREE_ARRAY() when this
 * call is made.
 */
void PNMImage::
set_array(xel *array) {
  if (_array != nullptr) {
    PANDA_FREE_ARRAY(_array);
  }
  _array = array;
}

/**
 * Replaces the underlying PNMImage alpha array with the indicated pointer.
 * Know what you are doing!  The new array must be the correct size and must
 * have been allocated via PANDA_MALLOC_ARRAY().  The PNMImage object becomes
 * the owner of this pointer and will eventually free it with
 * PANDA_FREE_ARRAY().  The previous array, if any, will be freed with
 * PANDA_FREE_ARRAY() when this call is made.
 */
void PNMImage::
set_alpha_array(xelval *alpha) {
  if (_alpha != nullptr) {
    PANDA_FREE_ARRAY(_alpha);
  }
  _alpha = alpha;
}

/**
 * Copies a rectangular area of another image into a rectangular area of this
 * image.  Both images must already have been initialized.  The upper-left
 * corner of the region in both images is specified, and the size of the area;
 * if the size is omitted, it defaults to the entire other image, or the
 * largest piece that will fit.
 */
void PNMImage::
copy_sub_image(const PNMImage &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() &&
      get_color_space() == copy.get_color_space()) {
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

/**
 * Behaves like copy_sub_image(), except the alpha channel of the copy is used
 * to blend the copy into the destination image, instead of overwriting pixels
 * unconditionally.
 *
 * If pixel_scale is not 1.0, it specifies an amount to scale each *alpha*
 * value of the source image before applying it to the target image.
 *
 * If pixel_scale is 1.0 and the copy has no alpha channel, this degenerates
 * into copy_sub_image().
 */
void PNMImage::
blend_sub_image(const PNMImage &copy, int xto, int yto,
                int xfrom, int yfrom, int x_size, int y_size,
                float pixel_scale) {
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

/**
 * Behaves like copy_sub_image(), except the copy pixels are added to the
 * pixels of the destination, after scaling by the specified pixel_scale.
 * Unlike blend_sub_image(), the alpha channel is not treated specially.
 */
void PNMImage::
add_sub_image(const PNMImage &copy, int xto, int yto,
              int xfrom, int yfrom, int x_size, int y_size,
              float pixel_scale) {
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
      LRGBColorf rgb1 = get_xel(x, y);
      LRGBColorf rgb2 = copy.get_xel(x, y);
      set_xel(x, y,
              rgb1[0] + rgb2[0] * pixel_scale,
              rgb1[1] + rgb2[1] * pixel_scale,
              rgb1[2] + rgb2[2] * pixel_scale);
    }
  }
}

/**
 * Behaves like copy_sub_image(), except the copy pixels are multiplied to the
 * pixels of the destination, after scaling by the specified pixel_scale.
 * Unlike blend_sub_image(), the alpha channel is not treated specially.
 */
void PNMImage::
mult_sub_image(const PNMImage &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size,
               float pixel_scale) {
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
      LRGBColorf rgb1 = get_xel(x, y);
      LRGBColorf rgb2 = copy.get_xel(x, y);
      set_xel(x, y,
              rgb1[0] * rgb2[0] * pixel_scale,
              rgb1[1] * rgb2[1] * pixel_scale,
              rgb1[2] * rgb2[2] * pixel_scale);
    }
  }
}

/**
 * Behaves like copy_sub_image(), but the resulting color will be the darker
 * of the source and destination colors at each pixel (and at each R, G, B, A
 * component value).
 *
 * If pixel_scale is not 1.0, it specifies an amount to scale each pixel value
 * of the source image before applying it to the target image.  The scale is
 * applied with the center at 1.0: scaling the pixel value smaller brings it
 * closer to 1.0.
 */
void PNMImage::
darken_sub_image(const PNMImage &copy, int xto, int yto,
                 int xfrom, int yfrom, int x_size, int y_size,
                 float pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() && pixel_scale == 1.0f &&
      get_color_space() == CS_linear && copy.get_color_space() == CS_linear) {
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
        LRGBColorf c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        LRGBColorf o = get_xel(x, y);
        LRGBColorf p;
        p.set(min(1.0f - ((1.0f - c[0]) * pixel_scale), o[0]),
              min(1.0f - ((1.0f - c[1]) * pixel_scale), o[1]),
              min(1.0f - ((1.0f - c[2]) * pixel_scale), o[2]));
        set_xel(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          float c = copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom);
          float o = get_alpha(x, y);
          set_alpha(x, y, min(1.0f - ((1.0f - c) * pixel_scale), o));
        }
      }
    }
  }
}

/**
 * Behaves like copy_sub_image(), but the resulting color will be the lighter
 * of the source and destination colors at each pixel (and at each R, G, B, A
 * component value).
 *
 * If pixel_scale is not 1.0, it specifies an amount to scale each pixel value
 * of the source image before applying it to the target image.
 */
void PNMImage::
lighten_sub_image(const PNMImage &copy, int xto, int yto,
                  int xfrom, int yfrom, int x_size, int y_size,
                  float pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() && pixel_scale == 1.0f &&
      get_color_space() == CS_linear && copy.get_color_space() == CS_linear) {
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
        LRGBColorf c = copy.get_xel(x - xmin + xfrom, y - ymin + yfrom);
        LRGBColorf o = get_xel(x, y);
        LRGBColorf p;
        p.set(max(c[0] * pixel_scale, o[0]),
              max(c[1] * pixel_scale, o[1]),
              max(c[2] * pixel_scale, o[2]));
        set_xel(x, y, p);
      }
    }

    if (has_alpha() && copy.has_alpha()) {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          float c = copy.get_alpha(x - xmin + xfrom, y - ymin + yfrom);
          float o = get_alpha(x, y);
          set_alpha(x, y, max(c * pixel_scale, o));
        }
      }
    }
  }
}

/**
 * Selectively copies each pixel from either one source or another source,
 * depending on the pixel value of the indicated channel of select_image.
 *
 * For each pixel (x, y):
 *
 * s = select_image.get_channel(x, y, channel). Set this image's (x, y) to:
 *
 * lt.get_xel(x, y) if s < threshold, or
 *
 * ge.get_xel(x, y) if s >= threshold
 *
 * Any of select_image, lt, or ge may be the same PNMImge object as this
 * image, or the same as each other; or they may all be different.  All images
 * must be the same size.  As a special case, lt and ge may both be 1x1 images
 * instead of the source image size.
 */
void PNMImage::
threshold(const PNMImage &select_image, int channel, float threshold,
          const PNMImage &lt, const PNMImage &ge) {
  nassertv(get_x_size() <= select_image.get_x_size() && get_y_size() <= select_image.get_y_size());
  nassertv(channel >= 0 && channel < select_image.get_num_channels());

  xelval threshold_val = select_image.to_val(threshold);

  if (lt.get_x_size() == 1 && lt.get_y_size() == 1 &&
      ge.get_x_size() == 1 && ge.get_y_size() == 1) {
    // FIXME: what if select_image has different color space?  1x1 source
    // images.
    xel lt_val = lt.get_xel_val(0, 0);
    xelval lt_alpha = 0;
    if (lt.has_alpha()) {
      lt_alpha = lt.get_alpha_val(0, 0);
    }
    if (lt.get_maxval() != get_maxval()) {
      float scale = (float)get_maxval() / (float)lt.get_maxval();
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
      float scale = (float)get_maxval() / (float)ge.get_maxval();
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

    if (get_maxval() == lt.get_maxval() && get_maxval() == ge.get_maxval() &&
        get_color_space() == lt.get_color_space() &&
        get_color_space() == ge.get_color_space()) {
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
      // General case: the maxvals are different.  Copy by floating-point
      // value.
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

/**
 * Replaces this image with a grayscale image whose gray channel represents
 * the linear Manhattan distance from the nearest dark pixel in the given mask
 * image, up to the specified radius value (which also becomes the new
 * maxval).  radius may range from 0 to maxmaxval; smaller values will compute
 * faster.  A dark pixel is defined as one whose pixel value is < threshold.
 *
 * If shrink_from_border is true, then the mask image is considered to be
 * surrounded by a border of dark pixels; otherwise, the border isn't
 * considered.
 *
 * This can be used, in conjunction with threshold, to shrink a mask image
 * inwards by a certain number of pixels.
 *
 * The mask image may be the same image as this one, in which case it is
 * destructively modified by this process.
 */
void PNMImage::
fill_distance_inside(const PNMImage &mask, float threshold, int radius, bool shrink_from_border) {
  nassertv(radius <= PNM_MAXMAXVAL);
  PNMImage dist(mask.get_x_size(), mask.get_y_size(), 1, radius, nullptr, CS_linear);
  dist.fill_val(radius);

  xelval threshold_val = mask.to_val(threshold);

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

/**
 * Replaces this image with a grayscale image whose gray channel represents
 * the linear Manhattan distance from the nearest white pixel in the given
 * mask image, up to the specified radius value (which also becomes the new
 * maxval).  radius may range from 0 to maxmaxval; smaller values will compute
 * faster.  A white pixel is defined as one whose pixel value is >= threshold.
 *
 * This can be used, in conjunction with threshold, to grow a mask image
 * outwards by a certain number of pixels.
 *
 * The mask image may be the same image as this one, in which case it is
 * destructively modified by this process.
 */
void PNMImage::
fill_distance_outside(const PNMImage &mask, float threshold, int radius) {
  nassertv(radius <= PNM_MAXMAXVAL);
  PNMImage dist(mask.get_x_size(), mask.get_y_size(), 1, radius, nullptr, CS_linear);
  dist.fill_val(radius);

  xelval threshold_val = mask.to_val(threshold);

  for (int yi = 0; yi < mask.get_y_size(); ++yi) {
    for (int xi = 0; xi < mask.get_x_size(); ++xi) {
      if (mask.get_gray_val(xi, yi) >= threshold_val) {
        dist.do_fill_distance(xi, yi, 0);
      }
    }
  }

  take_from(dist);
}

/**
 * index_image is a WxH grayscale image, while pixel_values is an Nx1 color
 * (or grayscale) image.  Typically pixel_values will be a 256x1 image.
 *
 * Fills the PNMImage with a new image the same width and height as
 * index_image, with the same number of channels as pixel_values.
 *
 * Each pixel of the new image is computed with the formula:
 *
 * new_image(x, y) = pixel_values(index_image(x, y)[channel], 0)
 *
 * At present, no interpolation is performed; the nearest value in
 * pixel_values is discovered.  This may change in the future.
 */
void PNMImage::
indirect_1d_lookup(const PNMImage &index_image, int channel,
                   const PNMImage &pixel_values) {
  clear(index_image.get_x_size(), index_image.get_y_size(),
        pixel_values.get_num_channels(), pixel_values.get_maxval());

  for (int yi = 0; yi < get_y_size(); ++yi) {
    for (int xi = 0; xi < get_x_size(); ++xi) {
      int v = int(index_image.get_channel(xi, yi, channel) * (pixel_values.get_x_size() - 1) + 0.5);
      nassertv(v >= 0 && v < pixel_values.get_x_size());
      set_xel_val(xi, yi, pixel_values.get_xel_val(v, 0));
      if (has_alpha()) {
        set_alpha_val(xi, yi, pixel_values.get_alpha_val(v, 0));
      }
    }
  }
}

/**
 * Rescales the RGB channel values so that any values in the original image
 * between min_val and max_val are expanded to the range 0 .. 1.  Values below
 * min_val are set to 0, and values above max_val are set to 1. Does not
 * affect the alpha channel, if any.
 */
void PNMImage::
rescale(float min_val, float max_val) {
  float scale = max_val - min_val;

  if (_num_channels <= 2) {
    // Grayscale.
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        float val = get_gray(x, y);
        set_gray(x, y, (val - min_val) / scale);
      }
    }
  } else {
    // RGB(A).
    for (int y = 0; y < get_y_size(); y++) {
      for (int x = 0; x < get_x_size(); x++) {
        LRGBColorf xel = get_xel(x, y);
        set_xel(x, y,
                (xel[0] - min_val) / scale,
                (xel[1] - min_val) / scale,
                (xel[2] - min_val) / scale);
      }
    }
  }
}

/**
 * Copies just a single channel from the source image into a single channel of
 * this image, leaving the remaining channels alone.
 */
void PNMImage::
copy_channel(const PNMImage &copy, int xto, int yto, int cto,
             int xfrom, int yfrom, int cfrom,
             int x_size, int y_size) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  if (get_maxval() == copy.get_maxval() &&
      get_color_space() == copy.get_color_space()) {
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

/**
 * Renders a solid-color circle, with a fuzzy edge, into the center of the
 * PNMImage.  If the PNMImage is non-square, this actually renders an ellipse.
 *
 * The min_radius and max_radius are in the scale 0..1, where 1.0 means the
 * full width of the image.  If min_radius == max_radius, the edge is sharp
 * (but still antialiased); otherwise, the pixels between min_radius and
 * max_radius are smoothly blended between fg and bg colors.
 */
void PNMImage::
render_spot(const LColorf &fg, const LColorf &bg,
            float min_radius, float max_radius) {
  if (_x_size == 0 || _y_size == 0) {
    return;
  }

  float x_scale = 2.0 / _x_size;
  float y_scale = 2.0 / _y_size;

  // If the width is even, x_center1 == x_center0.  If the width is odd,
  // x_center1 == x_center0 + 1.
  int x_center0 = _x_size / 2;
  int y_center0 = _y_size / 2;
  int x_center1 = (_x_size + 1) / 2;
  int y_center1 = (_y_size + 1) / 2;

  float min_r2 = min_radius * min_radius;
  float max_r2 = max_radius * max_radius;

  for (int yi = 0; yi < y_center1; ++yi) {
    float y = yi * y_scale;
    float y2_inner = y * y;
    float y2_outer = (y + y_scale) * (y + y_scale);
    for (int xi = 0; xi < x_center1; ++xi) {
      float x = xi * x_scale;
      float d2_inner = (x * x + y2_inner);
      float d2_outer = ((x + x_scale) * (x + x_scale) + y2_outer);
      float d2_a = ((x + x_scale) * (x + x_scale) + y2_inner);
      float d2_b = (x * x + y2_outer);

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
        LColorf c_outer, c_inner, c_a, c_b;
        compute_spot_pixel(c_outer, d2_outer, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_inner, d2_inner, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_a, d2_a, min_radius, max_radius, fg, bg);
        compute_spot_pixel(c_b, d2_b, min_radius, max_radius, fg, bg);

        // Now average all four pixels for the antialiased result.
        LColorf c;
        c = (c_outer + c_inner + c_a + c_b) * 0.25;

        set_xel_a(x_center1 - 1 - xi, y_center1 - 1 - yi, c);
        set_xel_a(x_center0 + xi, y_center1 - 1 - yi, c);
        set_xel_a(x_center1 - 1 - xi, y_center0 + yi, c);
        set_xel_a(x_center0 + xi, y_center0 + yi, c);
      }
    }
  }
}

/**
 * Expands the image by the indicated number of pixels on each edge.  The new
 * pixels are set to the indicated color.
 *
 * If any of the values is negative, this actually crops the image.
 */
void PNMImage::
expand_border(int left, int right, int bottom, int top,
              const LColorf &color) {
  PNMImage new_image(get_x_size() + left + right,
                     get_y_size() + bottom + top,
                     get_num_channels(), get_maxval(),
                     get_type(), get_color_space());
  new_image.fill(color[0], color[1], color[2]);
  if (has_alpha()) {
    new_image.alpha_fill(color[3]);
  }
  new_image.copy_sub_image(*this, left, top);

  take_from(new_image);
}

/**
 * Resizes from the indicated image into this one by performing a nearest-
 * point sample.
 */
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

/**
 * Computes a histogram of the colors used in the image.
 */
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
  std::sort(pixels.begin(), pixels.end());

  histogram.swap(pixels, hist_map);
}

/**
 * Reduces the number of unique colors in the image to (at most) the given
 * count.  Fewer colors than requested may be left in the image after this
 * operation, but never more.
 *
 * At present, this is only supported on images without an alpha channel.
 *
 * @since 1.10.5
 */
void PNMImage::
quantize(size_t max_colors) {
  nassertv(_array != nullptr);
  nassertv(!has_alpha());
  size_t array_size = _x_size * _y_size;

  // Get all the unique colors in this image.
  pmap<xel, xel> color_map;
  for (size_t i = 0; i < array_size; ++i) {
    color_map[_array[i]];
  }

  size_t num_colors = color_map.size();
  if (num_colors <= max_colors) {
    // We are already down to the requested number of colors.
    return;
  }

  // Collect all the colors into a contiguous array.
  xel *colors = (xel *)alloca(num_colors * sizeof(xel));
  size_t i = 0;
  for (pmap<xel, xel>::const_iterator it = color_map.begin();
       it != color_map.end(); ++it) {
    colors[i++] = it->first;
  }
  nassertv(i == num_colors);

  // Apply the median cut algorithm, which will give us a color map.
  r_quantize(color_map, max_colors, colors, num_colors);

  // Replace all the existing colors with the corresponding bucket average.
  for (size_t i = 0; i < array_size; ++i) {
    _array[i] = color_map[_array[i]];
  }
}

/**
 * Fills the image with a grayscale perlin noise pattern based on the
 * indicated parameters.  Uses set_xel to set the grayscale values.  The sx
 * and sy parameters are in multiples of the size of this image.  See also the
 * PerlinNoise2 class in mathutil.
 */
void PNMImage::
perlin_noise_fill(float sx, float sy, int table_size, unsigned long seed) {
  float x, y;
  float noise;
  PerlinNoise2 perlin (sx * _x_size, sy * _y_size, table_size, seed);
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      noise = perlin.noise(x, y);
      set_xel(x, y, 0.5 * (noise + 1.0));
    }
  }
}

/**
 * Variant of perlin_noise_fill that uses an existing StackedPerlinNoise2
 * object.
 */
void PNMImage::
perlin_noise_fill(StackedPerlinNoise2 &perlin) {
  float x, y;
  float noise;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      noise = perlin.noise(x / (float) _x_size, y / (float) _y_size);
      set_xel(x, y, 0.5 * (noise + 1.0));
    }
  }
}

/**
 * Transforms every pixel using the operation (Ro,Go,Bo) =
 * conv.xform_point(Ri,Gi,Bi); Input must be a color image.
 */
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

/**
 * Adjusts each channel of the image by raising the corresponding component
 * value to the indicated exponent, such that L' = L ^ exponent.  For a
 * grayscale image, the blue_exponent value is used for the grayscale value,
 * and red_exponent and green_exponent are unused.
 */
void PNMImage::
apply_exponent(float red_exponent, float green_exponent, float blue_exponent,
               float alpha_exponent) {
  int num_channels = _num_channels;
  if (has_alpha() && alpha_exponent == 1.0f) {
    // If the alpha_exponent is 1, don't bother to apply it.
    --num_channels;
  }

  int x, y;

  if (red_exponent == 1.0f && green_exponent == 1.0f && blue_exponent == 1.0f) {
    // If the RGB components are all 1, apply only to the alpha channel.
    switch (num_channels) {
    case 1:
    case 3:
      break;

    case 2:
    case 4:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          float alpha = get_alpha(x, y);
          alpha = cpow(alpha, blue_exponent);
          set_alpha(x, y, alpha);
        }
      }
      break;
    }

  } else {
    // Apply to the color andor alpha channels.

    switch (num_channels) {
    case 1:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          float gray = get_gray(x, y);
          gray = cpow(gray, blue_exponent);
          set_gray(x, y, gray);
        }
      }
      break;

    case 2:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          float gray = get_gray(x, y);
          gray = cpow(gray, blue_exponent);
          set_gray(x, y, gray);

          float alpha = get_alpha(x, y);
          alpha = cpow(alpha, blue_exponent);
          set_alpha(x, y, alpha);
        }
      }
      break;

    case 3:
      for (y = 0; y < _y_size; ++y) {
        for (x = 0; x < _x_size; ++x) {
          LRGBColorf color = get_xel(x, y);
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
          LColorf color = get_xel_a(x, y);
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

/**
 * Sets the _default_rc,bc,gc values appropriately according to the color type
 * of the image, so that get_bright() will return a meaningful value for both
 * color and grayscale images.
 */
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

/**
 * Sets the _xel_encoding value apppropriately according to the color space,
 * maxval and whether the image has an alpha channel, so that to_val and
 * from_val will work correctly (and possibly more efficiently). Should be
 * called after any call to set_maxval.
 */
void PNMImage::
setup_encoding() {
  if (_maxval == 0) {
    _inv_maxval = 0.0f;
  } else {
    _inv_maxval = 1.0f / (float)_maxval;
  }

  if (has_alpha()) {
    switch (_color_space) {
    case CS_linear:
      _xel_encoding = XE_generic_alpha;
      break;

    case CS_sRGB:
      if (get_maxval() == 255) {
        if (has_sse2_sRGB_encode()) {
          _xel_encoding = XE_uchar_sRGB_alpha_sse2;
        } else {
          _xel_encoding = XE_uchar_sRGB_alpha;
        }
      } else {
        _xel_encoding = XE_generic_sRGB_alpha;
      }
      break;

    case CS_scRGB:
      _xel_encoding = XE_scRGB_alpha;
      nassertv(get_maxval() == 65535);
      break;

    default:
      nassert_raise("invalid color space");
      break;
    }
  } else {
    switch (_color_space) {
    case CS_linear:
      _xel_encoding = XE_generic;
      break;

    case CS_sRGB:
      if (get_maxval() == 255) {
        if (has_sse2_sRGB_encode()) {
          _xel_encoding = XE_uchar_sRGB_sse2;
        } else {
          _xel_encoding = XE_uchar_sRGB;
        }
      } else {
        _xel_encoding = XE_generic_sRGB;
      }
      break;

    case CS_scRGB:
      _xel_encoding = XE_scRGB;
      nassertv(get_maxval() == 65535);
      break;

    default:
      nassert_raise("invalid color space");
      break;
    }
  }
}

/**
 * Recursive implementation of quantize() using the median cut algorithm.
 */
void PNMImage::
r_quantize(pmap<xel, xel> &color_map, size_t max_colors,
           xel *colors, size_t num_colors) {
  if (num_colors <= max_colors) {
    // All points in this bucket can be preserved 1:1.
    for (size_t i = 0; i < num_colors; ++i) {
      const xel &col = colors[i];
      color_map[col] = col;
    }
    return;
  }
  else if (max_colors == 1) {
    // We've reached the target.  Calculate the average, in linear space.
    LRGBColorf avg(0);
    for (size_t i = 0; i < num_colors; ++i) {
      avg += from_val(colors[i]);
    }
    avg *= 1.0f / num_colors;
    xel avg_val = to_val(avg);

    // Map all colors in this bucket to the avg.
    for (size_t i = 0; i < num_colors; ++i) {
      color_map[colors[i]] = avg_val;
    }
    return;
  }
  else if (max_colors == 0) {
    // Not sure how this happens, but we can't preserve any color here.
    return;
  }

  // Find the minimum/maximum RGB values.  We should probably do this in
  // linear space, but eh.
  xelval min_r = _maxval;
  xelval min_g = _maxval;
  xelval min_b = _maxval;
  xelval max_r = 0, max_g = 0, max_b = 0;
  for (size_t i = 0; i < num_colors; ++i) {
    const xel &col = colors[i];
    min_r = std::min(min_r, col.r);
    max_r = std::max(max_r, col.r);
    min_g = std::min(min_g, col.g);
    max_g = std::max(max_g, col.g);
    min_b = std::min(min_b, col.b);
    max_b = std::max(max_b, col.b);
  }

  int diff_r = max_r - min_r;
  int diff_g = max_g - min_g;
  int diff_b = max_b - min_b;

  auto sort_by_red = [](const xel &c1, const xel &c2) {
    return c1.r < c2.r;
  };
  auto sort_by_green = [](const xel &c1, const xel &c2) {
    return c1.g < c2.g;
  };
  auto sort_by_blue = [](const xel &c1, const xel &c2) {
    return c1.b < c2.b;
  };

  // Sort by the component with the most variation.
  if (diff_g >= diff_r) {
    if (diff_g >= diff_b) {
      std::sort(colors, colors + num_colors, sort_by_green);
    } else {
      std::sort(colors, colors + num_colors, sort_by_blue);
    }
  } else if (diff_r >= diff_b) {
    std::sort(colors, colors + num_colors, sort_by_red);
  } else {
    std::sort(colors, colors + num_colors, sort_by_blue);
  }

  // Subdivide the sorted colors into two buckets, and recurse.
  size_t max_colors_1 = max_colors / 2;
  size_t max_colors_2 = max_colors - max_colors_1;
  size_t num_colors_1 = num_colors / 2;
  size_t num_colors_2 = num_colors - num_colors_1;
  r_quantize(color_map, max_colors_1, colors, num_colors_1);
  r_quantize(color_map, max_colors_2, colors + num_colors_1, num_colors_2);
}

/**
 * Recursively fills in the minimum distance measured from a certain set of
 * points into the gray channel.
 */
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

/**
 * Returns the average color of all of the pixels in the image.
 */
LRGBColorf PNMImage::
get_average_xel() const {
  LRGBColorf color (LRGBColorf::zero());
  if (_x_size == 0 || _y_size == 0) {
    return color;
  }

  float factor = 1.0f / (float)(_x_size * _y_size);

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      color += get_xel(x, y) * factor;
    }
  }

  return color;
}

/**
 * Returns the average color of all of the pixels in the image, including the
 * alpha channel.
 */
LColorf PNMImage::
get_average_xel_a() const {
  LColorf color (LColorf::zero());
  if (_x_size == 0 || _y_size == 0) {
    return color;
  }

  float factor = 1.0f / (float)(_x_size * _y_size);

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      color += get_xel_a(x, y) * factor;
    }
  }

  return color;
}

/**
 * Returns the average grayscale component of all of the pixels in the image.
 */
float PNMImage::
get_average_gray() const {
  float gray = 0.0;
  if (_x_size == 0 || _y_size == 0) {
    return gray;
  }

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      gray += get_gray(x, y);
    }
  }

  gray /= (float)(_x_size * _y_size);
  return gray;
}

/**
 * Returns a new PNMImage that is the complement of this PNMImage.  This
 * operation is not color-space correct.
 */
PNMImage PNMImage::
operator ~ () const {
  PNMImage target (*this);
  size_t array_size = _x_size * _y_size;

  if (_array != nullptr && _alpha != nullptr) {
    for (size_t i = 0; i < array_size; ++i) {
      target._array[i].r = _maxval - _array[i].r;
      target._array[i].g = _maxval - _array[i].g;
      target._array[i].b = _maxval - _array[i].b;
      target._alpha[i] = _maxval - _alpha[i];
    }
  } else if (_array != nullptr) {
    for (size_t i = 0; i < array_size; ++i) {
      target._array[i].r = _maxval - _array[i].r;
      target._array[i].g = _maxval - _array[i].g;
      target._array[i].b = _maxval - _array[i].b;
    }
  } else if (_alpha != nullptr) {
    for (size_t i = 0; i < array_size; ++i) {
      target._alpha[i] = _maxval - _alpha[i];
    }
  }
  return target;
}

/**
 * Sets each pixel value to the sum of the corresponding pixel values in the
 * two given images.  Only valid when both images have the same size.
 */
void PNMImage::
operator += (const PNMImage &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(_x_size == other._x_size && _y_size == other._y_size);

  if (get_maxval() == other.get_maxval() &&
      get_color_space() == CS_linear &&
      other.get_color_space() == CS_linear) {
    size_t array_size = _x_size * _y_size;

    // Simple case: add vals directly.
    if (_alpha != nullptr && other._alpha != nullptr) {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r + (int)other._array[i].r);
        _array[i].g = clamp_val((int)_array[i].g + (int)other._array[i].g);
        _array[i].b = clamp_val((int)_array[i].b + (int)other._array[i].b);
        _alpha[i] = clamp_val((int)_alpha[i] + (int)other._alpha[i]);
      }
    } else {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r + (int)other._array[i].r);
        _array[i].g = clamp_val((int)_array[i].g + (int)other._array[i].g);
        _array[i].b = clamp_val((int)_array[i].b + (int)other._array[i].b);
      }
    }
  } else {
    // Not a linear color space: convert back and forth.
    int x, y;
    for (x = 0; x < _x_size; ++x) {
      for (y = 0; y < _y_size; ++y) {
        set_xel_a(x, y, get_xel_a(x, y) + other.get_xel_a(x, y));
      }
    }
  }
}

/**
 * Adds the provided color to each pixel in this image.
 */
void PNMImage::
operator += (const LColorf &other) {
  nassertv(is_valid());

  if (get_color_space() == CS_linear) {
    size_t array_size = _x_size * _y_size;

    // Note: don't use to_val here because it clamps values below 0
    int add_r = (int)(other.get_x() * get_maxval() + 0.5);
    int add_g = (int)(other.get_y() * get_maxval() + 0.5);
    int add_b = (int)(other.get_z() * get_maxval() + 0.5);
    int add_a = (int)(other.get_w() * get_maxval() + 0.5);

    if (_alpha != nullptr) {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r + add_r);
        _array[i].g = clamp_val((int)_array[i].g + add_g);
        _array[i].b = clamp_val((int)_array[i].b + add_b);
        _alpha[i] = clamp_val((int)_alpha[i] + add_a);
      }

    } else {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r + add_r);
        _array[i].g = clamp_val((int)_array[i].g + add_g);
        _array[i].b = clamp_val((int)_array[i].b + add_b);
      }
    }
  } else {
    // Not a linear color space: convert back and forth.
    int x, y;
    for (x = 0; x < _x_size; ++x) {
      for (y = 0; y < _y_size; ++y) {
        set_xel_a(x, y, get_xel_a(x, y) + other);
      }
    }
  }
}

/**
 * Subtracts each pixel from the right image from each pixel value in this
 * image.  Only valid when both images have the same size.
 */
void PNMImage::
operator -= (const PNMImage &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(_x_size == other._x_size && _y_size == other._y_size);

  if (get_maxval() == other.get_maxval() &&
      get_color_space() == CS_linear &&
      other.get_color_space() == CS_linear) {
    size_t array_size = _x_size * _y_size;

    // Simple case: subtract vals directly.
    if (_alpha != nullptr && other._alpha != nullptr) {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r - (int)other._array[i].r);
        _array[i].g = clamp_val((int)_array[i].g - (int)other._array[i].g);
        _array[i].b = clamp_val((int)_array[i].b - (int)other._array[i].b);
        _alpha[i] = clamp_val((int)_alpha[i] - (int)other._alpha[i]);
      }
    } else {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)_array[i].r - (int)other._array[i].r);
        _array[i].g = clamp_val((int)_array[i].g - (int)other._array[i].g);
        _array[i].b = clamp_val((int)_array[i].b - (int)other._array[i].b);
      }
    }
  } else {
    // Not a linear color space: convert back and forth.
    int x, y;
    for (x = 0; x < _x_size; ++x) {
      for (y = 0; y < _y_size; ++y) {
        set_xel_a(x, y, get_xel_a(x, y) - other.get_xel_a(x, y));
      }
    }
  }
}

/**
 * Subtracts the provided color from each pixel in this image.
 */
void PNMImage::
operator -= (const LColorf &other) {
  (*this) += -other;
}

/**
 * Multiples each pixel in this image by each pixel value from the right
 * image.  Note that the floating-point values in the 0..1 range are
 * multiplied, not in the 0..maxval range.  Only valid when both images have
 * the same size.
 */
void PNMImage::
operator *= (const PNMImage &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(_x_size == other._x_size && _y_size == other._y_size);

  int x, y;
  for (x = 0; x < _x_size; ++x) {
    for (y = 0; y < _y_size; ++y) {
      set_xel_a(x, y, get_xel_a(x, y) - other.get_xel_a(x, y));
    }
  }
}

/**
 * Multiplies every pixel value in the image by a constant floating-point
 * multiplier value.  This affects all channels.
 */
void PNMImage::
operator *= (float multiplier) {
  nassertv(is_valid());

  if (get_color_space() == CS_linear) {
    size_t array_size = _x_size * _y_size;

    if (_alpha != nullptr) {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)(_array[i].r * multiplier + 0.5f));
        _array[i].g = clamp_val((int)(_array[i].g * multiplier + 0.5f));
        _array[i].b = clamp_val((int)(_array[i].b * multiplier + 0.5f));
        _alpha[i] = clamp_val((int)(_alpha[i] * multiplier + 0.5f));
      }

    } else {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)(_array[i].r * multiplier + 0.5f));
        _array[i].g = clamp_val((int)(_array[i].g * multiplier + 0.5f));
        _array[i].b = clamp_val((int)(_array[i].b * multiplier + 0.5f));
      }
    }
  } else {
    // Not a linear color space: convert back and forth.
    int x, y;
    for (x = 0; x < _x_size; ++x) {
      for (y = 0; y < _y_size; ++y) {
        set_xel_a(x, y, get_xel_a(x, y) * multiplier);
      }
    }
  }
}

/**
 * Multiplies the provided color to each pixel in this image.  This is a
 * component-wise multiplication.
 */
void PNMImage::
operator *= (const LColorf &other) {
  nassertv(is_valid());

  if (get_color_space() == CS_linear) {
    size_t array_size = _x_size * _y_size;

    if (_alpha != nullptr) {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)(_array[i].r * other[0] + 0.5f));
        _array[i].g = clamp_val((int)(_array[i].g * other[1] + 0.5f));
        _array[i].b = clamp_val((int)(_array[i].b * other[2] + 0.5f));
        _alpha[i] = clamp_val((int)(_alpha[i] * other[3] + 0.5f));
      }

    } else {
      for (size_t i = 0; i < array_size; ++i) {
        _array[i].r = clamp_val((int)(_array[i].r * other[0] + 0.5f));
        _array[i].g = clamp_val((int)(_array[i].g * other[1] + 0.5f));
        _array[i].b = clamp_val((int)(_array[i].b * other[2] + 0.5f));
      }
    }
  } else {
    // Not a linear color space: convert back and forth.
    int x, y;
    for (x = 0; x < _x_size; ++x) {
      for (y = 0; y < _y_size; ++y) {
        LColorf color = get_xel_a(x, y);
        color.componentwise_mult(other);
        set_xel_a(x, y, color);
      }
    }
  }
}
