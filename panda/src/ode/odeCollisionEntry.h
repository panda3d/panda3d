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

#include "odeContactGeom.h"

class OdeUtil;

////////////////////////////////////////////////////////////////////
//       Class : OdeCollisionEntry
// Description : A class used to hold information about a collision
//               that has occurred.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeCollisionEntry : public TypedReferenceCount {
PUBLISHED:
  virtual ~OdeCollisionEntry();
  
  INLINE OdeGeom get_geom1() const;
  INLINE OdeGeom get_geom2() const;
  INLINE OdeBody get_body1() const;
  INLINE OdeBody get_body2() const;
  
  INLINE size_t get_num_contacts() const;
  INLINE LPoint3f get_contact_point(size_t n) const;
  INLINE OdeContactGeom get_contact_geom(size_t n) const;
  INLINE OdeContactGeom operator [] (size_t n) const;
  MAKE_SEQ(get_contact_points, get_num_contacts, get_contact_point);
  MAKE_SEQ(get_contact_geoms, get_num_contacts, get_contact_geom);
  
  INLINE operator bool () const;
  INLINE bool is_empty() const;

private:
  INLINE OdeCollisionEntry();
  dGeomID _geom1, _geom2;
  dBodyID _body1, _body2;
  size_t _num_contacts;
  OdeContactGeom *_contact_geoms;

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
  friend class OdeUtil;
};

#include "odeCollisionEntry.I"

#endif

