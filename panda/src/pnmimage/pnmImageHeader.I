// Filename: pnmImageHeader.I
// Created by:  drose (15Jun00)
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

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::
PNMImageHeader() {
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _maxval = 255;
  _type = (PNMFileType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::
PNMImageHeader(const PNMImageHeader &copy) :
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _num_channels(copy._num_channels),
  _maxval(copy._maxval),
  _type(copy._type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMImageHeader::
operator = (const PNMImageHeader &copy) {
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _num_channels = copy._num_channels;
  _maxval = copy._maxval;
  _type = copy._type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::
~PNMImageHeader() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_color_type
//       Access: Published
//  Description: Returns the image type of the image, as an enumerated
//               value.  This is really just the number of channels
//               cast to the enumerated type.
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::ColorType PNMImageHeader::
get_color_type() const {
  nassertr(_num_channels >= 1 && _num_channels <= 4, CT_invalid);
  return (ColorType)_num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_num_channels
//       Access: Published
//  Description: Returns the number of channels in the image.
////////////////////////////////////////////////////////////////////
INLINE int PNMImageHeader::
get_num_channels() const {
  nassertr(_num_channels >= 1 && _num_channels <= 4, 0);
  return _num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::is_grayscale
//       Access: Published, Static
//  Description: This static variant of is_grayscale() returns true if
//               the indicated image type represents a grayscale
//               image, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::
is_grayscale(PNMImageHeader::ColorType color_type) {
  return (color_type == CT_grayscale || color_type == CT_two_channel);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::is_grayscale
//       Access: Published
//  Description: Returns false if the image is a full-color image, and
//               has red, green, and blue components; true if it is a
//               grayscale image and has only a gray component.  (The
//               gray color is actually stored in the blue channel,
//               and the red and green channels are ignored.)
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::
is_grayscale() const {
  return is_grayscale(get_color_type());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::has_alpha
//       Access: Published, Static
//  Description: This static variant of has_alpha() returns true if
//               the indicated image type includes an alpha channel,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::
has_alpha(PNMImageHeader::ColorType color_type) {
  return (color_type == CT_two_channel || color_type == CT_four_channel);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::has_alpha
//       Access: Published
//  Description: Returns true if the image includes an alpha channel,
//               false otherwise.  Unlike is_grayscale(), if this
//               returns false it is an error to call any of the
//               functions accessing the alpha channel.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::
has_alpha() const {
  return has_alpha(get_color_type());
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_maxval
//       Access: Published
//  Description: Returns the maximum channel value allowable for any
//               pixel in this image; for instance, 255 for a typical
//               8-bit-per-channel image.  A pixel with this value is
//               full on.
////////////////////////////////////////////////////////////////////
INLINE xelval PNMImageHeader::
get_maxval() const {
  return _maxval;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_x_size
//       Access: Published
//  Description: Returns the number of pixels in the X direction.
//               This is one more than the largest allowable X
//               coordinate.
////////////////////////////////////////////////////////////////////
INLINE int PNMImageHeader::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_y_size
//       Access: Published
//  Description: Returns the number of pixels in the Y direction.
//               This is one more than the largest allowable Y
//               coordinate.
////////////////////////////////////////////////////////////////////
INLINE int PNMImageHeader::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::has_type
//       Access: Published
//  Description: Returns true if the PNMImageHeader knows what type it
//               is, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::
has_type() const {
  return _type != (PNMFileType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::get_type
//       Access: Published
//  Description: If the file type is known (e.g. has_type() returns
//               true), returns its PNMFileType pointer; otherwise,
//               returns NULL.
////////////////////////////////////////////////////////////////////
INLINE PNMFileType *PNMImageHeader::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::set_type
//       Access: Published
//  Description: Sets the file type of this PNMImage.  This will be
//               the default type used when an image is read, if the
//               type cannot be determined by magic number or inferred
//               by extension, or the type used when the image is
//               written, if the type cannot be inferred from the
//               filename extension.
////////////////////////////////////////////////////////////////////
INLINE void PNMImageHeader::
set_type(PNMFileType *type) {
  _type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::record_color
//       Access: Protected
//  Description: Records the indicated color in the histogram.
////////////////////////////////////////////////////////////////////
INLINE void PNMImageHeader::
record_color(PNMImageHeader::Histogram &hist, 
             const PNMImageHeader::PixelSpec &color) {
  // First, try to add the color with a count of 0, in case it does
  // not already exist in the table.
  Histogram::iterator hi = hist.insert(Histogram::value_type(color, 0)).first;

  // Now that either succeeded or failed, but either way hi is now the
  // iterator to the count value in the table associated with the
  // given color.  Increment that count.
  (*hi).second++;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval gray_value) :
  _red(gray_value),
  _green(gray_value),
  _blue(gray_value),
  _alpha(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval gray_value, xelval alpha) :
  _red(gray_value),
  _green(gray_value),
  _blue(gray_value),
  _alpha(alpha)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval red, xelval green, xelval blue) :
  _red(red),
  _green(green),
  _blue(blue),
  _alpha(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval red, xelval green, xelval blue, xelval alpha) :
  _red(red),
  _green(green),
  _blue(blue),
  _alpha(alpha)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMImageHeader::PixelSpec::
PixelSpec(const PixelSpec &copy) :
  _red(copy._red),
  _green(copy._green),
  _blue(copy._blue),
  _alpha(copy._alpha)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PNMImageHeader::PixelSpec::
operator = (const PixelSpec &copy) {
  _red = copy._red;
  _green = copy._green;
  _blue = copy._blue;
  _alpha = copy._alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::Comparison Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool PNMImageHeader::PixelSpec::
operator < (const PixelSpec &other) const {
  if (_red != other._red) {
    return _red < other._red;
  }
  if (_green != other._green) {
    return _green < other._green;
  }
  if (_blue != other._blue) {
    return _blue < other._blue;
  }
  return _alpha < other._alpha;
}
