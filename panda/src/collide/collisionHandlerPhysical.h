// Filename: collisionHandlerPhysical.h
// Created by:  drose (03Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONHANDLERPHYSICAL_H
#define COLLISIONHANDLERPHYSICAL_H

#include <pandabase.h>

#include "collisionHandlerEvent.h"
#include "collisionNode.h"

#include <driveInterface.h>
#include <pointerTo.h>
#include <pt_NodeRelation.h>

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionHandlerPhysical
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
  virtual void end_group();

PUBLISHED:
  void add_collider(CollisionNode *node, DriveInterface *drive_interface);
  void add_collider(CollisionNode *node, NodeRelation *arc);
  bool remove_collider(CollisionNode *node);
  bool has_collider(CollisionNode *node) const;
  void clear_colliders();

protected:
  virtual void handle_entries()=0;

protected:
  typedef vector< PT(CollisionEntry) > Entries;
  typedef map<PT(CollisionNode), Entries> FromEntries;
  FromEntries _from_entries;

  class ColliderDef {
  public:
    INLINE void set_drive_interface(DriveInterface *drive_interface);
    INLINE void set_arc(NodeRelation *arc);

    void get_mat(LMatrix4f &mat) const;
    void set_mat(const LMatrix4f &mat) const;

    PT(DriveInterface) _drive_interface;
    PT_NodeRelation _arc;
  };

  typedef map<PT(CollisionNode), ColliderDef> Colliders;
  Colliders _colliders;


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
  
  

