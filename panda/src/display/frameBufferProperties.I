// Filename: frameBufferProperties.I
// Created by:  drose (27Jan03)
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
//     Function: FrameBufferProperties::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FrameBufferProperties::
FrameBufferProperties(const FrameBufferProperties &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FrameBufferProperties::
~FrameBufferProperties() {
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
operator != (const FrameBufferProperties &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_any_specified
//       Access: Published
//  Description: Returns true if any properties have been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
is_any_specified() const {
  return (_specified != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_frame_buffer_mode
//       Access: Published
//  Description: Specifies the set of graphics properties that are
//               required for the context associated with the window.
//               This should be the union of the appropriate bits
//               defined in FrameBufferMode.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_frame_buffer_mode(int frameBuffer_mode) {
  _frame_buffer_mode = frameBuffer_mode;
  _specified |= S_frame_buffer_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_frame_buffer_mode
//       Access: Published
//  Description: Returns the set of graphics properties that are
//               in effect for the window.  This will be the union of
//               the corresponding bits from FrameBufferMode.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_frame_buffer_mode() const {
  nassertr(has_frame_buffer_mode(), false);
  return _frame_buffer_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_frame_buffer_mode
//       Access: Published
//  Description: Returns true if the frameBuffer mode has been
//               specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_frame_buffer_mode() const {
  return ((_specified & S_frame_buffer_mode) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_frame_buffer_mode
//       Access: Published
//  Description: Removes the frameBuffer_mode specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_frame_buffer_mode() {
  _specified &= ~S_frame_buffer_mode;
  _frame_buffer_mode = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_single_buffered
//       Access: Published
//  Description: Returns true if the frame buffer indicates a
//               single-buffered mode, false if it indicates double-
//               or triple-buffering.  This is a convenience function
//               to access this useful tidbit of data.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
is_single_buffered() const {
  nassertr(has_frame_buffer_mode(), false);
  return (_frame_buffer_mode & FM_buffer) == FM_single_buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_depth_bits
//       Access: Published
//  Description: Specifies the minimum number of bits that are
//               required for the depth buffer.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_depth_bits(int depth_bits) {
  _depth_bits = depth_bits;
  _specified |= S_depth_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_depth_bits
//       Access: Published
//  Description: Returns the number of bits specified for the depth
//               buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_depth_bits() const {
  return _depth_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_depth_bits
//       Access: Published
//  Description: Returns true if the number of bits for the depth
//               buffer has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_depth_bits() const {
  return ((_specified & S_depth_bits) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_depth_bits
//       Access: Published
//  Description: Removes the depth_bits specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_depth_bits() {
  _specified &= ~S_depth_bits;
  _depth_bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_color_bits
//       Access: Published
//  Description: Specifies the minimum number of bits that are
//               required for all three channels of the color buffer.
//               That is, this is the per-channel color requirement
//               times three.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_color_bits(int color_bits) {
  _color_bits = color_bits;
  _specified |= S_color_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_color_bits
//       Access: Published
//  Description: Returns the number of bits specified for the color
//               buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_color_bits() const {
  return _color_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_color_bits
//       Access: Published
//  Description: Returns true if the number of bits for the color
//               buffer has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_color_bits() const {
  return ((_specified & S_color_bits) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_color_bits
//       Access: Published
//  Description: Removes the color_bits specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_color_bits() {
  _specified &= ~S_color_bits;
  _color_bits = 0;
}

INLINE ostream &
operator << (ostream &out, const FrameBufferProperties &properties) {
  properties.output(out);
  return out;
}
