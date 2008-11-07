// Filename: physicalNode.h
// Created by:  charles (01Aug00)
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

#ifndef PHYSICALNODE_H
#define PHYSICALNODE_H

#include "pandabase.h"
#include "pandaNode.h"

#include "pvector.h"

#include "physical.h"
#include "config_physics.h"

////////////////////////////////////////////////////////////////////
//        Class : PhysicalNode
//  Description : Graph node that encapsulated a series of physical
//                objects
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PhysicalNode : public PandaNode {
PUBLISHED:
  PhysicalNode(const string &name);
  INLINE void clear();
  INLINE Physical *get_physical(int index) const;
  INLINE int get_num_physicals() const;
  MAKE_SEQ(get_physicals, get_num_physicals, get_physical);
  INLINE void add_physical(Physical *physical);

  void add_physicals_from(const PhysicalNode &other);
  void remove_physical(Physical *physical);
  void remove_physical(int index);
  
  virtual void write(ostream &out, unsigned int indent=0) const;

public:
  virtual ~PhysicalNode();
  virtual bool safe_to_flatten() const { return false; }
  virtual PandaNode *make_copy() const;

protected:
  PhysicalNode(const PhysicalNode &copy);

private:
  typedef pvector<PT(Physical)> PhysicalsVector;
  PhysicalsVector _physicals;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PhysicalNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physicalNode.I"

#endif // PHYSICALNODE_H
