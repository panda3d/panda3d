// Filename: texturePool.h
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREPOOL_H
#define TEXTUREPOOL_H

#include <pandabase.h>

#include "texture.h"

#include <filename.h>

#include <map>

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
  INLINE static Texture *load_texture(const string &filename);
  INLINE static Texture *load_texture(const string &filename,
                                      const string &grayfilename);
  INLINE static void add_texture(Texture *texture);
  INLINE static void release_texture(Texture *texture);
  INLINE static void release_all_textures();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);

private:
  INLINE TexturePool();

  bool ns_has_texture(Filename filename);
  Texture *ns_load_texture(Filename filename);
  Texture *ns_load_texture(Filename filename, Filename grayfilename);
  void ns_add_texture(Texture *texture);
  void ns_release_texture(Texture *texture);
  void ns_release_all_textures();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out);

  static TexturePool *get_ptr();

  static TexturePool *_global_ptr;
  typedef map<string,  PT(Texture) > Textures;
  Textures _textures;
};

#include "texturePool.I"

#endif

  
