// Filename: pnmImage.I
// Created by:  drose (15Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
PNMImage() {
  _array = NULL;
  _alpha = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
PNMImage(const Filename &filename, PNMFileType *type) {
  _array = NULL;
  _alpha = NULL;

  read(filename, type);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Constructor
//       Access: Public
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
//       Access: Public
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
//     Function: PNMImage::Copy Assigment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
operator = (const PNMImage &copy) {
  copy_from(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImage::
~PNMImage() {
  clear();
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImage::clamp_val
//       Access: Public
//  Description: A handy function to clamp values to
//               [0..get_maxval()].
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
clamp_val(int input_value) const {
  return (xelval)min(max(0, input_value), (int)get_maxval());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::to_val
//       Access: Public
//  Description: A handy function to scale values from [0..1] to
//               [0..get_maxval()].
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
to_val(double input_value) const {
  return clamp_val((int)(input_value * get_maxval() + 0.5));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::from_val
//       Access: Public
//  Description: A handy function to scale values from
//               [0..get_maxval()] to [0..1].
////////////////////////////////////////////////////////////////////
INLINE double PNMImage::
from_val(xelval input_value) const {
  return (double)input_value / (double)get_maxval();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Public
//  Description: Sets the entire image (except the alpha channel) to
//               the given color.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(double red, double green, double blue) {
  fill_val((xelval)(red * get_maxval()),
           (xelval)(green * get_maxval()),
           (xelval)(blue * get_maxval()));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill
//       Access: Public
//  Description: Sets the entire image (except the alpha channel) to
//               the given grayscale level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill(double gray) {
  fill(gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::fill_val
//       Access: Public
//  Description: Sets the entire image (except the alpha channel) to
//               the given grayscale level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
fill_val(xelval gray) {
  fill_val(gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::alpha_fill
//       Access: Public
//  Description: Sets the entire alpha channel to the given level.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
alpha_fill(double alpha) {
  alpha_fill_val((xelval)(alpha * get_maxval()));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::is_valid
//       Access: Public
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
//       Access: Public
//  Description: Changes the number of channels associated with the
//               image.  The new number of channels must be an integer
//               in the range 1 through 4, inclusive.  This will
//               allocate and/or deallocate memory as necessary to
//               accomodate; see set_color_type().
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_num_channels(int num_channels) {
  nassertv(num_channels >= 1 && num_channels <= 4);
  set_color_type((ColorType)num_channels);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::add_alpha
//       Access: Public
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
//       Access: Public
//  Description: Removes the image's alpha channel, if it exists.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
remove_alpha() {
  set_color_type(is_grayscale() ? CT_grayscale : CT_color);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_grayscale
//       Access: Public
//  Description: Converts the image from RGB to grayscale.  Any alpha
//               channel, if present, is left undisturbed.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
make_grayscale() {
  make_grayscale(_default_rc, _default_gc, _default_bc);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::make_rgb
//       Access: Public
//  Description: Converts the image from grayscale to RGB.  Any alpha
//               channel, if present, is left undisturbed.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
make_rgb() {
  set_color_type(has_alpha() ? CT_four_channel : CT_color);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel_val
//       Access: Public
//  Description: Returns the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE const xel &PNMImage::
get_xel_val(int x, int y) const {
  return row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Public
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, const xel &value) {
  row(y)[x] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Public
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, xelval r, xelval g, xelval b) {
  PPM_ASSIGN(row(y)[x], r, g, b);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel_val
//       Access: Public
//  Description: Changes all three color components at the indicated
//               pixel to the same value.  The value is in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel_val(int x, int y, xelval gray) {
  PPM_ASSIGN(row(y)[x], gray, gray, gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_red_val
//       Access: Public
//  Description: Returns the red component color at the indicated
//               pixel.  The value returned is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_red_val(int x, int y) const {
  return PPM_GETR(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_green_val
//       Access: Public
//  Description: Returns the green component color at the indicated
//               pixel.  The value returned is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_green_val(int x, int y) const {
  return PPM_GETG(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_blue_val
//       Access: Public
//  Description: Returns the blue component color at the indicated
//               pixel.  The value returned is in the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_blue_val(int x, int y) const {
  return PPM_GETB(get_xel_val(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_gray_val
//       Access: Public
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
//       Access: Public
//  Description: Returns the alpha component color at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value returned is in the
//               range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImage::
get_alpha_val(int x, int y) const {
  return alpha_row(y)[x];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_red_val
//       Access: Public
//  Description: Sets the red component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_red_val(int x, int y, xelval r) {
  PPM_PUTR(row(y)[x], r);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_green_val
//       Access: Public
//  Description: Sets the green component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_green_val(int x, int y, xelval g) {
  PPM_PUTG(row(y)[x], g);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_blue_val
//       Access: Public
//  Description: Sets the blue component color only at the indicated
//               pixel.  The value given should be in the range
//               0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_blue_val(int x, int y, xelval b) {
  PPM_PUTB(row(y)[x], b);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_gray_val
//       Access: Public
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
  PPM_PUTB(row(y)[x], gray);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_alpha_val
//       Access: Public
//  Description: Sets the alpha component color only at the indicated
//               pixel.  It is an error to call this unless
//               has_alpha() is true.  The value given should be in
//               the range 0..maxval.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_alpha_val(int x, int y, xelval a) {
  alpha_row(y)[x] = a;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_xel
//       Access: Public
//  Description: Returns the RGB color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE RGBColord PNMImage::
get_xel(int x, int y) const {
  return RGBColord(from_val(get_red_val(x, y)),
                   from_val(get_green_val(x, y)),
                   from_val(get_blue_val(x, y)));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Public
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, const RGBColord &value) {
  set_xel_val(x, y, to_val(value[0]), to_val(value[1]), to_val(value[2]));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Public
//  Description: Changes the RGB color at the indicated pixel.  Each
//               component is a double in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, double r, double g, double b) {
  set_xel_val(x, y, to_val(r), to_val(g), to_val(b));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::set_xel
//       Access: Public
//  Description: Changes all three color components at the indicated
//               pixel to the same value.  The value is a double in
//               the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
set_xel(int x, int y, double gray) {
  set_xel_val(x, y, to_val(gray), to_val(gray), to_val(gray));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::get_red
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Smoothly blends the indicated pixel value in with
//               whatever was already in the image, based on the given
//               alpha value.  An alpha of 1.0 is fully opaque and
//               completely replaces whatever was there previously;
//               alpha of 0.0 is fully transparent and does nothing.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
blend(int x, int y, const RGBColord &val, double alpha) {
  blend(x, y, val[0], val[1], val[2], alpha);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Public
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
operator [] (int y) {
  return row(y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::Array Operator
//       Access: Public
//  Description: Allows the PNMImage to appear to be a 2-d array of
//               xels.
////////////////////////////////////////////////////////////////////
INLINE const xel *PNMImage::
operator [] (int y) const {
  return row(y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::box_filter
//       Access: Public
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
//       Access: Public
//  Description: This flavor of gaussian_filter() will apply the filter
//               over the entire image without resizing or copying;
//               the effect is that of a blur operation.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
gaussian_filter(double radius) {
  gaussian_filter_from(radius, *this);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImage::allocate_array
//       Access: Private
//  Description: Allocates the internal memory for the RGB or
//               grayscale pixels in the image (except alpha).
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
allocate_array() {
  _array = new xel[_x_size * _y_size];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::allocate_alpha
//       Access: Private
//  Description: Allocates the internal memory for the alpha pixels in
//               the image.
////////////////////////////////////////////////////////////////////
INLINE void PNMImage::
allocate_alpha() {
  _alpha = new xelval[_x_size * _y_size];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::row
//       Access: Private
//  Description: Returns an array of xels corresponding to the nth row
//               of the image.
////////////////////////////////////////////////////////////////////
INLINE xel *PNMImage::
row(int y) const {
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
  return _alpha + y * _x_size;
}
