// Filename: qpcollisionHandlerPhysical.h
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

#ifndef qpCOLLISIONHANDLERPHYSICAL_H
#define qpCOLLISIONHANDLERPHYSICAL_H

#include "pandabase.h"

#include "qpcollisionHandlerEvent.h"
#include "qpcollisionNode.h"

#include "qpdriveInterface.h"
#include "pointerTo.h"
#include "pandaNode.h"

///////////////////////////////////////////////////////////////////
//       Class : qpCollisionHandlerPhysical
// Description : The abstract base class for a number of
//               qpCollisionHandlers that have some physical effect on
//               their moving bodies: they need to update the nodes'
//               positions based on the effects of the collision.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCollisionHandlerPhysical : public qpCollisionHandlerEvent {
public:
  qpCollisionHandlerPhysical();
  virtual ~qpCollisionHandlerPhysical();

  virtual void begin_group();
  virtual void add_entry(qpCollisionEntry *entry);
  virtual bool end_group();

PUBLISHED:
  void add_collider_drive(qpCollisionNode *node, qpDriveInterface *drive_interface);
  void add_collider_node(qpCollisionNode *node, PandaNode *target);
  bool remove_collider(qpCollisionNode *node);
  bool has_collider(qpCollisionNode *node) const;
  void clear_colliders();

protected:
  virtual bool handle_entries()=0;

protected:
  typedef pvector< PT(qpCollisionEntry) > Entries;
  typedef pmap<PT(qpCollisionNode), Entries> FromEntries;
  FromEntries _from_entries;

  class ColliderDef {
  public:
    INLINE void set_drive_interface(qpDriveInterface *drive_interface);
    INLINE void set_node(PandaNode *node);
    INLINE bool is_valid() const;

    void get_mat(LMatrix4f &mat) const;
    void set_mat(const LMatrix4f &mat);

    PT(qpDriveInterface) _drive_interface;
    PT(PandaNode) _node;
  };

  typedef pmap<PT(qpCollisionNode), ColliderDef> Colliders;
  Colliders _colliders;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpCollisionHandlerEvent::init_type();
    register_type(_type_handle, "qpCollisionHandlerPhysical",
                  qpCollisionHandlerEvent::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qpcollisionHandlerPhysical.I"

#endif



