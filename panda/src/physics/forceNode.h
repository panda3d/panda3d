// Filename: forceNode.h
// Created by:  charles (02Aug00)
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

#ifndef FORCENODE_H
#define FORCENODE_H

#include <namedNode.h>
#include <vector>

#include "baseForce.h"

////////////////////////////////////////////////////////////////////
//        Class : ForceNode
//  Description : A force that lives in the scene graph and is
//                therefore subject to local coordinate systems.
//                An example of this would be simulating gravity
//                in a rotating space station.  or something.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ForceNode : public NamedNode {
private:
  vector< PT(BaseForce) > _forces;

PUBLISHED:
  ForceNode(const string &name = "");
  ForceNode(const ForceNode &copy);
  virtual ~ForceNode(void);

  ForceNode &operator =(const ForceNode &copy);

  virtual bool safe_to_flatten(void) const { return false; }
  virtual Node *make_copy(void) const;

  INLINE void clear(void);
  INLINE BaseForce *get_force(int index) const;
  INLINE int get_num_forces(void) const;
  INLINE void add_force(BaseForce *force);

  void add_forces_from(const ForceNode &other);
  void remove_force(BaseForce *f);
  void remove_force(int index);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    NamedNode::init_type();
    register_type(_type_handle, "ForceNode",
                  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "forceNode.I"

#endif // FORCENODE_H
