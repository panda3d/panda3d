// Filename: textureCollection.h
// Created by:  drose (16Mar02)
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

#ifndef TEXTURECOLLECTION_H
#define TEXTURECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureCollection
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureCollection {
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
  Texture *operator [] (int index) const;

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


