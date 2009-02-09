// Filename: textureCollection.h
// Created by:  drose (16Mar02)
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

#ifndef TEXTURECOLLECTION_H
#define TEXTURECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureCollection
// Description : Manages a list of Texture objects, as returned by
//               TexturePool::find_all_textures().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ TextureCollection {
PUBLISHED:
  TextureCollection();
  TextureCollection(const TextureCollection &copy);
  void operator = (const TextureCollection &copy);
  INLINE ~TextureCollection();

  void add_texture(Texture *node_texture);
  bool remove_texture(Texture *node_texture);
  void add_textures_from(const TextureCollection &other);
  void remove_textures_from(const TextureCollection &other);
  void remove_duplicate_textures();
  bool has_texture(Texture *texture) const;
  void clear();

  Texture *find_texture(const string &name) const;

  int get_num_textures() const;
  Texture *get_texture(int index) const;
  MAKE_SEQ(get_textures, get_num_textures, get_texture);
  Texture *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const TextureCollection &other);
  INLINE TextureCollection operator + (const TextureCollection &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(Texture)) Textures;
  Textures _textures;
};

INLINE ostream &operator << (ostream &out, const TextureCollection &col) {
  col.output(out);
  return out;
}

#include "textureCollection.I"

#endif


