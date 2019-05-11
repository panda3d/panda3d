/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEBODY_H
#define ODEBODY_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeWorld.h"
#include "odeMass.h"

class OdeJoint;
class OdeGeom;
class OdeCollisionEntry;

/**
 *
 */
class EXPCL_PANDAODE OdeBody : public TypedObject {
  friend class OdeJoint;
  friend class OdeGeom;
  friend class OdeCollisionEntry;

public:
  OdeBody(dBodyID id);

PUBLISHED:
  OdeBody(OdeWorld &world);
  virtual ~OdeBody();
  void destroy();
  INLINE bool is_empty() const;
  INLINE dBodyID get_id() const;

  INLINE void set_auto_disable_linear_threshold(dReal linear_threshold);
  INLINE void set_auto_disable_angular_threshold(dReal angular_threshold);
  INLINE void set_auto_disable_steps(int steps);
  INLINE void set_auto_disable_time(dReal time);
  INLINE void set_auto_disable_flag(int do_auto_disable);
  INLINE void set_auto_disable_defaults();
  INLINE void set_data(void *data);
  EXTENSION(void set_data(PyObject *data));

  INLINE void set_position(dReal x, dReal y, dReal z);
  INLINE void set_position(const LVecBase3f &pos);
  INLINE void set_rotation(const LMatrix3f &r);
  INLINE void set_quaternion(const LQuaternionf &q);
  INLINE void set_linear_vel(dReal x, dReal y, dReal z);
  INLINE void set_linear_vel(const LVecBase3f &vel);
  INLINE void set_angular_vel(dReal x, dReal y, dReal z);
  INLINE void set_angular_vel(const LVecBase3f &vel);
  INLINE void set_mass(OdeMass &mass);


  INLINE dReal get_auto_disable_linear_threshold() const;
  INLINE dReal get_auto_disable_angular_threshold() const;
  INLINE int   get_auto_disable_steps() const;
  INLINE dReal get_auto_disable_time() const;
  INLINE int   get_auto_disable_flag() const;
#ifndef CPPPARSER
  INLINE void *get_data() const;
#endif
  EXTENSION(PyObject *get_data() const);

  INLINE LVecBase3f  get_position() const;
  INLINE LMatrix3f  get_rotation() const;
  INLINE LVecBase4f  get_quaternion() const;
  INLINE LVecBase3f  get_linear_vel() const;
  INLINE LVecBase3f  get_angular_vel() const;
  INLINE OdeMass     get_mass() const;

  INLINE void add_force(dReal fx, dReal fy, dReal fz);
  INLINE void add_force(const LVecBase3f &f);
  INLINE void add_torque(dReal fx, dReal fy, dReal fz);
  INLINE void add_torque(const LVecBase3f &f);
  INLINE void add_rel_force(dReal fx, dReal fy, dReal fz);
  INLINE void add_rel_force(const LVecBase3f &f);
  INLINE void add_rel_torque(dReal fx, dReal fy, dReal fz);
  INLINE void add_rel_torque(const LVecBase3f &f);
  INLINE void add_force_at_pos(dReal fx, dReal fy, dReal fz,
                               dReal px, dReal py, dReal pz);
  INLINE void add_force_at_pos(const LVecBase3f &f,
                               const LVecBase3f &pos);
  INLINE void add_force_at_rel_pos(dReal fx, dReal fy, dReal fz,
                                   dReal px, dReal py, dReal pz);
  INLINE void add_force_at_rel_pos(const LVecBase3f &f,
                                   const LVecBase3f &pos);
  INLINE void add_rel_force_at_pos(dReal fx, dReal fy, dReal fz,
                                   dReal px, dReal py, dReal pz);
  INLINE void add_rel_force_at_pos(const LVecBase3f &f,
                                   const LVecBase3f &pos);
  INLINE void add_rel_force_at_rel_pos(dReal fx, dReal fy, dReal fz,
                                       dReal px, dReal py, dReal pz);
  INLINE void add_rel_force_at_rel_pos(const LVecBase3f &f,
                                       const LVecBase3f &pos);
  INLINE void set_force(dReal x, dReal y, dReal z);
  INLINE void set_force(const LVecBase3f &f);
  INLINE void set_torque(dReal x, dReal y, dReal z);
  INLINE void set_torque(const LVecBase3f &f);

  INLINE LPoint3f get_rel_point_pos(dReal px, dReal py, dReal pz) const;
  INLINE LPoint3f get_rel_point_pos(const LVecBase3f &pos) const;
  INLINE LPoint3f get_rel_point_vel(dReal px, dReal py, dReal pz) const;
  INLINE LPoint3f get_rel_point_vel(const LVecBase3f &pos) const;
  INLINE LPoint3f get_point_vel(dReal px, dReal py, dReal pz) const;
  INLINE LPoint3f get_point_vel(const LVecBase3f &pos) const;
  INLINE LPoint3f get_pos_rel_point(dReal px, dReal py, dReal pz) const;
  INLINE LPoint3f get_pos_rel_point(const LVecBase3f &pos) const;
  INLINE LVecBase3f vector_to_world(dReal px, dReal py, dReal pz) const;
  INLINE LVecBase3f vector_to_world(const LVecBase3f &pos) const;
  INLINE LVecBase3f vector_from_world(dReal px, dReal py, dReal pz) const;
  INLINE LVecBase3f vector_from_world(const LVecBase3f &pos) const;

  INLINE void set_finite_rotation_mode(int mode);
  INLINE void set_finite_rotation_axis(dReal x, dReal y, dReal z);
  INLINE void set_finite_rotation_axis(const LVecBase3f &axis);
  INLINE int get_finite_rotation_mode() const;
  INLINE LVecBase3f get_finite_rotation_axis() const;

  INLINE int get_num_joints() const;
  OdeJoint get_joint(int index) const;
  MAKE_SEQ(get_joints, get_num_joints, get_joint);
  EXTENSION(INLINE PyObject *get_converted_joint(int i) const);
  MAKE_SEQ_PROPERTY(joints, get_num_joints, get_converted_joint);

  INLINE void enable();
  INLINE void disable();
  INLINE int is_enabled() const;
  INLINE void set_gravity_mode(int mode);
  INLINE int get_gravity_mode() const;

  virtual void write(std::ostream &out = std::cout, unsigned int indent=0) const;
  operator bool () const;
  INLINE int compare_to(const OdeBody &other) const;

private:
  dBodyID _id;

public:
  typedef void (*DestroyCallback)(OdeBody &body);
  DestroyCallback _destroy_callback = nullptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeBody",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeBody.I"

#endif
