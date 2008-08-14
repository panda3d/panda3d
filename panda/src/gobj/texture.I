// Filename: texture.I
// Created by:  drose (05Feb99)
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  return do_write(fullpath, z, n, write_pages, write_mipmaps);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published
//  Description: Replaces the texture with the indicated image.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
load(const PNMImage &pnmimage) {
  ReMutexHolder holder(_lock);
  clear();
  return do_load_one(pnmimage, get_name(), 0, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published
//  Description: Stores the indicated image in the given page and
//               mipmap level.  See read().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
load(const PNMImage &pnmimage, int z, int n) {
  ReMutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  return do_load_one(pnmimage, get_name(), z, n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store
//       Access: Published
//  Description: Saves the texture to the indicated PNMImage, but does
//               not write it to disk.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
store(PNMImage &pnmimage) const {
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  return do_store_one(pnmimage, z, n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_filename
//       Access: Published
//  Description: Returns true if the filename has been set and
//               is available.  See set_filename().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_filename() const {
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  if (x > _x_size) x = _x_size;
  if (y > _y_size) y = _y_size;
  if (z > _z_size) z = _z_size;
  _pad_x_size = x;
  _pad_y_size = y;
  _pad_z_size = z;
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
//               constants, then the texture requires mipmaps.
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
//               here.
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType Texture::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_anisotropic_degree
//       Access: Published
//  Description: Returns the degree of anisotropic filtering that
//               should be applied to the texture.  Normally, this is
//               1, to indicate that anisotropic filtering should be
//               disabled.  If this is a number higher than 1,
//               anisotropic filtering should be enabled (if the
//               rendering backend supports it).
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_anisotropic_degree() const {
  return _anisotropic_degree;
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
  if (_compression == CM_default) {
    return compressed_textures;
  } else {
    return (_compression != CM_off);
  }
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
  return is_mipmap(get_minfilter());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_quality_level
//       Access: Public
//  Description: Returns the current quality_level hint.  See
//               set_quality_level().
////////////////////////////////////////////////////////////////////
INLINE Texture::QualityLevel Texture::
get_quality_level() const {
  return _quality_level;
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
  ReMutexHolder holder(_lock);
  return (has_ram_image() || has_filename());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image_size
//       Access: Published
//  Description: Returns the number of bytes used by the in-memory
//               image, or 0 if there is no in-memory image.
////////////////////////////////////////////////////////////////////
INLINE size_t Texture::
get_ram_image_size() const {
  ReMutexHolder holder(_lock);
  if (_ram_images.empty()) {
    return 0;
  }
  return _ram_images[0]._image.size();
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
  ReMutexHolder holder(_lock);
  if (_ram_image_compression == CM_off || _ram_images.empty()) {
    return get_expected_ram_page_size();
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
  ReMutexHolder holder(_lock);
  return get_expected_ram_page_size() * (size_t)_z_size;
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
  ReMutexHolder holder(_lock);
  return (size_t)(_x_size * _y_size * _num_components * _component_width);
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
  ReMutexHolder holder(_lock);
  do_modify_ram_image();
  ++_image_modified;
  return _ram_images[0]._image;
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
  ReMutexHolder holder(_lock);
  ++_image_modified;
  do_make_ram_image();
  return _ram_images[0]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_keep_ram_image
//       Access: Published
//  Description: Sets the flag that indicates whether this Texture is
//               eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.
//
//               This will be true for most textures, which can reload
//               their images if needed by rereading the input file.
//               However, textures that were generated dynamically and
//               cannot be easily reloaded will want to set this flag
//               to true, so that the texture will always keep its
//               image copy around.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_keep_ram_image(bool keep_ram_image) {
  _keep_ram_image = keep_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_num_ram_mipmap_images
//       Access: Published
//  Description: Returns the maximum number of mipmap level images
//               available in system memory.  The actual number may be
//               less than this; use has_ram_mipmap_image() to verify
//               each level.
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_num_ram_mipmap_images() const {
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  return (n >= 0 && n < (int)_ram_images.size() && !_ram_images[n]._image.empty());
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  if (_ram_image_compression != CM_off) {
    if (n >= 0 && n < (int)_ram_images.size()) {
      return _ram_images[n]._page_size;
    }
    return 0;
  } else {
    return get_expected_ram_mipmap_page_size(n);
  }
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
  ReMutexHolder holder(_lock);
  return get_expected_ram_mipmap_page_size(n) * (size_t)get_expected_mipmap_z_size(n);
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
  ReMutexHolder holder(_lock);
  return (size_t)(get_expected_mipmap_x_size(n) * get_expected_mipmap_y_size(n) * _num_components * _component_width);
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
  ReMutexHolder holder(_lock);
  do_modify_ram_mipmap_image(n);
  ++_image_modified;
  return _ram_images[n]._image;
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  return _simple_ram_image._image;
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  if (_x_size != x_size) {
    _x_size = x_size;
    ++_image_modified;
    clear_ram_image();
    set_pad_size();
  }
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
  ReMutexHolder holder(_lock);
  if (_y_size != y_size) {
    nassertv(_texture_type != Texture::TT_1d_texture || y_size == 1);
    _y_size = y_size;
    ++_image_modified;
    clear_ram_image();
    set_pad_size();
  }
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
  ReMutexHolder holder(_lock);
  if (_z_size != z_size) {
    nassertv(_texture_type == Texture::TT_3d_texture ||
             (_texture_type == Texture::TT_cube_map && z_size == 6) ||
             (z_size == 1));
    _z_size = z_size;
    ++_image_modified;
    clear_ram_image();
    set_pad_size();
  }
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

