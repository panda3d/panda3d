/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerHighestEvent.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONHANDLERHIGHESTEVENT_H
#define COLLISIONHANDLERHIGHESTEVENT_H

#include "pandabase.h"

#include "collisionHandlerEvent.h"
#include "collisionNode.h"
#include "collisionEntry.h"

#include "vector_string.h"
#include "pointerTo.h"

/**
 * A specialized kind of CollisionHandler that throws an event for each
 * collision detected.  The event thrown may be based on the name of the
 * moving object or the struck object, or both.  The first parameter of the
 * event will be a pointer to the CollisionEntry that triggered it.
 */
class EXPCL_PANDA_COLLIDE CollisionHandlerHighestEvent : public CollisionHandlerEvent {
PUBLISHED:
  CollisionHandlerHighestEvent();

public:
  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual bool end_group();
private:
  double _collider_distance;
  PT(CollisionEntry) _closest_collider;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandler::init_type();
    register_type(_type_handle, "CollisionHandlerHighestEvent",
                  CollisionHandlerEvent::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
