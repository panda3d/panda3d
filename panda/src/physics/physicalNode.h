// Filename: physicalNode.h
// Created by:  charles (01Aug00)
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
private:
  pvector< PT(Physical) > _physicals;

PUBLISHED:
  PhysicalNode(const string &name);

protected:
  PhysicalNode(const PhysicalNode &copy);

public:
  virtual ~PhysicalNode(void);
  virtual bool safe_to_flatten(void) const { return false; }
  virtual PandaNode *make_copy(void) const;

PUBLISHED:
  INLINE void clear(void);
  INLINE Physical *get_physical(int index) const;
  INLINE int get_num_physicals(void) const;
  INLINE void add_physical(Physical *physical);

  void add_physicals_from(const PhysicalNode &other);
  void remove_physical(Physical *physical);
  void remove_physical(int index);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    PandaNode::init_type();
    register_type(_type_handle, "PhysicalNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physicalNode.I"

#endif // PHYSICALNODE_H
