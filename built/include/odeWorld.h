/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeWorld.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEWORLD_H
#define ODEWORLD_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "ode_includes.h"
#include "pmap.h"
#include "numeric_types.h"

#include "ode_includes.h"
#include "odeHelperStructs.h"

class OdeBody;
class OdeJoint;

/**
 *
 */
class EXPCL_PANDAODE OdeWorld : public TypedObject {
PUBLISHED:
  OdeWorld();
  OdeWorld(const OdeWorld &copy);
  virtual ~OdeWorld();
  void destroy();
  INLINE bool is_empty() const;
  INLINE dWorldID get_id() const;

  INLINE void set_gravity(dReal x, dReal y, dReal z);
  INLINE void set_gravity(const LVecBase3f &vec);
  INLINE void set_erp(dReal erp);
  INLINE void set_cfm(dReal cfm);
  INLINE void set_quick_step_num_iterations(int num);

  INLINE void set_quick_step_w(dReal over_relaxation);
  INLINE void set_contact_max_correcting_vel(dReal vel);
  INLINE void set_contact_surface_layer(dReal depth);
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

  INLINE int compare_to(const OdeWorld &other) const;

  void init_surface_table(uint8_t num_surfaces);
  // void assign_surface_body(OdeBody& body, int surface);
  void add_body_dampening(OdeBody& body, int surface);
  void set_surface_entry(uint8_t pos1, uint8_t pos2,
                         dReal mu,
                         dReal bounce,
                         dReal bounce_vel,
                         dReal soft_erp,
                         dReal soft_cfm,
                         dReal slip,
                         dReal dampen);
  float apply_dampening(float dt, OdeBody& body);

  operator bool () const;

public:
  sSurfaceParams& get_surface(uint8_t surface1, uint8_t surface2);
  void set_surface(int pos1, int pos2, sSurfaceParams& entry);
  sBodyParams get_surface_body(dBodyID id);
  void set_dampen_on_bodies(dBodyID id1, dBodyID id2,dReal damp);


private:
  dWorldID _id;
  sSurfaceParams *_surface_table;
  uint8_t _num_surfaces;
  typedef pmap<dBodyID, sBodyParams> BodyDampenMap;
  BodyDampenMap _body_dampen_map;




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
