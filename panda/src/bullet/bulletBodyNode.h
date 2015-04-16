// Filename: bulletBodyNode.h
// Created by:  enn0x (19Nov10)
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

#ifndef __BULLET_BODY_NODE_H__
#define __BULLET_BODY_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "pandaNode.h"
#include "collideMask.h"
#include "collisionNode.h"
#include "transformState.h"
#include "boundingSphere.h"

class BulletShape;

////////////////////////////////////////////////////////////////////
//       Class : BulletBodyNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletBodyNode : public PandaNode {
protected:
  BulletBodyNode(const char *name);

PUBLISHED:
  INLINE ~BulletBodyNode();

  // Shapes
  void add_shape(BulletShape *shape, const TransformState *xform=TransformState::make_identity());
  void remove_shape(BulletShape *shape);

  INLINE int get_num_shapes() const;
  INLINE BulletShape *get_shape(int idx) const;
  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);

  LPoint3 get_shape_pos(int idx) const;
  LMatrix4 get_shape_mat(int idx) const;
  CPT(TransformState) get_shape_transform(int idx) const;
  BoundingSphere get_shape_bounds() const;

  void add_shapes_from_collision_solids(CollisionNode *cnode);

  // Static and kinematic
  INLINE bool is_static() const;
  INLINE bool is_kinematic() const;

  INLINE void set_static(bool value);
  INLINE void set_kinematic(bool value);

  // Contacts
  INLINE void set_into_collide_mask(CollideMask mask);

  INLINE void notify_collisions(bool value);
  INLINE bool notifies_collisions() const;

  INLINE void set_collision_response(bool value);
  INLINE bool get_collision_response() const;

  bool check_collision_with(PandaNode *node);

  bool has_contact_response() const;

  PN_stdfloat get_contact_processing_threshold() const;
  void set_contact_processing_threshold(PN_stdfloat threshold);

  // Deactivation
  bool is_active() const;
  void set_active(bool active, bool force=false);

  void set_deactivation_time(PN_stdfloat dt);
  PN_stdfloat get_deactivation_time() const;

  void set_deactivation_enabled(bool enabled);
  bool is_deactivation_enabled() const;

  // Debug Visualistion
  INLINE void set_debug_enabled(const bool enabled);
  INLINE bool is_debug_enabled() const;

  // Friction and Restitution
  INLINE PN_stdfloat get_restitution() const;
  INLINE void set_restitution(PN_stdfloat restitution);

  INLINE PN_stdfloat get_friction() const;
  INLINE void set_friction(PN_stdfloat friction);

#if BT_BULLET_VERSION >= 281
  INLINE PN_stdfloat get_rolling_friction() const;
  INLINE void set_rolling_friction(PN_stdfloat friction);
#endif

  INLINE bool has_anisotropic_friction() const;
  void set_anisotropic_friction(const LVecBase3 &friction);
  LVecBase3 get_anisotropic_friction() const;

  // CCD
  PN_stdfloat get_ccd_swept_sphere_radius() const;
  PN_stdfloat get_ccd_motion_threshold() const;
  void set_ccd_swept_sphere_radius(PN_stdfloat radius);
  void set_ccd_motion_threshold(PN_stdfloat threshold);

  // Special
  void set_transform_dirty();

public:
  virtual btCollisionObject *get_object() const = 0;

  virtual CollideMask get_legal_collide_mask() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

  virtual void output(ostream &out) const;

protected:
  INLINE void set_collision_flag(int flag, bool value);
  INLINE bool get_collision_flag(int flag) const;

  btCollisionShape *_shape;

  typedef PTA(PT(BulletShape)) BulletShapes;
  BulletShapes _shapes;

private:
  virtual void shape_changed();

  static bool is_identity(btTransform &trans);

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);
  virtual bool require_fully_complete() const;

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "BulletBodyNode", 
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletBodyNode.I"

#endif // __BULLET_BODY_NODE_H__

