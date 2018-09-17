/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePathCollection.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef NODEPATHCOLLECTION_H
#define NODEPATHCOLLECTION_H

#include "pandabase.h"
#include "nodePath.h"
#include "pointerToArray.h"

/**
 * This is a set of zero or more NodePaths.  It's handy for returning from
 * functions that need to return multiple NodePaths (for instance,
 * NodePaths::get_children).
 */
class EXPCL_PANDA_PGRAPH NodePathCollection {
PUBLISHED:
  NodePathCollection() = default;

#ifdef HAVE_PYTHON
  EXTENSION(NodePathCollection(PyObject *self, PyObject *sequence));
  EXTENSION(PyObject *__reduce__(PyObject *self) const);
#endif

  void add_path(const NodePath &node_path);
  bool remove_path(const NodePath &node_path);
  void add_paths_from(const NodePathCollection &other);
  void remove_paths_from(const NodePathCollection &other);
  void remove_duplicate_paths();
  bool has_path(const NodePath &path) const;
  void clear();
  void reserve(size_t num);

  bool is_empty() const;
  int get_num_paths() const;
  NodePath get_path(int index) const;
  MAKE_SEQ(get_paths, get_num_paths, get_path);
  NodePath operator [] (size_t index) const;
  size_t size() const;
  INLINE void operator += (const NodePathCollection &other);
  INLINE NodePathCollection operator + (const NodePathCollection &other) const;

  // Method names to satisfy Python's conventions.
  INLINE void append(const NodePath &node_path);
  INLINE void extend(const NodePathCollection &other);

  // Handy operations on many NodePaths at once.
  INLINE void ls() const;
  void ls(std::ostream &out, int indent_level = 0) const;

  NodePathCollection find_all_matches(const std::string &path) const;
  void reparent_to(const NodePath &other);
  void wrt_reparent_to(const NodePath &other);

  void show();
  void hide();
  void stash();
  void unstash();
  void detach();

  CollideMask get_collide_mask() const;
  void set_collide_mask(CollideMask new_mask, CollideMask bits_to_change = CollideMask::all_on(),
                        TypeHandle node_type = TypeHandle::none());

  bool calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point) const;

  EXTENSION(PyObject *get_tight_bounds() const);

  void set_texture(Texture *tex, int priority = 0);
  void set_texture(TextureStage *stage, Texture *tex, int priority = 0);
  void set_texture_off(int priority = 0);
  void set_texture_off(TextureStage *stage, int priority = 0);

  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a = 1.0,
                        int priority = 0);
  void set_color(const LColor &color, int priority = 0);

  INLINE void set_color_scale(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a = 1.0,
                              int priority = 0);
  void set_color_scale(const LVecBase4 &scale, int priority = 0);

  INLINE void compose_color_scale(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a = 1.0,
                                  int priority = 0);
  void compose_color_scale(const LVecBase4 &scale, int priority = 0);

  void set_attrib(const RenderAttrib *attrib, int priority = 0);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(NodePath) NodePaths;
  NodePaths _node_paths;

  // This typedef is used in set_attrib() and similar methods.
  typedef pmap<CPT(RenderState), CPT(RenderState) > StateMap;
};

INLINE std::ostream &operator << (std::ostream &out, const NodePathCollection &col) {
  col.output(out);
  return out;
}

#include "nodePathCollection.I"

#endif
