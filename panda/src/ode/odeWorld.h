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
  INLINE void set_gravity(const LVecBase3f &vec);
  INLINE void set_erp(dReal erp);
  INLINE void set_cfm(dReal cfm);
  INLINE void set_quick_step_num_iterations(int num);

  INLINE void set_quick_step_w(dReal over_relaxation);
  INLINE void set_contact_max_correcting_vel(dReal vel);
  INLINE void set_contact_surface_layer(dReal depth);
  INLINE void set_auto_enable_depth_sf1(int auto_enable_depth);
  INLINE void set_auto_disable_linear_threshold(dReal linear_threshold);
  INLINE void set_auto_disable_angular_threshold(dReal angular_threshold);
  INLINE void set_auto_disable_steps(int steps);
  INLINE void set_auto_disable_time(dReal time);
  INLINE void set_auto_disable_flag(int do_auto_disable);

  INLINE LVecBase3f get_gravity() const;
  INLINE dReal get_erp() const;
  INLINE dReal get_cfm() const;
  INLINE int get_quick_step_num_iterations() const;
  INLINE dReal get_quick_step_w() const;
  INLINE dReal get_contact_max_correcting_vel() const;
  INLINE dReal get_contact_surface_layer() const;
  INLINE int get_auto_enable_depth_sf1() const;
  INLINE dReal get_auto_disable_linear_threshold() const;
  INLINE dReal get_auto_disable_angular_threshold() const;
  INLINE int get_auto_disable_steps() const;
  INLINE dReal get_auto_disable_time() const;
  INLINE int get_auto_disable_flag() const;

  INLINE LVecBase3f impulse_to_force(dReal stepsize, \
                                     dReal ix, dReal iy, dReal iz);
  INLINE LVecBase3f impulse_to_force(dReal stepsize, \
                                     const LVecBase3f &impulse);

  INLINE void step(dReal stepsize);
  INLINE void quick_step(dReal stepsize);
  INLINE void step_fast1(dReal stepsize, int maxiterations);

  INLINE int compare_to(const OdeWorld &other) const;
  INLINE void init_surface_table(uint8 num_surfaces);
  INLINE void assign_surface_body(OdeBody& body, int surface);
  INLINE void set_surface_entry(uint8 pos1, uint8 pos2, 
                                dReal mu, 
                                dReal bounce, 
                                dReal bounce_vel, 
                                dReal soft_erp,
                                dReal soft_cfm,
                                dReal slip);
  
    
public: 
  INLINE dWorldID get_id() const;
  INLINE dSurfaceParameters& get_surface(uint8 surface1, uint8 surface2);
  INLINE void set_surface(int pos1, int pos2, dSurfaceParameters& entry);
  INLINE int get_surface_body(dBodyID id);
  
private:
  dWorldID _id;
  dSurfaceParameters *_surface_table;
  uint8 _num_surfaces;
  typedef pmap<dBodyID, int> BodySurfaceMap;
  BodySurfaceMap _body_surface_map;

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

