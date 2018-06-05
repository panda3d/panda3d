/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPhysical.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONHANDLERPHYSICAL_H
#define COLLISIONHANDLERPHYSICAL_H

#include "pandabase.h"

#include "collisionHandlerEvent.h"
#include "collisionNode.h"

#include "driveInterface.h"
#include "pointerTo.h"
#include "pandaNode.h"

/**
 * The abstract base class for a number of CollisionHandlers that have some
 * physical effect on their moving bodies: they need to update the nodes'
 * positions based on the effects of the collision.
 */
class EXPCL_PANDA_COLLIDE CollisionHandlerPhysical : public CollisionHandlerEvent {
public:
  CollisionHandlerPhysical();
  virtual ~CollisionHandlerPhysical();

  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual bool end_group();

PUBLISHED:
  void add_collider(const NodePath &collider, const NodePath &target);
  void add_collider(const NodePath &collider, const NodePath &target,
                    DriveInterface *drive_interface);
  bool remove_collider(const NodePath &collider);
  bool has_collider(const NodePath &collider) const;
  void clear_colliders();

  INLINE void set_center(const NodePath &center);
  INLINE void clear_center();
  INLINE const NodePath &get_center() const;
  INLINE bool has_center() const;
  INLINE bool has_contact() const;

PUBLISHED:
  MAKE_PROPERTY2(center, has_center, get_center, set_center, clear_center);

protected:
  bool _has_contact; // Are we in contact with anything?


protected:
  class ColliderDef {
  public:
    INLINE void set_target(const NodePath &target,
                           DriveInterface *drive_interface = nullptr);
    INLINE void updated_transform();

    NodePath _target;
    PT(DriveInterface) _drive_interface;
  };

  virtual bool handle_entries()=0;
  virtual void apply_linear_force(ColliderDef &def, const LVector3 &force)=0;

  virtual bool validate_target(const NodePath &target);

  typedef pvector< PT(CollisionEntry) > Entries;
  typedef pmap<NodePath, Entries> FromEntries;
  FromEntries _from_entries;

  typedef pmap<NodePath, ColliderDef> Colliders;
  Colliders _colliders;

  NodePath _center;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerEvent::init_type();
    register_type(_type_handle, "CollisionHandlerPhysical",
                  CollisionHandlerEvent::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerPhysical.I"

#endif
