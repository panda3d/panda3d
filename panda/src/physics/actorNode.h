// Filename: actorNode.h
// Created by:  charles (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ACTORNODE_H
#define ACTORNODE_H

#include <pandabase.h>
#include "physicalNode.h"

#include <renderRelation.h>

////////////////////////////////////////////////////////////////////
//       Class : ActorNode
// Description : Like a physical node, but with a little more.  The
//               actornode assumes responsibility for its parent arc,
//               and changes in its own PhysicsObject will be
//               reflected as transforms in the arc.  This relation
//               goes both ways; changes in the arc will update the
//               object's position (shoves).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ActorNode : public PhysicalNode {
private:
  // node hook if the client changes the parent arc.
  virtual void transform_changed(NodeRelation *arc);

  RenderRelation *_parent_arc;
  PhysicsObject *_mass_center;

  bool _ok_to_callback;

public:
  ActorNode(const string &name = "");
  ActorNode(const ActorNode &copy);
  virtual ~ActorNode(void);

  // update the parent arc with PhysicsObject information
  void update_arc(void);

  INLINE void set_parent_arc(RenderRelation *arc);
  INLINE RenderRelation *get_parent_arc(void) const;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    PhysicalNode::init_type();
    register_type(_type_handle, "ActorNode",
		  PhysicalNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "actorNode.I"

#endif // ACTORNODE_H
