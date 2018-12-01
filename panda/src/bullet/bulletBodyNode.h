/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBodyNode.h
 * @author enn0x
 * @date 2010-11-19
 */

#ifndef __BULLET_BODY_NODE_H__
#define __BULLET_BODY_NODE_H__

#include "pandabase.h"

#include "bulletShape.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "pandaNode.h"
#include "collideMask.h"
#include "collisionNode.h"
#include "transformState.h"
#include "boundingSphere.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletBodyNode : public PandaNode {
protected:
  BulletBodyNode(const char *name);
  BulletBodyNode(const BulletBodyNode &copy);

PUBLISHED:
  INLINE ~BulletBodyNode();

  // Shapes
  void add_shape(BulletShape *shape, const TransformState *xform=TransformState::make_identity());
  void remove_shape(BulletShape *shape);

  int get_num_shapes() const;
  BulletShape *get_shape(int idx) const;
  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);

  LPoint3 get_shape_pos(int idx) const;
  LMatrix4 get_shape_mat(int idx) const;
  CPT(TransformState) get_shape_transform(int idx) const;
  BoundingSphere get_shape_bounds() const;

  void add_shapes_from_collision_solids(CollisionNode *cnode);

  // Static and kinematic
  bool is_static() const;
  bool is_kinematic() const;
  
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
  void force_active(bool active);

  void set_deactivation_time(PN_stdfloat dt);
  PN_stdfloat get_deactivation_time() const;

  void set_deactivation_enabled(bool enabled);
  bool is_deactivation_enabled() const;

  // Debug Visualisation
  INLINE void set_debug_enabled(const bool enabled);
  INLINE bool is_debug_enabled() const;

  // Friction and Restitution
  PN_stdfloat get_restitution() const;
  void set_restitution(PN_stdfloat restitution);

  PN_stdfloat get_friction() const;
  void set_friction(PN_stdfloat friction);

#if BT_BULLET_VERSION >= 281
  PN_stdfloat get_rolling_friction() const;
  void set_rolling_friction(PN_stdfloat friction);
  MAKE_PROPERTY(rolling_friction, get_rolling_friction, set_rolling_friction);
#endif

  bool has_anisotropic_friction() const;
  void set_anisotropic_friction(const LVecBase3 &friction);
  LVecBase3 get_anisotropic_friction() const;

  // CCD
  PN_stdfloat get_ccd_swept_sphere_radius() const;
  PN_stdfloat get_ccd_motion_threshold() const;
  void set_ccd_swept_sphere_radius(PN_stdfloat radius);
  void set_ccd_motion_threshold(PN_stdfloat threshold);

  // Special
  void set_transform_dirty();

  MAKE_SEQ_PROPERTY(shapes, get_num_shapes, get_shape);
  MAKE_SEQ_PROPERTY(shape_pos, get_num_shapes, get_shape_pos);
  MAKE_SEQ_PROPERTY(shape_mat, get_num_shapes, get_shape_mat);
  MAKE_SEQ_PROPERTY(shape_transform, get_num_shapes, get_shape_transform);
  MAKE_PROPERTY(shape_bounds, get_shape_bounds);
  MAKE_PROPERTY(static, is_static, set_static);
  MAKE_PROPERTY(kinematic, is_kinematic, set_kinematic);
  MAKE_PROPERTY(collision_notification, notifies_collisions, notify_collisions);
  MAKE_PROPERTY(collision_response, get_collision_response, set_collision_response);
  MAKE_PROPERTY(contact_response, has_contact_response);
  MAKE_PROPERTY(contact_processing_threshold, get_contact_processing_threshold, set_contact_processing_threshold);
  MAKE_PROPERTY(active, is_active, force_active);
  MAKE_PROPERTY(deactivation_time, get_deactivation_time, set_deactivation_time);
  MAKE_PROPERTY(deactivation_enabled, is_deactivation_enabled, set_deactivation_enabled);
  MAKE_PROPERTY(debug_enabled, is_debug_enabled, set_debug_enabled);
  MAKE_PROPERTY(restitution, get_restitution, set_restitution);
  MAKE_PROPERTY(friction, get_friction, set_friction);
  MAKE_PROPERTY(anisotropic_friction, get_anisotropic_friction, set_anisotropic_friction);
  MAKE_PROPERTY(ccd_swept_sphere_radius, get_ccd_swept_sphere_radius, set_ccd_swept_sphere_radius);
  MAKE_PROPERTY(ccd_motion_threshold, get_ccd_motion_threshold, set_ccd_motion_threshold);

public:
  virtual btCollisionObject *get_object() const = 0;

  virtual CollideMask get_legal_collide_mask() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

  virtual void output(std::ostream &out) const;
  virtual void do_output(std::ostream &out) const;

protected:
  void set_collision_flag(int flag, bool value);
  bool get_collision_flag(int flag) const;

  btCollisionShape *_shape;

  typedef PTA(PT(BulletShape)) BulletShapes;
  BulletShapes _shapes;

private:
  virtual void do_shape_changed();
  void do_add_shape(BulletShape *shape, const TransformState *xform=TransformState::make_identity());
  CPT(TransformState) do_get_shape_transform(int idx) const;

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
