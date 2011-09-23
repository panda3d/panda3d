// Filename: texturePool.I
// Created by:  drose (26Apr00)
// Updated by: fperazzi, PandaSE(29Apr10) (added load_2d_texture_array)
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
//     Function: TexturePool::has_texture
//       Access: Published, Static
//  Description: Returns true if the texture has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TexturePool::
has_texture(const Filename &filename) {
  return get_global_ptr()->ns_has_texture(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::verify_texture
//       Access: Published, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_texture() with the same texture name will
//               return a valid Texture pointer.
////////////////////////////////////////////////////////////////////
INLINE bool TexturePool::
verify_texture(const Filename &filename) {
  return load_texture(filename) != (Texture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_texture
//       Access: Published, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns the new texture.
//               If a texture with the same filename was previously
//               loaded, returns that one instead.  If the texture
//               file cannot be found, returns NULL.
//
//               If read_mipmaps is true, the filename should contain
//               a hash mark ('#'), which will be filled in with the
//               mipmap level number; and the texture will be defined
//               with a series of images, one for each mipmap level.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_texture(const Filename &filename, int primary_file_num_channels,
             bool read_mipmaps, const LoaderOptions &options) {
  return get_global_ptr()->ns_load_texture(filename, primary_file_num_channels,
                                           read_mipmaps, options);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_texture
//       Access: Published, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns the new texture.
//               If a texture with the same filename was previously
//               loaded, returns that one instead.  If the texture
//               file cannot be found, returns NULL.
//
//               If read_mipmaps is true, both filenames should
//               contain a hash mark ('#'), which will be filled in
//               with the mipmap level number; and the texture will be
//               defined with a series of images, two for each mipmap
//               level.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_texture(const Filename &filename, const Filename &alpha_filename,
             int primary_file_num_channels, int alpha_file_channel,
             bool read_mipmaps, const LoaderOptions &options) {
  return get_global_ptr()->ns_load_texture(filename, alpha_filename, 
                                           primary_file_num_channels,
                                           alpha_file_channel,
                                           read_mipmaps, options);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_3d_texture
//       Access: Published, Static
//  Description: Loads a 3-D texture that is specified with a series
//               of n pages, all numbered in sequence, and beginning
//               with index 0.  The filename should include a sequence
//               of one or more hash characters ("#") which will be
//               filled in with the index number of each level.
//
//               If read_mipmaps is true, the filename should contain
//               an additional hash mark.  The first hash mark will be
//               filled in with the mipmap level number, and the
//               second with the index number of each 3-d level.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_3d_texture(const Filename &filename_pattern, bool read_mipmaps, 
                const LoaderOptions &options) {
  return get_global_ptr()->ns_load_3d_texture(filename_pattern, read_mipmaps, 
                                              options);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_2d_texture_array
//       Access: Published, Static
//  Description: Loads a 2-D texture array that is specified with a series
//               of n pages, all numbered in sequence, and beginning
//               with index 0.  The filename should include a sequence
//               of one or more hash characters ("#") which will be
//               filled in with the index number of each level.
//
//               If read_mipmaps is true, the filename should contain
//               an additional hash mark.  The first hash mark will be
//               filled in with the mipmap level number, and the
//               second with the index number of each 2-d level.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_2d_texture_array(const Filename &filename_pattern, bool read_mipmaps, 
                const LoaderOptions &options) {
  return get_global_ptr()->ns_load_2d_texture_array(filename_pattern, read_mipmaps, 
                                                    options);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_cube_map
//       Access: Published, Static
//  Description: Loads a cube map texture that is specified with a
//               series of 6 pages, numbered 0 through 5.  The
//               filename should include a sequence of one or more
//               hash characters ("#") which will be filled in with
//               the index number of each pagee.
//
//               If read_mipmaps is true, the filename should contain
//               an additional hash mark.  The first hash mark will be
//               filled in with the mipmap level number, and the
//               second with the face number, 0 through 5.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_cube_map(const Filename &filename_pattern, bool read_mipmaps, 
              const LoaderOptions &options) {
  return get_global_ptr()->ns_load_cube_map(filename_pattern, read_mipmaps, 
                                            options);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_normalization_cube_map
//       Access: Published, Static
//  Description: Returns a standard Texture object that has been
//               created with
//               Texture::generate_normalization_cube_map().  This
//               Texture may be shared by any application code
//               requiring a normalization cube map.  It will be at
//               least as large as the specified size, though it may
//               be larger.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
get_normalization_cube_map(int size) {
  return get_global_ptr()->ns_get_normalization_cube_map(size);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_alpha_scale_map
//       Access: Published, Static
//  Description: Returns a standard Texture object that has been
//               created with Texture::generate_alpha_scale_map().
//
//               This Texture object is used internally by Panda to
//               apply an alpha scale to an object (instead of munging
//               its vertices) when gsg->get_alpha_scale_via_texture()
//               returns true.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
get_alpha_scale_map() {
  return get_global_ptr()->ns_get_alpha_scale_map();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::add_texture
//       Access: Published, Static
//  Description: Adds the indicated already-loaded texture to the
//               pool.  The texture must have a filename set for its
//               name.  The texture will always replace any
//               previously-loaded texture in the pool that had the
//               same filename.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
add_texture(Texture *texture) {
  get_global_ptr()->ns_add_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::release_texture
//       Access: Published, Static
//  Description: Removes the indicated texture from the pool,
//               indicating it will never be loaded again; the texture
//               may then be freed.  If this function is never called,
//               a reference count will be maintained on every texture
//               every loaded, and textures will never be freed.
//
//               The texture's name should not have been changed
//               during its lifetime, or this function may fail to
//               locate it in the pool.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
release_texture(Texture *texture) {
  get_global_ptr()->ns_release_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::release_all_textures
//       Access: Published, Static
//  Description: Releases all textures in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
release_all_textures() {
  get_global_ptr()->ns_release_all_textures();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::rehash
//       Access: Published, Static
//  Description: Should be called when the model-path changes, to blow
//               away the cache of texture pathnames found along the
//               model-path.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
rehash() {
  get_global_ptr()->_relpath_lookup.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::garbage_collect
//       Access: Published, Static
//  Description: Releases only those textures in the pool that have a
//               reference count of exactly 1; i.e. only those
//               textures that are not being used outside of the pool.
//               Returns the number of textures released.
////////////////////////////////////////////////////////////////////
INLINE int TexturePool::
garbage_collect() {
  return get_global_ptr()->ns_garbage_collect();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::list_contents
//       Access: Published, Static
//  Description: Lists the contents of the texture pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
list_contents(ostream &out) {
  get_global_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::list_contents
//       Access: Published, Static
//  Description: Lists the contents of the texture pool to cout
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
list_contents() {
  get_global_ptr()->ns_list_contents(cout);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::find_texture
//       Access: Published, Static
//  Description: Returns the first texture found in the pool that
//               matches the indicated name (which may contain
//               wildcards).  Returns the texture if it is found, or
//               NULL if it is not.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
find_texture(const string &name) {
  return get_global_ptr()->ns_find_texture(name);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::find_all_textures
//       Access: Published, Static
//  Description: Returns the set of all textures found in the pool
//               that match the indicated name (which may contain
//               wildcards).
////////////////////////////////////////////////////////////////////
INLINE TextureCollection TexturePool::
find_all_textures(const string &name) {
  return get_global_ptr()->ns_find_all_textures(name);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::set_fake_texture_image
//       Access: Published, Static
//  Description: Sets a bogus filename that will be loaded in lieu of
//               any textures requested from this point on.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
set_fake_texture_image(const Filename &filename) {
  get_global_ptr()->_fake_texture_image = filename;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::clear_fake_texture_image
//       Access: Published, Static
//  Description: Restores normal behavior of loading the textures
//               actually requested.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
clear_fake_texture_image() {
  set_fake_texture_image(string());
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::has_fake_texture_image
//       Access: Published, Static
//  Description: Returns true if fake_texture_image mode has been
//               enabled, false if we are in the normal mode.
////////////////////////////////////////////////////////////////////
INLINE bool TexturePool::
has_fake_texture_image() {
  return !get_fake_texture_image().empty();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_fake_texture_image
//       Access: Published, Static
//  Description: Returns the filename that was specified with a
//               previous call to set_fake_texture_image().
////////////////////////////////////////////////////////////////////
INLINE const Filename &TexturePool::
get_fake_texture_image() {
  return get_global_ptr()->_fake_texture_image;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::make_texture
//       Access: Published, Static
//  Description: Creates a new Texture object of the appropriate type
//               for the indicated filename extension, according to
//               the types that have been registered via
//               register_texture_type().
////////////////////////////////////////////////////////////////////
PT(Texture) TexturePool::
make_texture(const string &extension) {
  return get_global_ptr()->ns_make_texture(extension);
}
