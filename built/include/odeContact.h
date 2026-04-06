/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeContact.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODECONTACT_H
#define ODECONTACT_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeSurfaceParameters.h"
#include "odeContactGeom.h"


/**
 *
 */
class EXPCL_PANDAODE OdeContact : public TypedReferenceCount {
PUBLISHED:
  OdeContact();
  // OdeContact(const OdeContact &copy);
  OdeContact(const dContact &contact);
  virtual ~OdeContact();

  INLINE OdeSurfaceParameters get_surface() const;
  INLINE OdeContactGeom get_geom();
  INLINE LVecBase3f get_fdir1() const;

  INLINE void set_surface(const OdeSurfaceParameters &surface_parameters);
  INLINE void set_geom(const OdeContactGeom &contact_geom);
  INLINE void set_fdir1(const LVecBase3f &fdir1);

public:
  void operator = (const OdeContact &copy);
  bool operator == (const OdeContact &other);
  const dContact* get_contact_ptr() const;

private:
  dContact _contact;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "OdeContact",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeContact.I"

#endif
