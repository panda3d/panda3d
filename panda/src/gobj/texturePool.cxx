// Filename: texturePool.cxx
// Created by:  drose (26Apr00)
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

#include "texturePool.h"
#include "config_gobj.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"


TexturePool *TexturePool::_global_ptr = (TexturePool *)NULL;


////////////////////////////////////////////////////////////////////
//     Function: TexturePool::write
//       Access: Published, Static
//  Description: Lists the contents of the texture pool to the
//               indicated output stream.
//               For debugging.
////////////////////////////////////////////////////////////////////
void TexturePool::
write(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_has_texture
//       Access: Private
//  Description: The nonstatic implementation of has_texture().
////////////////////////////////////////////////////////////////////
bool TexturePool::
ns_has_texture(const Filename &orig_filename) {
  Filename filename(orig_filename);

  if (!_fake_texture_image.empty()) {
    filename = _fake_texture_image;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(filename, get_texture_path());
  vfs->resolve_filename(filename, get_model_path());

  Textures::const_iterator ti;
  ti = _textures.find(filename);
  if (ti != _textures.end()) {
    // This texture was previously loaded.
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_load_texture
//       Access: Private
//  Description: The nonstatic implementation of load_texture().
////////////////////////////////////////////////////////////////////
Texture *TexturePool::
ns_load_texture(const Filename &orig_filename, int primary_file_num_channels) {
  Filename filename(orig_filename);

  if (!_fake_texture_image.empty()) {
    filename = _fake_texture_image;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(filename, get_texture_path()) ||
    vfs->resolve_filename(filename, get_model_path());

  Textures::const_iterator ti;
  ti = _textures.find(filename);
  if (ti != _textures.end()) {
    // This texture was previously loaded.
    return (*ti).second;
  }

  gobj_cat.info()
    << "Loading texture " << filename << "\n";
  PT(Texture) tex = new Texture;
  if (!tex->read(filename, 0, primary_file_num_channels)) {
    // This texture was not found.
    gobj_cat.error()
      << "Unable to read texture \"" << filename << "\""
      << " on texture_path " << texture_path
      << " or model_path " << model_path <<"\n";
    return NULL;
  }

  // Set the original filename, before we searched along the path.
  tex->set_filename(orig_filename);

  _textures[filename] = tex;
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_load_texture
//       Access: Private
//  Description: The nonstatic implementation of load_texture().
////////////////////////////////////////////////////////////////////
Texture *TexturePool::
ns_load_texture(const Filename &orig_filename, 
                const Filename &orig_alpha_filename,
                int primary_file_num_channels,
                int alpha_file_channel) {
  Filename filename(orig_filename);
  Filename alpha_filename(orig_alpha_filename);

  if (!_fake_texture_image.empty()) {
    return ns_load_texture(_fake_texture_image, primary_file_num_channels);
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(filename, get_texture_path()) ||
    vfs->resolve_filename(filename, get_model_path());
  
  vfs->resolve_filename(alpha_filename, get_texture_path()) ||
    vfs->resolve_filename(alpha_filename, get_model_path());

  Textures::const_iterator ti;
  ti = _textures.find(filename);
  if (ti != _textures.end()) {
    // This texture was previously loaded.
    return (*ti).second;
  }

  gobj_cat.info()
    << "Loading texture " << filename << " and alpha component "
    << alpha_filename << endl;
  PT(Texture) tex = new Texture;
  if (!tex->read(filename, alpha_filename, 0, primary_file_num_channels,
                 alpha_file_channel)) {
    // This texture was not found.
    gobj_cat.error() << "Unable to read texture " << filename << "\n";
    return NULL;
  }

  // Set the original filenames, before we searched along the path.
  tex->set_filename(orig_filename);
  tex->set_alpha_filename(orig_alpha_filename);

  _textures[filename] = tex;
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_add_texture
//       Access: Private
//  Description: The nonstatic implementation of add_texture().
////////////////////////////////////////////////////////////////////
void TexturePool::
ns_add_texture(Texture *tex) {
  string filename = tex->get_filename();
  if (filename.empty()) {
    gobj_cat.error() << "Attempt to call add_texture() on an unnamed texture.\n";
  }

  // We blow away whatever texture was there previously, if any.
  _textures[filename] = tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_release_texture
//       Access: Private
//  Description: The nonstatic implementation of release_texture().
////////////////////////////////////////////////////////////////////
void TexturePool::
ns_release_texture(Texture *tex) {
  string filename = tex->get_filename();
  Textures::iterator ti;
  ti = _textures.find(filename);
  if (ti != _textures.end() && (*ti).second == tex) {
    _textures.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_release_all_textures
//       Access: Private
//  Description: The nonstatic implementation of release_all_textures().
////////////////////////////////////////////////////////////////////
void TexturePool::
ns_release_all_textures() {
  _textures.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int TexturePool::
ns_garbage_collect() {
  int num_released = 0;
  Textures new_set;

  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    Texture *tex = (*ti).second;
    if (tex->get_ref_count() == 1) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Releasing " << (*ti).first << "\n";
      }
      ++num_released;
    } else {
      new_set.insert(new_set.end(), *ti);
    }
  }

  _textures.swap(new_set);
  return num_released;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void TexturePool::
ns_list_contents(ostream &out) const {
  out << _textures.size() << " textures:\n";
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    Texture *texture = (*ti).second;
    out << "  " << (*ti).first
        << " (count = " << texture->get_ref_count() << ", ram = "
        << texture->get_ram_image_size() / 1024 << " Kb)\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one TexturePool object in the system.
////////////////////////////////////////////////////////////////////
TexturePool *TexturePool::
get_ptr() {
  if (_global_ptr == (TexturePool *)NULL) {
    _global_ptr = new TexturePool;
  }
  return _global_ptr;
}
