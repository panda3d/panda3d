/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionRecorder.h
 * @author drose
 * @date 2003-04-16
 */

#ifndef COLLISIONRECORDER_H
#define COLLISIONRECORDER_H

#include "pandabase.h"
#include "typeHandle.h"
#include "typedObject.h"

class CollisionTraverser;
class CollisionEntry;

#ifdef DO_COLLISION_RECORDING

/**
 * This class is used to help debug the work the collisions system is doing.
 * It is a virtual base class that just provides an interface for recording
 * collisions tested and detected each frame.
 */
class EXPCL_PANDA_COLLIDE CollisionRecorder : public TypedObject {
protected:
  CollisionRecorder();
public:
  virtual ~CollisionRecorder();

PUBLISHED:
  void output(std::ostream &out) const;

public:
  virtual void begin_traversal();
  virtual void collision_tested(const CollisionEntry &entry, bool detected);
  virtual void end_traversal();

private:
  int _num_missed;
  int _num_detected;
  CollisionTraverser *_trav;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "CollisionRecorder",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
};

#include "collisionRecorder.I"

#endif  // DO_COLLISION_RECORDING


#endif
