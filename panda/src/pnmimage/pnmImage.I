// Filename: pnmImage.I
// Created by:  drose (15Jun00)
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

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
PNMImage() {
  _array = NULL;
  _alpha = NULL;

  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
PNMImage(int x_size, int y_size, int num_channels, xelval maxval,
         PNMFileType *type, ColorSpace color_space) {
  _array = NULL;
  _alpha = NULL;

  clear(x_size, y_size, num_channels, maxval, type, color_space);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
PNMImage(const PNMImage &copy) {
  // We don't need to invoke PNMImageHeader's copy constructor,
  // because we'll just call copy_from().
  _array = NULL;
  _alpha = NULL;

  copy_from(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
operator = (const PNMImage &copy) {
  copy_from(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
~PNMImage() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clamp_val
//       Access: Published
//  Description: A handy function to clamp values to
//               [0..get_maxval()].
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
clamp_val(int input_value) const {
  return (xelval)min(max(0, input_value), (int)get_maxval());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::to_val
//       Access: Published
//  Description: A handy function to scale non-alpha values from
//               [0..1] to [0..get_maxval()].  Do not use this for
//               alpha values, see to_alpha_val.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
to_val(float input_value) const {
  switch (_xel_encoding) {
  case XE_generic:
  case XE_generic_alpha:
    return clamp_val((int)(input_value * get_maxval() + 0.5f));

  case XE_generic_sRGB:
  case XE_generic_sRGB_alpha:
    return clamp_val((int)
      (encode_sRGB_float(input_value) * get_maxval() + 0.5f));

  case XE_uchar_sRGB:
  case XE_uchar_sRGB_alpha:
    return encode_sRGB_uchar(input_value);

  case XE_uchar_sRGB_sse2:
  case XE_uchar_sRGB_alpha_sse2:
    return encode_sRGB_uchar_sse2(input_value);

  case XE_scRGB:
  case XE_scRGB_alpha:
    return min(max(0, (int)((8192 * input_value) + 4096.5f)), 65535);

  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::to_alpha_val
//       Access: Published
//  Description: A handy function to scale alpha values from [0..1]
//               to [0..get_maxval()].
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
to_alpha_val(float input_value) const {
  return clamp_val((int)(input_value * get_maxval() + 0.5));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::from_val
//       Access: Published
//  Description: A handy function to scale non-alpha values from
//               [0..get_maxval()] to [0..1].  Do not use this for
//               alpha values, see from_alpha_val.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
from_val(xelval input_value) const {
  switch (_xel_encoding) {
  case XE_generic:
  case XE_generic_alpha:
    return (float)input_value * _inv_maxval;

  case XE_generic_sRGB:
  case XE_generic_sRGB_alpha:
    return decode_sRGB_float((float)input_value * _inv_maxval);

  case XE_uchar_sRGB:
  case XE_uchar_sRGB_alpha:
  case XE_uchar_sRGB_sse2:
  case XE_uchar_sRGB_alpha_sse2:
    return decode_sRGB_float((unsigned char)input_value);

  case XE_scRGB:
  case XE_scRGB_alpha:
    return (input_value - 4096) * (1.f / 8192.f);

  default:
    return 0.0f;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::from_alpha_val
//       Access: Published
//  Description: A handy function to scale alpha values from
//               [0..get_maxval()] to [0..1].
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
from_alpha_val(xelval input_value) const {
  return (float)input_value * _inv_maxval;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given color.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(float red, float green, float blue) {
  fill_val(to_val(red), to_val(green), to_val(blue));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given grayscale level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(float gray) {
  fill(gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill_val
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given grayscale level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill_val(xelval gray) {
  fill_val(gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::alpha_fill
//       Access: Published
//  Description: Sets the entire alpha channel to the given level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
alpha_fill(float alpha) {
  alpha_fill_val(to_alpha_val(alpha));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_read_size
//       Access: Published
//  Description: Specifies the size to we'd like to scale the image
//               upon reading it.  This will affect the next call to
//               read().  This is usually used to reduce the image
//               size, e.g. for a thumbnail.
//
//               If the file type reader supports it (e.g. JPEG), then
//               this will scale the image during the read operation,
//               consequently reducing memory and CPU utilization.  If
//               the file type reader does not support it, this will
//               load the image normally, and them perform a linear
//               scale after it has been loaded.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_read_size(int x_size, int y_size) {
  _read_x_size = x_size;
  _read_y_size = y_size;
  _has_read_size = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clear_read_size
//       Access: Published
//  Description: Undoes the effect of a previous call to
//               set_read_size().
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
clear_read_size() {
  _has_read_size = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::has_read_size
//       Access: Published
//  Description: Returns true if set_read_size() has been called.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImage::
has_read_size() const {
  return _has_read_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_read_x_size
//       Access: Published
//  Description: Returns the requested x_size of the image if
//               set_read_size() has been called, or the image x_size
//               otherwise (if it is known).
////////////////////////////////////////////////////////////////////
INLINE int PNMImage::
get_read_x_size() const {
  return _has_read_size ? _read_x_size : get_x_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_read_y_size
//       Access: Published
//  Description: Returns the requested y_size of the image if
//               set_read_size() has been called, or the image y_size
//               otherwise (if it is known).
////////////////////////////////////////////////////////////////////
INLINE int PNMImage::
get_read_y_size() const {
  return _has_read_size ? _read_y_size : get_y_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_color_space
//       Access: Published
//  Description: Returns the color space in which the image is
//               encoded.
////////////////////////////////////////////////////////////////////
INLINE ColorSpace PNMImage::
get_color_space() const {
  return _color_space;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::is_valid
//       Access: Published
//  Description: Returns true if the image has been read in or
//               correctly initialized with a height and width.  If
//               this returns false, virtually all member functions
//               except clear() and read() are invalid function calls.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImage::
is_valid() const {
  return (_array != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_num_channels
//       Access: Published
//  Description: Changes the number of channels associated with the
//               image.  The new number of channels must be an integer
//               in the range 1 through 4, inclusive.  This will
//               allocate and/or deallocate memory as necessary to
//               accommodate; see set_color_type().
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_num_channels(int num_channels) {
  nassertv(num_channels >= 1 && num_channels <= 4);
  set_color_type((ColorType)num_channels);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::add_alpha
//       Access: Published
//  Description: Adds an alpha channel to the image, if it does not
//               already have one.  The alpha channel is initialized
//               to zeros.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
add_alpha() {
  set_color_type(is_grayscale() ? CT_two_channel : CT_four_channel);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::remove_alpha
//       Access: Published
//  Description: Removes the image's alpha channel, if it exists.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
remove_alpha() {
  set_color_type(is_grayscale() ? CT_grayscale : CT_color);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_grayscale
//       Access: Published
//  Description: Converts the image from RGB to grayscale.  Any alpha
//               channel, if present, is left undisturbed.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
make_grayscale() {
  make_grayscale(_default_rc, _default_gc, _default_bc);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_rgb
//       Access: Published
//  Description: Converts the image from grayscale to RGB.  Any alpha
//               channel, if present, is left undisturbed.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
make_rgb() {
  set_color_type(has_alpha() ? CT_four_channel : CT_color);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel_val
//       Access: Published
//  Description: Returns the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xel &PNMImage::
get_xel_val(int x, int y) {
  nassertr(x >= 0 && x < _x_size && y >= 0 && y < _y_size, _array[0]);
  return row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel_val
//       Access: Published
//  Description: Returns the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xel PNMImage::
get_xel_val(int x, int y) const {
  nassertr(x >= 0 && x < _x_size && y >= 0 && y < _y_size, _array[0]);
  return row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval, encoded in
//               the configured color space.  See set_xel if you
//               instead have a linearized and normalized
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, const xel &value) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  row(y)[x] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval, encoded in
//               the configured color space.  See set_xel if you
//               instead have a linearized and normalized
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, xelval r, xelval g, xelval b) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_ASSIGN(row(y)[x], r, g, b);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Published
//  Description: Changes all three color components at the indicated
//               pixel to the same value.  The value is in the range
//               component is in the range 0..maxval, encoded in
//               the configured color space.  See set_xel if you
//               instead have a linearized and normalized
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, xelval gray) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_ASSIGN(row(y)[x], gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_red_val
//       Access: Published
//  Description: Returns the red component color at the indicated
//               pixel.  The value returned is in the range 0..maxval
//               and encoded in the configured color space.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_red_val(int x, int y) const {
  return PPM_GETR(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_green_val
//       Access: Published
//  Description: Returns the green component color at the indicated
//               pixel.  The value returned is in the range 0..maxval
//               and encoded in the configured color space.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_green_val(int x, int y) const {
  return PPM_GETG(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_blue_val
//       Access: Published
//  Description: Returns the blue component color at the indicated
//               pixel.  The value returned is in the range 0..maxval
//               and encoded in the configured color space.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_blue_val(int x, int y) const {
  return PPM_GETB(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_gray_val
//       Access: Published
//  Description: Returns the gray component color at the indicated
//               pixel.  This only has a meaningful value for
//               grayscale images; for other image types, this returns
//               the value of the blue channel only.  However, also
//               see the get_bright() function.
//               The value returned is in the range 0..maxval and
//               encoded in the configured color space.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_gray_val(int x, int y) const {
  return PPM_GETB(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_alpha_val
//       Access: Published
//  Description: Returns the alpha component color at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value returned is in the
//               range 0..maxval and always linear.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_alpha_val(int x, int y) const {
  nassertr(_alpha != NULL && x >= 0 && x < _x_size && y >= 0 && y < _y_size, 0);
  return alpha_row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_red_val
//       Access: Published
//  Description: Sets the red component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval, encoded in the configured color space.
//               See set_red if you instead have a linearized and
//               normalized floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_red_val(int x, int y, xelval r) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_PUTR(row(y)[x], r);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_green_val
//       Access: Published
//  Description: Sets the green component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval, encoded in the configured color space.
//               See set_green if you instead have a linearized and
//               normalized floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_green_val(int x, int y, xelval g) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_PUTG(row(y)[x], g);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_blue_val
//       Access: Published
//  Description: Sets the blue component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval, encoded in the configured color space.
//               See set_blue if you instead have a linearized and
//               normalized floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_blue_val(int x, int y, xelval b) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_PUTB(row(y)[x], b);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_gray_val
//       Access: Published
//  Description: Sets the gray component color at the indicated
//               pixel.  This is only meaningful for grayscale images;
//               for other image types, this simply sets the blue
//               component color.  However, also see set_xel_val(),
//               which can set all the component colors to the same
//               grayscale level, and hence works correctly both for
//               grayscale and color images.  The value given should
//               be in the range 0..maxval, encoded in the configured
//               color space.  See set_gray if you instead have a
//               linearized normalized floating-point value.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_gray_val(int x, int y, xelval gray) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  PPM_PUTB(row(y)[x], gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_alpha_val
//       Access: Published
//  Description: Sets the alpha component color only at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value given should be in
//               the range 0..maxval.
//
//               This value is always linearly encoded, even if the
//               image is set to the sRGB color space.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_alpha_val(int x, int y, xelval a) {
  nassertv(_alpha != NULL && x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  alpha_row(y)[x] = a;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel
//       Access: Published
//  Description: Returns the RGB color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE LRGBColorf PNMImage::
get_xel(int x, int y) const {
  nassertr(x >= 0 && x < _x_size && y >= 0 && y < _y_size, LRGBColorf::zero());

  const xel &col = row(y)[x];

  switch (_xel_encoding) {
  case XE_generic:
  case XE_generic_alpha:
    return LRGBColorf(col.r, col.g, col.b) * _inv_maxval;

  case XE_generic_sRGB:
  case XE_generic_sRGB_alpha:
    return LRGBColorf(
      decode_sRGB_float(col.r * _inv_maxval),
      decode_sRGB_float(col.g * _inv_maxval),
      decode_sRGB_float(col.b * _inv_maxval));

  case XE_uchar_sRGB:
  case XE_uchar_sRGB_alpha:
  case XE_uchar_sRGB_sse2:
  case XE_uchar_sRGB_alpha_sse2:
    return LRGBColorf(
      decode_sRGB_float((unsigned char)col.r),
      decode_sRGB_float((unsigned char)col.g),
      decode_sRGB_float((unsigned char)col.b));

  case XE_scRGB:
  case XE_scRGB_alpha:
    return LRGBColorf((int)col.r - 4096,
                      (int)col.g - 4096,
                      (int)col.b - 4096) * (1.f / 8192.f);

  default:
    return LRGBColorf(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, const LRGBColorf &value) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);

  xel &col = row(y)[x];

  switch (_xel_encoding) {
  case XE_generic:
  case XE_generic_alpha:
    {
      LRGBColorf scaled = value * get_maxval() + 0.5f;
      col.r = clamp_val((int)scaled[0]);
      col.g = clamp_val((int)scaled[1]);
      col.b = clamp_val((int)scaled[2]);
    }
    break;

  case XE_generic_sRGB:
  case XE_generic_sRGB_alpha:
    col.r = clamp_val((int)
      (encode_sRGB_float(value[0]) * get_maxval() + 0.5f));
    col.g = clamp_val((int)
      (encode_sRGB_float(value[1]) * get_maxval() + 0.5f));
    col.b = clamp_val((int)
      (encode_sRGB_float(value[2]) * get_maxval() + 0.5f));
    break;

  case XE_uchar_sRGB:
  case XE_uchar_sRGB_alpha:
    encode_sRGB_uchar(LColorf(value, 0.0f), col);
    break;

  case XE_uchar_sRGB_sse2:
  case XE_uchar_sRGB_alpha_sse2:
    encode_sRGB_uchar_sse2(LColorf(value, 0.0f), col);
    break;

  case XE_scRGB:
  case XE_scRGB_alpha:
    {
      LRGBColorf scaled = value * 8192.f + 4096.5f;
      col.r = min(max(0, (int)scaled[0]), 65535);
      col.g = min(max(0, (int)scaled[1]), 65535);
      col.b = min(max(0, (int)scaled[2]), 65535);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, float r, float g, float b) {
  set_xel(x, y, LRGBColorf(r, g, b));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes all three color components at the indicated
//               pixel to the same value.  The value is a linearized
//               float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, float gray) {
  xelval val = to_val(gray);
  set_xel_val(x, y, val);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel_a
//       Access: Published
//  Description: Returns the RGBA color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE LColorf PNMImage::
get_xel_a(int x, int y) const {
  const xel &col = row(y)[x];

  switch (_xel_encoding) {
  case XE_generic:
    return LColorf(col.r, col.g, col.b, 0.0f) * _inv_maxval;

  case XE_generic_alpha:
    return LColorf(col.r, col.g, col.b, alpha_row(y)[x]) * _inv_maxval;

  case XE_generic_sRGB:
    return LColorf(
      decode_sRGB_float(col.r * _inv_maxval),
      decode_sRGB_float(col.g * _inv_maxval),
      decode_sRGB_float(col.b * _inv_maxval),
      0.0f);

  case XE_generic_sRGB_alpha:
    return LColorf(
      decode_sRGB_float(col.r * _inv_maxval),
      decode_sRGB_float(col.g * _inv_maxval),
      decode_sRGB_float(col.b * _inv_maxval),
      alpha_row(y)[x] * _inv_maxval);

  case XE_uchar_sRGB:
  case XE_uchar_sRGB_sse2:
    return LColorf(
      decode_sRGB_float((unsigned char)col.r),
      decode_sRGB_float((unsigned char)col.g),
      decode_sRGB_float((unsigned char)col.b),
      0.0f);

  case XE_uchar_sRGB_alpha:
  case XE_uchar_sRGB_alpha_sse2:
    return LColorf(
      decode_sRGB_float((unsigned char)col.r),
      decode_sRGB_float((unsigned char)col.g),
      decode_sRGB_float((unsigned char)col.b),
      alpha_row(y)[x] * (1.f / 255.f));

  case XE_scRGB:
    return LColorf((int)col.r - 4096,
                   (int)col.g - 4096,
                   (int)col.b - 4096,
                   0) * (1.f / 8192.f);

  case XE_scRGB_alpha:
    {
      static const LColorf scale(1.f / 8192.f, 1.f / 8192.f, 1.f / 8192.f, 1.f / 65535.f);
      LColorf color((int)col.r - 4096,
                    (int)col.g - 4096,
                    (int)col.b - 4096,
                    alpha_row(y)[x]);
      color.componentwise_mult(scale);
      return color;
    }

  default:
    return LColorf(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_a
//       Access: Published
//  Description: Changes the RGBA color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_a(int x, int y, const LColorf &value) {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);

  xel &col = row(y)[x];

  switch (_xel_encoding) {
  case XE_generic:
    {
      LColorf scaled = value * get_maxval() + 0.5f;
      col.r = clamp_val((int)scaled[0]);
      col.g = clamp_val((int)scaled[1]);
      col.b = clamp_val((int)scaled[2]);
    }
    break;

  case XE_generic_alpha:
    {
      LColorf scaled = value * get_maxval() + 0.5f;
      col.r = clamp_val((int)scaled[0]);
      col.g = clamp_val((int)scaled[1]);
      col.b = clamp_val((int)scaled[2]);
      alpha_row(y)[x] = clamp_val((int)scaled[3]);
    }
    break;

  case XE_generic_sRGB:
    col.r = clamp_val((int)
      (encode_sRGB_float(value[0]) * get_maxval() + 0.5f));
    col.g = clamp_val((int)
      (encode_sRGB_float(value[1]) * get_maxval() + 0.5f));
    col.b = clamp_val((int)
      (encode_sRGB_float(value[2]) * get_maxval() + 0.5f));
    break;

  case XE_generic_sRGB_alpha:
    col.r = clamp_val((int)
      (encode_sRGB_float(value[0]) * get_maxval() + 0.5f));
    col.g = clamp_val((int)
      (encode_sRGB_float(value[1]) * get_maxval() + 0.5f));
    col.b = clamp_val((int)
      (encode_sRGB_float(value[2]) * get_maxval() + 0.5f));
    alpha_row(y)[x] = clamp_val((int)(value[3] * get_maxval() + 0.5f));
    break;

  case XE_uchar_sRGB:
    encode_sRGB_uchar(value, col);
    break;

  case XE_uchar_sRGB_alpha:
    encode_sRGB_uchar(value, col, alpha_row(y)[x]);
    break;

  case XE_uchar_sRGB_sse2:
    encode_sRGB_uchar_sse2(value, col);
    break;

  case XE_uchar_sRGB_alpha_sse2:
    encode_sRGB_uchar_sse2(value, col, alpha_row(y)[x]);
    break;

  case XE_scRGB:
    {
      LColorf scaled = value * 8192.0f + 4096.5f;
      col.r = min(max(0, (int)scaled[0]), 65535);
      col.g = min(max(0, (int)scaled[1]), 65535);
      col.b = min(max(0, (int)scaled[2]), 65535);
    }
    break;

  case XE_scRGB_alpha:
    {
      LColorf scaled = value * 8192.0f + 4096.5f;
      col.r = min(max(0, (int)scaled[0]), 65535);
      col.g = min(max(0, (int)scaled[1]), 65535);
      col.b = min(max(0, (int)scaled[2]), 65535);
      alpha_row(y)[x] = min(max(0, (int)(value[3] * 65535 + 0.5f)), 65535);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_a
//       Access: Published
//  Description: Changes the RGBA color at the indicated pixel.  Each
//               component is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_a(int x, int y, float r, float g, float b, float a) {
  set_xel_a(x, y, LColorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_red
//       Access: Published
//  Description: Returns the red component color at the indicated
//               pixel.  The value returned is a linearized float
//               in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_red(int x, int y) const {
  return from_val(get_red_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_green
//       Access: Published
//  Description: Returns the green component color at the indicated
//               pixel.  The value returned is a linearized float
//               in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_green(int x, int y) const {
  return from_val(get_green_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_blue
//       Access: Published
//  Description: Returns the blue component color at the indicated
//               pixel.  The value returned is a linearized float
//               in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_blue(int x, int y) const {
  return from_val(get_blue_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_gray
//       Access: Published
//  Description: Returns the gray component color at the indicated
//               pixel.  This only has a meaningful value for
//               grayscale images; for other image types, this returns
//               the value of the blue channel only.  However, also
//               see the get_bright() function.  The value returned
//               is a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_gray(int x, int y) const {
  return from_val(get_gray_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_alpha
//       Access: Published
//  Description: Returns the alpha component color at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value returned is a float
//               in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_alpha(int x, int y) const {
  return from_alpha_val(get_alpha_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_red
//       Access: Published
//  Description: Sets the red component color only at the indicated
//               pixel.  The value given should be a linearized
//               float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_red(int x, int y, float r) {
  set_red_val(x, y, to_val(r));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_green
//       Access: Published
//  Description: Sets the green component color only at the indicated
//               pixel.  The value given should be a linearized
//               float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_green(int x, int y, float g) {
  set_green_val(x, y, to_val(g));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_blue
//       Access: Published
//  Description: Sets the blue component color only at the indicated
//               pixel.  The value given should be a linearized
//               float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_blue(int x, int y, float b) {
  set_blue_val(x, y, to_val(b));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_gray
//       Access: Published
//  Description: Sets the gray component color at the indicated
//               pixel.  This is only meaningful for grayscale images;
//               for other image types, this simply sets the blue
//               component color.  However, also see set_xel(), which
//               can set all the component colors to the same
//               grayscale level, and hence works correctly both for
//               grayscale and color images.  The value given should
//               be a linearized float in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_gray(int x, int y, float gray) {
  set_gray_val(x, y, to_val(gray));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_alpha
//       Access: Published
//  Description: Sets the alpha component color only at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value given should be in
//               the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_alpha(int x, int y, float a) {
  set_alpha_val(x, y, to_alpha_val(a));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_bright
//       Access: Published
//  Description: Returns the linear brightness of the given xel, as a
//               linearized float in the range 0..1.  This flavor of
//               get_bright() returns the correct grayscale brightness
//               level for both full-color and grayscale images.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_bright(int x, int y) const {
  return get_bright(x, y, _default_rc, _default_gc, _default_bc);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_bright
//       Access: Published
//  Description: This flavor of get_bright() works correctly only for
//               color images.  It returns a single brightness value
//               for the RGB color at the indicated pixel, based on
//               the supplied weights for each component.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_bright(int x, int y, float rc, float gc, float bc) const {
  return get_xel(x, y).dot(LVecBase3f(rc, gc, bc));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_bright
//       Access: Published
//  Description: This flavor of get_bright() works correctly only for
//               four-channel images.  It returns a single brightness
//               value for the RGBA color at the indicated pixel,
//               based on the supplied weights for each component.
////////////////////////////////////////////////////////////////////
INLINE float PNMImage::
get_bright(int x, int y, float rc, float gc, float bc, float ac) const {
  return get_xel_a(x, y).dot(LVecBase4f(rc, gc, bc, ac));
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
INLINE void PNMImage::
blend(int x, int y, const LRGBColorf &val, float alpha) {
  blend(x, y, val[0], val[1], val[2], alpha);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::box_filter
//       Access: Published
//  Description: This flavor of box_filter() will apply the filter
//               over the entire image without resizing or copying;
//               the effect is that of a blur operation.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
box_filter(float radius) {
  box_filter_from(radius, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::gaussian_filter
//       Access: Published
//  Description: This flavor of gaussian_filter() will apply the filter
//               over the entire image without resizing or copying;
//               the effect is that of a blur operation.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
gaussian_filter(float radius) {
  gaussian_filter_from(radius, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::gamma_correct
//       Access: Published
//  Description: Assuming the image was constructed with a gamma curve
//               of from_gamma in the RGB channels, converts it to an
//               image with a gamma curve of to_gamma in the RGB
//               channels.  Does not affect the alpha channel.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
gamma_correct(float from_gamma, float to_gamma) {
  apply_exponent(from_gamma / to_gamma);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::gamma_correct_alpha
//       Access: Published
//  Description: Assuming the image was constructed with a gamma curve
//               of from_gamma in the alpha channel, converts it to an
//               image with a gamma curve of to_gamma in the alpha
//               channel.  Does not affect the RGB channels.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
gamma_correct_alpha(float from_gamma, float to_gamma) {
  apply_exponent(1.0, from_gamma / to_gamma);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::apply_exponent
//       Access: Published
//  Description: Adjusts each channel of the image by raising the
//               corresponding component value to the indicated
//               exponent, such that L' = L ^ exponent.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
apply_exponent(float gray_exponent) {
  apply_exponent(gray_exponent, gray_exponent, gray_exponent, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::apply_exponent
//       Access: Published
//  Description: Adjusts each channel of the image by raising the
//               corresponding component value to the indicated
//               exponent, such that L' = L ^ exponent.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
apply_exponent(float gray_exponent, float alpha_exponent) {
  apply_exponent(gray_exponent, gray_exponent, gray_exponent, alpha_exponent);
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
INLINE void PNMImage::
apply_exponent(float red_exponent, float green_exponent, float blue_exponent) {
  apply_exponent(red_exponent, green_exponent, blue_exponent, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row Constructor
////////////////////////////////////////////////////////////////////
INLINE PNMImage::Row::
Row(PNMImage &image, int y) : _image(image), _y(y) {
  nassertv(y >= 0 && y < _image._y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::size
//       Access: Published
//  Description: Get the number of pixels in the row.
////////////////////////////////////////////////////////////////////
INLINE size_t PNMImage::Row::
size() const {
  return _image.get_x_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Fetch the RGB value at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE LColorf PNMImage::Row::
operator[](int x) const {
  return _image.get_xel_a(x, _y);
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Set the pixel at the given column in the row.  If
//               the image has no alpha channel, the alpha component
//               is ignored.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::Row::
__setitem__(int x, const LColorf &v) {
  _image.set_xel_a(x, _y, v);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Fetch the pixel at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE xel &PNMImage::Row::
get_xel_val(int x) {
  return _image.get_xel_val(x, _y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Set the pixel at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::Row::
set_xel_val(int x, const xel &v) {
  _image.set_xel_val(x, _y, v);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Fetch the alpha value at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::Row::
get_alpha_val(int x) const {
  return _image.get_alpha_val(x, _y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Row::Array Operator
//       Access: Published
//  Description: Set the alpha value at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::Row::
set_alpha_val(int x, xelval v) {
  _image.set_alpha_val(x, _y, v);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::CRow Constructor
////////////////////////////////////////////////////////////////////
INLINE PNMImage::CRow::
CRow(const PNMImage &image, int y) : _image(image), _y(y) {
  nassertv(y >= 0 && y < _image._y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::CRow::size
//       Access: Published
//  Description: Get the number of pixels in the row.
////////////////////////////////////////////////////////////////////
INLINE size_t PNMImage::CRow::
size() const {
  return _image.get_x_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::CRow::Array Operator
//       Access: Published
//  Description: Fetch the RGB value at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE LColorf PNMImage::CRow::
operator[](int x) const {
  return _image.get_xel_a(x, _y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::CRow::Array Operator
//       Access: Published
//  Description: Fetch the pixel at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE xel PNMImage::CRow::
get_xel_val(int x) const {
  return _image.get_xel_val(x, _y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::CRow::Array Operator
//       Access: Published
//  Description: Fetch the alpha value at the given column in the row.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::CRow::
get_alpha_val(int x) const {
  return _image.get_alpha_val(x, _y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Published
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE PNMImage::Row PNMImage::
operator [] (int y) {
  return Row(*this, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Published
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE PNMImage::CRow PNMImage::
operator [] (int y) const {
  return CRow(*this, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_array
//       Access: Public
//  Description: Directly access the underlying PNMImage array.  Know
//               what you are doing!
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
get_array() {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_array
//       Access: Public
//  Description: Directly access the underlying PNMImage array.  Know
//               what you are doing!
////////////////////////////////////////////////////////////////////
INLINE const xel *PNMImage::
get_array() const {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_alpha_array
//       Access: Public
//  Description: Directly access the underlying PNMImage array of
//               alpha values.  Know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE xelval *PNMImage::
get_alpha_array() {
  return _alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_alpha_array
//       Access: Public
//  Description: Directly access the underlying PNMImage array of
//               alpha values.  Know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE const xelval *PNMImage::
get_alpha_array() const {
  return _alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::take_array
//       Access: Public
//  Description: Returns the underlying PNMImage array and removes it
//               from the PNMImage.  You become the owner of this
//               array and must eventually free it with
//               PANDA_FREE_ARRAY() (or pass it to another PNMImage
//               with set_array()).  Know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
take_array() {
  xel *array = _array;
  _array = NULL;
  return array;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::take_alpha_array
//       Access: Public
//  Description: Returns the underlying PNMImage array and removes it
//               from the PNMImage.  You become the owner of this
//               array and must eventually free it with
//               PANDA_FREE_ARRAY() (or pass it to another PNMImage
//               with set_alpha_array()).  Know what you are doing!
////////////////////////////////////////////////////////////////////
INLINE xelval *PNMImage::
take_alpha_array() {
  xelval *alpha = _alpha;
  _alpha = NULL;
  return alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::allocate_array
//       Access: Private
//  Description: Allocates the internal memory for the RGB or
//               grayscale pixels in the image (except alpha).
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
allocate_array() {
  _array = (xel *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xel));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::allocate_alpha
//       Access: Private
//  Description: Allocates the internal memory for the alpha pixels in
//               the image.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
allocate_alpha() {
  _alpha = (xelval *)PANDA_MALLOC_ARRAY(_x_size * _y_size * sizeof(xelval));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::row
//       Access: Private
//  Description: Returns an array of xels corresponding to the nth row
//               of the image.
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
row(int y) const {
  nassertr(y >= 0 && y < _y_size, NULL);
  return _array + y * _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::alpha_row
//       Access: Private
//  Description: Returns an array of xelvals corresponding to the nth
//               row of the alpha channel.
////////////////////////////////////////////////////////////////////
INLINE xelval *PNMImage::
alpha_row(int y) const {
  nassertr(_alpha != NULL && y >= 0 && y < _y_size, NULL);
  return _alpha + y * _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::setup_sub_image
//       Access: Private
//  Description: Computes xmin, ymin, xmax, and ymax, based on the
//               input parameters for copy_sub_image() and related
//               methods.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
setup_sub_image(const PNMImage &copy, int &xto, int &yto,
                int &xfrom, int &yfrom, int &x_size, int &y_size,
                int &xmin, int &ymin, int &xmax, int &ymax) {
  if (x_size < 0) {
    x_size = copy.get_x_size() - xfrom;
  }
  if (y_size < 0) {
    y_size = copy.get_y_size() - yfrom;
  }

  if (xfrom < 0) {
    xto += -xfrom;
    x_size -= -xfrom;
    xfrom = 0;
  }
  if (yfrom < 0) {
    yto += -yfrom;
    y_size -= -yfrom;
    yfrom = 0;
  }

  if (xto < 0) {
    xfrom += -xto;
    x_size -= -xto;
    xto = 0;
  }
  if (yto < 0) {
    yfrom += -yto;
    y_size -= -yto;
    yto = 0;
  }

  x_size = min(x_size, copy.get_x_size() - xfrom);
  y_size = min(y_size, copy.get_y_size() - yfrom);

  xmin = xto;
  ymin = yto;

  xmax = min(xmin + x_size, get_x_size());
  ymax = min(ymin + y_size, get_y_size());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::compute_spot_pixel
//       Access: Private, Static
//  Description: Called by render_spot to compute the color of a
//               single pixel, based in (the square of) its distance
//               from the center.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
compute_spot_pixel(LColorf &c, float d2,
                   float min_radius, float max_radius,
                   const LColorf &fg, const LColorf &bg) {
  float d = sqrt(d2);
  if (d > max_radius) {
    c = bg;
  } else if (d > min_radius) {
    d = (d - min_radius) / (max_radius - min_radius);
    float d2 = d * d;
    float t = (3.0 * d2) - (2.0 * d * d2);
    c = fg + t * (bg - fg);
  } else {
    c = fg;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator +
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               is the sum of the corresponding pixel values
//               in the two given images.
//               Only valid when both images have the same size.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator + (const PNMImage &other) const {
  PNMImage target (*this);
  target += other;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator +
//       Access: Published
//  Description: Returns a new PNMImage in which the provided color
//               is added to each pixel in the provided image.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator + (const LColorf &other) const {
  PNMImage target (*this);
  target += other;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator -
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               from the right image is subtracted from each
//               pixel value from the left image.
//               Only valid when both images have the same size.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator - (const PNMImage &other) const {
  PNMImage target (*this);
  target -= other;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator -
//       Access: Published
//  Description: Returns a new PNMImage in which the provided color
//               is subtracted from each pixel in the provided image.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator - (const LColorf &other) const {
  PNMImage target (*this);
  target -= other;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *
//       Access: Published
//  Description: Returns a new PNMImage in which each pixel value
//               from the left image is multiplied by each
//               pixel value from the right image. Note that the
//               floating-point values in the 0..1 range are
//               multiplied, not in the 0..maxval range.
//               Only valid when both images have the same size.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator * (const PNMImage &other) const {
  PNMImage target (*this);
  target *= other;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *
//       Access: Published
//  Description: Multiplies every pixel value in the image by
//               a constant floating-point multiplier value.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator * (float multiplier) const {
  PNMImage target (*this);
  target *= multiplier;
  return target;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::operator *
//       Access: Published
//  Description: Returns a new PNMImage in which the provided color
//               is multiplied to each pixel in the provided image.
////////////////////////////////////////////////////////////////////
INLINE PNMImage PNMImage::
operator * (const LColorf &other) const {
  PNMImage target (*this);
  target *= other;
  return target;
}
