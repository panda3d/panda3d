/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderOptions.I
 * @author drose
 * @date 2005-10-05
 */

/**
 *
 */
constexpr LoaderOptions::
LoaderOptions(int flags, int texture_flags) :
  _flags(flags),
  _texture_flags(texture_flags),
  _texture_num_views(0),
  _auto_texture_scale(ATS_unspecified)
{
}

/**
 *
 */
INLINE void LoaderOptions::
set_flags(int flags) {
  _flags = flags;
}

/**
 *
 */
INLINE int LoaderOptions::
get_flags() const {
  return _flags;
}

/**
 *
 */
INLINE void LoaderOptions::
set_texture_flags(int texture_flags) {
  _texture_flags = texture_flags;
}

/**
 *
 */
INLINE int LoaderOptions::
get_texture_flags() const {
  return _texture_flags;
}

/**
 * Specifies the expected number of views to load for the texture.  This is
 * ignored unless TF_multiview is included in texture_flags.  This must be
 * specified when loading a 3-d multiview texture or 2-d texture array, in
 * which case it is used to differentiate z levels from separate views; it
 * may be zero in the case of 2-d textures or cube maps, in which case the
 * number of views can be inferred from the number of images found on disk.
 */
INLINE void LoaderOptions::
set_texture_num_views(int texture_num_views) {
  _texture_num_views = texture_num_views;
}

/**
 * See set_texture_num_views().
 */
INLINE int LoaderOptions::
get_texture_num_views() const {
  return _texture_num_views;
}

/**
 * Set this flag to ATS_none, ATS_up, ATS_down, or ATS_pad to control how a
 * texture is scaled from disk when it is subsequently loaded.  Set it to
 * ATS_unspecified to restore the default behavior.
 */
INLINE void LoaderOptions::
set_auto_texture_scale(AutoTextureScale scale) {
  _auto_texture_scale = scale;
}

/**
 * See set_auto_texture_scale().
 */
INLINE AutoTextureScale LoaderOptions::
get_auto_texture_scale() const {
  return _auto_texture_scale;
}
