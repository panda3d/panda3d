// Filename: qplensNode.h
// Created by:  drose (26Feb02)
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

#ifndef QPLENSNODE_H
#define QPLENSNODE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "lens.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpLensNode
// Description : A node that contains a Lens.  The most important
//               example of this kind of node is a Camera, but other
//               kinds of nodes also contain a lens (for instance, a
//               Spotlight).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpLensNode : public PandaNode {
PUBLISHED:
  INLINE qpLensNode(const string &name);

public:
  INLINE qpLensNode(const qpLensNode &copy);
  INLINE void operator = (const qpLensNode &copy);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual PandaNode *make_copy() const;

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
    PandaNode::init_type();
    register_type(_type_handle, "qpLensNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qplensNode.I"

#endif
