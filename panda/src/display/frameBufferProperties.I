// Filename: frameBufferProperties.I
// Created by:  drose (27Jan03)
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
//     Function: FrameBufferProperties::is_single_buffered
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
is_single_buffered() const {
  return (_property[FBP_back_buffers] == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_stereo
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool FrameBufferProperties::
is_stereo() const {
  return (_property[FBP_stereo] != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::operator <<
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ostream &
operator << (ostream &out, const FrameBufferProperties &properties) {
  properties.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_depth_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_depth_bits() const {
  return _property[FBP_depth_bits];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_color_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_color_bits() const {
  return _property[FBP_color_bits];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_alpha_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_alpha_bits() const {
  return _property[FBP_alpha_bits];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_stencil_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_stencil_bits() const {
  return _property[FBP_stencil_bits];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_accum_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_accum_bits() const {
  return _property[FBP_accum_bits];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_rgba
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_rgba() const {
  return _property[FBP_aux_rgba];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_hrgba
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_hrgba() const {
  return _property[FBP_aux_hrgba];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_float
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_aux_float() const {
  return _property[FBP_aux_float];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_multisamples
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_multisamples() const {
  return _property[FBP_multisamples];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_coverage_samples
//       Access: Published
//  Description: If coverage samples are specified, and there is
//               hardware support, we use coverage multisampling.
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_coverage_samples() const {
  return _property[FBP_coverage_samples];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_back_buffers
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_back_buffers() const {
  return _property[FBP_back_buffers];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_indexed_color
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_indexed_color() const {
  return _property[FBP_indexed_color];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_rgb_color
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_rgb_color() const {
  return _property[FBP_rgb_color];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_stereo
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_stereo() const {
  return _property[FBP_stereo];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_force_hardware
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_force_hardware() const {
  return _property[FBP_force_hardware];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_force_software
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int FrameBufferProperties::
get_force_software() const {
  return _property[FBP_force_software];
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_depth_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_depth_bits(int n) {
  _property[FBP_depth_bits] = n;
  _specified[FBP_depth_bits] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_color_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_color_bits(int n) {
  _property[FBP_color_bits] = n;
  _specified[FBP_color_bits] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_alpha_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_alpha_bits(int n) {
  _property[FBP_alpha_bits] = n;
  _specified[FBP_alpha_bits] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_stencil_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_stencil_bits(int n) {
  _property[FBP_stencil_bits] = n;
  _specified[FBP_stencil_bits] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_accum_bits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_accum_bits(int n) {
  _property[FBP_accum_bits] = n;
  _specified[FBP_accum_bits] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_rgba
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_rgba(int n) {
  nassertv(n < 4);
  _property[FBP_aux_rgba] = n;
  _specified[FBP_aux_rgba] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_hrgba
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_hrgba(int n) {
  nassertv(n < 4);
  _property[FBP_aux_hrgba] = n;
  _specified[FBP_aux_hrgba] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_aux_float
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_aux_float(int n) {
  nassertv(n < 4);
  _property[FBP_aux_float] = n;
  _specified[FBP_aux_float] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_multisamples
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_multisamples(int n) {
  _property[FBP_multisamples] = n;
  _specified[FBP_multisamples] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_coverage_samples
//       Access: Published
//  Description: If coverage samples are specified, and there is
//               hardware support, we use coverage multisampling
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_coverage_samples(int n) {
  _property[FBP_coverage_samples] = n;
  _specified[FBP_coverage_samples] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_back_buffers
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_back_buffers(int n) {
  _property[FBP_back_buffers] = n;
  _specified[FBP_back_buffers] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_indexed_color
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_indexed_color(int n) {
  _property[FBP_indexed_color] = n;
  _specified[FBP_indexed_color] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_rgb_color
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_rgb_color(int n) {
  _property[FBP_rgb_color] = n;
  _specified[FBP_rgb_color] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_stereo
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_stereo(int n) {
  _property[FBP_stereo] = n;
  _specified[FBP_stereo] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_force_hardware
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_force_hardware(int n) {
  _property[FBP_force_hardware] = n;
  _specified[FBP_force_hardware] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_force_software
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferProperties::
set_force_software(int n) {
  _property[FBP_force_software] = n;
  _specified[FBP_force_software] = true;
}
