// Filename: qpcollisionHandlerPusher.h
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

#ifndef qpCOLLISIONHANDLERPUSHER_H
#define qpCOLLISIONHANDLERPUSHER_H

#include "pandabase.h"

#include "qpcollisionHandlerPhysical.h"

///////////////////////////////////////////////////////////////////
//       Class : qpCollisionHandlerPusher
// Description : A specialized kind of qpCollisionHandler that simply
//               pushes back on things that attempt to move into solid
//               walls.  This is the simplest kind of "real-world"
//               collisions you can have.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCollisionHandlerPusher : public qpCollisionHandlerPhysical {
PUBLISHED:
  qpCollisionHandlerPusher();
  virtual ~qpCollisionHandlerPusher();

  INLINE void set_horizontal(bool flag);
  INLINE bool get_horizontal() const;

protected:
  virtual bool handle_entries();

private:
  bool _horizontal;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpCollisionHandlerPhysical::init_type();
    register_type(_type_handle, "qpCollisionHandlerPusher",
                  qpCollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qpcollisionHandlerPusher.I"

#endif



