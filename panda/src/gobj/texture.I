/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texture.I
 * @author drose
 * @date 1999-02-05
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

/**
 * Returns a new copy of the same Texture.  This copy, if applied to geometry,
 * will be copied into texture as a separate texture from the original, so it
 * will be duplicated in texture memory (and may be independently modified if
 * desired).
 *
 * If the Texture is a VideoTexture, the resulting duplicate may be animated
 * independently of the original.
 */
INLINE PT(Texture) Texture::
make_copy() const {
  PT(Texture) tex = make_copy_impl();
  CDWriter cdata_tex(tex->_cycler, true);
  cdata_tex->inc_properties_modified();
  cdata_tex->inc_image_modified();
  cdata_tex->inc_simple_image_modified();
  return tex;
}

/**
 * Reinitializes the texture to its default, empty state (except for the
 * name).
 */
INLINE void Texture::
clear() {
  CDWriter cdata(_cycler, true);
  do_clear(cdata);
}

/**
 * Sets the texture to the indicated type and dimensions, presumably in
 * preparation for calling read() or load(), or set_ram_image() or
 * modify_ram_image(), or use set_clear_color to let the texture be cleared to
 * a solid color.
 */
INLINE void Texture::
setup_texture(Texture::TextureType texture_type, int x_size, int y_size,
              int z_size, Texture::ComponentType component_type,
              Texture::Format format) {
  CDWriter cdata(_cycler, true);
  do_setup_texture(cdata, texture_type, x_size, y_size, z_size,
                   component_type, format);
}

/**
 * Sets the texture as an empty 1-d texture with no dimensions.  Follow up
 * with read() or load() to fill the texture properties and image data, or use
 * set_clear_color to let the texture be cleared to a solid color.
 */
INLINE void Texture::
setup_1d_texture() {
  setup_1d_texture(0, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as an empty 1-d texture with the specified dimensions and
 * properties.  Follow up with set_ram_image() or modify_ram_image() to fill
 * the image data, or use set_clear_color to let the texture be cleared to a
 * solid color.
 */
INLINE void Texture::
setup_1d_texture(int x_size, ComponentType component_type, Format format) {
  setup_texture(TT_1d_texture, x_size, 1, 1, component_type, format);
}

/**
 * Sets the texture as an empty 2-d texture with no dimensions.  Follow up
 * with read() or load() to fill the texture properties and image data, or use
 * set_clear_color to let the texture be cleared to a solid color.
 */
INLINE void Texture::
setup_2d_texture() {
  setup_2d_texture(0, 1, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as an empty 2-d texture with the specified dimensions and
 * properties.  Follow up with set_ram_image() or modify_ram_image() to fill
 * the image data, or use set_clear_color to let the texture be cleared to a
 * solid color.
 */
INLINE void Texture::
setup_2d_texture(int x_size, int y_size, ComponentType component_type,
                 Format format) {
  setup_texture(TT_2d_texture, x_size, y_size, 1, component_type, format);
}

/**
 * Sets the texture as an empty 3-d texture with no dimensions (though if you
 * know the depth ahead of time, it saves a bit of reallocation later). Follow
 * up with read() or load() to fill the texture properties and image data, or
 * use set_clear_color to let the texture be cleared to a solid color.
 */
INLINE void Texture::
setup_3d_texture(int z_size) {
  setup_3d_texture(0, 1, z_size, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as an empty 3-d texture with the specified dimensions and
 * properties.  Follow up with set_ram_image() or modify_ram_image() to fill
 * the image data.
 */
INLINE void Texture::
setup_3d_texture(int x_size, int y_size, int z_size,
                 ComponentType component_type, Format format) {
  setup_texture(TT_3d_texture, x_size, y_size, z_size, component_type, format);
}

/**
 * Sets the texture as an empty 2-d texture array with no dimensions (though
 * if you know the depth ahead of time, it saves a bit of reallocation later).
 * Follow up with read() or load() to fill the texture properties and image
 * data, or use set_clear_color to let the texture be cleared to a solid
 * color.
 */
INLINE void Texture::
setup_2d_texture_array(int z_size) {
  setup_2d_texture_array(0, 1, z_size, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as an empty 2-d texture array with the specified
 * dimensions and properties.  Follow up with set_ram_image() or
 * modify_ram_image() to fill the image data, or use set_clear_color to let
 * the texture be cleared to a solid color.
 */
INLINE void Texture::
setup_2d_texture_array(int x_size, int y_size, int z_size,
                       ComponentType component_type, Format format) {
  setup_texture(TT_2d_texture_array, x_size, y_size, z_size, component_type, format);
}

/**
 * Sets the texture as an empty cube map texture with no dimensions.  Follow
 * up with read() or load() to fill the texture properties and image data, or
 * use set_clear_color to let the texture be cleared to a solid color.
 */
INLINE void Texture::
setup_cube_map() {
  setup_cube_map(0, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as an empty cube map texture with the specified dimensions
 * and properties.  Follow up with set_ram_image() or modify_ram_image() to
 * fill the image data, or use set_clear_color to let the texture be cleared
 * to a solid color.
 *
 * Note that a cube map should always consist of six square images, so x_size
 * and y_size will be the same, and z_size is always 6.
 */
INLINE void Texture::
setup_cube_map(int size, ComponentType component_type, Format format) {
  setup_texture(TT_cube_map, size, size, 6, component_type, format);
}

/**
 * Sets the texture as cube map array with N cube maps.  Note that this number
 * is not the same as the z_size.  Follow up with read() or load() to fill the
 * texture properties and image data, or use set_clear_color to let the
 * texture be cleared to a solid color.
 *
 * @since 1.10.0
 */
INLINE void Texture::
setup_cube_map_array(int num_cube_maps) {
  setup_cube_map_array(0, num_cube_maps, T_unsigned_byte, F_rgb);
}

/**
 * Sets the texture as cube map array with N cube maps with the specified
 * dimensions and format.  Follow up with set_ram_image() or
 * modify_ram_image() to fill the image data, or use set_clear_color to let
 * the texture be cleared to a solid color.
 *
 * The num_cube_maps given here is multiplied by six to become the z_size of
 * the image.
 *
 * @since 1.10.0
 */
INLINE void Texture::
setup_cube_map_array(int size, int num_cube_maps, ComponentType component_type, Format format) {
  setup_texture(TT_cube_map_array, size, size, num_cube_maps * 6, component_type, format);
}

/**
 * Sets the texture as an empty buffer texture with the specified size and
 * properties.  Follow up with set_ram_image() or modify_ram_image() to fill
 * the image data, or use set_clear_color to let the texture be cleared to a
 * solid color.
 *
 * Note that a buffer texture's format needs to match the component type.
 */
INLINE void Texture::
setup_buffer_texture(int size, ComponentType component_type, Format format,
                     GeomEnums::UsageHint usage) {
  setup_texture(TT_buffer_texture, size, 1, 1, component_type, format);
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage;
}

/**
 * Clears the texture data without changing its format or resolution.  The
 * texture is cleared on both the graphics hardware and from RAM, unlike
 * clear_ram_image, which only removes the data from RAM.
 *
 * If a clear color has been specified using set_clear_color, the texture will
 * be cleared using a solid color.
 *
 * The texture data will be cleared the first time in which the texture is
 * used after this method is called.
 */
INLINE void Texture::
clear_image() {
  CDWriter cdata(_cycler, true);
  do_clear_ram_image(cdata);
  do_clear_simple_ram_image(cdata);
  cdata->inc_image_modified();
  cdata->inc_simple_image_modified();
}

/**
 * Returns true if a color was previously set using set_clear_color.
 */
INLINE bool Texture::
has_clear_color() const {
  CDReader cdata(_cycler);
  return cdata->_has_clear_color;
}

/**
 * Returns the color that was previously set using set_clear_color.
 */
INLINE LColor Texture::
get_clear_color() const {
  CDReader cdata(_cycler);
  return cdata->_clear_color;
}

/**
 * Sets the color that will be used to fill the texture image in absence of
 * any image data.  It is used when any of the setup_texture functions or
 * clear_image is called and image data is not provided using read() or
 * modify_ram_image().
 *
 * This does not affect a texture that has already been cleared; call
 * clear_image to clear it again.
 */
INLINE void Texture::
set_clear_color(const LColor &color) {
  CDWriter cdata(_cycler, true);
  cdata->_clear_color = color;
  cdata->_has_clear_color = true;
}

/**
 * The opposite of set_clear_color.  If the image is cleared after setting
 * this, its contents may be undefined (or may in fact not be cleared at all).
 */
INLINE void Texture::
clear_clear_color() {
  CDWriter cdata(_cycler, true);
  cdata->_has_clear_color = true;
}

/**
 * Returns the raw image data for a single pixel if it were set to the clear
 * color.
 */
INLINE vector_uchar Texture::
get_clear_data() const {
  CDReader cdata(_cycler);
  vector_uchar data(16);
  data.resize(do_get_clear_data(cdata, &data[0]));
  return data;
}

/**
 * Writes the texture to the named filename.
 */
INLINE bool Texture::
write(const Filename &fullpath) {
  CDWriter cdata(_cycler);

  // do_write() is non-const, because it might have to reload the ram image.
  return do_write(cdata, fullpath, 0, 0, false, false);
}

/**
 * Writes a single page or mipmap level to a single file, or automatically
 * writes a series of pages and/or mipmap levels to a numbered series of
 * files.
 *
 * If the filename ends in the extension .txo, this implicitly writes a Panda
 * texture object (.txo) instead of an image file.  In this case, the
 * remaining parameters are ignored, and only one file is written, which will
 * contain all of the pages and resident mipmap levels in the texture.
 *
 * If write_pages is false, then z indicates the page number to write.  3-D
 * textures have one page number for each level of depth; cube maps have six
 * pages number 0 through 5.  Other kinds of textures have only one page,
 * numbered 0.  If there are multiple views, the range of z is increased; the
 * total range is [0, get_num_pages()).
 *
 * If write_pages is true, then all pages of the texture will be written.  In
 * this case z is ignored, and the filename should contain a sequence of hash
 * marks ("#") which will be filled in with the page index number.
 *
 * If write_mipmaps is false, then n indicates the mipmap level number to
 * write.  Normally, this is 0, for the base texture image.  Normally, the
 * mipmap levels of a texture are not available in RAM (they are generated
 * automatically by the graphics card). However, if you have the mipmap levels
 * available, for instance because you called generate_ram_mipmap_images() to
 * generate them internally, or you called
 * GraphicsEngine::extract_texture_data() to retrieve them from the graphics
 * card, then you may write out each mipmap level with this parameter.
 *
 * If write_mipmaps is true, then all mipmap levels of the texture will be
 * written.  In this case n is ignored, and the filename should contain a
 * sequence of hash marks ("#") which will be filled in with the mipmap level
 * number.
 *
 * If both write_pages and write_mipmaps is true, then all pages and all
 * mipmap levels will be written.  In this case, the filename should contain
 * two different sequences of hash marks, separated by a character such as a
 * hyphen, underscore, or dot.  The first hash mark sequence will be filled in
 * with the mipmap level, while the second hash mark sequence will be the page
 * index.
 */
INLINE bool Texture::
write(const Filename &fullpath, int z, int n,
      bool write_pages, bool write_mipmaps) {
  CDWriter cdata(_cycler, false);
  return do_write(cdata, fullpath, z, n, write_pages, write_mipmaps);
}

/**
 * Replaces the texture with the indicated image.
 */
INLINE bool Texture::
load(const PNMImage &pnmimage, const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  do_clear(cdata);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  if (do_load_one(cdata, pnmimage, get_name(), 0, 0, options)) {
    bool generate_mipmaps = ((options.get_texture_flags() & LoaderOptions::TF_generate_mipmaps) != 0);
    consider_auto_process_ram_image(generate_mipmaps || uses_mipmaps(), true);
    return true;
  }
  return false;
}

/**
 * Stores the indicated image in the given page and mipmap level.  See read().
 */
INLINE bool Texture::
load(const PNMImage &pnmimage, int z, int n, const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  if (do_load_one(cdata, pnmimage, get_name(), z, n, options)) {
    return true;
  }
  return false;
}

/**
 * Replaces the texture with the indicated image.
 */
INLINE bool Texture::
load(const PfmFile &pfm, const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  do_clear(cdata);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  if (do_load_one(cdata, pfm, get_name(), 0, 0, options)) {
    bool generate_mipmaps = ((options.get_texture_flags() & LoaderOptions::TF_generate_mipmaps) != 0);
    consider_auto_process_ram_image(generate_mipmaps || uses_mipmaps(), true);
    return true;
  }
  return false;
}

/**
 * Stores the indicated image in the given page and mipmap level.  See read().
 */
INLINE bool Texture::
load(const PfmFile &pfm, int z, int n, const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  if (do_load_one(cdata, pfm, get_name(), z, n, options)) {
    return true;
  }
  return false;
}

/**
 * Stores the indicated image in a region of the texture.  The texture
 * properties remain unchanged.  This can be more efficient than updating an
 * entire texture, but has a few restrictions: for one, you must ensure that
 * the texture is still in RAM (eg.  using set_keep_ram_image) and it may not
 * be compressed.
 */
INLINE bool Texture::
load_sub_image(const PNMImage &image, int x, int y, int z, int n) {
  CDWriter cdata(_cycler, true);
  return do_load_sub_image(cdata, image, x, y, z, n);
}

/**
 * Saves the texture to the indicated PNMImage, but does not write it to disk.
 */
INLINE bool Texture::
store(PNMImage &pnmimage) const {
  CDWriter cdata(((Texture *)this)->_cycler, false);
  return ((Texture *)this)->do_store_one(cdata, pnmimage, 0, 0);
}

/**
 * Saves the indicated page and mipmap level of the texture to the PNMImage.
 */
INLINE bool Texture::
store(PNMImage &pnmimage, int z, int n) const {
  CDWriter cdata(((Texture *)this)->_cycler, false);
  return ((Texture *)this)->do_store_one(cdata, pnmimage, z, n);
}

/**
 * Saves the texture to the indicated PfmFile, but does not write it to disk.
 */
INLINE bool Texture::
store(PfmFile &pfm) const {
  CDWriter cdata(((Texture *)this)->_cycler, false);
  return ((Texture *)this)->do_store_one(cdata, pfm, 0, 0);
}

/**
 * Saves the indicated page and mipmap level of the texture to the PfmFile.
 */
INLINE bool Texture::
store(PfmFile &pfm, int z, int n) const {
  CDWriter cdata(((Texture *)this)->_cycler, false);
  return ((Texture *)this)->do_store_one(cdata, pfm, z, n);
}

/**
 * Re-reads the Texture from its disk file.  Useful when you know the image on
 * disk has recently changed, and you want to update the Texture image.
 *
 * Returns true on success, false on failure (in which case, the Texture may
 * or may not still be valid).
 */
bool Texture::
reload() {
  CDWriter cdata(_cycler, true);
  return do_reload(cdata);
}

/**
 * Returns true if the filename has been set and is available.  See
 * set_filename().
 */
INLINE bool Texture::
has_filename() const {
  CDReader cdata(_cycler);
  return !cdata->_filename.empty();
}

/**
 * Returns the filename that has been set.  This is the name of the file as it
 * was requested.  Also see get_fullpath().
 */
INLINE const Filename &Texture::
get_filename() const {
  CDReader cdata(_cycler);
  return cdata->_filename;
}

/**
 * Returns true if the alpha_filename has been set and is available.  See
 * set_alpha_filename().
 */
INLINE bool Texture::
has_alpha_filename() const {
  CDReader cdata(_cycler);
  return !cdata->_alpha_filename.empty();
}

/**
 * Returns the alpha_filename that has been set.  If this is set, it
 * represents the name of the alpha component, which is stored in a separate
 * file.  See also get_filename(), and get_alpha_fullpath().
 */
INLINE const Filename &Texture::
get_alpha_filename() const {
  CDReader cdata(_cycler);
  return cdata->_alpha_filename;
}

/**
 * Returns true if the fullpath has been set and is available.  See
 * set_fullpath().
 */
INLINE bool Texture::
has_fullpath() const {
  CDReader cdata(_cycler);
  return !cdata->_fullpath.empty();
}

/**
 * Returns the fullpath that has been set.  This is the full path to the file
 * as it was found along the texture search path.
 */
INLINE const Filename &Texture::
get_fullpath() const {
  CDReader cdata(_cycler);
  return cdata->_fullpath;
}

/**
 * Returns true if the alpha_fullpath has been set and is available.  See
 * set_alpha_fullpath().
 */
INLINE bool Texture::
has_alpha_fullpath() const {
  CDReader cdata(_cycler);
  return !cdata->_alpha_fullpath.empty();
}

/**
 *
 * Returns the alpha_fullpath that has been set.  This is the full path to the
 * alpha part of the image file as it was found along the texture search path.
 */
INLINE const Filename &Texture::
get_alpha_fullpath() const {
  CDReader cdata(_cycler);
  return cdata->_alpha_fullpath;
}


/**
 * Returns the width of the texture image in texels.
 */
INLINE int Texture::
get_x_size() const {
  CDReader cdata(_cycler);
  return cdata->_x_size;
}

/**
 * Returns the height of the texture image in texels.  For a 1-d texture, this
 * will be 1.
 */
INLINE int Texture::
get_y_size() const {
  CDReader cdata(_cycler);
  return cdata->_y_size;
}

/**
 * Returns the depth of the texture image in texels.  For a 1-d texture or 2-d
 * texture, this will be 1. For a cube map texture, this will be 6.
 */
INLINE int Texture::
get_z_size() const {
  CDReader cdata(_cycler);
  return cdata->_z_size;
}

/**
 * Returns the number of "views" in the texture.  A view is a completely
 * separate image stored within the Texture object.  Most textures have only
 * one view, but a stereo texture, for instance, may have two views, a left
 * and a right image.  Other uses for multiple views are not yet defined.
 *
 * If this value is greater than one, the additional views are accessed as
 * additional pages beyond get_z_size().
 */
INLINE int Texture::
get_num_views() const {
  CDReader cdata(_cycler);
  return cdata->_num_views;
}

/**
 * Returns the total number of pages in the texture.  Each "page" is a 2-d
 * texture image within the larger image--a face of a cube map, or a level of
 * a 3-d texture.  Normally, get_num_pages() is the same as get_z_size().
 * However, in a multiview texture, this returns get_z_size() *
 * get_num_views().
 */
INLINE int Texture::
get_num_pages() const {
  return get_z_size() * get_num_views();
}

/**
 * Returns size of the pad region.  See set_pad_size.
 */
INLINE int Texture::
get_pad_x_size() const {
  CDReader cdata(_cycler);
  return cdata->_pad_x_size;
}

/**
 * Returns size of the pad region.  See set_pad_size.
 */
INLINE int Texture::
get_pad_y_size() const {
  CDReader cdata(_cycler);
  return cdata->_pad_y_size;
}

/**
 * Returns size of the pad region.  See set_pad_size.
 */
INLINE int Texture::
get_pad_z_size() const {
  CDReader cdata(_cycler);
  return cdata->_pad_z_size;
}

/**
 * Returns a scale pair that is suitable for applying to geometry via
 * NodePath::set_tex_scale(), which will convert texture coordinates on the
 * geometry from the range 0..1 into the appropriate range to render the video
 * part of the texture.
 *
 * This is necessary only if a padding size has been set via set_pad_size()
 * (or implicitly via something like "textures-power-2 pad" in the config.prc
 * file).  In this case, this is a convenient way to generate UV's that
 * reflect the built-in padding size.
 */
INLINE LVecBase2 Texture::
get_tex_scale() const {
  CDReader cdata(_cycler);
  if (cdata->_pad_x_size == 0 || cdata->_pad_y_size == 0 ||
      cdata->_x_size == 0 || cdata->_y_size == 0) {
    LVecBase2(1.0f, 1.0f);
  }
  return LVecBase2((PN_stdfloat)(cdata->_x_size - cdata->_pad_x_size) / (PN_stdfloat)cdata->_x_size,
                   (PN_stdfloat)(cdata->_y_size - cdata->_pad_y_size) / (PN_stdfloat)cdata->_y_size);
}

/**
 * Sets the size of the pad region.
 *
 * Sometimes, when a video card demands power-of-two textures, it is necessary
 * to create a big texture and then only use a portion of it.  The pad region
 * indicates which portion of the texture is not really in use.  All
 * operations use the texture as a whole, including the pad region, unless
 * they explicitly state that they use only the non-pad region.
 *
 * Changing the texture's size clears the pad region.
 */
INLINE void Texture::
set_pad_size(int x, int y, int z) {
  CDWriter cdata(_cycler, true);
  do_set_pad_size(cdata, x, y, z);
}

/**
 * Returns the X size of the original disk image that this Texture was loaded
 * from (if it came from a disk file), before any automatic rescaling by
 * Panda.
 */
INLINE int Texture::
get_orig_file_x_size() const {
  CDReader cdata(_cycler);
  return cdata->_orig_file_x_size;
}

/**
 * Returns the Y size of the original disk image that this Texture was loaded
 * from (if it came from a disk file), before any automatic rescaling by
 * Panda.
 */
INLINE int Texture::
get_orig_file_y_size() const {
  CDReader cdata(_cycler);
  return cdata->_orig_file_y_size;
}

/**
 * Returns the Z size of the original disk image that this Texture was loaded
 * from (if it came from a disk file), before any automatic rescaling by
 * Panda.
 */
INLINE int Texture::
get_orig_file_z_size() const {
  // At the moment, we perform no automatic adjustment of Z size.  So we can
  // just return the current value, since it would be the same thing.
  CDReader cdata(_cycler);
  return cdata->_z_size;
}

/**
 * Returns the number of color components for each texel of the texture image.
 * This is 3 for an rgb texture or 4 for an rgba texture; it may also be 1 or
 * 2 for a grayscale texture.
 */
INLINE int Texture::
get_num_components() const {
  CDReader cdata(_cycler);
  return cdata->_num_components;
}

/**
 * Returns the number of bytes stored for each color component of a texel.
 * Typically this is 1, but it may be 2 for 16-bit texels.
 */
INLINE int Texture::
get_component_width() const {
  CDReader cdata(_cycler);
  return cdata->_component_width;
}

/**
 * Returns the overall interpretation of the texture.
 */
INLINE Texture::TextureType Texture::
get_texture_type() const {
  CDReader cdata(_cycler);
  return cdata->_texture_type;
}

/**
 * Returns the format of the texture, which represents both the semantic
 * meaning of the texels and, to some extent, their storage information.
 */
INLINE Texture::Format Texture::
get_format() const {
  CDReader cdata(_cycler);
  return cdata->_format;
}

/**
 * Returns the numeric interpretation of each component of the texture.
 */
INLINE Texture::ComponentType Texture::
get_component_type() const {
  CDReader cdata(_cycler);
  return cdata->_component_type;
}

/**
 * Returns the usage hint specified for buffer textures, or UH_unspecified for
 * all other texture types.
 */
INLINE GeomEnums::UsageHint Texture::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

/**
 * This setting determines what happens when the texture is sampled with a U
 * value outside the range 0.0-1.0.  The default is WM_repeat, which indicates
 * that the texture should repeat indefinitely.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_wrap_u(SamplerState::WrapMode wrap) {
  CDWriter cdata(_cycler, true);
  do_set_wrap_u(cdata, wrap);
}

/**
 * This setting determines what happens when the texture is sampled with a V
 * value outside the range 0.0-1.0.  The default is WM_repeat, which indicates
 * that the texture should repeat indefinitely.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_wrap_v(SamplerState::WrapMode wrap) {
  CDWriter cdata(_cycler, true);
  do_set_wrap_v(cdata, wrap);
}

/**
 * The W wrap direction is only used for 3-d textures.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_wrap_w(SamplerState::WrapMode wrap) {
  CDWriter cdata(_cycler, true);
  do_set_wrap_w(cdata, wrap);
}

/**
 * Sets the filtering method that should be used when viewing the texture from
 * a distance.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_minfilter(SamplerState::FilterType filter) {
  CDWriter cdata(_cycler, true);
  do_set_minfilter(cdata, filter);
}

/**
 * Sets the filtering method that should be used when viewing the texture up
 * close.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_magfilter(SamplerState::FilterType filter) {
  CDWriter cdata(_cycler, true);
  do_set_magfilter(cdata, filter);
}

/**
 * Specifies the level of anisotropic filtering to apply to the texture.  Set
 * this 0 to indicate the default value, which is specified in the texture-
 * anisotropic-degree config variable.
 *
 * To explicitly disable anisotropic filtering, set this value to 1.  To
 * explicitly enable anisotropic filtering, set it to a value higher than 1;
 * larger numbers indicate greater degrees of filtering.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_anisotropic_degree(int anisotropic_degree) {
  CDWriter cdata(_cycler, true);
  do_set_anisotropic_degree(cdata, anisotropic_degree);
}

/**
 * Specifies the solid color of the texture's border.  Some OpenGL
 * implementations use a border for tiling textures; in Panda, it is only used
 * for specifying the clamp color.
 *
 * This sets the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE void Texture::
set_border_color(const LColor &color) {
  CDWriter cdata(_cycler, true);
  do_set_border_color(cdata, color);
}

/**
 * Requests that this particular Texture be compressed when it is loaded into
 * texture memory.
 *
 * This refers to the internal compression of the texture image within texture
 * memory; it is not related to jpeg or png compression, which are disk file
 * compression formats.  The actual disk file that generated this texture may
 * be stored in a compressed or uncompressed format supported by Panda; it
 * will be decompressed on load, and then recompressed by the graphics API if
 * this parameter is not CM_off.
 *
 * If the GSG does not support this texture compression mode, the texture will
 * silently be loaded uncompressed.
 */
INLINE void Texture::
set_compression(Texture::CompressionMode compression) {
  CDWriter cdata(_cycler, true);
  do_set_compression(cdata, compression);
}

/**
 * Sets a flag on the texture that indicates whether the texture is intended
 * to be used as a direct-render target, by binding a framebuffer to a texture
 * and rendering directly into the texture.
 *
 * This controls some low-level choices made about the texture object itself.
 * For instance, compressed textures are disallowed when this flag is set
 * true.
 *
 * Normally, a user should not need to set this flag directly; it is set
 * automatically by the low-level display code when a texture is bound to a
 * framebuffer.
 */
INLINE void Texture::
set_render_to_texture(bool render_to_texture) {
  CDWriter cdata(_cycler, false);
  cdata->_render_to_texture = render_to_texture;
}

/**
 * This returns the default sampler state for this texture, containing the
 * wrap and filter properties specified on the texture level; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE const SamplerState &Texture::
get_default_sampler() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler;
}

/**
 * This sets the default sampler state for this texture, containing the wrap
 * and filter properties specified on the texture level; it may still be
 * overridden by a sampler state specified at a higher level.  This
 * encompasses the settings for get_wrap_u, get_minfilter,
 * get_anisotropic_degree, etc.
 *
 * This makes a copy of the SamplerState object, so future modifications of
 * the same SamplerState will have no effect on this texture unless you call
 * set_default_sampler again.
 */
INLINE void Texture::
set_default_sampler(const SamplerState &sampler) {
  CDWriter cdata(_cycler, true);
  cdata->_default_sampler = sampler;
  cdata->inc_properties_modified();
}

/**
 * Returns the wrap mode of the texture in the U direction.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE SamplerState::WrapMode Texture::
get_wrap_u() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_wrap_u();
}

/**
 * Returns the wrap mode of the texture in the V direction.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE SamplerState::WrapMode Texture::
get_wrap_v() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_wrap_v();
}

/**
 * Returns the wrap mode of the texture in the W direction.  This is the depth
 * direction of 3-d textures.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE SamplerState::WrapMode Texture::
get_wrap_w() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_wrap_w();
}

/**
 * Returns the filter mode of the texture for minification.  If this is one of
 * the mipmap constants, then the texture requires mipmaps.  This may return
 * FT_default; see also get_effective_minfilter().
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE SamplerState::FilterType Texture::
get_minfilter() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_minfilter();
}

/**
 * Returns the filter mode of the texture for magnification.  The mipmap
 * constants are invalid here.  This may return FT_default; see also
 * get_effective_minfilter().
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE SamplerState::FilterType Texture::
get_magfilter() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_magfilter();
}

/**
 * Returns the filter mode of the texture for minification, with special
 * treatment for FT_default.  This will normally not return FT_default, unless
 * there is an error in the config file.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
SamplerState::FilterType Texture::
get_effective_minfilter() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_effective_minfilter();
}

/**
 * Returns the filter mode of the texture for magnification, with special
 * treatment for FT_default.  This will normally not return FT_default, unless
 * there is an error in the config file.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
SamplerState::FilterType Texture::
get_effective_magfilter() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_effective_magfilter();
}

/**
 * Returns the degree of anisotropic filtering that should be applied to the
 * texture.  This value may return 0, indicating the default value; see also
 * get_effective_anisotropic_degree.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE int Texture::
get_anisotropic_degree() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_anisotropic_degree();
}

/**
 * Returns the degree of anisotropic filtering that should be applied to the
 * texture.  This value will normally not return 0, unless there is an error
 * in the config file.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE int Texture::
get_effective_anisotropic_degree() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_effective_anisotropic_degree();
}

/**
 * Returns the solid color of the texture's border.  Some OpenGL
 * implementations use a border for tiling textures; in Panda, it is only used
 * for specifying the clamp color.
 *
 * This returns the default sampler state for this texture; it may still be
 * overridden by a sampler state specified at a higher level.
 */
INLINE LColor Texture::
get_border_color() const {
  CDReader cdata(_cycler);
  return cdata->_default_sampler.get_border_color();
}

/**
 * Returns the compression mode requested for this particular texture, or
 * CM_off if the texture is not to be compressed.
 *
 * If a value other than CM_off is returned, this is not a guarantee that the
 * texture is actually successfully compressed on the GSG.  It may be that the
 * GSG does not support the requested compression mode, in which case the
 * texture may actually be stored uncompressed in texture memory.
 */
INLINE Texture::CompressionMode Texture::
get_compression() const {
  CDReader cdata(_cycler);
  return cdata->_compression;
}

/**
 * Returns true if the texture indicates it wants to be compressed, either
 * with CM_on or higher, or CM_default and compressed-textures is true.
 *
 * If true returned, this is not a guarantee that the texture is actually
 * successfully compressed on the GSG.  It may be that the GSG does not
 * support the requested compression mode, in which case the texture may
 * actually be stored uncompressed in texture memory.
 */
INLINE bool Texture::
has_compression() const {
  CDReader cdata(_cycler);
  return do_has_compression(cdata);
}

/**
 * Returns a flag on the texture that indicates whether the texture is
 * intended to be used as a direct-render target, by binding a framebuffer to
 * a texture and rendering directly into the texture.
 *
 * Normally, a user should not need to set this flag directly; it is set
 * automatically by the low-level display code when a texture is bound to a
 * framebuffer.
 */
INLINE bool Texture::
get_render_to_texture() const {
  CDReader cdata(_cycler);
  return cdata->_render_to_texture;
}

/**
 * Returns true if the minfilter settings on this texture indicate the use of
 * mipmapping, false otherwise.
 */
INLINE bool Texture::
uses_mipmaps() const {
  return SamplerState::is_mipmap(get_effective_minfilter());
}

/**
 * Sets a hint to the renderer about the desired performance / quality
 * tradeoff for this particular texture.  This is most useful for the
 * tinydisplay software renderer; for normal, hardware-accelerated renderers,
 * this may have little or no effect.
 */
INLINE void Texture::
set_quality_level(Texture::QualityLevel quality_level) {
  CDWriter cdata(_cycler, true);
  do_set_quality_level(cdata, quality_level);
}

/**
 * Returns the current quality_level hint.  See set_quality_level().  This
 * value may return QL_default; see get_effective_quality_level().
 */
INLINE Texture::QualityLevel Texture::
get_quality_level() const {
  CDReader cdata(_cycler);
  return cdata->_quality_level;
}

/**
 * Returns the current quality_level hint, or the global default quality_level
 * if this texture doesn't specify a quality level.  This value will not
 * normally return QL_default (unless there is an error in the config file)
 */
INLINE Texture::QualityLevel Texture::
get_effective_quality_level() const {
  CDReader cdata(_cycler);
  if (cdata->_quality_level == QL_default) {
    return texture_quality_level;
  }
  return cdata->_quality_level;
}

/**
 * Returns the number of mipmap levels that should be defined for this
 * texture, given the texture's size.
 *
 * Note that this returns a number appropriate for mipmapping, even if the
 * texture does not currently have mipmapping enabled.
 */
INLINE int Texture::
get_expected_num_mipmap_levels() const {
  CDReader cdata(_cycler);
  return do_get_expected_num_mipmap_levels(cdata);
}

/**
 * Returns the x_size that the nth mipmap level should have, based on the
 * texture's size.
 */
INLINE int Texture::
get_expected_mipmap_x_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_mipmap_x_size(cdata, n);
}

/**
 * Returns the y_size that the nth mipmap level should have, based on the
 * texture's size.
 */
INLINE int Texture::
get_expected_mipmap_y_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_mipmap_y_size(cdata, n);
}

/**
 * Returns the z_size that the nth mipmap level should have, based on the
 * texture's size.
 */
INLINE int Texture::
get_expected_mipmap_z_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_mipmap_z_size(cdata, n);
}

/**
 * Returns the total number of pages that the nth mipmap level should have,
 * based on the texture's size.  This is usually the same as
 * get_expected_mipmap_z_size(), except for a multiview texture, in which case
 * it is get_expected_mipmap_z_size() * get_num_views().
 */
INLINE int Texture::
get_expected_mipmap_num_pages(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_mipmap_num_pages(cdata, n);
}

/**
 * Returns true if the Texture has its image contents available in main RAM,
 * false if it exists only in texture memory or in the prepared GSG context.
 *
 * Note that this has nothing to do with whether get_ram_image() will fail or
 * not.  Even if has_ram_image() returns false, get_ram_image() may still
 * return a valid RAM image, because get_ram_image() will automatically load
 * the texture from disk if necessary.  The only thing has_ram_image() tells
 * you is whether the texture is available right now without hitting the disk
 * first.
 *
 * Note also that if an application uses only one GSG, it may appear that
 * has_ram_image() returns true if the texture has not yet been loaded by the
 * GSG, but this correlation is not true in general and should not be depended
 * on.  Specifically, if an application ever uses multiple GSG's in its
 * lifetime (for instance, by opening more than one window, or by closing its
 * window and opening another one later), then has_ram_image() may well return
 * false on textures that have never been loaded on the current GSG.
 */
INLINE bool Texture::
has_ram_image() const {
  CDReader cdata(_cycler);
  return do_has_ram_image(cdata);
}

/**
 * Returns true if the Texture has its image contents available in main RAM
 * and is uncompressed, false otherwise.  See has_ram_image().
 */
INLINE bool Texture::
has_uncompressed_ram_image() const {
  CDReader cdata(_cycler);
  return do_has_uncompressed_ram_image(cdata);
}

/**
 * Returns true if the texture's image contents are currently available in
 * main RAM, or there is reason to believe it can be loaded on demand.  That
 * is, this function returns a "best guess" as to whether get_ram_image() will
 * succeed without actually calling it first.
 */
INLINE bool Texture::
might_have_ram_image() const {
  CDReader cdata(_cycler);
  return (do_has_ram_image(cdata) || !cdata->_fullpath.empty());
}

/**
 * Returns the total number of bytes used by the in-memory image, across all
 * pages and views, or 0 if there is no in-memory image.
 */
INLINE size_t Texture::
get_ram_image_size() const {
  CDReader cdata(_cycler);
  return do_get_ram_image_size(cdata);
}

/**
 * Returns the number of bytes used by the in-memory image per view, or 0 if
 * there is no in-memory image.  Since each view is a stack of z_size pages,
 * this is get_z_size() * get_ram_page_size().
 */
INLINE size_t Texture::
get_ram_view_size() const {
  CDReader cdata(_cycler);
  if (cdata->_ram_image_compression == CM_off || cdata->_ram_images.empty()) {
    return do_get_expected_ram_view_size(cdata);
  } else {
    return cdata->_z_size * cdata->_ram_images[0]._page_size;
  }
}

/**
 * Returns the number of bytes used by the in-memory image per page, or 0 if
 * there is no in-memory image.
 *
 * For a non-compressed texture, this is the same as
 * get_expected_ram_page_size().  For a compressed texture, this may be a
 * smaller value.  (We do assume that all pages will be the same size on a
 * compressed texture).
 */
INLINE size_t Texture::
get_ram_page_size() const {
  CDReader cdata(_cycler);
  if (cdata->_ram_image_compression == CM_off || cdata->_ram_images.empty()) {
    return do_get_expected_ram_page_size(cdata);
  } else {
    return cdata->_ram_images[0]._page_size;
  }
}

/**
 * Returns the number of bytes that *ought* to be used by the in-memory image,
 * based on the texture parameters.
 */
INLINE size_t Texture::
get_expected_ram_image_size() const {
  CDReader cdata(_cycler);
  return do_get_expected_ram_image_size(cdata);
}

/**
 * Returns the number of bytes that should be used per each Z page of the 3-d
 * texture.  For a 2-d or 1-d texture, this is the same as
 * get_expected_ram_image_size().
 */
INLINE size_t Texture::
get_expected_ram_page_size() const {
  CDReader cdata(_cycler);
  return do_get_expected_ram_page_size(cdata);
}

/**
 * Returns the system-RAM image data associated with the texture.  If the
 * texture does not currently have an associated RAM image, and the texture
 * was generated by loading an image from a disk file (the most common case),
 * this forces the reload of the same texture.  This can happen if
 * keep_texture_ram is configured to false, and we have previously prepared
 * this texture with a GSG.
 *
 * Note that it is not correct to call has_ram_image() first to test whether
 * this function will fail.  A false return value from has_ram_image()
 * indicates only that get_ram_image() may need to reload the texture from
 * disk, which it will do automatically.  However, you can call
 * might_have_ram_image(), which will return true if the ram image exists, or
 * there is a reasonable reason to believe it can be loaded.
 *
 * On the other hand, it is possible that the texture cannot be found on disk
 * or is otherwise unavailable.  If that happens, this function will return
 * NULL. There is no way to predict with 100% accuracy whether get_ram_image()
 * will return NULL without calling it first; might_have_ram_image() is the
 * closest.
 */
INLINE CPTA_uchar Texture::
get_ram_image() {
  CDWriter cdata(_cycler, unlocked_ensure_ram_image(true));
  return do_get_ram_image(cdata);
}

/**
 * Returns the compression mode in which the ram image is already stored pre-
 * compressed.  If this is other than CM_off, you cannot rely on the contents
 * of the ram image to be anything predicatable (it will not be an array of x
 * by y pixels, and it probably won't have the same length as
 * get_expected_ram_image_size()).
 */
INLINE Texture::CompressionMode Texture::
get_ram_image_compression() const {
  CDReader cdata(_cycler);
  return cdata->_ram_image_compression;
}

/**
 * Returns a modifiable pointer to the system-RAM image.  This assumes the RAM
 * image should be uncompressed.  If the RAM image has been dumped, or is
 * stored compressed, creates a new one.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE PTA_uchar Texture::
modify_ram_image() {
  CDWriter cdata(_cycler, true);
  cdata->inc_image_modified();
  return do_modify_ram_image(cdata);
}

/**
 * Returns the system-RAM image associated with the texture, in an
 * uncompressed form if at all possible.
 *
 * If get_ram_image_compression() is CM_off, then the system-RAM image is
 * already uncompressed, and this returns the same thing as get_ram_image().
 *
 * If get_ram_image_compression() is anything else, then the system-RAM image
 * is compressed.  In this case, the image will be reloaded from the
 * *original* file (not from the cache), in the hopes that an uncompressed
 * image will be found there.
 *
 * If an uncompressed image cannot be found, returns NULL.
 */
INLINE CPTA_uchar Texture::
get_uncompressed_ram_image() {
  CDWriter cdata(_cycler, false);
  return do_get_uncompressed_ram_image(cdata);
}

/**
 * Discards the current system-RAM image for the texture, if any, and
 * allocates a new buffer of the appropriate size.  Returns the new buffer.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE PTA_uchar Texture::
make_ram_image() {
  CDWriter cdata(_cycler, true);
  cdata->inc_image_modified();
  return do_make_ram_image(cdata);
}

/**
 * Replaces the current system-RAM image with the new data.  If compression is
 * not CM_off, it indicates that the new data is already pre-compressed in the
 * indicated format.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE void Texture::
set_ram_image(CPTA_uchar image, Texture::CompressionMode compression,
              size_t page_size) {
  CDWriter cdata(_cycler, true);
  do_set_ram_image(cdata, image, compression, page_size);
}

/**
 * Discards the current system-RAM image.
 */
INLINE void Texture::
clear_ram_image() {
  CDWriter cdata(_cycler, false);
  do_clear_ram_image(cdata);
}

/**
 * Sets the flag that indicates whether this Texture is eligible to have its
 * main RAM copy of the texture memory dumped when the texture is prepared for
 * rendering.
 *
 * This will be false for most textures, which can reload their images if
 * needed by rereading the input file.  However, textures that were generated
 * dynamically and cannot be easily reloaded will want to set this flag to
 * true, so that the texture will always keep its image copy around.
 */
INLINE void Texture::
set_keep_ram_image(bool keep_ram_image) {
  CDWriter cdata(_cycler, true);
  cdata->_keep_ram_image = keep_ram_image;
}

/**
 * Attempts to compress the texture's RAM image internally, to a format
 * supported by the indicated GSG.  In order for this to work, the squish
 * library must have been compiled into Panda.
 *
 * If compression is CM_on, then an appropriate compression method that is
 * supported by the indicated GSG is automatically chosen.  If the GSG pointer
 * is NULL, any of the standard DXT1/3/5 compression methods will be used,
 * regardless of whether it is supported.
 *
 * If compression is any specific compression method, that method is used
 * regardless of whether the GSG supports it.
 *
 * quality_level determines the speed/quality tradeoff of the compression.  If
 * it is QL_default, the texture's own quality_level parameter is used.
 *
 * Returns true if successful, false otherwise.
 */
INLINE bool Texture::
compress_ram_image(Texture::CompressionMode compression,
                   Texture::QualityLevel quality_level,
                   GraphicsStateGuardianBase *gsg) {
  CDWriter cdata(_cycler, false);
  if (do_compress_ram_image(cdata, compression, quality_level, gsg)) {
    cdata->inc_image_modified();
    return true;
  }
  return false;
}

/**
 * Attempts to uncompress the texture's RAM image internally.  In order for
 * this to work, the squish library must have been compiled into Panda, and
 * the ram image must be compressed in a format supported by squish.
 *
 * Returns true if successful, false otherwise.
 */
INLINE bool Texture::
uncompress_ram_image() {
  CDWriter cdata(_cycler, false);
  if (do_uncompress_ram_image(cdata)) {
    cdata->inc_image_modified();
    return true;
  }
  return false;
}

/**
 * Returns the maximum number of mipmap level images available in system
 * memory.  The actual number may be less than this (that is, there might be
 * gaps in the sequence); use has_ram_mipmap_image() to verify each level.
 *
 * Also see get_num_loadable_ram_mipmap_images().
 */
INLINE int Texture::
get_num_ram_mipmap_images() const {
  CDReader cdata(_cycler);
  return cdata->_ram_images.size();
}

/**
 * Returns true if the Texture has the nth mipmap level available in system
 * memory, false otherwise.  If the texture's minfilter mode requires
 * mipmapping (see uses_mipmaps()), and all the texture's mipmap levels are
 * not available when the texture is rendered, they will be generated
 * automatically.
 */
INLINE bool Texture::
has_ram_mipmap_image(int n) const {
  CDReader cdata(_cycler);
  return do_has_ram_mipmap_image(cdata, n);
}

/**
 * Returns true if all expected mipmap levels have been defined and exist in
 * the system RAM, or false if even one mipmap level is missing.
 */
INLINE bool Texture::
has_all_ram_mipmap_images() const {
  CDReader cdata(_cycler);
  return do_has_all_ram_mipmap_images(cdata);
}

/**
 * Returns the number of bytes used by the in-memory image for mipmap level n,
 * or 0 if there is no in-memory image for this mipmap level.
 */
INLINE size_t Texture::
get_ram_mipmap_image_size(int n) const {
  CDReader cdata(_cycler);
  if (n >= 0 && n < (int)cdata->_ram_images.size()) {
    if (cdata->_ram_images[n]._pointer_image == nullptr) {
      return cdata->_ram_images[n]._image.size();
    } else {
      // Calculate it based on the given page size.
      return do_get_ram_mipmap_page_size(cdata, n) *
             do_get_expected_mipmap_z_size(cdata, n) *
             cdata->_num_views;
    }
  }
  return 0;
}

/**
 * Returns the number of bytes used by the in-memory image per view for mipmap
 * level n, or 0 if there is no in-memory image for this mipmap level.
 *
 * A "view" is a collection of z_size pages for each mipmap level.  Most
 * textures have only one view, except for multiview or stereo textures.
 *
 * For a non-compressed texture, this is the same as
 * get_expected_ram_mipmap_view_size().  For a compressed texture, this may be
 * a smaller value.  (We do assume that all pages will be the same size on a
 * compressed texture).
 */
INLINE size_t Texture::
get_ram_mipmap_view_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_ram_mipmap_page_size(cdata, n) * do_get_expected_mipmap_z_size(cdata, n);
}

/**
 * Returns the number of bytes used by the in-memory image per page for mipmap
 * level n, or 0 if there is no in-memory image for this mipmap level.
 *
 * For a non-compressed texture, this is the same as
 * get_expected_ram_mipmap_page_size().  For a compressed texture, this may be
 * a smaller value.  (We do assume that all pages will be the same size on a
 * compressed texture).
 */
INLINE size_t Texture::
get_ram_mipmap_page_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_ram_mipmap_page_size(cdata, n);
}

/**
 * Returns the number of bytes that *ought* to be used by the in-memory image
 * for mipmap level n, based on the texture parameters.
 */
INLINE size_t Texture::
get_expected_ram_mipmap_image_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_ram_mipmap_image_size(cdata, n);
}

/**
 * Returns the number of bytes that *ought* to be used by each view of the in-
 * memory image for mipmap level n, based on the texture parameters.  For a
 * normal, non-multiview texture, this is the same as
 * get_expected_ram_mipmap_image_size(n).
 */
INLINE size_t Texture::
get_expected_ram_mipmap_view_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_ram_mipmap_view_size(cdata, n);
}

/**
 * Returns the number of bytes that should be used per each Z page of the 3-d
 * texture, for mipmap level n.  For a 2-d or 1-d texture, this is the same as
 * get_expected_ram_mipmap_view_size(n).
 */
INLINE size_t Texture::
get_expected_ram_mipmap_page_size(int n) const {
  CDReader cdata(_cycler);
  return do_get_expected_ram_mipmap_page_size(cdata, n);
}

/**
 * Returns a modifiable pointer to the system-RAM image for the nth mipmap
 * level.  This assumes the RAM image is uncompressed; if this is not the
 * case, raises an assertion.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE PTA_uchar Texture::
modify_ram_mipmap_image(int n) {
  CDWriter cdata(_cycler, false);
  cdata->inc_image_modified();
  return do_modify_ram_mipmap_image(cdata, n);
}

/**
 * Discards the current system-RAM image for the nth mipmap level, if any, and
 * allocates a new buffer of the appropriate size.  Returns the new buffer.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE PTA_uchar Texture::
make_ram_mipmap_image(int n) {
  CDWriter cdata(_cycler, false);
  cdata->inc_image_modified();
  return do_make_ram_mipmap_image(cdata, n);
}

/**
 * Replaces the current system-RAM image for the indicated mipmap level with
 * the new data.  If compression is not CM_off, it indicates that the new data
 * is already pre-compressed in the indicated format.
 *
 * This does *not* affect keep_ram_image.
 */
INLINE void Texture::
set_ram_mipmap_image(int n, CPTA_uchar image, size_t page_size) {
  CDWriter cdata(_cycler, false);
  do_set_ram_mipmap_image(cdata, n, image, page_size);
}

/**
 * Discards the current system-RAM image for all mipmap levels, except level 0
 * (the base image).
 */
INLINE void Texture::
clear_ram_mipmap_images() {
  CDWriter cdata(_cycler, false);
  cdata->inc_image_modified();
  do_clear_ram_mipmap_images(cdata);
}

/**
 * Automatically fills in the n mipmap levels of the Texture, based on the
 * texture's source image.  This requires the texture's uncompressed ram image
 * to be available in system memory.  If it is not already, it will be fetched
 * if possible.
 *
 * This call is not normally necessary, since the mipmap levels will be
 * generated automatically if needed.  But there may be certain cases in which
 * you would like to call this explicitly.
 */
INLINE void Texture::
generate_ram_mipmap_images() {
  // Don't use unlocked_ensure_ram_image here, because
  // do_generate_ram_mipmap_images will want to decompress and recompress the
  // image itself.
  CDWriter cdata(_cycler, false);
  cdata->inc_image_modified();
  do_generate_ram_mipmap_images(cdata, true);
}

/**
 * Returns the width of the "simple" image in texels.
 */
INLINE int Texture::
get_simple_x_size() const {
  CDReader cdata(_cycler);
  return cdata->_simple_x_size;
}

/**
 * Returns the height of the "simple" image in texels.
 */
INLINE int Texture::
get_simple_y_size() const {
  CDReader cdata(_cycler);
  return cdata->_simple_y_size;
}

/**
 * Returns true if the Texture has a "simple" image available in main RAM.
 */
INLINE bool Texture::
has_simple_ram_image() const {
  CDReader cdata(_cycler);
  return !cdata->_simple_ram_image._image.empty();
}

/**
 * Returns the number of bytes used by the "simple" image, or 0 if there is no
 * simple image.
 */
INLINE size_t Texture::
get_simple_ram_image_size() const {
  CDReader cdata(_cycler);
  return cdata->_simple_ram_image._image.size();
}

/**
 * Returns the image data associated with the "simple" texture image.  This is
 * provided for some textures as an option to display while the main texture
 * image is being loaded from disk.
 *
 * Unlike get_ram_image(), this function will always return immediately.
 * Either the simple image is available, or it is not.
 *
 * The "simple" image is always 4 components, 1 byte each, regardless of the
 * parameters of the full texture.  The simple image is only supported for
 * ordinary 2-d textures.
 */
INLINE CPTA_uchar Texture::
get_simple_ram_image() const {
  CDReader cdata(_cycler);
  return cdata->_simple_ram_image._image;
}

/**
 * Replaces the internal "simple" texture image.  This can be used as an
 * option to display while the main texture image is being loaded from disk.
 * It is normally a very small image, 16x16 or smaller (and maybe even 1x1),
 * that is designed to give just enough sense of color to serve as a
 * placeholder until the full texture is available.
 *
 * The "simple" image is always 4 components, 1 byte each, regardless of the
 * parameters of the full texture.  The simple image is only supported for
 * ordinary 2-d textures.
 *
 * Also see generate_simple_ram_image(), modify_simple_ram_image(), and
 * new_simple_ram_image().
 */
INLINE void Texture::
set_simple_ram_image(CPTA_uchar image, int x_size, int y_size) {
  CDWriter cdata(_cycler, true);
  do_set_simple_ram_image(cdata, image, x_size, y_size);
}

/**
 * Discards the current "simple" image.
 */
INLINE void Texture::
clear_simple_ram_image() {
  CDWriter cdata(_cycler, true);
  do_clear_simple_ram_image(cdata);
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the texture properties (unrelated to the image) are modified.
 */
INLINE UpdateSeq Texture::
get_properties_modified() const {
  CDReader cdata(_cycler);
  return cdata->_properties_modified;
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the texture image data (including mipmap levels) are modified.
 */
INLINE UpdateSeq Texture::
get_image_modified() const {
  CDReader cdata(_cycler);
  return cdata->_image_modified;
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the texture's "simple" image data is modified.
 */
INLINE UpdateSeq Texture::
get_simple_image_modified() const {
  CDReader cdata(_cycler);
  return cdata->_simple_image_modified;
}

/**
 * Specifies the power-of-2 texture-scaling mode that will be applied to this
 * particular texture when it is next loaded from disk.  See
 * set_textures_power_2().
 */
INLINE void Texture::
set_auto_texture_scale(AutoTextureScale scale) {
  CDWriter cdata(_cycler, true);
  cdata->_auto_texture_scale = scale;
}

/**
 * Returns the power-of-2 texture-scaling mode that will be applied to this
 * particular texture when it is next loaded from disk.  See
 * set_textures_power_2().
 */
INLINE AutoTextureScale Texture::
get_auto_texture_scale() const {
  CDReader cdata(_cycler);
  return do_get_auto_texture_scale(cdata);
}

/**
 * Returns true if set_auto_texture_scale() has been set to something other
 * than ATS_unspecified for this particular texture.
 */
INLINE bool Texture::
has_auto_texture_scale() const {
  CDReader cdata(_cycler);
  return (cdata->_auto_texture_scale != ATS_unspecified);
}

/**
 * Set this flag to ATS_none, ATS_up, ATS_down, or ATS_pad to control the
 * scaling of textures in general, if a particular texture does not override
 * this.  See also set_auto_texture_scale() for the per-texture override.
 */
INLINE void Texture::
set_textures_power_2(AutoTextureScale scale) {
  _textures_power_2 = scale;
}

/**
 * This flag returns ATS_none, ATS_up, or ATS_down and controls the scaling of
 * textures in general.  It is initialized from the config variable of the
 * same name, but it can be subsequently adjusted.  See also
 * get_auto_texture_scale().
 */
INLINE AutoTextureScale Texture::
get_textures_power_2() {
  if (_textures_power_2 == ATS_unspecified) {
    return textures_power_2;
  } else {
    return _textures_power_2;
  }
}

/**
 * If true, then get_textures_power_2 has been set using set_textures_power_2.
 * If false, then get_textures_power_2 simply returns the config variable of
 * the same name.
 */
INLINE bool Texture::
has_textures_power_2() {
  return (_textures_power_2 != ATS_unspecified);
}

/**
 * Sets the name of the file that contains the image's contents.  Normally,
 * this is set automatically when the image is loaded, for instance via
 * Texture::read().
 *
 * The Texture's get_name() function used to return the filename, but now
 * returns just the basename (without the extension), which is a more useful
 * name for identifying an image in show code.
 */
INLINE void Texture::
set_filename(const Filename &filename) {
  CDWriter cdata(_cycler, true);
  cdata->_filename = filename;
}

/**
 * Removes the alpha filename, if it was previously set.  See set_filename().
 */
INLINE void Texture::
clear_filename() {
  CDWriter cdata(_cycler, true);
  cdata->_filename = Filename();
}

/**
 * Sets the name of the file that contains the image's alpha channel contents.
 * Normally, this is set automatically when the image is loaded, for instance
 * via Texture::read().
 *
 * The Texture's get_filename() function returns the name of the image file
 * that was loaded into the buffer.  In the case where a texture specified two
 * separate files to load, a 1- or 3-channel color image and a 1-channel alpha
 * image, this Filename is update to contain the name of the image file that
 * was loaded into the buffer's alpha channel.
 */
INLINE void Texture::
set_alpha_filename(const Filename &alpha_filename) {
  CDWriter cdata(_cycler, true);
  cdata->_alpha_filename = alpha_filename;
}

/**
 * Removes the alpha filename, if it was previously set.  See
 * set_alpha_filename().
 */
INLINE void Texture::
clear_alpha_filename() {
  CDWriter cdata(_cycler, true);
  cdata->_alpha_filename = Filename();
}

/**
 * Sets the full pathname to the file that contains the image's contents, as
 * found along the search path.  Normally, this is set automatically when the
 * image is loaded, for instance via Texture::read().
 */
INLINE void Texture::
set_fullpath(const Filename &fullpath) {
  CDWriter cdata(_cycler, true);
  cdata->_fullpath = fullpath;
}

/**
 * Removes the alpha fullpath, if it was previously set.  See set_fullpath().
 */
INLINE void Texture::
clear_fullpath() {
  CDWriter cdata(_cycler, true);
  cdata->_fullpath = Filename();
}

/**
 * Sets the full pathname to the file that contains the image's alpha channel
 * contents, as found along the search path.  Normally, this is set
 * automatically when the image is loaded, for instance via Texture::read().
 */
INLINE void Texture::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  CDWriter cdata(_cycler, true);
  cdata->_alpha_fullpath = alpha_fullpath;
}

/**
 * Removes the alpha fullpath, if it was previously set.  See
 * set_alpha_fullpath().
 */
INLINE void Texture::
clear_alpha_fullpath() {
  CDWriter cdata(_cycler, true);
  cdata->_alpha_fullpath = Filename();
}

/**
 * Changes the x size indicated for the texture.  This also implicitly unloads
 * the texture if it has already been loaded.
 */
INLINE void Texture::
set_x_size(int x_size) {
  CDWriter cdata(_cycler, true);
  do_set_x_size(cdata, x_size);
}

/**
 * Changes the y size indicated for the texture.  This also implicitly unloads
 * the texture if it has already been loaded.
 */
INLINE void Texture::
set_y_size(int y_size) {
  CDWriter cdata(_cycler, true);
  do_set_y_size(cdata, y_size);
}

/**
 * Changes the z size indicated for the texture.  This also implicitly unloads
 * the texture if it has already been loaded.
 */
INLINE void Texture::
set_z_size(int z_size) {
  CDWriter cdata(_cycler, true);
  do_set_z_size(cdata, z_size);
}

/**
 * Sets the number of "views" within a texture.  A view is a completely
 * separate image stored within the Texture object.  Most textures have only
 * one view, but a stereo texture, for instance, may have two views, a left
 * and a right image.  Other uses for multiple views are not yet defined.
 *
 * If this value is greater than one, the additional views are accessed as
 * additional pages beyond get_z_size().
 *
 * This also implicitly unloads the texture if it has already been loaded.
 */
INLINE void Texture::
set_num_views(int num_views) {
  CDWriter cdata(_cycler, true);
  do_set_num_views(cdata, num_views);
}

/**
 * Changes the format value for the texture components.  This implicitly sets
 * num_components as well.
 */
INLINE void Texture::
set_format(Texture::Format format) {
  CDWriter cdata(_cycler, true);
  do_set_format(cdata, format);
}

/**
 * Changes the data value for the texture components.  This implicitly sets
 * component_width as well.
 */
INLINE void Texture::
set_component_type(Texture::ComponentType component_type) {
  CDWriter cdata(_cycler, true);
  do_set_component_type(cdata, component_type);
}

/**
 * Sets the flag that indicates the texture has been loaded from a disk file
 * or PNMImage.  You should also ensure the filename has been set correctly.
 * When this flag is true, the texture may be automatically reloaded when its
 * ram image needs to be replaced.
 */
INLINE void Texture::
set_loaded_from_image(bool flag) {
  CDWriter cdata(_cycler, false);
  cdata->_loaded_from_image = flag;
}

/**
 * Returns the flag that indicates the texture has been loaded from a disk
 * file or PNMImage.  See set_loaded_from_image().
 */
INLINE bool Texture::
get_loaded_from_image() const {
  CDReader cdata(_cycler);
  return cdata->_loaded_from_image;
}

/**
 * Sets the flag that indicates the texture has been loaded from a txo file.
 * You probably shouldn't be setting this directly; it is set automatically
 * when a Texture is loaded.
 */
INLINE void Texture::
set_loaded_from_txo(bool flag) {
  CDWriter cdata(_cycler, false);
  cdata->_loaded_from_txo = flag;
}

/**
 * Returns the flag that indicates the texture has been loaded from a txo
 * file.
 */
INLINE bool Texture::
get_loaded_from_txo() const {
  CDReader cdata(_cycler);
  return cdata->_loaded_from_txo;
}

/**
 * Returns true if the special flag was set that indicates to the GSG that the
 * Texture's format should be chosen to exactly match the framebuffer's
 * format, presumably because the application intends to copy image data from
 * the framebuffer into the Texture (or vice-versa).
 */
INLINE bool Texture::
get_match_framebuffer_format() const {
  CDReader cdata(_cycler);
  return cdata->_match_framebuffer_format;
}

/**
 * Sets the special flag that, if true, indicates to the GSG that the
 * Texture's format should be chosen to exactly match the framebuffer's
 * format, presumably because the application intends to copy image data from
 * the framebuffer into the Texture (or vice-versa).
 *
 * This sets only the graphics card's idea of the texture format; it is not
 * related to the system-memory format.
 */
INLINE void Texture::
set_match_framebuffer_format(bool flag) {
  CDWriter cdata(_cycler, true);
  cdata->_match_framebuffer_format = flag;
}

/**
 * Returns the setting of the post_load_store_cache flag.  See
 * set_post_load_store_cache().
 */
INLINE bool Texture::
get_post_load_store_cache() const {
  CDReader cdata(_cycler);
  return cdata->_post_load_store_cache;
}

/**
 * Sets the post_load_store_cache flag.  When this is set, the next time the
 * texture is loaded on a GSG, it will automatically extract its RAM image
 * from the GSG and save it to the global BamCache.
 *
 * This is used to store compressed RAM images in the BamCache.  This flag
 * should not be set explicitly; it is set automatically by the TexturePool
 * when model-cache-compressed-textures is set true.
 */
INLINE void Texture::
set_post_load_store_cache(bool flag) {
  CDWriter cdata(_cycler, true);
  cdata->_post_load_store_cache = flag;
}

/**
 * This method is similar to consider_rescale(), but instead of scaling a
 * separate PNMImage, it will ask the Texture to rescale its own internal
 * image to a power of 2, according to the config file requirements.  This may
 * be useful after loading a Texture image by hand, instead of reading it from
 * a disk file.  Returns true if the texture is changed, false if it was not.
 */
INLINE bool Texture::
rescale_texture() {
  CDWriter cdata(_cycler, true);
  return do_rescale_texture(cdata);
}

/**
 * Works like adjust_size, but also considers the texture class.  Movie
 * textures, for instance, always pad outwards, regardless of textures-
 * power-2.
 */
INLINE bool Texture::
adjust_this_size(int &x_size, int &y_size, const std::string &name,
                 bool for_padding) const {
  CDReader cdata(_cycler);
  return do_adjust_this_size(cdata, x_size, y_size, name, for_padding);
}

/**
 *
 */
INLINE size_t Texture::
do_get_ram_image_size(const CData *cdata) const {
  if (cdata->_ram_images.empty()) {
    return 0;
  }
  return cdata->_ram_images[0]._image.size();
}

/**
 *
 */
INLINE bool Texture::
do_has_ram_mipmap_image(const CData *cdata, int n) const {
  return (n >= 0 && n < (int)cdata->_ram_images.size() &&
          !cdata->_ram_images[n]._image.empty());
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_image_size(const CData *cdata) const {
  return do_get_expected_ram_view_size(cdata) * (size_t)cdata->_num_views;
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_view_size(const CData *cdata) const {
  return do_get_expected_ram_page_size(cdata) * (size_t)cdata->_z_size;
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_page_size(const CData *cdata) const {
  return (size_t)(cdata->_x_size * cdata->_y_size * cdata->_num_components * cdata->_component_width);
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_mipmap_image_size(const CData *cdata, int n) const {
  return do_get_expected_ram_mipmap_view_size(cdata, n) * (size_t)cdata->_num_views;
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_mipmap_view_size(const CData *cdata, int n) const {
  return do_get_expected_ram_mipmap_page_size(cdata, n) * (size_t)do_get_expected_mipmap_z_size(cdata, n);
}

/**
 *
 */
INLINE size_t Texture::
do_get_expected_ram_mipmap_page_size(const CData *cdata, int n) const {
  return (size_t)(do_get_expected_mipmap_x_size(cdata, n) * do_get_expected_mipmap_y_size(cdata, n) * cdata->_num_components * cdata->_component_width);
}

/**
 *
 */
INLINE int Texture::
do_get_expected_mipmap_num_pages(const CData *cdata, int n) const {
  return do_get_expected_mipmap_z_size(cdata, n) * cdata->_num_views;
}

/**
 *
 */
INLINE void Texture::
do_clear_ram_image(CData *cdata) {
  cdata->_ram_image_compression = CM_off;
  cdata->_ram_images.clear();
}

/**
 *
 */
INLINE AutoTextureScale Texture::
do_get_auto_texture_scale(const CData *cdata) const {
  if (cdata->_auto_texture_scale == ATS_unspecified) {
    return get_textures_power_2();
  } else {
    return cdata->_auto_texture_scale;
  }
}

/**
 * This is used by load() to store the next consecutive component value into
 * the indicated element of the array, which is taken to be an array of
 * unsigned bytes.  The value is assumed to be in the range 0-255.
 */
INLINE void Texture::
store_unscaled_byte(unsigned char *&p, int value) {
  (*p++) = (uchar)value;
}

/**
 * This is used by load() to store the next consecutive component value into
 * the indicated element of the array, which is taken to be an array of
 * unsigned shorts.  The value is assumed to be in the range 0-65535.
 */
INLINE void Texture::
store_unscaled_short(unsigned char *&p, int value) {
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.us = (ushort)value;
  (*p++) = v.uc[0];
  (*p++) = v.uc[1];
}

/**
 * This is used by load() to store the next consecutive component value into
 * the indicated element of the array, which is taken to be an array of
 * unsigned bytes.  The value will be scaled by the indicated factor before
 * storing it.
 */
INLINE void Texture::
store_scaled_byte(unsigned char *&p, int value, double scale) {
  store_unscaled_byte(p, (int)(value * scale));
}

/**
 * This is used by load() to store the next consecutive component value into
 * the indicated element of the array, which is taken to be an array of
 * unsigned shorts.  The value will be scaled by the indicated factor before
 * storing it.
 */
INLINE void Texture::
store_scaled_short(unsigned char *&p, int value, double scale) {
  store_unscaled_short(p, (int)(value * scale));
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * unsigned bytes.
 */
INLINE double Texture::
get_unsigned_byte(const unsigned char *&p) {
  return (double)(*p++) / 255.0;
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * unsigned shorts.
 */
INLINE double Texture::
get_unsigned_short(const unsigned char *&p) {
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.uc[0] = (*p++);
  v.uc[1] = (*p++);
  return (double)v.us / 65535.0;
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * unsigned ints.
 */
INLINE double Texture::
get_unsigned_int(const unsigned char *&p) {
  union {
    unsigned int ui;
    uchar uc[4];
  } v;
  v.uc[0] = (*p++);
  v.uc[1] = (*p++);
  v.uc[2] = (*p++);
  v.uc[3] = (*p++);
  return (double)v.ui / 4294967295.0;
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * unsigned ints with the value packed in the 24 least significant bits.
 */
INLINE double Texture::
get_unsigned_int_24(const unsigned char *&p) {
  union {
    uint32_t ui;
    uint8_t uc[4];
  } v;
  v.uc[0] = (*p++);
  v.uc[1] = (*p++);
  v.uc[2] = (*p++);
  v.uc[3] = (*p++);
  return (double)(v.ui & 0xffffff) / (double)0xffffff;
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * floats.
 */
INLINE double Texture::
get_float(const unsigned char *&p) {
  double v = *((float *)p);
  p += 4;
  return v;
}

/**
 * This is used by store() to retrieve the next consecutive component value
 * from the indicated element of the array, which is taken to be an array of
 * half-floats.
 */
INLINE double Texture::
get_half_float(const unsigned char *&p) {
  union {
    uint32_t ui;
    float uf;
  } v;
  uint16_t in = *(uint16_t *)p;
  p += 2;
  uint32_t t1 = in & 0x7fff; // Non-sign bits
  uint32_t t2 = in & 0x8000; // Sign bit
  uint32_t t3 = in & 0x7c00; // Exponent
  t1 <<= 13; // Align mantissa on MSB
  t2 <<= 16; // Shift sign bit into position
  if (t3 != 0x7c00) {
    t1 += 0x38000000; // Adjust bias
    t1 = (t3 == 0 ? 0 : t1); // Denormals-as-zero
  } else {
    // Infinity / NaN
    t1 |= 0x7f800000;
  }
  t1 |= t2; // Re-insert sign bit
  v.ui = t1;
  return v.uf;
}

/**
 * Returns true if the indicated filename ends in .txo or .txo.pz or .txo.gz,
 * false otherwise.
 */
INLINE bool Texture::
is_txo_filename(const Filename &fullpath) {
  std::string extension = fullpath.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz" || extension == "gz") {
    extension = Filename(fullpath.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  return (extension == "txo");
}

/**
 * Returns true if the indicated filename ends in .dds or .dds.pz or .dds.gz,
 * false otherwise.
 */
INLINE bool Texture::
is_dds_filename(const Filename &fullpath) {
  std::string extension = fullpath.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz" || extension == "gz") {
    extension = Filename(fullpath.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  return (downcase(extension) == "dds");
}

/**
 * Returns true if the indicated filename ends in .ktx or .ktx.pz or .ktx.gz,
 * false otherwise.
 */
INLINE bool Texture::
is_ktx_filename(const Filename &fullpath) {
  std::string extension = fullpath.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz" || extension == "gz") {
    extension = Filename(fullpath.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  return (downcase(extension) == "ktx");
}

/**
 *
 */
INLINE void Texture::CData::
inc_properties_modified() {
  ++_properties_modified;
}

/**
 *
 */
INLINE void Texture::CData::
inc_image_modified() {
  ++_image_modified;
}

/**
 *
 */
INLINE void Texture::CData::
inc_simple_image_modified() {
  ++_simple_image_modified;
}

/**
 *
 */
INLINE Texture::RamImage::
RamImage() :
  _page_size(0),
  _pointer_image(nullptr)
{
}
