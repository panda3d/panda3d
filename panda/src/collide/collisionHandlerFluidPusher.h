// Filename: collisionHandlerFluidPusher.h
// Created by:  drose (16Mar02)
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

#ifndef COLLISIONHANDLERFLUIDPUSHER_H
#define COLLISIONHANDLERFLUIDPUSHER_H

#include "pandabase.h"

#include "collisionSolid.h"
#include "collisionHandlerPusher.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionHandlerFluidPusher
// Description : A CollisionHandlerPusher that makes use of timing
//               and spatial information from fluid collisions to improve
//               collision response
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionHandlerFluidPusher : public CollisionHandlerPusher {
PUBLISHED:
  CollisionHandlerFluidPusher();

public:
  virtual void add_entry(CollisionEntry *entry);

protected:
  virtual bool handle_entries();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPusher::init_type();
    register_type(_type_handle, "CollisionHandlerFluidPusher",
                  CollisionHandlerPusher::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerFluidPusher.I"

#endif



