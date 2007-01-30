// Filename: odeContactGeom.h
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ODECONTACTGEOM_H
#define ODECONTACTGEOM_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "ode_includes.h"
#include "OdeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeContactGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeContactGeom : public TypedReferenceCount {
  friend class OdeContact;

protected:
  OdeContactGeom(const dContactGeom &contact_geom);

PUBLISHED:
  OdeContactGeom();
  OdeContactGeom(const OdeContactGeom &copy);
  virtual ~OdeContactGeom();

  INLINE LVecBase3f get_pos() const; 
  INLINE LVecBase3f get_normal() const;
  INLINE dReal get_depth() const;          
  INLINE OdeGeom get_g1() const;
  INLINE OdeGeom get_g2() const;
  INLINE int get_side1() const;
  INLINE int get_side2() const;

  INLINE void set_pos(const LVecBase3f &pos); 
  INLINE void set_normal(const LVecBase3f &normal);
  INLINE void set_depth(const dReal depth);          
  INLINE void set_g1(const OdeGeom &geom);
  INLINE void set_g2(const OdeGeom &geom);

public:
  const dContactGeom* get_contact_geom_ptr() const;

private:
  void operator = (const OdeContactGeom &copy);
  dContactGeom _contact_geom;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "OdeContactGeom",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeContactGeom.I"

#endif
