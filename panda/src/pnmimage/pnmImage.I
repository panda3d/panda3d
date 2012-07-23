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
         PNMFileType *type) {
  _array = NULL;
  _alpha = NULL;

  clear(x_size, y_size, num_channels, maxval, type);
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
//  Description: A handy function to scale values from [0..1] to
//               [0..get_maxval()].
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
to_val(double input_value) const {
  return clamp_val((int)(input_value * get_maxval() + 0.5));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::from_val
//       Access: Published
//  Description: A handy function to scale values from
//               [0..get_maxval()] to [0..1].
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
from_val(xelval input_value) const {
  return (double)input_value / (double)get_maxval();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given color.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(double red, double green, double blue) {
  fill_val(to_val(red), to_val(green), to_val(blue));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Published
//  Description: Sets the entire image (except the alpha channel) to
//               the given grayscale level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(double gray) {
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
alpha_fill(double alpha) {
  alpha_fill_val(to_val(alpha));
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
INLINE const xel &PNMImage::
get_xel_val(int x, int y) const {
  nassertr(x >= 0 && x < _x_size && y >= 0 && y < _y_size, _array[0]);
  return row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
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
//               component is in the range 0..maxval.
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
//               0..maxval.
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
//               pixel.  The value returned is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_red_val(int x, int y) const {
  return PPM_GETR(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_green_val
//       Access: Published
//  Description: Returns the green component color at the indicated
//               pixel.  The value returned is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_green_val(int x, int y) const {
  return PPM_GETG(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_blue_val
//       Access: Published
//  Description: Returns the blue component color at the indicated
//               pixel.  The value returned is in the range 0..maxval.
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
//               see the get_bright() function.  The value returned is
//               in the range 0..maxval.
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
//               range 0..maxval.
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
//               0..maxval.
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
//               0..maxval.
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
//               0..maxval.
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
//               be in the range 0..maxval.
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
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE LRGBColord PNMImage::
get_xel(int x, int y) const {
  return LRGBColord(from_val(get_red_val(x, y)),
                   from_val(get_green_val(x, y)),
                   from_val(get_blue_val(x, y)));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, const LRGBColord &value) {
  set_xel_val(x, y, to_val(value[0]), to_val(value[1]), to_val(value[2]));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, double r, double g, double b) {
  set_xel_val(x, y, to_val(r), to_val(g), to_val(b));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Published
//  Description: Changes all three color components at the indicated
//               pixel to the same value.  The value is a double in
//               the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, double gray) {
  set_xel_val(x, y, to_val(gray), to_val(gray), to_val(gray));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel_a
//       Access: Published
//  Description: Returns the RGBA color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE LColord PNMImage::
get_xel_a(int x, int y) const {
  if (has_alpha()) {
    return LColord(from_val(get_red_val(x, y)),
                  from_val(get_green_val(x, y)),
                  from_val(get_blue_val(x, y)),
                  from_val(get_alpha_val(x, y)));
  } else {
    return LColord(from_val(get_red_val(x, y)),
                  from_val(get_green_val(x, y)),
                  from_val(get_blue_val(x, y)),
                  0.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_a
//       Access: Published
//  Description: Changes the RGBA color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_a(int x, int y, const LColord &value) {
  set_xel_val(x, y, to_val(value[0]), to_val(value[1]), to_val(value[2]));
  if (has_alpha()) {
    set_alpha_val(x, y, to_val(value[3]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_a
//       Access: Published
//  Description: Changes the RGBA color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_a(int x, int y, double r, double g, double b, double a) {
  set_xel_val(x, y, to_val(r), to_val(g), to_val(b));
  if (has_alpha()) {
    set_alpha_val(x, y, to_val(a));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_red
//       Access: Published
//  Description: Returns the red component color at the indicated
//               pixel.  The value returned is a double in the range
//               0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_red(int x, int y) const {
  return from_val(get_red_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_green
//       Access: Published
//  Description: Returns the green component color at the indicated
//               pixel.  The value returned is a double in the range
//               0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_green(int x, int y) const {
  return from_val(get_green_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_blue
//       Access: Published
//  Description: Returns the blue component color at the indicated
//               pixel.  The value returned is a double in the range
//               0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
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
//               see the get_bright() function.  The value returned is
//               a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_gray(int x, int y) const {
  return from_val(get_gray_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_alpha
//       Access: Published
//  Description: Returns the alpha component color at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value returned is a double
//               in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_alpha(int x, int y) const {
  return from_val(get_alpha_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_red
//       Access: Published
//  Description: Sets the red component color only at the indicated
//               pixel.  The value given should be a double in the
//               range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_red(int x, int y, double r) {
  set_red_val(x, y, to_val(r));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_green
//       Access: Published
//  Description: Sets the green component color only at the indicated
//               pixel.  The value given should be a double in the
//               range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_green(int x, int y, double r) {
  set_green_val(x, y, to_val(r));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_blue
//       Access: Published
//  Description: Sets the blue component color only at the indicated
//               pixel.  The value given should be a double in the
//               range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_blue(int x, int y, double r) {
  set_blue_val(x, y, to_val(r));
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
//               be a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_gray(int x, int y, double r) {
  set_gray_val(x, y, to_val(r));
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
set_alpha(int x, int y, double r) {
  set_alpha_val(x, y, to_val(r));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_channel
//       Access: Published
//  Description: Returns the nth component color at the indicated
//               pixel.  The channel index should be in the range
//               0..(get_num_channels()-1).  The channels are ordered B,
//               G, R, A.  This is slightly less optimal than
//               accessing the component values directly by named
//               methods.  The value returned is a double in the range
//               0..1.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_channel(int x, int y, int channel) const {
  return from_val(get_channel_val(x, y, channel));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_channel_val
//       Access: Published
//  Description: Sets the nth component color at the indicated
//               pixel.  The channel index should be in the range
//               0..(get_num_channels()-1).  The channels are ordered B,
//               G, R, A.  This is slightly less optimal than
//               setting the component values directly by named
//               methods.  The value given should be a double in the
//               range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_channel(int x, int y, int channel, double value) {
  set_channel_val(x, y, channel, to_val(value));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_bright
//       Access: Published
//  Description: Returns the linear brightness of the given xel, as a
//               double in the range 0..1.  This flavor of
//               get_bright() returns the correct grayscale brightness
//               level for both full-color and grayscale images.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
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
INLINE double PNMImage::
get_bright(int x, int y, double rc, double gc, double bc) const {
  return from_val((int)(rc * get_red_val(x, y) +
                        gc * get_green_val(x, y) +
                        bc * get_blue_val(x, y)));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_bright
//       Access: Published
//  Description: This flavor of get_bright() works correctly only for
//               four-channel images.  It returns a single brightness
//               value for the RGBA color at the indicated pixel,
//               based on the supplied weights for each component.
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
get_bright(int x, int y, double rc, double gc, double bc, double ac) const {
  return from_val((int)(rc * get_red_val(x, y) +
                        gc * get_green_val(x, y) +
                        bc * get_blue_val(x, y) +
                        ac * get_alpha_val(x, y)));
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
blend(int x, int y, const LRGBColord &val, double alpha) {
  blend(x, y, val[0], val[1], val[2], alpha);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Published
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
operator [] (int y) {
  return row(y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Published
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE const xel *PNMImage::
operator [] (int y) const {
  return row(y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::box_filter
//       Access: Published
//  Description: This flavor of box_filter() will apply the filter
//               over the entire image without resizing or copying;
//               the effect is that of a blur operation.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
box_filter(double radius) {
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
gaussian_filter(double radius) {
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
gamma_correct(double from_gamma, double to_gamma) {
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
gamma_correct_alpha(double from_gamma, double to_gamma) {
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
apply_exponent(double gray_exponent) {
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
apply_exponent(double gray_exponent, double alpha_exponent) {
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
apply_exponent(double red_exponent, double green_exponent, double blue_exponent) {
  apply_exponent(red_exponent, green_exponent, blue_exponent, 1.0);
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
compute_spot_pixel(LColord &c, double d2,
                   double min_radius, double max_radius,
                   const LColord &fg, const LColord &bg) {
  double d = sqrt(d2);
  if (d > max_radius) {
    c = bg;
  } else if (d > min_radius) {
    d = (d - min_radius) / (max_radius - min_radius);
    double d2 = d * d;
    double t = (3.0 * d2) - (2.0 * d * d2);
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
operator + (const LColord &other) const {
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
operator - (const LColord &other) const {
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
operator * (double multiplier) const {
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
operator * (const LColord &other) const {
  PNMImage target (*this);
  target *= other;
  return target;
}
