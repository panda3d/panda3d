// Filename: lensNode.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef LENSNODE_H
#define LENSNODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "namedNode.h"
#include "lens.h"

////////////////////////////////////////////////////////////////////
//       Class : LensNode
// Description : A node that contains a Lens.  The most important
//               example of this kind of node is a Camera, but other
//               kinds of nodes also contain a lens (for instance, a
//               Spotlight).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LensNode : public NamedNode {
PUBLISHED:
  INLINE LensNode(const string &name = "");

public:
  INLINE LensNode(const LensNode &copy);
  INLINE void operator = (const LensNode &copy);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual Node *make_copy() const;

PUBLISHED:
  INLINE void copy_lens(const Lens &lens);
  INLINE void set_lens(Lens *lens);
  INLINE Lens *get_lens();

  bool is_in_view(const LPoint3f &pos);

protected:
  PT(Lens) _lens;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type( _type_handle, "LensNode",
                   NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle                       _type_handle;
};

#include "lensNode.I"

#endif
