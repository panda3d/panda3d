/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeContactGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODECONTACTGEOM_H
#define ODECONTACTGEOM_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

class OdeSpace;
class OdeUtil;

/**
 *
 */
class EXPCL_PANDAODE OdeContactGeom : public TypedReferenceCount {
  friend class OdeContact;
  friend class OdeSpace;
  friend class OdeUtil;

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
  OdeContactGeom(const dContactGeom &copy);
  const dContactGeom* get_contact_geom_ptr() const;

private:
  void operator = (const OdeContactGeom &copy);
  void operator = (const dContactGeom &copy);
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
