// Filename: physicsObjectCollection.h
// Created by:  joswilso (12Jul06)
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

#ifndef PHYSICSOBJECTCOLLECTION_H
#define PHYSICSOBJECTCOLLECTION_H

#include "pandabase.h"
#include "physicsObject.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysicsObjectCollection
// Description : This is a set of zero or more PhysicsObjects.  It's handy
//               for returning from functions that need to return
//               multiple PhysicsObjects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PhysicsObjectCollection {
PUBLISHED:
  PhysicsObjectCollection();
  PhysicsObjectCollection(const PhysicsObjectCollection &copy);
  void operator = (const PhysicsObjectCollection &copy);
  INLINE ~PhysicsObjectCollection();
  
  void add_physics_object(PT(PhysicsObject) physics_object);
  bool remove_physics_object(PT(PhysicsObject) physics_object);
  void add_physics_objects_from(const PhysicsObjectCollection &other);
  void remove_physics_objects_from(const PhysicsObjectCollection &other);
  void remove_duplicate_physics_objects();
  bool has_physics_object(PT(PhysicsObject) physics_object) const;
  void clear();

  bool is_empty() const;
  int get_num_physics_objects() const;
  PT(PhysicsObject) get_physics_object(int index) const;
  MAKE_SEQ(get_physics_objects, get_num_physics_objects, get_physics_object);
  PT(PhysicsObject) operator [] (int index) const;
  int size() const;
  INLINE void operator += (const PhysicsObjectCollection &other);
  INLINE PhysicsObjectCollection operator + (const PhysicsObjectCollection &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(PhysicsObject)) PhysicsObjects;
  PhysicsObjects _physics_objects;
};

/*
INLINE ostream &operator << (ostream &out, const PhysicsObjectCollection &col) {
  col.output(out);
  return out;
}
*/
#include "physicsObjectCollection.I"

#endif


