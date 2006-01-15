// Filename: internalNameCollection.h
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

#ifndef INTERNALNAMECOLLECTION_H
#define INTERNALNAMECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "internalName.h"

////////////////////////////////////////////////////////////////////
//       Class : InternalNameCollection
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA InternalNameCollection {
PUBLISHED:
  InternalNameCollection();
  InternalNameCollection(const InternalNameCollection &copy);
  void operator = (const InternalNameCollection &copy);
  INLINE ~InternalNameCollection();

  void add_name(InternalName *name);
  bool remove_name(InternalName *name);
  void add_names_from(const InternalNameCollection &other);
  void remove_names_from(const InternalNameCollection &other);
  void remove_duplicate_names();
  bool has_name(InternalName *name) const;
  void clear();

  int get_num_names() const;
  InternalName *get_name(int index) const;
  InternalName *operator [] (int index) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(InternalName)) InternalNames;
  InternalNames _names;
};

INLINE ostream &operator << (ostream &out, const InternalNameCollection &col) {
  col.output(out);
  return out;
}

#include "internalNameCollection.I"

#endif


