// Filename: collisionHandler.h
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

#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

#include "pandabase.h"

#include "typedReferenceCount.h"

class CollisionEntry;

///////////////////////////////////////////////////////////////////
//       Class : CollisionHandler
// Description : The abstract interface to a number of classes that
//               decide what to do when a collision is detected.  One
//               of these must be assigned to the CollisionTraverser
//               that is processing collisions in order to specify how
//               to dispatch detected collisions.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandler : public TypedReferenceCount {
public:
  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual bool end_group();


PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CollisionHandler",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
};

#endif



