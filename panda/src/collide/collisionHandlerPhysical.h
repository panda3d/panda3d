// Filename: collisionHandlerPhysical.h
// Created by:  drose (16Mar02)
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

#ifndef COLLISIONHANDLERPHYSICAL_H
#define COLLISIONHANDLERPHYSICAL_H

#include "pandabase.h"

#include "collisionHandlerEvent.h"
#include "collisionNode.h"

#include "driveInterface.h"
#include "pointerTo.h"
#include "pandaNode.h"

///////////////////////////////////////////////////////////////////
//       Class : CollisionHandlerPhysical
// Description : The abstract base class for a number of
//               CollisionHandlers that have some physical effect on
//               their moving bodies: they need to update the nodes'
//               positions based on the effects of the collision.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandlerPhysical : public CollisionHandlerEvent {
public:
  CollisionHandlerPhysical();
  virtual ~CollisionHandlerPhysical();

  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual bool end_group();

PUBLISHED:
  void add_collider_node(CollisionNode *node, PandaNode *target);
  bool remove_collider(CollisionNode *node);
  bool has_collider(CollisionNode *node) const;
  void clear_colliders();

  // add_collider_drive() is becoming obsolete.  If you need it, let us know.
  void add_collider_drive(CollisionNode *node, DriveInterface *drive_interface);

protected:
  typedef pvector< PT(CollisionEntry) > Entries;
  typedef pmap<PT(CollisionNode), Entries> FromEntries;
  FromEntries _from_entries;

  class ColliderDef {
  public:
    INLINE void set_drive_interface(DriveInterface *drive_interface);
    INLINE void set_node(PandaNode *node);
    INLINE bool is_valid() const;

    void get_mat(LMatrix4f &mat) const;
    void set_mat(const LMatrix4f &mat);

    PT(DriveInterface) _drive_interface;
    PT(PandaNode) _node;
  };

  typedef pmap<PT(CollisionNode), ColliderDef> Colliders;
  Colliders _colliders;

  virtual bool handle_entries()=0;
  virtual void apply_linear_force(ColliderDef &def, const LVector3f &force)=0;

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



