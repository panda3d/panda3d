// Filename: texturePool.h
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

#ifndef TEXTUREPOOL_H
#define TEXTUREPOOL_H

#include "pandabase.h"
#include "texture.h"
#include "filename.h"
#include "config_gobj.h"
#include "hashFilename.h"

#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : TexturePool
// Description : This is the preferred interface for loading textures
//               from image files.  It unifies all references to the
//               same filename, so that multiple models that reference
//               the same textures don't waste texture memory
//               unnecessarily.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexturePool {
PUBLISHED:
  // These functions take string parameters instead of Filenames
  // because that's somewhat more convenient to the scripting
  // language.
  INLINE static bool has_texture(const string &filename);
  INLINE static bool verify_texture(const string &filename);
  INLINE static Texture *load_texture(const string &filename, 
                                      int primary_file_num_channels = 0);
  INLINE static Texture *load_texture(const string &filename,
                                      const string &alpha_filename, 
                                      int primary_file_num_channels = 0,
                                      int alpha_file_channel = 0);
  INLINE static Texture *load_3d_texture(const string &filename_template);
  INLINE static Texture *load_cube_map(const string &filename_template);

  INLINE static Texture *get_normalization_cube_map(int size);

  INLINE static void add_texture(Texture *texture);
  INLINE static void release_texture(Texture *texture);
  INLINE static void release_all_textures();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);

  INLINE static void set_fake_texture_image(const string &filename);
  INLINE static void clear_fake_texture_image();
  INLINE static bool has_fake_texture_image();
  INLINE static const string &get_fake_texture_image();
  
  static void write(ostream &out);

public:
  typedef PT(Texture) MakeTextureFunc();
  void register_texture_type(MakeTextureFunc *func, const string &extensions);
  
  MakeTextureFunc *get_texture_type(const string &extension) const;
  PT(Texture) make_texture(const string &extension) const;
  void write_texture_types(ostream &out, int indent_level) const;

  static TexturePool *get_global_ptr();

private:
  TexturePool();

  bool ns_has_texture(const Filename &orig_filename);
  Texture *ns_load_texture(const Filename &orig_filename, int primary_file_num_channels);
  Texture *ns_load_texture(const Filename &orig_filename, 
                           const Filename &orig_alpha_filename, 
                           int primary_file_num_channels,
                           int alpha_file_channel);
  Texture *ns_load_3d_texture(const HashFilename &filename_template);
  Texture *ns_load_cube_map(const HashFilename &filename_template);
  Texture *ns_get_normalization_cube_map(int size);

  void ns_add_texture(Texture *texture);
  void ns_release_texture(Texture *texture);
  void ns_release_all_textures();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out) const;

  void report_texture_unreadable(const Filename &filename) const;

  static TexturePool *_global_ptr;
  typedef phash_map<string,  PT(Texture), string_hash> Textures;
  Textures _textures;
  string _fake_texture_image;

  PT(Texture) _normalization_cube_map;

  typedef pmap<string, MakeTextureFunc *> TypeRegistry;
  TypeRegistry _type_registry;
};

#include "texturePool.I"

#endif


