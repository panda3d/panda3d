/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalNameCollection.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef INTERNALNAMECOLLECTION_H
#define INTERNALNAMECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "internalName.h"

/**
 *
 */
class EXPCL_PANDA_PGRAPH InternalNameCollection {
PUBLISHED:
  InternalNameCollection();
  InternalNameCollection(const InternalNameCollection &copy);
  void operator = (const InternalNameCollection &copy);
  INLINE ~InternalNameCollection();

  void add_name(const InternalName *name);
  bool remove_name(const InternalName *name);
  void add_names_from(const InternalNameCollection &other);
  void remove_names_from(const InternalNameCollection &other);
  void remove_duplicate_names();
  bool has_name(const InternalName *name) const;
  void clear();

  int get_num_names() const;
  const InternalName *get_name(int index) const;
  MAKE_SEQ(get_names, get_num_names, get_name);
  const InternalName *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const InternalNameCollection &other);
  INLINE InternalNameCollection operator + (const InternalNameCollection &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(CPT(InternalName)) InternalNames;
  InternalNames _names;
};

INLINE std::ostream &operator << (std::ostream &out, const InternalNameCollection &col) {
  col.output(out);
  return out;
}

#include "internalNameCollection.I"

#endif
