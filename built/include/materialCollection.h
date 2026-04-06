/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialCollection.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef MATERIALCOLLECTION_H
#define MATERIALCOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "material.h"

/**
 *
 */
class EXPCL_PANDA_PGRAPH MaterialCollection {
PUBLISHED:
  MaterialCollection();
  MaterialCollection(const MaterialCollection &copy);
  void operator = (const MaterialCollection &copy);
  INLINE ~MaterialCollection();

  void add_material(Material *node_material);
  bool remove_material(Material *node_material);
  void add_materials_from(const MaterialCollection &other);
  void remove_materials_from(const MaterialCollection &other);
  void remove_duplicate_materials();
  bool has_material(Material *material) const;
  void clear();

  Material *find_material(const std::string &name) const;

  int get_num_materials() const;
  Material *get_material(int index) const;
  Material *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const MaterialCollection &other);
  INLINE MaterialCollection operator + (const MaterialCollection &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(Material)) Materials;
  Materials _materials;
};

INLINE std::ostream &operator << (std::ostream &out, const MaterialCollection &col) {
  col.output(out);
  return out;
}

#include "materialCollection.I"

#endif
