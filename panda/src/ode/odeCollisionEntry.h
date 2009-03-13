// Filename: odeCollisionEntry.h
// Created by:  pro-rsoft (05Mar09)
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

#ifndef ODECOLLISIONENTRY_H
#define ODECOLLISIONENTRY_H

#include "pandabase.h"
#include "typedReferenceCount.h"

class OdeGeom;

////////////////////////////////////////////////////////////////////
//       Class : OdeCollisionEntry
// Description : A class used to hold information about a collision
//               that has occurred.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDASKEL OdeCollisionEntry : public TypedReferenceCount {
PUBLISHED:
  virtual ~OdeCollisionEntry() {};
  
  const OdeGeom get_geom1();
  const OdeGeom get_geom2();
  const OdeBody get_body1();
  const OdeBody get_body2();

private:
  OdeCollisionEntry();
  dGeomID _geom1, _geom2;
  dBodyID _body1, _body2;
  LPoint3f *_points;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "OdeCollisionEntry",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  friend class OdeSpace;
};


#endif

