// Filename: qpcollisionHandlerFloor.h
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

#ifndef qpCOLLISIONHANDLERFLOOR_H
#define qpCOLLISIONHANDLERFLOOR_H

#include "pandabase.h"

#include "qpcollisionHandlerPhysical.h"

///////////////////////////////////////////////////////////////////
//       Class : qpCollisionHandlerFloor
// Description : A specialized kind of qpCollisionHandler that sets the
//               Z height of the collider to a fixed linear offset
//               from the highest detected collision point each frame.
//               It's intended to implement walking around on a floor
//               of varying height by casting a ray down from the
//               avatar's head.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCollisionHandlerFloor : public qpCollisionHandlerPhysical {
PUBLISHED:
  qpCollisionHandlerFloor();
  virtual ~qpCollisionHandlerFloor();

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;

  INLINE void set_max_velocity(float max_vel);
  INLINE float get_max_velocity() const;

protected:
  virtual bool handle_entries();

private:
  float _offset;
  float _max_velocity;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpCollisionHandlerPhysical::init_type();
    register_type(_type_handle, "qpCollisionHandlerFloor",
                  qpCollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qpcollisionHandlerFloor.I"

#endif



