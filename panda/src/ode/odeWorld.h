// Filename: odeWorld.h
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

#ifndef ODEWORLD_H
#define ODEWORLD_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
class OdeBody;
class OdeJoint;

////////////////////////////////////////////////////////////////////
//       Class : OdeWorld
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeWorld : public TypedObject {
PUBLISHED:
  OdeWorld();
  OdeWorld(const OdeWorld &copy);
  virtual ~OdeWorld();
  void destroy();

  INLINE void set_gravity(dReal x, dReal y, dReal z);
  INLINE LVecBase3f get_gravity() const;
  INLINE void set_erp(dReal erp);
  INLINE dReal get_erp() const;
  INLINE void set_cfm(dReal cfm);
  INLINE dReal get_cfm() const;
  INLINE void step(dReal stepsize);
  INLINE LVecBase3f impulse_to_force(dReal stepsize, \
                                     dReal ix, dReal iy, dReal iz);
  INLINE void quick_step(dReal stepsize);
  INLINE void set_quick_step_num_iterations(int num);
  INLINE int get_quick_step_num_iterations() const;
  INLINE void set_quick_step_w(dReal over_relaxation);
  INLINE dReal get_quick_step_w() const;
  INLINE void set_contact_max_correcting_vel(dReal vel);
  INLINE dReal get_contact_max_correcting_vel() const;
  INLINE void set_contact_surface_layer(dReal depth);
  INLINE dReal get_contact_surface_layer() const;
  INLINE void step_fast1(dReal stepsize, int maxiterations);
  INLINE void set_auto_enable_depth_sf1(int auto_enable_depth);
  INLINE int get_auto_enable_depth_sf1() const;
  INLINE dReal get_auto_disable_linear_threshold() const;
  INLINE void set_auto_disable_linear_threshold(dReal linear_threshold);
  INLINE dReal get_auto_disable_angular_threshold() const;
  INLINE void set_auto_disable_angular_threshold(dReal angular_threshold);
  INLINE int get_auto_disable_steps() const;
  INLINE void set_auto_disable_steps(int steps);
  INLINE dReal get_auto_disable_time() const;
  INLINE void set_auto_disable_time(dReal time);
  INLINE int get_auto_disable_flag() const;
  INLINE void set_auto_disable_flag(int do_auto_disable);

  INLINE int compare_to(const OdeWorld &other) const;
public: 
  INLINE dWorldID get_id() const;
  
private:
  dWorldID _id;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeWorld",
		  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeWorld.I"

#endif

