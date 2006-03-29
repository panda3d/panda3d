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
  recalc_buffer_mask();
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
  recalc_buffer_mask();
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
//     Function: FrameBufferProperties::has_mode
//       Access: Published
//  Description: Returns true if the frame buffer mode, logically
//               anded with the given mask, is nonzero.  This is a
//               convenience function to access this useful tidbit of
//               data.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_mode(int bits) const {
  nassertr(has_frame_buffer_mode(), false);
  return (_frame_buffer_mode & bits) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_stereo
//       Access: Published
//  Description: Returns true if the frame buffer indicates a
//               hardware stereo mode, false otherwise.  This is a
//               convenience function to access this useful tidbit of
//               data.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
is_stereo() const {
  nassertr(has_frame_buffer_mode(), false);
  return (_frame_buffer_mode & FM_stereo) != 0;
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
  _depth_bits = 1;
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
  _color_bits = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_alpha_bits
//       Access: Published
//  Description: Specifies the minimum number of bits that are
//               required of the alpha buffer.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_alpha_bits(int alpha_bits) {
  _alpha_bits = alpha_bits;
  _specified |= S_alpha_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_alpha_bits
//       Access: Published
//  Description: Returns the number of bits specified for the alpha
//               buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_alpha_bits() const {
  return _alpha_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_alpha_bits
//       Access: Published
//  Description: Returns true if the number of bits for the alpha
//               buffer has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_alpha_bits() const {
  return ((_specified & S_alpha_bits) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_alpha_bits
//       Access: Published
//  Description: Removes the alpha_bits specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_alpha_bits() {
  _specified &= ~S_alpha_bits;
  _alpha_bits = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_stencil_bits
//       Access: Published
//  Description: Specifies the minimum number of bits that are
//               required for the stencil buffer.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_stencil_bits(int stencil_bits) {
  _stencil_bits = stencil_bits;
  _specified |= S_stencil_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_stencil_bits
//       Access: Published
//  Description: Returns the number of bits specified for the stencil
//               buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_stencil_bits() const {
  return _stencil_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_stencil_bits
//       Access: Published
//  Description: Returns true if the number of bits for the stencil
//               buffer has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_stencil_bits() const {
  return ((_specified & S_stencil_bits) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_stencil_bits
//       Access: Published
//  Description: Removes the stencil_bits specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_stencil_bits() {
  _specified &= ~S_stencil_bits;
  _stencil_bits = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_multisamples
//       Access: Published
//  Description: Specifies the minimum number of samples that are
//               required for the multisample buffer.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_multisamples(int multisamples) {
  _multisamples = multisamples;
  _specified |= S_multisamples;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_multisamples
//       Access: Published
//  Description: Returns the number of samples specified for the
//               multisample buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_multisamples() const {
  return _multisamples;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_multisamples
//       Access: Published
//  Description: Returns true if the number of bits for the multisample
//               buffer has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_multisamples() const {
  return ((_specified & S_multisamples) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_multisamples
//       Access: Published
//  Description: Removes the multisamples specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_multisamples() {
  _specified &= ~S_multisamples;
  _multisamples = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_rgba
//       Access: Published
//  Description: Specifies the exact number of auxiliary RGBA
//               bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_rgba(int aux_rgba) {
  _aux_rgba = aux_rgba;
  _specified |= S_aux_rgba;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_rgba
//       Access: Published
//  Description: Returns the exact number of auxiliary RGBA
//               bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_rgba() const {
  return _aux_rgba;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_aux_rgba
//       Access: Published
//  Description: Returns true if the number auxiliary RGBA
//               bitplanes has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_aux_rgba() const {
  return ((_specified & S_aux_rgba) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_aux_rgba
//       Access: Published
//  Description: Removes the aux_rgba specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_aux_rgba() {
  _specified &= ~S_aux_rgba;
  _aux_rgba = 0;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_hrgba
//       Access: Published
//  Description: Specifies the exact number of auxiliary half-float
//               RGBA bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_hrgba(int aux_hrgba) {
  _aux_hrgba = aux_hrgba;
  _specified |= S_aux_hrgba;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_hrgba
//       Access: Published
//  Description: Returns the exact number of auxiliary half-float
//               RGBA bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_hrgba() const {
  return _aux_hrgba;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_aux_hrgba
//       Access: Published
//  Description: Returns true if the number of auxiliary half-float
//               RGBA bitplanes has been specified, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_aux_hrgba() const {
  return ((_specified & S_aux_hrgba) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_aux_hrgba
//       Access: Published
//  Description: Removes the aux_hrgba specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_aux_hrgba() {
  _specified &= ~S_aux_hrgba;
  _aux_hrgba = 0;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_float
//       Access: Published
//  Description: Specifies the exact number of auxiliary float
//               single-channel bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_float(int aux_float) {
  _aux_float = aux_float;
  _specified |= S_aux_float;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_float
//       Access: Published
//  Description: Returns the exact number of auxiliary float
//               single-channel bitplanes that are required.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_float() const {
  return _aux_float;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::has_aux_float
//       Access: Published
//  Description: Returns true if the number of auxiliary float
//               single-channel bitplanes has been specified, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
has_aux_float() const {
  return ((_specified & S_aux_float) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear_aux_float
//       Access: Published
//  Description: Removes the aux_float specification from the
//               properties.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
clear_aux_float() {
  _specified &= ~S_aux_float;
  _aux_float = 0;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_specified
//       Access: Published
//  Description: Sets all the specified bits.  Effectively, this
//               causes the default values to be specified for all
//               not-yet-specified fields.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_specified() {
  _specified = S_ALL_SPECIFIED;
  recalc_buffer_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_buffer_mask
//       Access: Public
//  Description: Returns the buffer mask for the bitplanes
//               in the frame buffer.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_buffer_mask() const {
  return _buffer_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::buffer_mask_remove
//       Access: Public
//  Description: Removes one or more bits from the buffer mask.
//               This is generally called by the gsg when one of the
//               bitplanes is failing for some implementation-
//               dependent reason.  Changing the properties will
//               cause the _buffer_mask information to be lost.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
buffer_mask_remove(int bits) {
  _buffer_mask &= (~bits);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::buffer_mask_add
//       Access: Public
//  Description: Adds one or more bits to the buffer mask.
//               This is generally called by the gsg when the
//               implementation supplies a bitplane that was not
//               specifically requested (say, a freebie stencil
//               buffer). Changing the properties will
//               cause the _buffer_mask information to be lost.
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
buffer_mask_add(int bits) {
  _buffer_mask |= bits;
}

INLINE ostream &
operator << (ostream &out, const FrameBufferProperties &properties) {
  properties.output(out);
  return out;
}
