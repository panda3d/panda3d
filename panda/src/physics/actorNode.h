// Filename: actorNode.h
// Created by:  charles (07Aug00)
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

#ifndef ACTORNODE_H
#define ACTORNODE_H

#include "pandabase.h"
#include "physicalNode.h"

////////////////////////////////////////////////////////////////////
//       Class : ActorNode
// Description : Like a physical node, but with a little more.  The
//               actornode assumes responsibility for its own
//               transform, and changes in its own PhysicsObject will
//               be reflected as transforms.  This relation goes both
//               ways; changes in the transform will update the
//               object's position (shoves).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ActorNode : public PhysicalNode {
public:
  ActorNode(const string &name = "");
  ActorNode(const ActorNode &copy);
  virtual ~ActorNode();

  // update the parent arc with PhysicsObject information
  void update_transform();
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  // node hook if the client changes the node's transform.
  virtual void transform_changed();

  PhysicsObject *_mass_center;

  bool _ok_to_callback;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysicalNode::init_type();
    register_type(_type_handle, "ActorNode",
                  PhysicalNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "actorNode.I"

#endif // ACTORNODE_H
