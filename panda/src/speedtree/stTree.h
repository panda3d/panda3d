// Filename: stTree.h
// Created by:  drose (06Oct10)
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

#ifndef STTREE_H
#define STTREE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "speedtree_api.h"

class SpeedTreeNode;

////////////////////////////////////////////////////////////////////
//       Class : STTree
// Description : Encapsulates a single tree model in the SpeedTree
//               library, as loaded from an SRT file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDASPEEDTREE STTree : public TypedReferenceCount, public Namable {
PUBLISHED:
  STTree(const Filename &fullpath);
private:
  STTree(const STTree &copy);

PUBLISHED:
  INLINE const Filename &get_fullpath() const;

  INLINE bool is_valid() const;

  virtual void output(ostream &out) const;

public:
  INLINE const SpeedTree::CTreeRender *get_tree() const;
  INLINE SpeedTree::CTreeRender *modify_tree();

private:
  Filename _fullpath;
  bool _is_valid;
  SpeedTree::CTreeRender _tree;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "STTree",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const STTree &tree) {
  tree.output(out);
  return out;
}

#include "stTree.I"

#endif
