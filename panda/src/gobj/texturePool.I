// Filename: texturePool.I
// Created by:  drose (26Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::has_texture
//       Access: Public, Static
//  Description: Returns true if the texture has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TexturePool::
has_texture(const string &filename) {
  return get_ptr()->ns_has_texture(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::verify_texture
//       Access: Public, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_texture() with the same texture name will
//               return a valid Texture pointer.
////////////////////////////////////////////////////////////////////
INLINE bool TexturePool::
verify_texture(const string &filename) {
  return load_texture(filename) != (Texture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_texture
//       Access: Public, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns the new texture.
//               If a texture with the same filename was previously
//               loaded, returns that one instead.  If the texture
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_texture(const string &filename, int num_components) {
  return get_ptr()->ns_load_texture(filename, num_components);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::load_texture
//       Access: Public, Static
//  Description: Loads the given filename up into a texture, if it has
//               not already been loaded, and returns the new texture.
//               If a texture with the same filename was previously
//               loaded, returns that one instead.  If the texture
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexturePool::
load_texture(const string &filename, const string &alpha_filename,
             int num_components) {
  return get_ptr()->ns_load_texture(filename, alpha_filename, num_components);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::add_texture
//       Access: Public, Static
//  Description: Adds the indicated already-loaded texture to the
//               pool.  The texture must have a filename set for its
//               name.  The texture will always replace any
//               previously-loaded texture in the pool that had the
//               same filename.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
add_texture(Texture *texture) {
  get_ptr()->ns_add_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::release_texture
//       Access: Public, Static
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
  get_ptr()->ns_release_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::release_all_textures
//       Access: Public, Static
//  Description: Releases all textures in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
release_all_textures() {
  get_ptr()->ns_release_all_textures();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::garbage_collect
//       Access: Public, Static
//  Description: Releases only those textures in the pool that have a
//               reference count of exactly 1; i.e. only those
//               textures that are not being used outside of the pool.
//               Returns the number of textures released.
////////////////////////////////////////////////////////////////////
INLINE int TexturePool::
garbage_collect() {
  return get_ptr()->ns_garbage_collect();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::list_contents
//       Access: Public, Static
//  Description: Lists the contents of the texture pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void TexturePool::
list_contents(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one TexturePool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE TexturePool::
TexturePool() {
}
