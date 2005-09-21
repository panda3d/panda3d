// Filename: texture.I
// Created by:  drose (05Feb99)
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
//     Function: Texture::has_filename
//       Access: Published
//  Description: Returns true if the filename has been set and
//               is available.  See set_filename().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_filename() const {
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
  return _image.size();
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
  return (size_t)(_x_size * _y_size * _num_components * _component_width);
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
  if (_x_size != x_size) {
    _x_size = x_size;
    clear_ram_image();
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
  if (_y_size != y_size) {
    nassertv(_texture_type != Texture::TT_1d_texture || y_size == 1);
    _y_size = y_size;
    clear_ram_image();
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
  if (_z_size != z_size) {
    nassertv(_texture_type == Texture::TT_3d_texture ||
             (_texture_type == Texture::TT_cube_map && z_size == 6) ||
             (z_size == 1));
    _z_size = z_size;
    clear_ram_image();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_loaded_from_disk
//       Access: Published
//  Description: Sets the flag that indicates the texture has been
//               loaded from a disk file.  You should also ensure the
//               filename has been set correctly.  When this flag is
//               true, the texture may be automatically reloaded when
//               its ram image needs to be replaced.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_loaded_from_disk() {
  _loaded_from_disk = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_loaded_from_disk
//       Access: Published
//  Description: Returns the flag that indicates the texture has been
//               loaded from a disk file.  See set_loaded_from_disk().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_loaded_from_disk() const {
  return _loaded_from_disk;
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
//     Function: Texture::store_unscaled_byte
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value is assumed to be in the range
//               0-255.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_unscaled_byte(int &index, int value) {
  _image[index++] = (uchar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_unscaled_short
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value is assumed to be in the range
//               0-65535.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_unscaled_short(int &index, int value) {
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.us = (ushort)value;
  _image[index++] = v.uc[0];
  _image[index++] = v.uc[1];
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_scaled_byte
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               bytes.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_scaled_byte(int &index, int value, double scale) {
  store_unscaled_byte(index, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store_scaled_short
//       Access: Private
//  Description: This is used by load() to store the next consecutive
//               component value into the indicated element of the
//               array, which is taken to be an array of unsigned
//               shorts.  The value will be scaled by the indicated
//               factor before storing it.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
store_scaled_short(int &index, int value, double scale) {
  store_unscaled_short(index, (int)(value * scale));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_unsigned_byte
//       Access: Private
//  Description: This is used by store() to retieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned bytes.
////////////////////////////////////////////////////////////////////
INLINE double Texture::
get_unsigned_byte(int &index) const {
  nassertr(index >= 0 && index < (int)_image.size(), 0.0);
  return (double)_image[index++] / 255.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_unsigned_short
//       Access: Private
//  Description: This is used by store() to retieve the next
//               consecutive component value from the indicated
//               element of the array, which is taken to be an array
//               of unsigned shorts.
////////////////////////////////////////////////////////////////////
INLINE double Texture::
get_unsigned_short(int &index) const {
  nassertr(index >= 0 && index+1 < (int)_image.size(), 0.0);
  union {
    ushort us;
    uchar uc[2];
  } v;
  v.uc[0] = _image[index++];
  v.uc[1] = _image[index++];
  return (double)v.us / 65535.0;
}
