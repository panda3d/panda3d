/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionEntry.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONENTRY_H
#define COLLISIONENTRY_H

#include "pandabase.h"

#include "collisionTraverser.h"
#include "collisionSolid.h"
#include "collisionNode.h"
#include "collisionRecorder.h"

#include "transformState.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "pointerTo.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "clipPlaneAttrib.h"

/**
 * Defines a single collision event.  One of these is created for each
 * collision detected by a CollisionTraverser, to be dealt with by the
 * CollisionHandler.
 *
 * A CollisionEntry provides slots for a number of data values (such as
 * intersection point and normal) that might or might not be known for each
 * collision.  It is up to the handler to determine what information is known
 * and to do the right thing with it.
 */
class EXPCL_PANDA_COLLIDE CollisionEntry : public TypedWritableReferenceCount {
public:
  INLINE CollisionEntry();
  CollisionEntry(const CollisionEntry &copy);
  void operator = (const CollisionEntry &copy);

PUBLISHED:
  INLINE const CollisionSolid *get_from() const;
  INLINE bool has_into() const;
  INLINE const CollisionSolid *get_into() const;

  INLINE CollisionNode *get_from_node() const;
  INLINE PandaNode *get_into_node() const;
  INLINE NodePath get_from_node_path() const;
  INLINE NodePath get_into_node_path() const;

  INLINE void set_t(PN_stdfloat t);
  INLINE PN_stdfloat get_t() const;
  INLINE bool collided() const;
  INLINE void reset_collided();

  INLINE bool get_respect_prev_transform() const;

  INLINE void set_surface_point(const LPoint3 &point);
  INLINE void set_surface_normal(const LVector3 &normal);
  INLINE void set_interior_point(const LPoint3 &point);

  INLINE bool has_surface_point() const;
  INLINE bool has_surface_normal() const;
  INLINE bool has_interior_point() const;

  INLINE void set_contact_pos(const LPoint3 &pos);
  INLINE void set_contact_normal(const LVector3 &normal);

  INLINE bool has_contact_pos() const;
  INLINE bool has_contact_normal() const;

  LPoint3 get_surface_point(const NodePath &space) const;
  LVector3 get_surface_normal(const NodePath &space) const;
  LPoint3 get_interior_point(const NodePath &space) const;
  bool get_all(const NodePath &space,
               LPoint3 &surface_point,
               LVector3 &surface_normal,
               LPoint3 &interior_point) const;

  LPoint3 get_contact_pos(const NodePath &space) const;
  LVector3 get_contact_normal(const NodePath &space) const;
  bool get_all_contact_info(const NodePath &space,
                            LPoint3 &contact_pos,
                            LVector3 &contact_normal) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

PUBLISHED:
  MAKE_PROPERTY(from_solid, get_from);
  MAKE_PROPERTY(into_solid, get_into);
  MAKE_PROPERTY(from_node, get_from_node);
  MAKE_PROPERTY(into_node, get_into_node);
  MAKE_PROPERTY(from_node_path, get_from_node_path);
  MAKE_PROPERTY(into_node_path, get_into_node_path);

  MAKE_PROPERTY(t, get_t, set_t);
  MAKE_PROPERTY(respect_prev_transform, get_respect_prev_transform);

public:
  INLINE CPT(TransformState) get_wrt_space() const;
  INLINE CPT(TransformState) get_inv_wrt_space() const;
  INLINE CPT(TransformState) get_wrt_prev_space() const;

  INLINE const LMatrix4 &get_wrt_mat() const;
  INLINE const LMatrix4 &get_inv_wrt_mat() const;
  INLINE const LMatrix4 &get_wrt_prev_mat() const;

  INLINE const ClipPlaneAttrib *get_into_clip_planes() const;

private:
  INLINE void test_intersection(CollisionHandler *record,
                                const CollisionTraverser *trav) const;
  void check_clip_planes();

  CPT(CollisionSolid) _from;
  CPT(CollisionSolid) _into;

  PT(CollisionNode) _from_node;
  PT(PandaNode) _into_node;
  NodePath _from_node_path;
  NodePath _into_node_path;
  CPT(ClipPlaneAttrib) _into_clip_planes;
  PN_stdfloat _t;

  enum Flags {
    F_has_surface_point       = 0x0001,
    F_has_surface_normal      = 0x0002,
    F_has_interior_point      = 0x0004,
    F_respect_prev_transform  = 0x0008,
    F_checked_clip_planes     = 0x0010,
    F_has_contact_pos         = 0x0020,
    F_has_contact_normal      = 0x0040,
  };

  int _flags;

  LPoint3 _surface_point;
  LVector3 _surface_normal;
  LPoint3 _interior_point;

  LPoint3 _contact_pos;
  LVector3 _contact_normal;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "CollisionEntry",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
  friend class CollisionHandlerFluidPusher;
};

INLINE std::ostream &operator << (std::ostream &out, const CollisionEntry &entry);

#include "collisionEntry.I"

#endif
