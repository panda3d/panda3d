/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerState.I
 * @author rdb
 * @date 2014-12-09
 */

/**
 * Creates a new SamplerState initialized to the default values.
 */
INLINE SamplerState::
SamplerState() :
  _border_color(0, 0, 0, 1),
  _wrap_u(WM_repeat),
  _wrap_v(WM_repeat),
  _wrap_w(WM_repeat),
  _minfilter(FT_default),
  _magfilter(FT_default),
  _min_lod(-1000),
  _max_lod(1000),
  _lod_bias(0),
  _anisotropic_degree(0)
{
}

/**
 * Returns a reference to the global default immutable SamplerState object.
 */
INLINE const SamplerState &SamplerState::
get_default() {
  return _default;
}

/**
 * This setting determines what happens when the SamplerState is sampled with
 * a U value outside the range 0.0-1.0.  The default is WM_repeat, which
 * indicates that the SamplerState should repeat indefinitely.
 */
INLINE void SamplerState::
set_wrap_u(SamplerState::WrapMode wrap) {
  _wrap_u = wrap;
}

/**
 * This setting determines what happens when the SamplerState is sampled with
 * a V value outside the range 0.0-1.0.  The default is WM_repeat, which
 * indicates that the SamplerState should repeat indefinitely.
 */
INLINE void SamplerState::
set_wrap_v(SamplerState::WrapMode wrap) {
  _wrap_v = wrap;
}

/**
 * The W wrap direction is only used for 3-d SamplerStates.
 */
INLINE void SamplerState::
set_wrap_w(SamplerState::WrapMode wrap) {
  _wrap_w = wrap;
}

/**
 * Sets the filtering method that should be used when viewing the SamplerState
 * from a distance.
 */
INLINE void SamplerState::
set_minfilter(SamplerState::FilterType filter) {
  _minfilter = filter;
}

/**
 * Sets the filtering method that should be used when viewing the SamplerState
 * up close.
 */
INLINE void SamplerState::
set_magfilter(SamplerState::FilterType filter) {
  _magfilter = filter;
}

/**
 * Specifies the level of anisotropic filtering to apply to the SamplerState.
 * Set this 0 to indicate the default value, which is specified in the
 * SamplerState-anisotropic-degree config variable.
 *
 * To explicitly disable anisotropic filtering, set this value to 1.  To
 * explicitly enable anisotropic filtering, set it to a value higher than 1;
 * larger numbers indicate greater degrees of filtering.
 */
INLINE void SamplerState::
set_anisotropic_degree(int anisotropic_degree) {
  _anisotropic_degree = anisotropic_degree;
}

/**
 * Specifies the solid color of the SamplerState's border.  Some OpenGL
 * implementations use a border for tiling SamplerStates; in Panda, it is only
 * used for specifying the clamp color.
 */
INLINE void SamplerState::
set_border_color(const LColor &color) {
  _border_color = color;
}

/**
 * Sets the minimum level of detail that will be used when sampling this
 * texture.  This may be a negative value.
 */
INLINE void SamplerState::
set_min_lod(PN_stdfloat min_lod) {
  _min_lod = min_lod;
}

/**
 * Sets the maximum level of detail that will be used when sampling this
 * texture.  This may exceed the number of mipmap levels that the texture has.
 */
INLINE void SamplerState::
set_max_lod(PN_stdfloat max_lod) {
  _max_lod = max_lod;
}

/**
 * Sets the value that will be added to the level of detail when sampling the
 * texture.  This may be a negative value, although some graphics hardware may
 * not support the use of negative LOD values.
 */
INLINE void SamplerState::
set_lod_bias(PN_stdfloat lod_bias) {
  _lod_bias = lod_bias;
}

/**
 * Returns the wrap mode of the texture in the U direction.
 */
INLINE SamplerState::WrapMode SamplerState::
get_wrap_u() const {
  return _wrap_u;
}

/**
 * Returns the wrap mode of the texture in the V direction.
 */
INLINE SamplerState::WrapMode SamplerState::
get_wrap_v() const {
  return _wrap_v;
}

/**
 * Returns the wrap mode of the texture in the W direction.  This is the depth
 * direction of 3-d textures.
 */
INLINE SamplerState::WrapMode SamplerState::
get_wrap_w() const {
  return _wrap_w;
}

/**
 * Returns the filter mode of the texture for minification.  If this is one of
 * the mipmap constants, then the texture requires mipmaps.  This may return
 * FT_default; see also get_effective_minfilter().
 */
INLINE SamplerState::FilterType SamplerState::
get_minfilter() const {
  return _minfilter;
}

/**
 * Returns the filter mode of the texture for magnification.  The mipmap
 * constants are invalid here.  This may return FT_default; see also
 * get_effective_minfilter().
 */
INLINE SamplerState::FilterType SamplerState::
get_magfilter() const {
  return _magfilter;
}

/**
 * Returns the degree of anisotropic filtering that should be applied to the
 * texture.  This value may return 0, indicating the default value; see also
 * get_effective_anisotropic_degree.
 */
INLINE int SamplerState::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

/**
 * Returns the degree of anisotropic filtering that should be applied to the
 * texture.  This value will normally not return 0, unless there is an error
 * in the config file.
 */
INLINE int SamplerState::
get_effective_anisotropic_degree() const {
  if (_anisotropic_degree != 0) {
    return _anisotropic_degree;
  }
  return texture_anisotropic_degree;
}

/**
 * Returns the solid color of the texture's border.  Some OpenGL
 * implementations use a border for tiling textures; in Panda, it is only used
 * for specifying the clamp color.
 */
INLINE const LColor &SamplerState::
get_border_color() const {
  return _border_color;
}

/**
 * Returns the minimum level of detail that will be observed when sampling
 * this texture.
 */
INLINE PN_stdfloat SamplerState::
get_min_lod() const {
  return _min_lod;
}

/**
 * Returns the maximum level of detail that will be observed when sampling
 * this texture.
 */
INLINE PN_stdfloat SamplerState::
get_max_lod() const {
  return _max_lod;
}

/**
 * Returns the bias that will be added to the texture level of detail when
 * sampling this texture.
 */
INLINE PN_stdfloat SamplerState::
get_lod_bias() const {
  return _lod_bias;
}

/**
 * Returns true if the minfilter settings on this sampler indicate the use of
 * mipmapping, false otherwise.
 */
INLINE bool SamplerState::
uses_mipmaps() const {
  return is_mipmap(get_effective_minfilter());
}

/**
 * Returns true if the indicated filter type requires the use of mipmaps, or
 * false if it does not.
 */
INLINE bool SamplerState::
is_mipmap(FilterType filter_type) {
  switch (filter_type) {
  case SamplerState::FT_nearest_mipmap_nearest:
  case SamplerState::FT_linear_mipmap_nearest:
  case SamplerState::FT_nearest_mipmap_linear:
  case SamplerState::FT_linear_mipmap_linear:
    return true;

  default:
    return false;
  }
}

/**
 *
 */
INLINE bool SamplerState::
operator == (const SamplerState &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool SamplerState::
operator != (const SamplerState &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE bool SamplerState::
operator < (const SamplerState &other) const {
  return compare_to(other) < 0;
}
