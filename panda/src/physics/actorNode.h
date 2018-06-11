/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file actorNode.h
 * @author charles
 * @date 2000-08-07
 */

#ifndef ACTORNODE_H
#define ACTORNODE_H

#include "pandabase.h"
#include "physicalNode.h"

/**
 * Like a physical node, but with a little more.  The actornode assumes
 * responsibility for its own transform, and changes in its own PhysicsObject
 * will be reflected as transforms.  This relation goes both ways; changes in
 * the transform will update the object's position (shoves).
 */
class EXPCL_PANDA_PHYSICS ActorNode : public PhysicalNode {
PUBLISHED:
  explicit ActorNode(const std::string &name = "");
  ActorNode(const ActorNode &copy);
  virtual ~ActorNode();

  PhysicsObject *get_physics_object() { return _mass_center; }

  void set_contact_vector(const LVector3 &contact_vector);
  const LVector3 &get_contact_vector() const;

  // update the parent scene graph node with PhysicsObject information i.e.
  // copy from PhysicsObject to PandaNode
  void update_transform();

  void set_transform_limit(PN_stdfloat limit) { _transform_limit = limit; };
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PhysicsObject *_mass_center;
  LVector3 _contact_vector;
  bool _ok_to_callback;
  PN_stdfloat _transform_limit;

  // node hook if the client changes the node's transform.  i.e.  copy from
  // PandaNode to PhysicsObject
  virtual void transform_changed();
  void test_transform(const TransformState *ts) const;

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
