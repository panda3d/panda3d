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
#include "string_utils.h"
#include "virtualFileSystem.h"
#include "pnmFileTypeRegistry.h"


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
  get_global_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::register_texture_type
//       Access: Public
//  Description: Records a factory function that makes a Texture
//               object of the appropriate type for one or more
//               particular filename extensions.  The string
//               extensions may be a string that contains
//               space-separated list of extensions, case-insensitive.
////////////////////////////////////////////////////////////////////
void TexturePool::
register_texture_type(MakeTextureFunc *func, const string &extensions) {
  vector_string words;
  extract_words(downcase(extensions), words);

  vector_string::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    _type_registry[*wi] = func;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_texture_type
//       Access: Public
//  Description: Returns the factory function to construct a new
//               texture of the type appropriate for the indicated
//               filename extension, if any, or NULL if the extension
//               is not one of the extensions for a texture file.
////////////////////////////////////////////////////////////////////
TexturePool::MakeTextureFunc *TexturePool::
get_texture_type(const string &extension) const {
  string c = downcase(extension);
  TypeRegistry::const_iterator ti;
  ti = _type_registry.find(extension);
  if (ti != _type_registry.end()) {
    return (*ti).second;
  }

  // Check the PNM type registry.
  PNMFileTypeRegistry *pnm_reg = PNMFileTypeRegistry::get_global_ptr();
  PNMFileType *type = pnm_reg->get_type_from_extension(extension);
  if (type != (PNMFileType *)NULL) {
    // This is a known image type; create an ordinary Texture.
    ((TexturePool *)this)->_type_registry[extension] = Texture::make_texture;
    return Texture::make_texture;
  }

  // This is an unknown texture type.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::make_texture
//       Access: Public
//  Description: Creates a new Texture object of the appropriate type
//               for the indicated filename extension, according to
//               the types that have been registered via
//               register_texture_type().
////////////////////////////////////////////////////////////////////
PT(Texture) TexturePool::
make_texture(const string &extension) const {
  MakeTextureFunc *func = get_texture_type(extension);
  if (func != NULL) {
    return func();
  }

  // We don't know what kind of file type this is; return an ordinary
  // Texture in case it's an image file with no extension.
  return new Texture;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::write_texture_types
//       Access: Public
//  Description: Outputs a list of the available texture types to the
//               indicated output stream.  This is mostly the list of
//               available image types, with maybe a few additional
//               ones for video textures.
////////////////////////////////////////////////////////////////////
void TexturePool::
write_texture_types(ostream &out, int indent_level) const {
  PNMFileTypeRegistry *pnm_reg = PNMFileTypeRegistry::get_global_ptr();
  pnm_reg->write(out, indent_level);

  // Also output any of the additional texture types, that aren't
  // strictly images (these are typically video textures).
  TypeRegistry::const_iterator ti;
  for (ti = _type_registry.begin(); ti != _type_registry.end(); ++ti) {
    string extension = (*ti).first;
    MakeTextureFunc *func = (*ti).second;

    if (pnm_reg->get_type_from_extension(extension) == NULL) {
      PT(Texture) tex = func();
      string name = tex->get_type().get_name();
      indent(out, indent_level) << name;
      indent(out, max(30 - (int)name.length(), 0))
	<< "  ." << extension << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::get_global_ptr
//       Access: Public, Static
//  Description: Initializes and/or returns the global pointer to the
//               one TexturePool object in the system.
////////////////////////////////////////////////////////////////////
TexturePool *TexturePool::
get_global_ptr() {
  if (_global_ptr == (TexturePool *)NULL) {
    _global_ptr = new TexturePool;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one TexturePool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
TexturePool::
TexturePool() {
  ConfigVariableString fake_texture_image
    ("fake-texture-image", "",
     PRC_DESC("Set this to enable a speedy-load mode in which you don't care "
	      "what the world looks like, you just want it to load in minimal "
	      "time.  This causes all texture loads via the TexturePool to use "
	      "the same texture file, which will presumably only be loaded "
	      "once."));
  _fake_texture_image = fake_texture_image;
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
  PT(Texture) tex = make_texture(filename.get_extension());
  if (!tex->read(filename, 0, primary_file_num_channels)) {
    // This texture was not found or could not be read.
    report_texture_unreadable(filename);
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
  PT(Texture) tex = make_texture(filename.get_extension());
  if (!tex->read(filename, alpha_filename, 0, primary_file_num_channels,
                 alpha_file_channel)) {
    // This texture was not found or could not be read.
    report_texture_unreadable(filename);
    return NULL;
  }

  // Set the original filenames, before we searched along the path.
  tex->set_filename(orig_filename);
  tex->set_alpha_filename(orig_alpha_filename);

  _textures[filename] = tex;
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_load_3d_texture
//       Access: Private
//  Description: The nonstatic implementation of load_3d_texture().
////////////////////////////////////////////////////////////////////
Texture *TexturePool::
ns_load_3d_texture(const Filename &filename_pattern) {
  Filename filename(filename_pattern);
  filename.set_pattern(true);

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
    << "Loading 3-d texture " << filename << "\n";
  PT(Texture) tex = make_texture(filename.get_extension());
  tex->setup_3d_texture();
  if (!tex->read_pages(filename)) {
    // This texture was not found or could not be read.
    report_texture_unreadable(filename);
    return NULL;
  }

  // Set the original filename, before we searched along the path.
  tex->set_filename(filename_pattern);

  _textures[filename] = tex;
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_load_cube_map
//       Access: Private
//  Description: The nonstatic implementation of load_cube_map().
////////////////////////////////////////////////////////////////////
Texture *TexturePool::
ns_load_cube_map(const Filename &filename_pattern) {
  Filename filename(filename_pattern);
  filename.set_pattern(true);

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
    << "Loading cube map texture " << filename << "\n";
  PT(Texture) tex = make_texture(filename.get_extension());
  tex->setup_cube_map();
  if (!tex->read_pages(filename)) {
    // This texture was not found or could not be read.
    report_texture_unreadable(filename);
    return NULL;
  }

  // Set the original filename, before we searched along the path.
  tex->set_filename(filename_pattern);

  _textures[filename] = tex;
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePool::ns_get_normalization_cube_map
//       Access: Private
//  Description: The nonstatic implementation of get_normalization_cube_map().
////////////////////////////////////////////////////////////////////
Texture *TexturePool::
ns_get_normalization_cube_map(int size) {
  if (_normalization_cube_map == (Texture *)NULL) {
    _normalization_cube_map = new Texture("normalization_cube_map");
  }
  if (_normalization_cube_map->get_x_size() < size ||
      _normalization_cube_map->get_texture_type() != Texture::TT_cube_map) {
    _normalization_cube_map->generate_normalization_cube_map(size);
  }

  return _normalization_cube_map;
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
  _normalization_cube_map = NULL;
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

  if (_normalization_cube_map != (Texture *)NULL &&
      _normalization_cube_map->get_ref_count() == 1) {
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
	<< "Releasing normalization cube map\n";
    }
    ++num_released;
    _normalization_cube_map = NULL;
  }

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
//     Function: TexturePool::report_texture_unreadable
//       Access: Private
//  Description: Prints a suitable error message when a texture could
//               not be loaded.
////////////////////////////////////////////////////////////////////
void TexturePool::
report_texture_unreadable(const Filename &filename) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->exists(filename)) {
    if (filename.is_local()) {
      // The file doesn't exist, and it wasn't
      // fully-qualified--therefore, it wasn't found along either
      // search path.
      gobj_cat.error()
	<< "Unable to find texture \"" << filename << "\""
	<< " on texture_path " << texture_path
	<< " or model_path " << model_path <<"\n";
    } else {
      // A fully-specified filename is not searched along the path, so
      // don't mislead the user with the error message.
      gobj_cat.error()
	<< "Texture \"" << filename << "\" does not exist.\n";
    }

  } else {
    // The file exists, but it couldn't be read for some reason.
    gobj_cat.error()
      << "Texture \"" << filename << "\" exists but cannot be read.\n";

    // Maybe the filename extension is unknown.
    MakeTextureFunc *func = get_texture_type(filename.get_extension());
    if (func == (MakeTextureFunc *)NULL) {
      gobj_cat.error()
	<< "Texture extension \"" << filename.get_extension() 
	<< "\" is unknown.  Supported texture types:\n";
      write_texture_types(gobj_cat.error(false), 2);
    }
  }
}
