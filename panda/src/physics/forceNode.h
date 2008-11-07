// Filename: forceNode.h
// Created by:  charles (02Aug00)
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

#ifndef FORCENODE_H
#define FORCENODE_H

#include "pandaNode.h"
#include "pvector.h"

#include "baseForce.h"

////////////////////////////////////////////////////////////////////
//        Class : ForceNode
//  Description : A force that lives in the scene graph and is
//                therefore subject to local coordinate systems.
//                An example of this would be simulating gravity
//                in a rotating space station.  or something.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ForceNode : public PandaNode {
PUBLISHED:
  ForceNode(const string &name);
  INLINE void clear();
  INLINE BaseForce *get_force(int index) const;
  INLINE int get_num_forces() const;
  MAKE_SEQ(get_forces, get_num_forces, get_force);
  INLINE void add_force(BaseForce *force);

  void add_forces_from(const ForceNode &other);
  void remove_force(BaseForce *f);
  void remove_force(int index);
  
  virtual void output(ostream &out) const;
  virtual void write_forces(ostream &out, unsigned int indent=0) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

public:
  virtual ~ForceNode();
  virtual bool safe_to_flatten() const { return false; }
  virtual PandaNode *make_copy() const;

protected:
  ForceNode(const ForceNode &copy);

private:
  typedef pvector< PT(BaseForce) > ForceVector;
  ForceVector _forces;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ForceNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "forceNode.I"

#endif // FORCENODE_H
