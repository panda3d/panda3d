// Filename: texture.I
// Created by:  drose (05Feb99)
// Updated by: fperazzi, PandaSE(29Apr10) (added setup_2d_texture_array)
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
//     Function: Texture::make_copy
//       Access: Published
//  Description: Returns a new copy of the same Texture.  This copy,
//               if applied to geometry, will be copied into texture
//               as a separate texture from the original, so it will
//               be duplicated in texture memory (and may be
//               independently modified if desired).
//
//               If the Texture is a VideoTexture, the resulting
//               duplicate may be animated independently of the
//               original.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) Texture::
make_copy() {
  MutexHolder holder(_lock);
  PT(Texture) tex = do_make_copy();
  ++(tex->_properties_modified);
  ++(tex->_image_modified);
  ++(tex->_simple_image_modified);
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear
//       Access: Published, Virtual
//  Description: Reinitializes the texture to its default, empty
//               state (except for the name).
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear() {
  MutexHolder holder(_lock);
  do_clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_texture
//       Access: Published
//  Description: Sets the texture to the indicated type and
//               dimensions, presumably in preparation for calling
//               read() or load(), or set_ram_image() or
//               modify_ram_image().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_texture(Texture::TextureType texture_type, int x_size, int y_size,
              int z_size, Texture::ComponentType component_type,
              Texture::Format format) {
  MutexHolder holder(_lock);
  do_setup_texture(texture_type, x_size, y_size, z_size,
                   component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_1d_texture
//       Access: Published
//  Description: Sets the texture as an empty 1-d texture with no
//               dimensions.  Follow up with read() or load() to fill
//               the texture properties and image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_1d_texture() {
  setup_1d_texture(0, T_unsigned_byte, F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_1d_texture
//       Access: Published
//  Description: Sets the texture as an empty 1-d texture with the
//               specified dimensions and properties.  Follow up with
//               set_ram_image() or modify_ram_image() to fill the
//               image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_1d_texture(int x_size, ComponentType component_type, Format format) {
  setup_texture(TT_1d_texture, x_size, 1, 1, component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_2d_texture
//       Access: Published
//  Description: Sets the texture as an empty 2-d texture with no
//               dimensions.  Follow up with read() or load() to fill
//               the texture properties and image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_2d_texture() {
  setup_2d_texture(0, 1, T_unsigned_byte, F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_2d_texture
//       Access: Published
//  Description: Sets the texture as an empty 2-d texture with the
//               specified dimensions and properties.  Follow up with
//               set_ram_image() or modify_ram_image() to fill the
//               image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_2d_texture(int x_size, int y_size, ComponentType component_type, 
                 Format format) {
  setup_texture(TT_2d_texture, x_size, y_size, 1, component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_3d_texture
//       Access: Published
//  Description: Sets the texture as an empty 3-d texture with no
//               dimensions (though if you know the depth ahead
//               of time, it saves a bit of reallocation later).
//               Follow up with read() or load() to fill the texture
//               properties and image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_3d_texture(int z_size) {
  setup_3d_texture(0, 1, z_size, T_unsigned_byte, F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_3d_texture
//       Access: Published
//  Description: Sets the texture as an empty 3-d texture with the
//               specified dimensions and properties.  Follow up with
//               set_ram_image() or modify_ram_image() to fill the
//               image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_3d_texture(int x_size, int y_size, int z_size,
                 ComponentType component_type, Format format) {
  setup_texture(TT_3d_texture, x_size, y_size, z_size, component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_2d_texture_array
//       Access: Published
//  Description: Sets the texture as an empty 2-d texture array with
//               no dimensions (though if you know the depth ahead
//               of time, it saves a bit of reallocation later).
//               Follow up with read() or load() to fill the texture
//               properties and image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_2d_texture_array(int z_size) {
  setup_2d_texture_array(0, 1, z_size, T_unsigned_byte, F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_2d_texture_array
//       Access: Published
//  Description: Sets the texture as an empty 2-d texture array with the
//               specified dimensions and properties. Follow up with
//               set_ram_image() or modify_ram_image() to fill the
//               image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_2d_texture_array(int x_size, int y_size, int z_size,
                 ComponentType component_type, Format format) {
  setup_texture(TT_2d_texture_array, x_size, y_size, z_size, component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_cube_map
//       Access: Published
//  Description: Sets the texture as an empty cube map texture with no
//               dimensions.  Follow up with read() or load() to fill
//               the texture properties and image data.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_cube_map() {
  setup_cube_map(0, T_unsigned_byte, F_rgb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_cube_map
//       Access: Published
//  Description: Sets the texture as an empty cube map texture with
//               the specified dimensions and properties.  Follow up
//               with set_ram_image() or modify_ram_image() to fill
//               the image data.  
//
//               Note that a cube map should always consist of six
//               square images, so x_size and y_size will be the same,
//               and z_size is always 6.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
setup_cube_map(int size, ComponentType component_type, 
               Format format) {
  setup_texture(TT_cube_map, size, size, 6, component_type, format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write
//       Access: Published
//  Description: Writes the texture to the named filename.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
write(const Filename &fullpath) {
  MutexHolder holder(_lock);
  return do_write(fullpath, 0, 0, false, false);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write
//       Access: Published
//  Description: Writes a single page or mipmap level to a single
//               file, or automatically writes a series of pages
//               and/or mipmap levels to a numbered series of files.
//
//               If the filename ends in the extension .txo, this
//               implicitly writes a Panda texture object (.txo)
//               instead of an image file.  In this case, the
//               remaining parameters are ignored, and only one file
//               is written, which will contain all of the pages and
//               resident mipmap levels in the texture.
//
//               If write_pages is false, then z indicates the page
//               number to write.  3-D textures have one page number
//               for each level of depth; cube maps have six pages
//               number 0 through 5.  Other kinds of textures have
//               only one page, numbered 0.
//
//               If write_pages is true, then all pages of the texture
//               will be written.  In this case z is ignored, and the
//               filename should contain a sequence of hash marks
//               ("#") which will be filled in with the page index
//               number.
//
//               If write_mipmaps is false, then n indicates the
//               mipmap level number to write.  Normally, this is 0,
//               for the base texture image.  Normally, the mipmap
//               levels of a texture are not available in RAM (they
//               are generated automatically by the graphics card).
//               However, if you have the mipmap levels available, for
//               instance because you called
//               generate_ram_mipmap_images() to generate them
//               internally, or you called
//               GraphicsEngine::extract_texture_data() to retrieve
//               them from the graphics card, then you may write out
//               each mipmap level with this parameter.
//
//               If write_mipmaps is true, then all mipmap levels of
//               the texture will be written.  In this case n is
//               ignored, and the filename should contain a sequence
//               of hash marks ("#") which will be filled in with the
//               mipmap level number.
//
//               If both write_pages and write_mipmaps is true, then
//               all pages and all mipmap levels will be written.  In
//               this case, the filename should contain two different
//               sequences of hash marks, separated by a character
//               such as a hyphen, underscore, or dot.  The first hash
//               mark sequence will be filled in with the mipmap
//               level, while the second hash mark sequence will be
//               the page index.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
write(const Filename &fullpath, int z, int n, 
      bool write_pages, bool write_mipmaps) {
  MutexHolder holder(_lock);
  return do_write(fullpath, z, n, write_pages, write_mipmaps);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published
//  Description: Replaces the texture with the indicated image.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
load(const PNMImage &pnmimage, const LoaderOptions &options) {
  MutexHolder holder(_lock);
  do_clear();
  ++_properties_modified;
  ++_image_modified;
  if (do_load_one(pnmimage, get_name(), 0, 0, options)) {
    bool generate_mipmaps = ((options.get_texture_flags() & LoaderOptions::TF_generate_mipmaps) != 0);
    consider_auto_process_ram_image(generate_mipmaps || uses_mipmaps(), true);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published
//  Description: Stores the indicated image in the given page and
//               mipmap level.  See read().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
load(const PNMImage &pnmimage, int z, int n, const LoaderOptions &options) {
  MutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  if (do_load_one(pnmimage, get_name(), z, n, options)) {
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store
//       Access: Published
//  Description: Saves the texture to the indicated PNMImage, but does
//               not write it to disk.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
store(PNMImage &pnmimage) const {
  MutexHolder holder(_lock);
  return do_store_one(pnmimage, 0, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store
//       Access: Published
//  Description: Saves the indicated page and mipmap level of the
//               texture to the PNMImage.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
store(PNMImage &pnmimage, int z, int n) const {
  MutexHolder holder(_lock);
  return do_store_one(pnmimage, z, n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::reload
//       Access: Published
//  Description: Re-reads the Texture from its disk file.  Useful when
//               you know the image on disk has recently changed, and
//               you want to update the Texture image.
//
//               Returns true on success, false on failure (in which
//               case, the Texture may or may not still be valid).
////////////////////////////////////////////////////////////////////
bool Texture::
reload() {
  MutexHolder holder(_lock);
  return do_reload();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_filename
//       Access: Published
//  Description: Returns true if the filename has been set and
//               is available.  See set_filename().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_filename() const {
  MutexHolder holder(_lock);
  return !_filename.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_filename
//       Access: Published
//  Description: Returns the filename that has been set.  This is the
//               name of the file as it was requested.  Also see
//               get_fullpath().
////////////////////////////////////////////////////////////////////
INLINE const Filename &Texture::
get_filename() const {
  MutexHolder holder(_lock);
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_alpha_filename
//       Access: Published
//  Description: Returns true if the alpha_filename has been set and
//               is available.  See set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_alpha_filename() const {
  MutexHolder holder(_lock);
  return !_alpha_filename.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_alpha_filename
//       Access: Published
//  Description: Returns the alpha_filename that has been set.  If
//               this is set, it represents the name of the alpha
//               component, which is stored in a separate file.  See
//               also get_filename(), and get_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE const Filename &Texture::
get_alpha_filename() const {
  MutexHolder holder(_lock);
  return _alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_fullpath
//       Access: Published
//  Description: Returns true if the fullpath has been set and
//               is available.  See set_fullpath().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_fullpath() const {
  MutexHolder holder(_lock);
  return !_fullpath.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_fullpath
//       Access: Published
//  Description: Returns the fullpath that has been set.  This is the
//               full path to the file as it was found along the
//               texture search path.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Texture::
get_fullpath() const {
  MutexHolder holder(_lock);
  return _fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_alpha_fullpath
//       Access: Published
//  Description: Returns true if the alpha_fullpath has been set and
//               is available.  See set_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_alpha_fullpath() const {
  MutexHolder holder(_lock);
  return !_alpha_fullpath.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_alpha_fullpath
//       Access: Published
//  Description: 
//               Returns the alpha_fullpath that has been set.  This
//               is the full path to the alpha part of the image file
//               as it was found along the texture search path.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Texture::
get_alpha_fullpath() const {
  MutexHolder holder(_lock);
  return _alpha_fullpath;
}


////////////////////////////////////////////////////////////////////
//     Function: Texture::get_x_size
//       Access: Published
//  Description: Returns the width of the texture image in texels.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_y_size
//       Access: Published
//  Description: Returns the height of the texture image in texels.
//               For a 1-d texture, this will be 1.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_z_size
//       Access: Published
//  Description: Returns the depth of the texture image in texels.
//               For a 1-d texture or 2-d texture, this will be 1.
//               For a cube map texture, this will be 6.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_z_size() const {
  return _z_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_pad_x_size
//       Access: Published
//  Description: Returns size of the pad region.  See set_pad_size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_pad_x_size() const {
  return _pad_x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_pad_y_size
//       Access: Published
//  Description: Returns size of the pad region.  See set_pad_size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_pad_y_size() const {
  return _pad_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_pad_z_size
//       Access: Published
//  Description: Returns size of the pad region.  See set_pad_size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_pad_z_size() const {
  return _pad_z_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_tex_scale
//       Access: Published
//  Description: Returns a scale pair that is suitable for applying to
//               geometry via NodePath::set_tex_scale(), which will
//               convert texture coordinates on the geometry from the
//               range 0..1 into the appropriate range to render the
//               video part of the texture.
//
//               This is necessary only if a padding size has been set
//               via set_pad_size() (or implicitly via something like
//               "textures-power-2 pad" in the config.prc file).  In
//               this case, this is a convenient way to generate UV's
//               that reflect the built-in padding size.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f Texture::
get_tex_scale() const {
  if (_pad_x_size == 0 || _pad_y_size == 0 ||
      _x_size == 0 || _y_size == 0) {
    LVecBase2f(1.0f, 1.0f);
  }
  return LVecBase2f((float)(_x_size - _pad_x_size) / (float)_x_size,
                    (float)(_y_size - _pad_y_size) / (float)_y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_pad_size
//       Access: Published
//  Description: Sets the size of the pad region.
//
//               Sometimes, when a video card demands power-of-two
//               textures, it is necessary to create a big texture
//               and then only use a portion of it.  The pad region
//               indicates which portion of the texture is not
//               really in use.  All operations use the texture
//               as a whole, including the pad region, unless they
//               explicitly state that they use only the non-pad
//               region.
//
//               Changing the texture's size clears the pad region.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_pad_size(int x, int y, int z) {
  MutexHolder holder(_lock);
  do_set_pad_size(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_orig_file_x_size
//       Access: Published
//  Description: Returns the X size of the original disk image that
//               this Texture was loaded from (if it came from a disk
//               file), before any automatic rescaling by Panda.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_orig_file_x_size() const {
  return _orig_file_x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_orig_file_y_size
//       Access: Published
//  Description: Returns the Y size of the original disk image that
//               this Texture was loaded from (if it came from a disk
//               file), before any automatic rescaling by Panda.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_orig_file_y_size() const {
  return _orig_file_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_orig_file_z_size
//       Access: Published
//  Description: Returns the Z size of the original disk image that
//               this Texture was loaded from (if it came from a disk
//               file), before any automatic rescaling by Panda.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_orig_file_z_size() const {
  // At the moment, we perform no automatic adjustment of Z size.  So
  // we can just return the current value, since it would be the same
  // thing.
  return _z_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_num_components
//       Access: Published
//  Description: Returns the number of color components for each texel
//               of the texture image.  This is 3 for an rgb texture
//               or 4 for an rgba texture; it may also be 1 or 2 for a
//               grayscale texture.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_component_width
//       Access: Published
//  Description: Returns the number of bytes stored for each color
//               component of a texel.  Typically this is 1, but it
//               may be 2 for 16-bit texels.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_component_width() const {
  return _component_width;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_texture_type
//       Access: Published
//  Description: Returns the overall interpretation of the texture.
////////////////////////////////////////////////////////////////////
INLINE Texture::TextureType Texture::
get_texture_type() const {
  return _texture_type;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_format
//       Access: Published
//  Description: Returns the format of the texture, which represents
//               both the semantic meaning of the texels and, to some
//               extent, their storage information.
////////////////////////////////////////////////////////////////////
INLINE Texture::Format Texture::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_component_type
//       Access: Published
//  Description: Returns the numeric interpretation of each component
//               of the texture.
////////////////////////////////////////////////////////////////////
INLINE Texture::ComponentType Texture::
get_component_type() const {
  return _component_type;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_wrap_u
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_wrap_u(Texture::WrapMode wrap) {
  MutexHolder holder(_lock);
  do_set_wrap_u(wrap);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_wrap_v
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_wrap_v(Texture::WrapMode wrap) {
  MutexHolder holder(_lock);
  do_set_wrap_v(wrap);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_wrap_w
//       Access: Published
//  Description: The W wrap direction is only used for 3-d textures.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_wrap_w(Texture::WrapMode wrap) {
  MutexHolder holder(_lock);
  do_set_wrap_w(wrap);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_minfilter(Texture::FilterType filter) {
  MutexHolder holder(_lock);
  do_set_minfilter(filter);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_magfilter(Texture::FilterType filter) {
  MutexHolder holder(_lock);
  do_set_magfilter(filter);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_anisotropic_degree
//       Access: Published
//  Description: Specifies the level of anisotropic filtering to apply
//               to the texture.  Set this 0 to indicate the default
//               value, which is specified in the
//               texture-anisotropic-degree config variable.
//
//               To explicitly disable anisotropic filtering, set this
//               value to 1.  To explicitly enable anisotropic
//               filtering, set it to a value higher than 1; larger
//               numbers indicate greater degrees of filtering.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_anisotropic_degree(int anisotropic_degree) {
  MutexHolder holder(_lock);
  do_set_anisotropic_degree(anisotropic_degree);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_border_color
//       Access: Published
//  Description: Specifies the solid color of the texture's border.
//               Some OpenGL implementations use a border for tiling
//               textures; in Panda, it is only used for specifying
//               the clamp color.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_border_color(const Colorf &color) {
  MutexHolder holder(_lock);
  do_set_border_color(color);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_compression
//       Access: Published
//  Description: Requests that this particular Texture be compressed
//               when it is loaded into texture memory.  
//
//               This refers to the internal compression of the
//               texture image within texture memory; it is not
//               related to jpeg or png compression, which are disk
//               file compression formats.  The actual disk file that
//               generated this texture may be stored in a compressed
//               or uncompressed format supported by Panda; it will be
//               decompressed on load, and then recompressed by the
//               graphics API if this parameter is not CM_off.
//
//               If the GSG does not support this texture compression
//               mode, the texture will silently be loaded
//               uncompressed.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_compression(Texture::CompressionMode compression) {
  MutexHolder holder(_lock);
  do_set_compression(compression);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_render_to_texture
//       Access: Published
//  Description: Sets a flag on the texture that indicates whether the
//               texture is intended to be used as a direct-render
//               target, by binding a framebuffer to a texture and
//               rendering directly into the texture.
//
//               This controls some low-level choices made about the
//               texture object itself.  For instance, compressed
//               textures are disallowed when this flag is set true.
//
//               Normally, a user should not need to set this flag
//               directly; it is set automatically by the low-level
//               display code when a texture is bound to a
//               framebuffer.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_render_to_texture(bool render_to_texture) {
  _render_to_texture = render_to_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_wrap_u
//       Access: Published
//  Description: Returns the wrap mode of the texture in the U
//               direction.
////////////////////////////////////////////////////////////////////
INLINE Texture::WrapMode Texture::
get_wrap_u() const {
  return _wrap_u;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_wrap_v
//       Access: Published
//  Description: Returns the wrap mode of the texture in the V
//               direction.
////////////////////////////////////////////////////////////////////
INLINE Texture::WrapMode Texture::
get_wrap_v() const {
  return _wrap_v;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_wrap_w
//       Access: Published
//  Description: Returns the wrap mode of the texture in the W
//               direction.  This is the depth direction of 3-d
//               textures.
////////////////////////////////////////////////////////////////////
INLINE Texture::WrapMode Texture::
get_wrap_w() const {
  return _wrap_w;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_minfilter
//       Access: Published
//  Description: Returns the filter mode of the texture for
//               minification.  If this is one of the mipmap
//               constants, then the texture requires mipmaps.  This
//               may return FT_default; see also
//               get_effective_minfilter().
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType Texture::
get_minfilter() const {
  return _minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_magfilter
//       Access: Published
//  Description: Returns the filter mode of the texture for
//               magnification.  The mipmap constants are invalid
//               here.  This may return FT_default; see also
//               get_effective_minfilter().
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType Texture::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_anisotropic_degree
//       Access: Published
//  Description: Returns the degree of anisotropic filtering that
//               should be applied to the texture.  This value may
//               return 0, indicating the default value; see also
//               get_effective_anisotropic_degree.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_effective_anisotropic_degree
//       Access: Published
//  Description: Returns the degree of anisotropic filtering that
//               should be applied to the texture.  This value will
//               normally not return 0, unless there is an error in
//               the config file.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_effective_anisotropic_degree() const {
  if (_anisotropic_degree != 0) {
    return _anisotropic_degree;
  }
  return texture_anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_border_color
//       Access: Published
//  Description: Returns the solid color of the texture's border.
//               Some OpenGL implementations use a border for tiling
//               textures; in Panda, it is only used for specifying
//               the clamp color.
////////////////////////////////////////////////////////////////////
INLINE Colorf Texture::
get_border_color() const {
  return _border_color;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_compression
//       Access: Published
//  Description: Returns the compression mode requested for this
//               particular texture, or CM_off if the texture is not
//               to be compressed.
//
//               If a value other than CM_off is returned, this is
//               not a guarantee that the texture is actually
//               successfully compressed on the GSG.  It may be that
//               the GSG does not support the requested compression
//               mode, in which case the texture may actually be
//               stored uncompressed in texture memory.
////////////////////////////////////////////////////////////////////
INLINE Texture::CompressionMode Texture::
get_compression() const {
  return _compression;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_compression
//       Access: Published
//  Description: Returns true if the texture indicates it wants to be
//               compressed, either with CM_on or higher, or
//               CM_default and compressed-textures is true.
//
//               If true returned, this is not a guarantee that the
//               texture is actually successfully compressed on the
//               GSG.  It may be that the GSG does not support the
//               requested compression mode, in which case the texture
//               may actually be stored uncompressed in texture
//               memory.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_compression() const {
  MutexHolder holder(_lock);
  return do_has_compression();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_render_to_texture
//       Access: Published
//  Description: Returns a flag on the texture that indicates whether the
//               texture is intended to be used as a direct-render
//               target, by binding a framebuffer to a texture and
//               rendering directly into the texture.
//
//               Normally, a user should not need to set this flag
//               directly; it is set automatically by the low-level
//               display code when a texture is bound to a
//               framebuffer.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_render_to_texture() const {
  return _render_to_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::uses_mipmaps
//       Access: Public
//  Description: Returns true if the minfilter settings on this
//               texture indicate the use of mipmapping, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
uses_mipmaps() const {
  return is_mipmap(get_effective_minfilter());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_quality_level
//       Access: Public
//  Description: Sets a hint to the renderer about the desired
//               performance / quality tradeoff for this particular
//               texture.  This is most useful for the tinydisplay
//               software renderer; for normal, hardware-accelerated
//               renderers, this may have little or no effect.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_quality_level(Texture::QualityLevel quality_level) {
  MutexHolder holder(_lock);
  do_set_quality_level(quality_level);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_quality_level
//       Access: Public
//  Description: Returns the current quality_level hint.  See
//               set_quality_level().  This value may return
//               QL_default; see get_effective_quality_level().
////////////////////////////////////////////////////////////////////
INLINE Texture::QualityLevel Texture::
get_quality_level() const {
  return _quality_level;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_effective_quality_level
//       Access: Public
//  Description: Returns the current quality_level hint, or the global
//               default quality_level if this texture doesn't specify
//               a quality level.  This value will not normally return
//               QL_default (unless there is an error in the config
//               file)
////////////////////////////////////////////////////////////////////
INLINE Texture::QualityLevel Texture::
get_effective_quality_level() const {
  if (_quality_level == QL_default) {
    return texture_quality_level;
  }
  return _quality_level;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_num_mipmap_levels
//       Access: Published
//  Description: Returns the number of mipmap levels that should be
//               defined for this texture, given the texture's size.
//
//               Note that this returns a number appropriate for
//               mipmapping, even if the texture does not currently
//               have mipmapping enabled.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_expected_num_mipmap_levels() const {
  MutexHolder holder(_lock);
  return do_get_expected_num_mipmap_levels();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_x_size
//       Access: Published
//  Description: Returns the x_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_expected_mipmap_x_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_expected_mipmap_x_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_y_size
//       Access: Published
//  Description: Returns the y_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_expected_mipmap_y_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_expected_mipmap_y_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_z_size
//       Access: Published
//  Description: Returns the z_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_expected_mipmap_z_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_expected_mipmap_z_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_ram_image
//       Access: Published
//  Description: Returns true if the Texture has its image contents
//               available in main RAM, false if it exists only in
//               texture memory or in the prepared GSG context.
//
//               Note that this has nothing to do with whether
//               get_ram_image() will fail or not.  Even if
//               has_ram_image() returns false, get_ram_image() may
//               still return a valid RAM image, because
//               get_ram_image() will automatically load the texture
//               from disk if necessary.  The only thing
//               has_ram_image() tells you is whether the texture is
//               available right now without hitting the disk first.
//
//               Note also that if an application uses only one GSG,
//               it may appear that has_ram_image() returns true if
//               the texture has not yet been loaded by the GSG, but
//               this correlation is not true in general and should
//               not be depended on.  Specifically, if an application
//               ever uses multiple GSG's in its lifetime (for
//               instance, by opening more than one window, or by
//               closing its window and opening another one later),
//               then has_ram_image() may well return false on
//               textures that have never been loaded on the current
//               GSG.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_ram_image() const {
  MutexHolder holder(_lock);
  return do_has_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_uncompressed_ram_image
//       Access: Published
//  Description: Returns true if the Texture has its image contents
//               available in main RAM and is uncompressed, false
//               otherwise.  See has_ram_image().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_uncompressed_ram_image() const {
  MutexHolder holder(_lock);
  return do_has_uncompressed_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::might_have_ram_image
//       Access: Published
//  Description: Returns true if the texture's image contents are
//               currently available in main RAM, or there is reason
//               to believe it can be loaded on demand.  That is, this
//               function returns a "best guess" as to whether
//               get_ram_image() will succeed without actually calling
//               it first.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
might_have_ram_image() const {
  MutexHolder holder(_lock);
  return (do_has_ram_image() || !_fullpath.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image_size
//       Access: Published
//  Description: Returns the number of bytes used by the in-memory
//               image, or 0 if there is no in-memory image.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_ram_image_size() const {
  MutexHolder holder(_lock);
  return do_get_ram_image_size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_page_size
//       Access: Published
//  Description: Returns the number of bytes used by the in-memory
//               image per page, or 0 if there is no in-memory image.
//
//               For a non-compressed texture, this is the same as
//               get_expected_ram_page_size().  For a compressed
//               texture, this may be a smaller value.  (We do assume
//               that all pages will be the same size on a compressed
//               texture).
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_ram_page_size() const {
  MutexHolder holder(_lock);
  if (_ram_image_compression == CM_off || _ram_images.empty()) {
    return do_get_expected_ram_page_size();
  } else {
    return _ram_images[0]._page_size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_ram_image_size
//       Access: Published
//  Description: Returns the number of bytes that *ought* to be used
//               by the in-memory image, based on the texture
//               parameters.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_expected_ram_image_size() const {
  MutexHolder holder(_lock);
  return do_get_expected_ram_image_size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_ram_page_size
//       Access: Published
//  Description: Returns the number of bytes that should be used per
//               each Z page of the 3-d texture.  For a 2-d or 1-d
//               texture, this is the same as
//               get_expected_ram_image_size().
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_expected_ram_page_size() const {
  MutexHolder holder(_lock);
  return do_get_expected_ram_page_size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image
//       Access: Published
//  Description: Returns the system-RAM image data associated with the
//               texture.  If the texture does not currently have an
//               associated RAM image, and the texture was generated
//               by loading an image from a disk file (the most common
//               case), this forces the reload of the same texture.
//               This can happen if keep_texture_ram is configured to
//               false, and we have previously prepared this texture
//               with a GSG.
//
//               Note that it is not correct to call has_ram_image()
//               first to test whether this function will fail.  A
//               false return value from has_ram_image() indicates
//               only that get_ram_image() may need to reload the
//               texture from disk, which it will do automatically.
//               However, you can call might_have_ram_image(), which
//               will return true if the ram image exists, or there is
//               a reasonable reason to believe it can be loaded.
//
//               On the other hand, it is possible that the texture
//               cannot be found on disk or is otherwise unavailable.
//               If that happens, this function will return NULL.
//               There is no way to predict with 100% accuracy whether
//               get_ram_image() will return NULL without calling it
//               first; might_have_ram_image() is the closest.
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar Texture::
get_ram_image() {
  MutexHolder holder(_lock);
  return do_get_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image_compression
//       Access: Published
//  Description: Returns the compression mode in which the ram image
//               is already stored pre-compressed.  If this is other
//               than CM_off, you cannot rely on the contents of the
//               ram image to be anything predicatable (it will not be
//               an array of x by y pixels, and it probably won't have
//               the same length as get_expected_ram_image_size()).
////////////////////////////////////////////////////////////////////
INLINE Texture::CompressionMode Texture::
get_ram_image_compression() const {
  return _ram_image_compression;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::modify_ram_image
//       Access: Published
//  Description: Returns a modifiable pointer to the system-RAM image.
//               This assumes the RAM image should be uncompressed.
//               If the RAM image has been dumped, or is stored
//               compressed, creates a new one.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar Texture::
modify_ram_image() {
  MutexHolder holder(_lock);
  ++_image_modified;
  return do_modify_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_uncompressed_ram_image
//       Access: Published
//  Description: Returns the system-RAM image associated with the
//               texture, in an uncompressed form if at all possible.
//
//               If get_ram_image_compression() is CM_off, then the
//               system-RAM image is already uncompressed, and this
//               returns the same thing as get_ram_image().
//
//               If get_ram_image_compression() is anything else, then
//               the system-RAM image is compressed.  In this case,
//               the image will be reloaded from the *original* file
//               (not from the cache), in the hopes that an
//               uncompressed image will be found there.
//
//               If an uncompressed image cannot be found, returns
//               NULL.
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar Texture::
get_uncompressed_ram_image() {
  MutexHolder holder(_lock);
  return do_get_uncompressed_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_ram_image
//       Access: Published
//  Description: Discards the current system-RAM image for the
//               texture, if any, and allocates a new buffer of the
//               appropriate size.  Returns the new buffer.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar Texture::
make_ram_image() {
  MutexHolder holder(_lock);
  ++_image_modified;
  return do_make_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_image
//       Access: Published
//  Description: Discards the current system-RAM image.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_ram_image() {
  MutexHolder holder(_lock);
  do_clear_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_keep_ram_image
//       Access: Published
//  Description: Sets the flag that indicates whether this Texture is
//               eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.
//
//               This will be false for most textures, which can
//               reload their images if needed by rereading the input
//               file.  However, textures that were generated
//               dynamically and cannot be easily reloaded will want
//               to set this flag to true, so that the texture will
//               always keep its image copy around.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_keep_ram_image(bool keep_ram_image) {
  _keep_ram_image = keep_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::compress_ram_image
//       Access: Published
//  Description: Attempts to compress the texture's RAM image
//               internally, to a format supported by the indicated
//               GSG.  In order for this to work, the squish library
//               must have been compiled into Panda.
//
//               If compression is CM_on, then an appropriate
//               compression method that is supported by the indicated
//               GSG is automatically chosen.  If the GSG pointer is
//               NULL, any of the standard DXT1/3/5 compression
//               methods will be used, regardless of whether it is
//               supported.
//
//               If compression is any specific compression method,
//               that method is used regardless of whether the GSG
//               supports it.
//
//               quality_level determines the speed/quality tradeoff
//               of the compression.  If it is QL_default, the
//               texture's own quality_level parameter is used.
//
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
compress_ram_image(Texture::CompressionMode compression,
                   Texture::QualityLevel quality_level,
                   GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);
  if (do_compress_ram_image(compression, quality_level, gsg)) {
    ++_image_modified;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::uncompress_ram_image
//       Access: Published
//  Description: Attempts to uncompress the texture's RAM image
//               internally.  In order for this to work, the squish
//               library must have been compiled into Panda, and the
//               ram image must be compressed in a format supported by
//               squish.
//
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
uncompress_ram_image() {
  MutexHolder holder(_lock);
  if (do_uncompress_ram_image()) {
    ++_image_modified;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_num_ram_mipmap_images
//       Access: Published
//  Description: Returns the maximum number of mipmap level images
//               available in system memory.  The actual number may be
//               less than this (that is, there might be gaps in the
//               sequence); use has_ram_mipmap_image() to verify each
//               level.
//
//               Also see get_num_loadable_ram_mipmap_images().
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_num_ram_mipmap_images() const {
  MutexHolder holder(_lock);
  return _ram_images.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_ram_mipmap_image
//       Access: Published
//  Description: Returns true if the Texture has the nth mipmap level
//               available in system memory, false otherwise.  If the
//               texture's minfilter mode requires mipmapping (see
//               uses_mipmaps()), and all the texture's mipmap levels
//               are not available when the texture is rendered, they
//               will be generated automatically.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_ram_mipmap_image(int n) const {
  MutexHolder holder(_lock);
  return do_has_ram_mipmap_image(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_all_ram_mipmap_images
//       Access: Published
//  Description: Returns true if all expected mipmap levels have been
//               defined and exist in the system RAM, or false if even
//               one mipmap level is missing.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_all_ram_mipmap_images() const {
  MutexHolder holder(_lock);
  return do_has_all_ram_mipmap_images();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_mipmap_image_size
//       Access: Published
//  Description: Returns the number of bytes used by the in-memory
//               image for mipmap level n, or 0 if there is no
//               in-memory image for this mipmap level.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_ram_mipmap_image_size(int n) const {
  MutexHolder holder(_lock);
  if (n >= 0 && n < (int)_ram_images.size()) {
    return _ram_images[n]._image.size();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_mipmap_page_size
//       Access: Published
//  Description: Returns the number of bytes used by the in-memory
//               image per page for mipmap level n, or 0 if there is
//               no in-memory image for this mipmap level.
//
//               For a non-compressed texture, this is the same as
//               get_expected_ram_mipmap_page_size().  For a compressed
//               texture, this may be a smaller value.  (We do assume
//               that all pages will be the same size on a compressed
//               texture).
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_ram_mipmap_page_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_ram_mipmap_page_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_ram_mipmap_image_size
//       Access: Published
//  Description: Returns the number of bytes that *ought* to be used
//               by the in-memory image for mipmap level n, based on
//               the texture parameters.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_expected_ram_mipmap_image_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_expected_ram_mipmap_image_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_ram_mipmap_page_size
//       Access: Published
//  Description: Returns the number of bytes that should be used per
//               each Z page of the 3-d texture, for mipmap level n.
//               For a 2-d or 1-d texture, this is the same as
//               get_expected_ram_mipmap_image_size(n).
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_expected_ram_mipmap_page_size(int n) const {
  MutexHolder holder(_lock);
  return do_get_expected_ram_mipmap_page_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::modify_ram_mipmap_image
//       Access: Published
//  Description: Returns a modifiable pointer to the system-RAM image
//               for the nth mipmap level.  This assumes the RAM image
//               is uncompressed; if this is not the case, raises an
//               assertion.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar Texture::
modify_ram_mipmap_image(int n) {
  MutexHolder holder(_lock);
  ++_image_modified;
  return do_modify_ram_mipmap_image(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_ram_mipmap_image
//       Access: Published
//  Description: Discards the current system-RAM image for the
//               nth mipmap level, if any, and allocates a new buffer
//               of the appropriate size.  Returns the new buffer.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar Texture::
make_ram_mipmap_image(int n) {
  MutexHolder holder(_lock);
  ++_image_modified;
  return do_make_ram_mipmap_image(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_mipmap_image
//       Access: Published
//  Description: Replaces the current system-RAM image for the
//               indicated mipmap level with the new data.  If
//               compression is not CM_off, it indicates that the new
//               data is already pre-compressed in the indicated
//               format.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_ram_mipmap_image(int n, CPTA_uchar image, size_t page_size) {
  MutexHolder holder(_lock);
  do_set_ram_mipmap_image(n, image, page_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_mipmap_images
//       Access: Published
//  Description: Discards the current system-RAM image for all
//               mipmap levels, except level 0 (the base image).
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_ram_mipmap_images() {
  MutexHolder holder(_lock);
  ++_image_modified;
  do_clear_ram_mipmap_images();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::generate_ram_mipmap_images
//       Access: Published
//  Description: Automatically fills in the n mipmap levels of the
//               Texture, based on the texture's source image.  This
//               requires the texture's uncompressed ram image to be
//               available in system memory.  If it is not already, it
//               will be fetched if possible.
//
//               This call is not normally necessary, since the mipmap
//               levels will be generated automatically if needed.
//               But there may be certain cases in which you would
//               like to call this explicitly.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
generate_ram_mipmap_images() {
  MutexHolder holder(_lock);
  ++_image_modified;
  do_generate_ram_mipmap_images();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_simple_x_size
//       Access: Published
//  Description: Returns the width of the "simple" image in texels.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_simple_x_size() const {
  return _simple_x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_simple_y_size
//       Access: Published
//  Description: Returns the height of the "simple" image in texels.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_simple_y_size() const {
  return _simple_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_simple_ram_image
//       Access: Published, Virtual
//  Description: Returns true if the Texture has a "simple" image
//               available in main RAM.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_simple_ram_image() const {
  MutexHolder holder(_lock);
  return !_simple_ram_image._image.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_simple_ram_image_size
//       Access: Published
//  Description: Returns the number of bytes used by the "simple"
//               image, or 0 if there is no simple image.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_simple_ram_image_size() const {
  MutexHolder holder(_lock);
  return _simple_ram_image._image.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_simple_ram_image
//       Access: Published
//  Description: Returns the image data associated with the "simple"
//               texture image.  This is provided for some textures as
//               an option to display while the main texture image is
//               being loaded from disk.
//
//               Unlike get_ram_image(), this function will always
//               return immediately.  Either the simple image is
//               available, or it is not.
//
//               The "simple" image is always 4 components, 1 byte
//               each, regardless of the parameters of the full
//               texture.  The simple image is only supported for
//               ordinary 2-d textures.
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar Texture::
get_simple_ram_image() const {
  MutexHolder holder(_lock);
  return _simple_ram_image._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_simple_ram_image
//       Access: Published
//  Description: Replaces the internal "simple" texture image.  This
//               can be used as an option to display while the main
//               texture image is being loaded from disk.  It is
//               normally a very small image, 16x16 or smaller (and
//               maybe even 1x1), that is designed to give just enough
//               sense of color to serve as a placeholder until the
//               full texture is available.
//
//               The "simple" image is always 4 components, 1 byte
//               each, regardless of the parameters of the full
//               texture.  The simple image is only supported for
//               ordinary 2-d textures.
//
//               Also see generate_simple_ram_image(),
//               modify_simple_ram_image(), and
//               new_simple_ram_image().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_simple_ram_image(CPTA_uchar image, int x_size, int y_size) {
  MutexHolder holder(_lock);
  do_set_simple_ram_image(image, x_size, y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_simple_ram_image
//       Access: Published
//  Description: Discards the current "simple" image.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_simple_ram_image() {
  MutexHolder holder(_lock);
  do_clear_simple_ram_image();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_properties_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the texture properties
//               (unrelated to the image) are modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Texture::
get_properties_modified() const {
  return _properties_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_image_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the texture image data
//               (including mipmap levels) are modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Texture::
get_image_modified() const {
  return _image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_simple_image_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the texture's "simple"
//               image data is modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Texture::
get_simple_image_modified() const {
  return _simple_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_filename
//       Access: Published
//  Description: Sets the name of the file that contains the image's
//               contents.  Normally, this is set automatically when
//               the image is loaded, for instance via
//               Texture::read().
//
//               The Texture's get_name() function used to return
//               the filename, but now returns just the basename
//               (without the extension), which is a more useful name
//               for identifying an image in show code.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_filename(const Filename &filename) {
  MutexHolder holder(_lock);
  _filename = filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_filename
//       Access: Published
//  Description: Removes the alpha filename, if it was previously set.
//               See set_filename().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_filename() {
  MutexHolder holder(_lock);
  _filename = Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_alpha_filename
//       Access: Published
//  Description: Sets the name of the file that contains the image's
//               alpha channel contents.  Normally, this is set
//               automatically when the image is loaded, for instance
//               via Texture::read().
//
//               The Texture's get_filename() function returns the
//               name of the image file that was loaded into the
//               buffer.  In the case where a texture specified two
//               separate files to load, a 1- or 3-channel color image
//               and a 1-channel alpha image, this Filename is update
//               to contain the name of the image file that was loaded
//               into the buffer's alpha channel.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_alpha_filename(const Filename &alpha_filename) {
  MutexHolder holder(_lock);
  _alpha_filename = alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_alpha_filename
//       Access: Published
//  Description: Removes the alpha filename, if it was previously set.
//               See set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_alpha_filename() {
  MutexHolder holder(_lock);
  _alpha_filename = Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_fullpath
//       Access: Published
//  Description: Sets the full pathname to the file that contains the
//               image's contents, as found along the search path.
//               Normally, this is set automatically when the image is
//               loaded, for instance via Texture::read().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_fullpath(const Filename &fullpath) {
  MutexHolder holder(_lock);
  _fullpath = fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_fullpath
//       Access: Published
//  Description: Removes the alpha fullpath, if it was previously set.
//               See set_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_fullpath() {
  MutexHolder holder(_lock);
  _fullpath = Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_alpha_fullpath
//       Access: Published
//  Description: Sets the full pathname to the file that contains the
//               image's alpha channel contents, as found along the
//               search path.  Normally, this is set automatically
//               when the image is loaded, for instance via
//               Texture::read().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  MutexHolder holder(_lock);
  _alpha_fullpath = alpha_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_alpha_fullpath
//       Access: Published
//  Description: Removes the alpha fullpath, if it was previously set.
//               See set_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void Texture::
clear_alpha_fullpath() {
  MutexHolder holder(_lock);
  _alpha_fullpath = Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_x_size
//       Access: Published
//  Description: Changes the x size indicated for the texture.  This
//               also implicitly unloads the texture if it has already
//               been loaded.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_x_size(int x_size) {
  MutexHolder holder(_lock);
  do_set_x_size(x_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_y_size
//       Access: Published
//  Description: Changes the y size indicated for the texture.  This
//               also implicitly unloads the texture if it has already
//               been loaded.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_y_size(int y_size) {
  MutexHolder holder(_lock);
  do_set_y_size(y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_z_size
//       Access: Published
//  Description: Changes the z size indicated for the texture.  This
//               also implicitly unloads the texture if it has already
//               been loaded.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_z_size(int z_size) {
  MutexHolder holder(_lock);
  do_set_z_size(z_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_format
//       Access: Published
//  Description: Changes the format value for the texture components.
//               This implicitly sets num_components as well.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_format(Texture::Format format) {
  MutexHolder holder(_lock);
  do_set_format(format);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_component_type
//       Access: Published
//  Description: Changes the data value for the texture components.
//               This implicitly sets component_width as well.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_component_type(Texture::ComponentType component_type) {
  MutexHolder holder(_lock);
  do_set_component_type(component_type);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_loaded_from_image
//       Access: Published
//  Description: Sets the flag that indicates the texture has been
//               loaded from a disk file or PNMImage.  You should also
//               ensure the filename has been set correctly.  When
//               this flag is true, the texture may be automatically
//               reloaded when its ram image needs to be replaced.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_loaded_from_image() {
  _loaded_from_image = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_loaded_from_image
//       Access: Published
//  Description: Returns the flag that indicates the texture has been
//               loaded from a disk file or PNMImage.  See
//               set_loaded_from_image().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_loaded_from_image() const {
  return _loaded_from_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_loaded_from_txo
//       Access: Published
//  Description: Sets the flag that indicates the texture has been
//               loaded from a txo file.  You probably shouldn't be
//               setting this directly; it is set automatically when a
//               Texture is loaded.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_loaded_from_txo() {
  _loaded_from_txo = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_loaded_from_txo
//       Access: Published
//  Description: Returns the flag that indicates the texture has been
//               loaded from a txo file.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_loaded_from_txo() const {
  return _loaded_from_txo;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_match_framebuffer_format
//       Access: Public
//  Description: Returns true if the special flag was set that
//               indicates to the GSG that the Texture's format should
//               be chosen to exactly match the framebuffer's format,
//               presumably because the application intends to copy
//               image data from the framebuffer into the Texture (or
//               vice-versa).
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_match_framebuffer_format() const {
  return _match_framebuffer_format;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_match_framebuffer_format
//       Access: Public
//  Description: Sets the special flag that, if true, indicates to the
//               GSG that the Texture's format should be chosen to
//               exactly match the framebuffer's format, presumably
//               because the application intends to copy image data
//               from the framebuffer into the Texture (or
//               vice-versa).
//
//               This sets only the graphics card's idea of the
//               texture format; it is not related to the
//               system-memory format.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_match_framebuffer_format(bool flag) {
  _match_framebuffer_format = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_post_load_store_cache
//       Access: Public
//  Description: Returns the setting of the post_load_store_cache
//               flag.  See set_post_load_store_cache().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_post_load_store_cache() const {
  return _post_load_store_cache;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_post_load_store_cache
//       Access: Public
//  Description: Sets the post_load_store_cache flag.  When this is
//               set, the next time the texture is loaded on a GSG, it
//               will automatically extract its RAM image from the GSG
//               and save it to the global BamCache.
//
//               This is used to store compressed RAM images in the
//               BamCache.  This flag should not be set explicitly; it
//               is set automatically by the TexturePool when
//               model-cache-compressed-textures is set true.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_post_load_store_cache(bool flag) {
  _post_load_store_cache = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_ram_image_size
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
do_get_ram_image_size() const {
  if (_ram_images.empty()) {
    return 0;
  }
  return _ram_images[0]._image.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_has_ram_mipmap_image
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
do_has_ram_mipmap_image(int n) const {
  return (n >= 0 && n < (int)_ram_images.size() && !_ram_images[n]._image.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_ram_image_size
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
do_get_expected_ram_image_size() const {
  return do_get_expected_ram_page_size() * (size_t)_z_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_ram_page_size
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
do_get_expected_ram_page_size() const {
  return (size_t)(_x_size * _y_size * _num_components * _component_width);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_ram_mipmap_image_size
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
do_get_expected_ram_mipmap_image_size(int n) const {
  return do_get_expected_ram_mipmap_page_size(n) * (size_t)do_get_expected_mipmap_z_size(n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_ram_mipmap_page_size
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
do_get_expected_ram_mipmap_page_size(int n) const {
  return (size_t)(do_get_expected_mipmap_x_size(n) * do_get_expected_mipmap_y_size(n) * _num_components * _component_width);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_clear_ram_image
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void Texture::
do_clear_ram_image() {
  _ram_image_compression = CM_off;
  _ram_images.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_unscaled_byte
//       Access: Private, Static
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value is assumed to be in the range
//               0-255.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_unscaled_byte(unsigned char *&p, int value) {
  (*p++) = (uchar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_unscaled_short
//       Access: Private, Static
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value is assumed to be in the range
//               0-65535.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_scaled_byte
//       Access: Private, Static
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_scaled_byte(unsigned char *&p, int value, double scale) {
  store_unscaled_byte(p, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_scaled_short
//       Access: Private, Static
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_scaled_short(unsigned char *&p, int value, double scale) {
  store_unscaled_short(p, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_unsigned_byte
//       Access: Private, Static
//  Description: This is used by store() to retrieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned bytes.
////////////////////////////////////////////////////////////////////
INLINE double Texture::
get_unsigned_byte(const unsigned char *&p) {
  return (double)(*p++) / 255.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_unsigned_short
//       Access: Private, Static
//  Description: This is used by store() to retrieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned shorts.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_txo_filename
//       Access: Private, Static
//  Description: Returns true if the indicated filename ends in .txo
//               or .txo.pz, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
is_txo_filename(const Filename &fullpath) {
  string extension = fullpath.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz") {
    extension = Filename(fullpath.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  return (extension == "txo");
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_dds_filename
//       Access: Private, Static
//  Description: Returns true if the indicated filename ends in .dds
//               or .dds.pz, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
is_dds_filename(const Filename &fullpath) {
  string extension = fullpath.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz") {
    extension = Filename(fullpath.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  return (downcase(extension) == "dds");
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_textures_power_2
//       Access: Published, Static
//  Description: Set this flag to ATS_none, ATS_up, or ATS_down
//               to control the scaling of textures.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_textures_power_2(AutoTextureScale scale) {
  _textures_power_2 = scale;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_textures_power_2
//       Access: Published, Static
//  Description: This flag returns ATS_none, ATS_up, or ATS_down
//               and controls the scaling of textures.  It is
//               initialized from the config variable of the same
//               name, but it can be subsequently adjusted.
////////////////////////////////////////////////////////////////////
INLINE AutoTextureScale Texture::
get_textures_power_2() {
  if (_textures_power_2 == ATS_UNSPECIFIED) {
    return textures_power_2;
  } else {
    return _textures_power_2;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::have_textures_power_2
//       Access: Published, Static
//  Description: If true, then get_textures_power_2 has been
//               set using set_textures_power_2.
//               If false, then get_textures_power_2 simply
//               returns the config variable of the same name.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
have_textures_power_2() {
  return (_textures_power_2 != ATS_UNSPECIFIED);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::RamImage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Texture::RamImage::
RamImage() :
  _page_size(0),
  _pointer_image(NULL)
{
}
