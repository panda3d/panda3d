// Filename: eggTexture.I
// Created by:  drose (18Jan99)
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
//     Function: EggTexture::set_texture_type
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_texture_type(TextureType texture_type) {
  _texture_type = texture_type;

  bool pattern_filename = 
    (_texture_type == TT_3d_texture || _texture_type == TT_cube_map);

  _filename.set_pattern(pattern_filename);
  _fullpath.set_pattern(pattern_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_texture_type
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::TextureType EggTexture::
get_texture_type() const {
  return _texture_type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_format
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_format(Format format) {
  _format = format;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_format
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::Format EggTexture::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_compression_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_compression_mode(CompressionMode mode) {
  _compression_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_compression_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::CompressionMode EggTexture::
get_compression_mode() const {
  return _compression_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_mode(WrapMode mode) {
  _wrap_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_mode() const {
  return _wrap_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_u
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_u(WrapMode mode) {
  _wrap_u = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_u
//       Access: Published
//  Description: Returns the amount specified for U wrap.  This may be
//               unspecified, even if there is an overall wrap value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_u() const {
  return _wrap_u;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::determine_wrap_u
//       Access: Published
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_v(WrapMode mode) {
  _wrap_v = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_v
//       Access: Published
//  Description: Returns the amount specified for V wrap.  This may be
//               unspecified, even if there is an overall wrap value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_v() const {
  return _wrap_v;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::determine_wrap_v
//       Access: Published
//  Description: Determines the appropriate wrap in the V direction.
//               This is different from get_wrap_v() in that if the V
//               wrap is unspecified, it returns the overall wrap
//               value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_v() const {
  return (_wrap_v == WM_unspecified) ? get_wrap_mode() : get_wrap_v();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_wrap_w
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_wrap_w(WrapMode mode) {
  _wrap_w = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_wrap_w
//       Access: Published
//  Description: Returns the amount specified for W wrap.  This may be
//               unspecified, even if there is an overall wrap value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
get_wrap_w() const {
  return _wrap_w;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::determine_wrap_w
//       Access: Published
//  Description: Determines the appropriate wrap in the W direction.
//               This is different from get_wrap_w() in that if the W
//               wrap is unspecified, it returns the overall wrap
//               value.
////////////////////////////////////////////////////////////////////
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_w() const {
  return (_wrap_w == WM_unspecified) ? get_wrap_mode() : get_wrap_w();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_minfilter(FilterType type) {
  _minfilter = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::FilterType EggTexture::
get_minfilter() const {
  return _minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_magfilter(FilterType type) {
  _magfilter = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::FilterType EggTexture::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_anisotropic_degree
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_env_type(EnvType type) {
  _env_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_env_type
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::EnvType EggTexture::
get_env_type() const {
  return _env_type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_combine_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_combine_mode(CombineChannel channel, CombineMode cm) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  _combiner[channel]._mode = cm;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_combine_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::CombineMode EggTexture::
get_combine_mode(CombineChannel channel) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CM_unspecified);
  return _combiner[channel]._mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_combine_source
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_combine_source(CombineChannel channel, int n, CombineSource cs) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  nassertv(n >= 0 && n < (int)CI_num_indices);
  _combiner[channel]._ops[n]._source = cs;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_combine_source
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::CombineSource EggTexture::
get_combine_source(CombineChannel channel, int n) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CS_unspecified);
  nassertr(n >= 0 && n < (int)CI_num_indices, CS_unspecified);
  return _combiner[channel]._ops[n]._source;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_combine_operand
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_combine_operand(CombineChannel channel, int n, CombineOperand co) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  nassertv(n >= 0 && n < (int)CI_num_indices);
  _combiner[channel]._ops[n]._operand = co;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_combine_operand
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::CombineOperand EggTexture::
get_combine_operand(CombineChannel channel, int n) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CO_unspecified);
  nassertr(n >= 0 && n < (int)CI_num_indices, CO_unspecified);
  return _combiner[channel]._ops[n]._operand;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_saved_result
//       Access: Published
//  Description: Sets the saved_result flag.  When this is true, the
//               output of this stage is not part of the normal
//               pipeline--that is, it will not be supplied as the
//               "previous" source for the next texture stage--but it
//               will instead be supplied as the "last_saved_result"
//               source for any future stages, until the next
//               TextureStage with a saved_result set true is
//               encountered.
//
//               This can be used to reuse the results of this texture
//               stage as input to more than one stage later in the
//               pipeline.
//
//               The last texture in the pipeline (the one with the
//               highest sort value) should not have this flag set.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_saved_result(bool saved_result) {
  _saved_result = saved_result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_saved_result
//       Access: Published
//  Description: Returns the current setting of the saved_result flag.
//               See set_saved_result().
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
get_saved_result() const {
  return _saved_result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_tex_gen
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_tex_gen(TexGen tex_gen) {
  _tex_gen = tex_gen;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_tex_gen
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::TexGen EggTexture::
get_tex_gen() const {
  return _tex_gen;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_quality_level
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_quality_level(QualityLevel quality_level) {
  _quality_level = quality_level;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_quality_level
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTexture::QualityLevel EggTexture::
get_quality_level() const {
  return _quality_level;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_stage_name
//       Access: Published
//  Description: Specifies the particular TextureStage this texture
//               will be rendered on by name.  If this is omitted, the
//               texture will be rendered on the default TextureStage,
//               unless some other stage-specific property is
//               specificied, in which case the texture will be
//               rendered on a TextureStage with the same name as the
//               tref.  This is in support of multitexturing.
//
//               Each different TextureStage in the world must be
//               uniquely named.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_stage_name(const string &stage_name) {
  _stage_name = stage_name;
  _flags |= F_has_stage_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_stage_name
//       Access: Published
//  Description: Removes the named TextureStage specification.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_stage_name() {
  _stage_name = string();
  _flags &= ~F_has_stage_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_stage_name
//       Access: Published
//  Description: Returns true if a stage name has been explicitly
//               specified for this texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_stage_name() const {
  return (_flags & F_has_stage_name) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_stage_name
//       Access: Published
//  Description: Returns the stage name that has been specified for
//               this texture, or the tref name if no texture stage
//               has explicitly been specified.
////////////////////////////////////////////////////////////////////
INLINE const string &EggTexture::
get_stage_name() const {
  return has_stage_name() ? _stage_name : get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_priority
//       Access: Published
//  Description: Sets the importance of this texture with respect to
//               other textures also applied on the same geometry.
//               This is only meaningful in the presence of
//               multitexturing.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_priority(int priority) {
  _priority = priority;
  _flags |= F_has_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_priority
//       Access: Published
//  Description: Removes the specification of multitexture priority
//               from the texture.  The default priority value is 0.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_priority() {
  _priority = 0;
  _flags &= ~F_has_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_priority
//       Access: Published
//  Description: Returns true if a priority value for multitexture
//               importance has been specified for the texture, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_priority() const {
  return (_flags & F_has_priority) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_priority
//       Access: Published
//  Description: Returns the multitexture importance value that has been
//               specified for the texture, or 0 if no priority value has
//               been specified.
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_color(const LColor &color) {
  _color = color;
  _flags |= F_has_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_color() {
  _color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _flags &= ~F_has_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_color
//       Access: Published
//  Description: Returns true if a blend color has been
//               specified for the texture.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_color() const {
  return (_flags & F_has_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_color
//       Access: Published
//  Description: Returns the blend color if one has been
//               specified, or (0, 0, 0, 1) otherwise.
////////////////////////////////////////////////////////////////////
INLINE const LColor &EggTexture::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_border_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_border_color(const LColor &border_color) {
  _border_color = border_color;
  _flags |= F_has_border_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_border_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_border_color() {
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _flags &= ~F_has_border_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_border_color
//       Access: Published
//  Description: Returns true if a border color has been
//               specified for the texture.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_border_color() const {
  return (_flags & F_has_border_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_border_color
//       Access: Published
//  Description: Returns the border color if one has been
//               specified, or (0, 0, 0, 1) otherwise.
////////////////////////////////////////////////////////////////////
INLINE const LColor &EggTexture::
get_border_color() const {
  return _border_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_uv_name
//       Access: Published
//  Description: Specifies the named set of texture coordinates that
//               this texture will use when it is applied to geometry.
//               Geometry may have multiple sets of texture
//               coordinates defined, by name.
//
//               If this is not specified for a particular texture,
//               the default set of texture coordinates will be used.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_uv_name(const string &uv_name) {
  if (uv_name == "default" || uv_name.empty()) {
    clear_uv_name();
  } else {
    _uv_name = uv_name;
    _flags |= F_has_uv_name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_uv_name
//       Access: Published
//  Description: Removes the restriction to a particular named set of
//               texture coordinates and restores the texture to using
//               the default texture coordinates.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_uv_name() {
  _uv_name = string();
  _flags &= ~F_has_uv_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_uv_name
//       Access: Published
//  Description: Returns true if a texcoord name has been explicitly
//               specified for this texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_uv_name() const {
  return (_flags & F_has_uv_name) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_uv_name
//       Access: Published
//  Description: Returns the texcoord name that has been specified for
//               this texture, or the empty string if no texcoord name
//               has explicitly been specified.
////////////////////////////////////////////////////////////////////
INLINE const string &EggTexture::
get_uv_name() const {
  return _uv_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_rgb_scale
//       Access: Published
//  Description: Sets an additional factor that will scale all three
//               r, g, b components after the texture has been
//               applied.  This is used only when a combine mode is in
//               effect.
//
//               The only legal values are 1, 2, or 4.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_rgb_scale(int rgb_scale) {
  _rgb_scale = rgb_scale;
  _flags |= F_has_rgb_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_rgb_scale
//       Access: Published
//  Description: Removes the rgb_scale from the texture and restores
//               it to the default value of 1.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_rgb_scale() {
  _rgb_scale = 1;
  _flags &= ~F_has_rgb_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_rgb_scale
//       Access: Published
//  Description: Returns true if an rgb_scale has been specified for
//               the texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_rgb_scale() const {
  return (_flags & F_has_rgb_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_rgb_scale
//       Access: Published
//  Description: Returns the rgb_scale value that has been
//               specified for the texture, or 1 if no rgb_scale value
//               has been specified.
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_rgb_scale() const {
  return _rgb_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_scale
//       Access: Published
//  Description: Sets an additional factor that will scale the
//               alpha component after the texture has been applied.
//               This is used only when a combine mode is in effect.
//
//               The only legal values are 1, 2, or 4.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_alpha_scale(int alpha_scale) {
  _alpha_scale = alpha_scale;
  _flags |= F_has_alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_alpha_scale
//       Access: Published
//  Description: Removes the alpha_scale from the texture and restores
//               it to the default value of 1.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_alpha_scale() {
  _alpha_scale = 1;
  _flags &= ~F_has_alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_alpha_scale
//       Access: Published
//  Description: Returns true if an alpha_scale has been specified for
//               the texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_alpha_scale() const {
  return (_flags & F_has_alpha_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_alpha_scale
//       Access: Published
//  Description: Returns the alpha_scale value that has been
//               specified for the texture, or 1 if no alpha_scale
//               value has been specified.
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_alpha_scale() const {
  return _alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_filename
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Records the full pathname to the file, for the
//               benefit of get_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  _alpha_fullpath = alpha_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_alpha_file_channel
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Returns true if a particular channel has been
//               specified for the alpha-file image, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_alpha_file_channel() const {
  return (_flags & F_has_alpha_file_channel) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_alpha_file_channel
//       Access: Published
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
//     Function: EggTexture::set_multiview
//       Access: Published
//  Description: Sets the multiview flag.
//
//               If multiview is true, the filename should contain a
//               hash mark ('#'), which will be filled in with the
//               view number; and a multiview texture will be defined
//               with a series of images, one for each view.
//
//               A multiview texture is most often used for stereo
//               textures, but other uses are also possible, such as
//               for texture animation.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_multiview(bool multiview) {
  _multiview = multiview;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_multiview
//       Access: Published
//  Description: Returns the current setting of the multiview flag.
//               See set_multiview().
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
get_multiview() const {
  return _multiview;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_num_views
//       Access: Published
//  Description: When loading a 3-D multiview texture, this parameter
//               is necessary to specify how many views will be
//               expected.  The z size is determined implicitly from
//               the number of images loaded.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_num_views(int num_views) {
  _num_views = num_views;
  _flags |= F_has_num_views;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_num_views
//       Access: Published
//  Description: Removes the specification of the number of views
//               for a 3-D multiview texture.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_num_views() {
  _num_views = 0;
  _flags &= ~F_has_num_views;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_num_views
//       Access: Published
//  Description: Returns true if the number of views has been
//               specified for the 3-D multiview texture, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_num_views() const {
  return (_flags & F_has_num_views) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_num_views
//       Access: Published
//  Description: Returns the specified number of views specified for
//               the 3-D multiview texture.  See set_num_views().
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_num_views() const {
  return _num_views;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_read_mipmaps
//       Access: Published
//  Description: Sets the read_mipmaps flag.
//
//               If read_mipmaps is true, the filename should contain
//               a hash mark ('#'), which will be filled in with the
//               mipmap level number; and the texture will be defined
//               with a series of images, one for each mipmap level.
//
//               If the filename is of a type that already requires a
//               hash mark, such as a cube map or a 3-d texture, then
//               the filename should now require two hash marks, and
//               the first one indicates the mipmap level number,
//               while the second indicates the face number or 3-d
//               level number.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_read_mipmaps(bool read_mipmaps) {
  _read_mipmaps = read_mipmaps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_read_mipmaps
//       Access: Published
//  Description: Returns the current setting of the read_mipmaps flag.
//               See set_read_mipmaps().
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
get_read_mipmaps() const {
  return _read_mipmaps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_min_lod
//       Access: Published
//  Description: Sets the minimum mipmap level that may be sampled.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_min_lod(double min_lod) {
  _min_lod = min_lod;
  _flags |= F_has_min_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_min_lod
//       Access: Published
//  Description: Removes the specification of a minimum mipmap level
//               from the texture.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_min_lod() {
  _min_lod = -1000;
  _flags &= ~F_has_min_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_min_lod
//       Access: Published
//  Description: Returns true if a value for the minimum mipmap level
//               has been specified for this texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_min_lod() const {
  return (_flags & F_has_min_lod) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_min_lod
//       Access: Published
//  Description: Returns the minimum mipmap level that has been
//               specified for this texture.
////////////////////////////////////////////////////////////////////
INLINE double EggTexture::
get_min_lod() const {
  return _min_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_max_lod
//       Access: Published
//  Description: Sets the maximum mipmap level that may be sampled.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_max_lod(double max_lod) {
  _max_lod = max_lod;
  _flags |= F_has_max_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_max_lod
//       Access: Published
//  Description: Removes the specification of a maximum mipmap level
//               from the texture.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_max_lod() {
  _max_lod = 1000;
  _flags &= ~F_has_max_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_max_lod
//       Access: Published
//  Description: Returns true if a value for the maximum mipmap level
//               has been specified for this texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_max_lod() const {
  return (_flags & F_has_max_lod) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_max_lod
//       Access: Published
//  Description: Returns the maximum mipmap level that has been
//               specified for this texture.
////////////////////////////////////////////////////////////////////
INLINE double EggTexture::
get_max_lod() const {
  return _max_lod;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::set_lod_bias
//       Access: Published
//  Description: Sets the mipmap level bias that is added to the
//               mipmap level to be sampled.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
set_lod_bias(double lod_bias) {
  _lod_bias = lod_bias;
  _flags |= F_has_lod_bias;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::clear_lod_bias
//       Access: Published
//  Description: Removes the specification of a maximum mipmap level
//               from the texture.
////////////////////////////////////////////////////////////////////
INLINE void EggTexture::
clear_lod_bias() {
  _lod_bias = 1000;
  _flags &= ~F_has_lod_bias;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_lod_bias
//       Access: Published
//  Description: Returns true if a value for the maximum mipmap level
//               has been specified for this texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTexture::
has_lod_bias() const {
  return (_flags & F_has_lod_bias) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_lod_bias
//       Access: Published
//  Description: Returns the maximum mipmap level that has been
//               specified for this texture.
////////////////////////////////////////////////////////////////////
INLINE double EggTexture::
get_lod_bias() const {
  return _lod_bias;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::get_multitexture_sort
//       Access: Published
//  Description: Returns an integer that represents the depth to which
//               this texture is layered on all other textures in the
//               egg file.  In general, if texture A is layered over
//               texture B, then sort(A) > sort(B).  If texture A is
//               never layered over any other texture, then sort(A) ==
//               0.  More than that is difficult to guarantee.
////////////////////////////////////////////////////////////////////
INLINE int EggTexture::
get_multitexture_sort() const {
  return _multitexture_sort;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::SourceAndOperand::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggTexture::SourceAndOperand::
SourceAndOperand() :
  _source(CS_unspecified),
  _operand(CO_unspecified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::Combiner::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggTexture::Combiner::
Combiner() :
  _mode(CM_unspecified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueEggTextures::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UniqueEggTextures::
UniqueEggTextures(int eq) : _eq(eq) {
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueEggTextures::Function operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UniqueEggTextures::
operator ()(const EggTexture *t1, const EggTexture *t2) const {
  return t1->sorts_less_than(*t2, _eq);
}

