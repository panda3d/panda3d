// Filename: collisionRecorder.h
// Created by:  drose (16Apr03)
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

#ifndef COLLISIONRECORDER_H
#define COLLISIONRECORDER_H

#include "pandabase.h"
#include "typeHandle.h"

class CollisionTraverser;

#ifdef DO_COLLISION_RECORDING

////////////////////////////////////////////////////////////////////
//       Class : CollisionRecorder
// Description : This class is used to help debug the work the
//               collisions system is doing.  It is a virtual base
//               class that just provides an interface for recording
//               collisions tested and detected each frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionRecorder {
protected:
  CollisionRecorder();
public:
  virtual ~CollisionRecorder();

PUBLISHED:
  void output(ostream &out) const;

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
    register_type(_type_handle, "CollisionRecorder");
  }

PUBLISHED:
  // We have to publish this explicitly because the CollisionRecorder
  // object does not inherit from TypedObject.
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  INLINE int get_type_index() const;

private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
};

#include "collisionRecorder.I"

#endif  // DO_COLLISION_RECORDING


#endif

