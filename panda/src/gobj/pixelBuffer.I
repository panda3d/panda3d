// Filename: pixelBuffer.I
// Created by:  drose (05Feb99)
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
//     Function: rgb_buffer
//       Access: Public
//  Description: Constructs a PixelBuffer suitable for RGB
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer PixelBuffer::
rgb_buffer(int xsize, int ysize) {
  return PixelBuffer(xsize, ysize, 3, sizeof(uchar), T_unsigned_byte,
                     F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: rgba_buffer
//       Access: Public
//  Description: Constructs a PixelBuffer suitable for RGBA
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer PixelBuffer::
rgba_buffer(int xsize, int ysize) {
  return PixelBuffer(xsize, ysize, 4, sizeof(uchar), T_unsigned_byte,
                     F_rgba);
}

////////////////////////////////////////////////////////////////////
//     Function: depth_buffer
//       Access: Public
//  Description: Constructs a PixelBuffer suitable for depth maps
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer PixelBuffer::
depth_buffer(int xsize, int ysize) {
  return PixelBuffer(xsize, ysize, 1, sizeof(float), T_float,
                     F_depth_component);
}

////////////////////////////////////////////////////////////////////
//     Function: stencil_buffer
//       Access: Public
//  Description: Constructs a PixelBuffer suitable for stencil buffers
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer PixelBuffer::
stencil_buffer(int xsize, int ysize) {
  return PixelBuffer(xsize, ysize, 1, sizeof(uchar), T_unsigned_byte,
                     F_stencil_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer::
~PixelBuffer(void) {
}


////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_xsize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::set_xsize(int size)
{
  if (_xsize != size) {
    _xsize = size;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_ysize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::set_ysize(int size)
{
  if (_ysize != size) {
    _ysize = size;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_xorg
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::set_xorg(int org)
{
  if (_xorg != org) {
    _xorg = org;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_yorg
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::set_yorg(int org)
{
  if (_yorg != org) {
    _yorg = org;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
set_size(int x_org, int y_org, int x_size, int y_size) {
  if ((_xsize != x_size) || (_ysize != y_size) ||
      (_xorg != x_org) || (_yorg != y_org)) {
    make_dirty();
  }

  _xsize = x_size;
  _ysize = y_size;
  _xorg = x_org;
  _yorg = y_org;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_num_components
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
set_num_components(int num_components) {
  if (_num_components != num_components) {
    _num_components = num_components;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_component_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
set_component_width(int component_width) {
  if (_component_width != component_width) {
    _component_width = component_width;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_format
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
set_format(PixelBuffer::Format format) {
  if (_format != format) {
    _format = format;
    make_dirty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_loaded
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::set_loaded()
{
  _loaded = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_xsize
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_xsize() const {
  return _xsize;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_ysize
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_ysize() const {
  return _ysize;
}


////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_xorg
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_xorg() const {
  return _xorg;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_yorg
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_yorg() const {
  return _yorg;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_border
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_border() const {
  return _border;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_num_components
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_component_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PixelBuffer::
get_component_width() const {
  return _component_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_format
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer::Format PixelBuffer::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_image_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PixelBuffer::Type PixelBuffer::
get_image_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::set_uchar_rgb_texel
//       Access: Public
//  Description: This is only valid when the PixelBuffer is an RGB
//               buffer with uchar components.
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
set_uchar_rgb_texel(const uchar color[3], int x, int y, int width)
{
  int i = y * 3 * width + x * 3;
  _image[i] = color[0];
  _image[i+1] = color[1];
  _image[i+2] = color[2];
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::store_unscaled_byte
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value is assumed to be in the range
//               0-255.
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
store_unscaled_byte(int &index, int value) {
  _image[index++] = (uchar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::store_unscaled_short
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value is assumed to be in the range
//               0-65535.
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
store_unscaled_short(int &index, int value) {
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.us = (ushort)value;
  _image[index++] = v.uc[0];
  _image[index++] = v.uc[1];
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::store_scaled_byte
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
store_scaled_byte(int &index, int value, double scale) {
  store_unscaled_byte(index, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::store_scaled_short
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void PixelBuffer::
store_scaled_short(int &index, int value, double scale) {
  store_unscaled_short(index, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_unsigned_byte
//       Access: Private
//  Description: This is used by store() to retieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned bytes.
////////////////////////////////////////////////////////////////////
INLINE double PixelBuffer::
get_unsigned_byte(int &index) const {
  nassertr(index >= 0 && index < (int)_image.size(), 0.0);
  return (double)_image[index++] / 255.0;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::get_unsigned_short
//       Access: Private
//  Description: This is used by store() to retieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned shorts.
////////////////////////////////////////////////////////////////////
INLINE double PixelBuffer::
get_unsigned_short(int &index) const {
  nassertr(index >= 0 && index+1 < (int)_image.size(), 0.0);
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.uc[0] = _image[index++];
  v.uc[1] = _image[index++];
  return (double)v.us / 65535.0;
}
