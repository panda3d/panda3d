// Filename: eggTexture.I
// Created by:  drose (18Jan99)
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
//     Function: EggTexture::set_format
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_format(Format format) {
  _format = format;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_format
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::Format EggTexture::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_mode(WrapMode mode) {
  _wrap_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_mode() const {
  return _wrap_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_u
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_u(WrapMode mode) {
  _wrap_u = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_u
//       Access: Public
//  Description: Returns the amount specified for U wrap.  This may be
//               unspecified, even if there is an overall wrap value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_u() const {
  return _wrap_u;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::determine_wrap_u
//       Access: Public
//  Description: Determines the appropriate wrap in the U direction.
//               This is different from get_wrap_u() in that if the U
//               wrap is unspecified, it returns the overall wrap
//               value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_u() const {
  return (_wrap_u == WM_unspecified) ? get_wrap_mode() : get_wrap_u();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_v
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_v(WrapMode mode) {
  _wrap_v = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_v
//       Access: Public
//  Description: Returns the amount specified for V wrap.  This may be
//               unspecified, even if there is an overall wrap value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_v() const {
  return _wrap_v;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::determine_wrap_v
//       Access: Public
//  Description: Determines the appropriate wrap in the V direction.
//               This is different from get_wrap_v() in that if the U
//               wrap is unspecified, it returns the overall wrap
//               value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_v() const {
  return (_wrap_v == WM_unspecified) ? get_wrap_mode() : get_wrap_v();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_minfilter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_minfilter(FilterType type) {
  _minfilter = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_minfilter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::FilterType EggTexture::
get_minfilter() const {
  return _minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_magfilter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_magfilter(FilterType type) {
  _magfilter = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_magfilter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::FilterType EggTexture::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_anisotropic_degree
//       Access: Public
//  Description: Sets the degree of anisotropic filtering for this
//               texture.  1 is off; higher levels indicate filtering
//               in effect.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_anisotropic_degree(int anisotropic_degree) {
  _anisotropic_degree = anisotropic_degree;
  _flags |= F_has_anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_anisotropic_degree
//       Access: Public
//  Description: Removes the specification of anisotropic filtering
//               from the texture.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_anisotropic_degree() {
  _anisotropic_degree = 0;
  _flags &= ~F_has_anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_anisotropic_degree
//       Access: Public
//  Description: Returns true if a value for the anisotropic filtering
//               degree has been specified for this texture, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_anisotropic_degree() const {
  return (_flags & F_has_anisotropic_degree) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_anisotropic_degree
//       Access: Public
//  Description: Returns the anisotropic filtering degree that has
//               been specified for this texture, or 0 if nothing has
//               been specified.
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_anisotropic_degree() const {
  // note: _anisotropic_degree of 0 and 1 are equivalent (no
  // anisotropic filtering to be done by gsg)
  return _anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_env_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_env_type(EnvType type) {
  _env_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_env_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::EnvType EggTexture::
get_env_type() const {
  return _env_type;
}


////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_transform
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_transform(const LMatrix3d &transform) {
  _transform = transform;
  _flags |= F_has_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_transform
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_transform() {
  _transform = LMatrix3d::ident_mat();
  _flags &= ~F_has_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_transform
//       Access: Public
//  Description: Returns true if a texture matrix transform has been
//               specified for the texture (even if the transform is
//               identity).
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_transform() const {
  return (_flags & F_has_transform) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_transform
//       Access: Public
//  Description: Returns the texture matrix transform if one has been
//               specified, or identity matrix otherwise.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix3d &EggTexture::
get_transform() const {
  return _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::transform_is_identity()
//       Access: Public
//  Description: Returns true if no texture matrix transform has been
//               specified, or if the one specified is the identity
//               transform.  Returns false only if a nonidentity
//               transform has been applied.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
transform_is_identity() const {
  return (!has_transform() ||
          _transform.almost_equal(LMatrix3d::ident_mat(), 0.0001));
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_filename
//       Access: Public
//  Description: Specifies a separate file that will be loaded in with
//               the 1- or 3-component texture and applied as the
//               alpha channel.  This is useful when loading textures
//               from file formats that do not support alpha, for
//               instance jpg.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_alpha_filename(const Filename &alpha_filename) {
  _alpha_filename = alpha_filename;
  _alpha_fullpath = alpha_filename;
  _flags |= F_has_alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_alpha_filename
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_alpha_filename() {
  _alpha_filename = Filename();
  _alpha_fullpath = Filename();
  _flags &= ~F_has_alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_alpha_filename
//       Access: Public
//  Description: Returns true if a separate file for the alpha
//               component has been applied, false otherwise.  See
//               set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_alpha_filename() const {
  return (_flags & F_has_alpha_filename) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_alpha_filename
//       Access: Public
//  Description: Returns the separate file assigned for the alpha
//               channel.  It is an error to call this unless
//               has_alpha_filename() returns true.  See set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggTexture::
get_alpha_filename() const {
  nassertr(has_alpha_filename(), _alpha_filename);
  return _alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_alpha_fullpath
//       Access: Public
//  Description: Returns the full pathname to the alpha file, if it is
//               known; otherwise, returns the same thing as
//               get_alpha_filename().
//
//               This function simply returns whatever was set by the
//               last call to set_alpha_fullpath().  This string is
//               not written to the egg file; its main purpose is to
//               record the full path to the alpha filename if it is
//               known, for egg structures that are generated
//               in-memory and then immediately converted to a scene
//               graph.
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggTexture::
get_alpha_fullpath() const {
  return _alpha_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_fullpath
//       Access: Public
//  Description: Records the full pathname to the file, for the
//               benefit of get_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  _alpha_fullpath = alpha_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_file_channel
//       Access: Public
//  Description: If a separate alpha-file is specified, this indicates
//               which channel number should be extracted from this
//               file to derive the alpha channel for the final image.
//               The default is 0, which means the grayscale
//               combination of r, g, b.  Otherwise, this should be
//               the 1-based channel number, for instance 1, 2, or 3
//               for r, g, or b, respectively, or 4 for the alpha
//               channel of a four-component image.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_alpha_file_channel(int alpha_file_channel) {
  _alpha_file_channel = alpha_file_channel;
  _flags |= F_has_alpha_file_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_alpha_file_channel
//       Access: Public
//  Description: Removes the specification of a particular channel to
//               use from the alpha-file image.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_alpha_file_channel() {
  _alpha_file_channel = 0;
  _flags &= ~F_has_alpha_file_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_alpha_file_channel
//       Access: Public
//  Description: Returns true if a particular channel has been
//               specified for the alpha-file image, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_alpha_file_channel() const {
  return (_flags & F_has_alpha_file_channel) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_alpha_file_channel
//       Access: Public
//  Description: Returns the particular channel that has been
//               specified for the alpha-file image, or 0 if no
//               channel has been specified.  See
//               set_alpha_file_channel().
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_alpha_file_channel() const {
  return _alpha_file_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueEggTextures::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UniqueEggTextures::
UniqueEggTextures(int eq) : _eq(eq) {
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueEggTextures::Function operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UniqueEggTextures::
operator ()(const EggTexture *t1, const EggTexture *t2) const {
  return t1->sorts_less_than(*t2, _eq);
}

