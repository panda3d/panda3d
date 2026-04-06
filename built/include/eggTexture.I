/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTexture.I
 * @author drose
 * @date 1999-01-18
 */

/**
 *
 */
INLINE void EggTexture::
set_texture_type(TextureType texture_type) {
  _texture_type = texture_type;

  bool pattern_filename =
    (_texture_type == TT_3d_texture || _texture_type == TT_cube_map);

  _filename.set_pattern(pattern_filename);
  _fullpath.set_pattern(pattern_filename);
}

/**
 *
 */
INLINE EggTexture::TextureType EggTexture::
get_texture_type() const {
  return _texture_type;
}

/**
 *
 */
INLINE void EggTexture::
set_format(Format format) {
  _format = format;
}

/**
 *
 */
INLINE EggTexture::Format EggTexture::
get_format() const {
  return _format;
}

/**
 *
 */
INLINE void EggTexture::
set_compression_mode(CompressionMode mode) {
  _compression_mode = mode;
}

/**
 *
 */
INLINE EggTexture::CompressionMode EggTexture::
get_compression_mode() const {
  return _compression_mode;
}

/**
 *
 */
INLINE void EggTexture::
set_wrap_mode(WrapMode mode) {
  _wrap_mode = mode;
}

/**
 *
 */
INLINE EggTexture::WrapMode EggTexture::
get_wrap_mode() const {
  return _wrap_mode;
}

/**
 *
 */
INLINE void EggTexture::
set_wrap_u(WrapMode mode) {
  _wrap_u = mode;
}

/**
 * Returns the amount specified for U wrap.  This may be unspecified, even if
 * there is an overall wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
get_wrap_u() const {
  return _wrap_u;
}

/**
 * Determines the appropriate wrap in the U direction.  This is different from
 * get_wrap_u() in that if the U wrap is unspecified, it returns the overall
 * wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_u() const {
  return (_wrap_u == WM_unspecified) ? get_wrap_mode() : get_wrap_u();
}

/**
 *
 */
INLINE void EggTexture::
set_wrap_v(WrapMode mode) {
  _wrap_v = mode;
}

/**
 * Returns the amount specified for V wrap.  This may be unspecified, even if
 * there is an overall wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
get_wrap_v() const {
  return _wrap_v;
}

/**
 * Determines the appropriate wrap in the V direction.  This is different from
 * get_wrap_v() in that if the V wrap is unspecified, it returns the overall
 * wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_v() const {
  return (_wrap_v == WM_unspecified) ? get_wrap_mode() : get_wrap_v();
}

/**
 *
 */
INLINE void EggTexture::
set_wrap_w(WrapMode mode) {
  _wrap_w = mode;
}

/**
 * Returns the amount specified for W wrap.  This may be unspecified, even if
 * there is an overall wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
get_wrap_w() const {
  return _wrap_w;
}

/**
 * Determines the appropriate wrap in the W direction.  This is different from
 * get_wrap_w() in that if the W wrap is unspecified, it returns the overall
 * wrap value.
 */
INLINE EggTexture::WrapMode EggTexture::
determine_wrap_w() const {
  return (_wrap_w == WM_unspecified) ? get_wrap_mode() : get_wrap_w();
}

/**
 *
 */
INLINE void EggTexture::
set_minfilter(FilterType type) {
  _minfilter = type;
}

/**
 *
 */
INLINE EggTexture::FilterType EggTexture::
get_minfilter() const {
  return _minfilter;
}

/**
 *
 */
INLINE void EggTexture::
set_magfilter(FilterType type) {
  _magfilter = type;
}

/**
 *
 */
INLINE EggTexture::FilterType EggTexture::
get_magfilter() const {
  return _magfilter;
}

/**
 * Sets the degree of anisotropic filtering for this texture.  1 is off;
 * higher levels indicate filtering in effect.
 */
INLINE void EggTexture::
set_anisotropic_degree(int anisotropic_degree) {
  _anisotropic_degree = anisotropic_degree;
  _flags |= F_has_anisotropic_degree;
}

/**
 * Removes the specification of anisotropic filtering from the texture.
 */
INLINE void EggTexture::
clear_anisotropic_degree() {
  _anisotropic_degree = 0;
  _flags &= ~F_has_anisotropic_degree;
}

/**
 * Returns true if a value for the anisotropic filtering degree has been
 * specified for this texture, false otherwise.
 */
INLINE bool EggTexture::
has_anisotropic_degree() const {
  return (_flags & F_has_anisotropic_degree) != 0;
}

/**
 * Returns the anisotropic filtering degree that has been specified for this
 * texture, or 0 if nothing has been specified.
 */
INLINE int EggTexture::
get_anisotropic_degree() const {
  // note: _anisotropic_degree of 0 and 1 are equivalent (no anisotropic
  // filtering to be done by gsg)
  return _anisotropic_degree;
}

/**
 *
 */
INLINE void EggTexture::
set_env_type(EnvType type) {
  _env_type = type;
}

/**
 *
 */
INLINE EggTexture::EnvType EggTexture::
get_env_type() const {
  return _env_type;
}

/**
 *
 */
INLINE void EggTexture::
set_combine_mode(CombineChannel channel, CombineMode cm) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  _combiner[channel]._mode = cm;
}

/**
 *
 */
INLINE EggTexture::CombineMode EggTexture::
get_combine_mode(CombineChannel channel) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CM_unspecified);
  return _combiner[channel]._mode;
}

/**
 *
 */
INLINE void EggTexture::
set_combine_source(CombineChannel channel, int n, CombineSource cs) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  nassertv(n >= 0 && n < (int)CI_num_indices);
  _combiner[channel]._ops[n]._source = cs;
}

/**
 *
 */
INLINE EggTexture::CombineSource EggTexture::
get_combine_source(CombineChannel channel, int n) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CS_unspecified);
  nassertr(n >= 0 && n < (int)CI_num_indices, CS_unspecified);
  return _combiner[channel]._ops[n]._source;
}

/**
 *
 */
INLINE void EggTexture::
set_combine_operand(CombineChannel channel, int n, CombineOperand co) {
  nassertv((int)channel >= 0 && (int)channel < (int)CC_num_channels);
  nassertv(n >= 0 && n < (int)CI_num_indices);
  _combiner[channel]._ops[n]._operand = co;
}

/**
 *
 */
INLINE EggTexture::CombineOperand EggTexture::
get_combine_operand(CombineChannel channel, int n) const {
  nassertr((int)channel >= 0 && (int)channel < (int)CC_num_channels, CO_unspecified);
  nassertr(n >= 0 && n < (int)CI_num_indices, CO_unspecified);
  return _combiner[channel]._ops[n]._operand;
}

/**
 * Sets the saved_result flag.  When this is true, the output of this stage is
 * not part of the normal pipeline--that is, it will not be supplied as the
 * "previous" source for the next texture stage--but it will instead be
 * supplied as the "last_saved_result" source for any future stages, until the
 * next TextureStage with a saved_result set true is encountered.
 *
 * This can be used to reuse the results of this texture stage as input to
 * more than one stage later in the pipeline.
 *
 * The last texture in the pipeline (the one with the highest sort value)
 * should not have this flag set.
 */
INLINE void EggTexture::
set_saved_result(bool saved_result) {
  _saved_result = saved_result;
}

/**
 * Returns the current setting of the saved_result flag.  See
 * set_saved_result().
 */
INLINE bool EggTexture::
get_saved_result() const {
  return _saved_result;
}

/**
 *
 */
INLINE void EggTexture::
set_tex_gen(TexGen tex_gen) {
  _tex_gen = tex_gen;
}

/**
 *
 */
INLINE EggTexture::TexGen EggTexture::
get_tex_gen() const {
  return _tex_gen;
}

/**
 *
 */
INLINE void EggTexture::
set_quality_level(QualityLevel quality_level) {
  _quality_level = quality_level;
}

/**
 *
 */
INLINE EggTexture::QualityLevel EggTexture::
get_quality_level() const {
  return _quality_level;
}

/**
 * Specifies the particular TextureStage this texture will be rendered on by
 * name.  If this is omitted, the texture will be rendered on the default
 * TextureStage, unless some other stage-specific property is specificied, in
 * which case the texture will be rendered on a TextureStage with the same
 * name as the tref.  This is in support of multitexturing.
 *
 * Each different TextureStage in the world must be uniquely named.
 */
INLINE void EggTexture::
set_stage_name(const std::string &stage_name) {
  _stage_name = stage_name;
  _flags |= F_has_stage_name;
}

/**
 * Removes the named TextureStage specification.
 */
INLINE void EggTexture::
clear_stage_name() {
  _stage_name = std::string();
  _flags &= ~F_has_stage_name;
}

/**
 * Returns true if a stage name has been explicitly specified for this
 * texture, false otherwise.
 */
INLINE bool EggTexture::
has_stage_name() const {
  return (_flags & F_has_stage_name) != 0;
}

/**
 * Returns the stage name that has been specified for this texture, or the
 * tref name if no texture stage has explicitly been specified.
 */
INLINE const std::string &EggTexture::
get_stage_name() const {
  return has_stage_name() ? _stage_name : get_name();
}

/**
 * Sets the importance of this texture with respect to other textures also
 * applied on the same geometry.  This is only meaningful in the presence of
 * multitexturing.
 */
INLINE void EggTexture::
set_priority(int priority) {
  _priority = priority;
  _flags |= F_has_priority;
}

/**
 * Removes the specification of multitexture priority from the texture.  The
 * default priority value is 0.
 */
INLINE void EggTexture::
clear_priority() {
  _priority = 0;
  _flags &= ~F_has_priority;
}

/**
 * Returns true if a priority value for multitexture importance has been
 * specified for the texture, false otherwise.
 */
INLINE bool EggTexture::
has_priority() const {
  return (_flags & F_has_priority) != 0;
}

/**
 * Returns the multitexture importance value that has been specified for the
 * texture, or 0 if no priority value has been specified.
 */
INLINE int EggTexture::
get_priority() const {
  return _priority;
}

/**
 *
 */
INLINE void EggTexture::
set_color(const LColor &color) {
  _color = color;
  _flags |= F_has_color;
}

/**
 *
 */
INLINE void EggTexture::
clear_color() {
  _color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _flags &= ~F_has_color;
}

/**
 * Returns true if a blend color has been specified for the texture.
 */
INLINE bool EggTexture::
has_color() const {
  return (_flags & F_has_color) != 0;
}

/**
 * Returns the blend color if one has been specified, or (0, 0, 0, 1)
 * otherwise.
 */
INLINE const LColor &EggTexture::
get_color() const {
  return _color;
}

/**
 *
 */
INLINE void EggTexture::
set_border_color(const LColor &border_color) {
  _border_color = border_color;
  _flags |= F_has_border_color;
}

/**
 *
 */
INLINE void EggTexture::
clear_border_color() {
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _flags &= ~F_has_border_color;
}

/**
 * Returns true if a border color has been specified for the texture.
 */
INLINE bool EggTexture::
has_border_color() const {
  return (_flags & F_has_border_color) != 0;
}

/**
 * Returns the border color if one has been specified, or (0, 0, 0, 1)
 * otherwise.
 */
INLINE const LColor &EggTexture::
get_border_color() const {
  return _border_color;
}

/**
 * Specifies the named set of texture coordinates that this texture will use
 * when it is applied to geometry.  Geometry may have multiple sets of texture
 * coordinates defined, by name.
 *
 * If this is not specified for a particular texture, the default set of
 * texture coordinates will be used.
 */
INLINE void EggTexture::
set_uv_name(const std::string &uv_name) {
  if (uv_name == "default" || uv_name.empty()) {
    clear_uv_name();
  } else {
    _uv_name = uv_name;
    _flags |= F_has_uv_name;
  }
}

/**
 * Removes the restriction to a particular named set of texture coordinates
 * and restores the texture to using the default texture coordinates.
 */
INLINE void EggTexture::
clear_uv_name() {
  _uv_name = std::string();
  _flags &= ~F_has_uv_name;
}

/**
 * Returns true if a texcoord name has been explicitly specified for this
 * texture, false otherwise.
 */
INLINE bool EggTexture::
has_uv_name() const {
  return (_flags & F_has_uv_name) != 0;
}

/**
 * Returns the texcoord name that has been specified for this texture, or the
 * empty string if no texcoord name has explicitly been specified.
 */
INLINE const std::string &EggTexture::
get_uv_name() const {
  return _uv_name;
}

/**
 * Sets an additional factor that will scale all three r, g, b components
 * after the texture has been applied.  This is used only when a combine mode
 * is in effect.
 *
 * The only legal values are 1, 2, or 4.
 */
INLINE void EggTexture::
set_rgb_scale(int rgb_scale) {
  _rgb_scale = rgb_scale;
  _flags |= F_has_rgb_scale;
}

/**
 * Removes the rgb_scale from the texture and restores it to the default value
 * of 1.
 */
INLINE void EggTexture::
clear_rgb_scale() {
  _rgb_scale = 1;
  _flags &= ~F_has_rgb_scale;
}

/**
 * Returns true if an rgb_scale has been specified for the texture, false
 * otherwise.
 */
INLINE bool EggTexture::
has_rgb_scale() const {
  return (_flags & F_has_rgb_scale) != 0;
}

/**
 * Returns the rgb_scale value that has been specified for the texture, or 1
 * if no rgb_scale value has been specified.
 */
INLINE int EggTexture::
get_rgb_scale() const {
  return _rgb_scale;
}

/**
 * Sets an additional factor that will scale the alpha component after the
 * texture has been applied.  This is used only when a combine mode is in
 * effect.
 *
 * The only legal values are 1, 2, or 4.
 */
INLINE void EggTexture::
set_alpha_scale(int alpha_scale) {
  _alpha_scale = alpha_scale;
  _flags |= F_has_alpha_scale;
}

/**
 * Removes the alpha_scale from the texture and restores it to the default
 * value of 1.
 */
INLINE void EggTexture::
clear_alpha_scale() {
  _alpha_scale = 1;
  _flags &= ~F_has_alpha_scale;
}

/**
 * Returns true if an alpha_scale has been specified for the texture, false
 * otherwise.
 */
INLINE bool EggTexture::
has_alpha_scale() const {
  return (_flags & F_has_alpha_scale) != 0;
}

/**
 * Returns the alpha_scale value that has been specified for the texture, or 1
 * if no alpha_scale value has been specified.
 */
INLINE int EggTexture::
get_alpha_scale() const {
  return _alpha_scale;
}

/**
 * Specifies a separate file that will be loaded in with the 1- or 3-component
 * texture and applied as the alpha channel.  This is useful when loading
 * textures from file formats that do not support alpha, for instance jpg.
 */
INLINE void EggTexture::
set_alpha_filename(const Filename &alpha_filename) {
  _alpha_filename = alpha_filename;
  _alpha_fullpath = alpha_filename;
  _flags |= F_has_alpha_filename;
}

/**
 *
 */
INLINE void EggTexture::
clear_alpha_filename() {
  _alpha_filename = Filename();
  _alpha_fullpath = Filename();
  _flags &= ~F_has_alpha_filename;
}

/**
 * Returns true if a separate file for the alpha component has been applied,
 * false otherwise.  See set_alpha_filename().
 */
INLINE bool EggTexture::
has_alpha_filename() const {
  return (_flags & F_has_alpha_filename) != 0;
}

/**
 * Returns the separate file assigned for the alpha channel.  It is an error
 * to call this unless has_alpha_filename() returns true.  See
 * set_alpha_filename().
 */
INLINE const Filename &EggTexture::
get_alpha_filename() const {
  nassertr(has_alpha_filename(), _alpha_filename);
  return _alpha_filename;
}

/**
 * Returns the full pathname to the alpha file, if it is known; otherwise,
 * returns the same thing as get_alpha_filename().
 *
 * This function simply returns whatever was set by the last call to
 * set_alpha_fullpath().  This string is not written to the egg file; its main
 * purpose is to record the full path to the alpha filename if it is known,
 * for egg structures that are generated in-memory and then immediately
 * converted to a scene graph.
 */
INLINE const Filename &EggTexture::
get_alpha_fullpath() const {
  return _alpha_fullpath;
}

/**
 * Records the full pathname to the file, for the benefit of
 * get_alpha_fullpath().
 */
INLINE void EggTexture::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  _alpha_fullpath = alpha_fullpath;
}

/**
 * If a separate alpha-file is specified, this indicates which channel number
 * should be extracted from this file to derive the alpha channel for the
 * final image.  The default is 0, which means the grayscale combination of r,
 * g, b.  Otherwise, this should be the 1-based channel number, for instance
 * 1, 2, or 3 for r, g, or b, respectively, or 4 for the alpha channel of a
 * four-component image.
 */
INLINE void EggTexture::
set_alpha_file_channel(int alpha_file_channel) {
  _alpha_file_channel = alpha_file_channel;
  _flags |= F_has_alpha_file_channel;
}

/**
 * Removes the specification of a particular channel to use from the alpha-
 * file image.
 */
INLINE void EggTexture::
clear_alpha_file_channel() {
  _alpha_file_channel = 0;
  _flags &= ~F_has_alpha_file_channel;
}

/**
 * Returns true if a particular channel has been specified for the alpha-file
 * image, false otherwise.
 */
INLINE bool EggTexture::
has_alpha_file_channel() const {
  return (_flags & F_has_alpha_file_channel) != 0;
}

/**
 * Returns the particular channel that has been specified for the alpha-file
 * image, or 0 if no channel has been specified.  See
 * set_alpha_file_channel().
 */
INLINE int EggTexture::
get_alpha_file_channel() const {
  return _alpha_file_channel;
}

/**
 * Sets the multiview flag.
 *
 * If multiview is true, the filename should contain a hash mark ('#'), which
 * will be filled in with the view number; and a multiview texture will be
 * defined with a series of images, one for each view.
 *
 * A multiview texture is most often used for stereo textures, but other uses
 * are also possible, such as for texture animation.
 */
INLINE void EggTexture::
set_multiview(bool multiview) {
  _multiview = multiview;
}

/**
 * Returns the current setting of the multiview flag.  See set_multiview().
 */
INLINE bool EggTexture::
get_multiview() const {
  return _multiview;
}

/**
 * When loading a 3-D multiview texture, this parameter is necessary to
 * specify how many views will be expected.  The z size is determined
 * implicitly from the number of images loaded.
 */
INLINE void EggTexture::
set_num_views(int num_views) {
  _num_views = num_views;
  _flags |= F_has_num_views;
}

/**
 * Removes the specification of the number of views for a 3-D multiview
 * texture.
 */
INLINE void EggTexture::
clear_num_views() {
  _num_views = 0;
  _flags &= ~F_has_num_views;
}

/**
 * Returns true if the number of views has been specified for the 3-D
 * multiview texture, false otherwise.
 */
INLINE bool EggTexture::
has_num_views() const {
  return (_flags & F_has_num_views) != 0;
}

/**
 * Returns the specified number of views specified for the 3-D multiview
 * texture.  See set_num_views().
 */
INLINE int EggTexture::
get_num_views() const {
  return _num_views;
}

/**
 * Sets the read_mipmaps flag.
 *
 * If read_mipmaps is true, the filename should contain a hash mark ('#'),
 * which will be filled in with the mipmap level number; and the texture will
 * be defined with a series of images, one for each mipmap level.
 *
 * If the filename is of a type that already requires a hash mark, such as a
 * cube map or a 3-d texture, then the filename should now require two hash
 * marks, and the first one indicates the mipmap level number, while the
 * second indicates the face number or 3-d level number.
 */
INLINE void EggTexture::
set_read_mipmaps(bool read_mipmaps) {
  _read_mipmaps = read_mipmaps;
}

/**
 * Returns the current setting of the read_mipmaps flag.  See
 * set_read_mipmaps().
 */
INLINE bool EggTexture::
get_read_mipmaps() const {
  return _read_mipmaps;
}

/**
 * Sets the minimum mipmap level that may be sampled.
 */
INLINE void EggTexture::
set_min_lod(double min_lod) {
  _min_lod = min_lod;
  _flags |= F_has_min_lod;
}

/**
 * Removes the specification of a minimum mipmap level from the texture.
 */
INLINE void EggTexture::
clear_min_lod() {
  _min_lod = -1000;
  _flags &= ~F_has_min_lod;
}

/**
 * Returns true if a value for the minimum mipmap level has been specified for
 * this texture, false otherwise.
 */
INLINE bool EggTexture::
has_min_lod() const {
  return (_flags & F_has_min_lod) != 0;
}

/**
 * Returns the minimum mipmap level that has been specified for this texture.
 */
INLINE double EggTexture::
get_min_lod() const {
  return _min_lod;
}

/**
 * Sets the maximum mipmap level that may be sampled.
 */
INLINE void EggTexture::
set_max_lod(double max_lod) {
  _max_lod = max_lod;
  _flags |= F_has_max_lod;
}

/**
 * Removes the specification of a maximum mipmap level from the texture.
 */
INLINE void EggTexture::
clear_max_lod() {
  _max_lod = 1000;
  _flags &= ~F_has_max_lod;
}

/**
 * Returns true if a value for the maximum mipmap level has been specified for
 * this texture, false otherwise.
 */
INLINE bool EggTexture::
has_max_lod() const {
  return (_flags & F_has_max_lod) != 0;
}

/**
 * Returns the maximum mipmap level that has been specified for this texture.
 */
INLINE double EggTexture::
get_max_lod() const {
  return _max_lod;
}

/**
 * Sets the mipmap level bias that is added to the mipmap level to be sampled.
 */
INLINE void EggTexture::
set_lod_bias(double lod_bias) {
  _lod_bias = lod_bias;
  _flags |= F_has_lod_bias;
}

/**
 * Removes the specification of a maximum mipmap level from the texture.
 */
INLINE void EggTexture::
clear_lod_bias() {
  _lod_bias = 1000;
  _flags &= ~F_has_lod_bias;
}

/**
 * Returns true if a value for the maximum mipmap level has been specified for
 * this texture, false otherwise.
 */
INLINE bool EggTexture::
has_lod_bias() const {
  return (_flags & F_has_lod_bias) != 0;
}

/**
 * Returns the maximum mipmap level that has been specified for this texture.
 */
INLINE double EggTexture::
get_lod_bias() const {
  return _lod_bias;
}

/**
 * Returns an integer that represents the depth to which this texture is
 * layered on all other textures in the egg file.  In general, if texture A is
 * layered over texture B, then sort(A) > sort(B).  If texture A is never
 * layered over any other texture, then sort(A) == 0.  More than that is
 * difficult to guarantee.
 */
INLINE int EggTexture::
get_multitexture_sort() const {
  return _multitexture_sort;
}

/**
 *
 */
INLINE EggTexture::SourceAndOperand::
SourceAndOperand() :
  _source(CS_unspecified),
  _operand(CO_unspecified)
{
}

/**
 *
 */
INLINE EggTexture::Combiner::
Combiner() :
  _mode(CM_unspecified)
{
}

/**
 *
 */
INLINE UniqueEggTextures::
UniqueEggTextures(int eq) : _eq(eq) {
}

/**
 *
 */
INLINE bool UniqueEggTextures::
operator ()(const EggTexture *t1, const EggTexture *t2) const {
  return t1->sorts_less_than(*t2, _eq);
}
