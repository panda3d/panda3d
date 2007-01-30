// Filename: odeGeom.h
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

#ifndef ODEGEOM_H
#define ODEGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeSpace.h" // Needed for derived classes
#include "odeBody.h"

class OdeTriMeshGeom;

////////////////////////////////////////////////////////////////////
//       Class : OdeGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeGeom : public TypedObject {
  friend class OdeContactGeom;
  friend class OdeSpace;

protected:
  OdeGeom(dGeomID id);

PUBLISHED:
  OdeGeom();
  virtual ~OdeGeom();
  void destroy();

  //INLINE void set_data(void* data);
  INLINE void set_body(OdeBody &body);
  INLINE OdeBody get_body() const;
  INLINE void set_position(dReal x, dReal y, dReal z);
  INLINE void set_rotation(const LMatrix3f &r);
  INLINE void set_quaternion(const LQuaternionf &q);
  INLINE LPoint3f get_position() const;
  INLINE LMatrix3f get_rotation() const;
  INLINE LQuaternionf get_quaternion() const;
  //INLINE void get_aabb(dReal aabb[6]) const;
  INLINE int is_space();
  INLINE int get_class() const;
  INLINE void set_category_bits(unsigned long bits);
  INLINE void set_collide_bits(unsigned long bits);
  INLINE void enable();
  INLINE void disable();
  INLINE int is_enabled();
  INLINE void set_offset_position(dReal x, dReal y, dReal z);
  INLINE void set_offset_rotation(const LMatrix3f &r);
  INLINE void set_offset_quaternion(const LQuaternionf &q);
  INLINE void set_offset_world_position(dReal x, dReal y, dReal z);
  INLINE void set_offset_world_rotation(const LMatrix3f &r);
  INLINE void set_offset_world_quaternion(const LQuaternionf &q);
  INLINE void clear_offset();
  INLINE int is_offset();
  INLINE LPoint3f get_offset_position() const;
  INLINE LMatrix3f get_offset_rotation() const;
  INLINE LQuaternionf get_offset_quaternion() const;

  void get_space(OdeSpace &space) const;
  virtual void write(ostream &out = cout, unsigned int indent=0) const;
public:
  INLINE dGeomID get_id() const;

  INLINE static int get_geom_class() { return -1; };

protected:
  dGeomID _id;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeGeom",
		  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeGeom.I"

#endif
