/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureCollection.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef TEXTURECOLLECTION_H
#define TEXTURECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "texture.h"

/**
 * Manages a list of Texture objects, as returned by
 * TexturePool::find_all_textures().
 */
class EXPCL_PANDA_GOBJ TextureCollection {
PUBLISHED:
  TextureCollection();
  TextureCollection(const TextureCollection &copy);
  void operator = (const TextureCollection &copy);
  INLINE ~TextureCollection();

#ifdef HAVE_PYTHON
  EXTENSION(TextureCollection(PyObject *self, PyObject *sequence));
  EXTENSION(PyObject *__reduce__(PyObject *self) const);
#endif

  void add_texture(Texture *texture);
  bool remove_texture(Texture *texture);
  void add_textures_from(const TextureCollection &other);
  void remove_textures_from(const TextureCollection &other);
  void remove_duplicate_textures();
  bool has_texture(Texture *texture) const;
  void clear();
  void reserve(size_t num);

  Texture *find_texture(const std::string &name) const;

  int get_num_textures() const;
  Texture *get_texture(int index) const;
  MAKE_SEQ(get_textures, get_num_textures, get_texture);
  Texture *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const TextureCollection &other);
  INLINE TextureCollection operator + (const TextureCollection &other) const;

  // Method names to satisfy Python's conventions.
  INLINE void append(Texture *texture);
  INLINE void extend(const TextureCollection &other);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(Texture)) Textures;
  Textures _textures;
};

INLINE std::ostream &operator << (std::ostream &out, const TextureCollection &col) {
  col.output(out);
  return out;
}

#include "textureCollection.I"

#endif
