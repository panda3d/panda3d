/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUniversalJoint.h
 * @author joswilso
 * @date 2006-12-27
 */



#ifndef ODEUNIVERSALJOINT_H
#define ODEUNIVERSALJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

/**
 *
 */
class EXPCL_PANDAODE OdeUniversalJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeUniversalJoint(dJointID id);

PUBLISHED:
  OdeUniversalJoint(OdeWorld &world);
  OdeUniversalJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeUniversalJoint();

  INLINE void set_anchor(dReal x, dReal y, dReal z);
  INLINE void set_anchor(const LVecBase3f &anchor);
  INLINE void set_axis1(dReal x, dReal y, dReal z);
  INLINE void set_axis1(const LVecBase3f &axis);
  INLINE void set_axis2(dReal x, dReal y, dReal z);
  INLINE void set_axis2(const LVecBase3f &axis);
  INLINE void add_torques(dReal torque1, dReal torque2);

  INLINE LVecBase3f get_anchor() const;
  INLINE LVecBase3f get_anchor2() const;
  INLINE LVecBase3f get_axis1() const;
  INLINE LVecBase3f get_axis2() const;
  INLINE dReal get_angle1() const;
  INLINE dReal get_angle2() const;
  INLINE dReal get_angle1_rate() const;
  INLINE dReal get_angle2_rate() const;

  INLINE void set_param_lo_stop(int axis, dReal val);
  INLINE void set_param_hi_stop(int axis, dReal val);
  INLINE void set_param_vel(int axis, dReal val);
  INLINE void set_param_f_max(int axis, dReal val);
  INLINE void set_param_fudge_factor(int axis, dReal val);
  INLINE void set_param_bounce(int axis, dReal val);
  INLINE void set_param_CFM(int axis, dReal val);
  INLINE void set_param_stop_ERP(int axis, dReal val);
  INLINE void set_param_stop_CFM(int axis, dReal val);

  INLINE dReal get_param_lo_stop(int axis) const;
  INLINE dReal get_param_hi_stop(int axis) const;
  INLINE dReal get_param_vel(int axis) const;
  INLINE dReal get_param_f_max(int axis) const;
  INLINE dReal get_param_fudge_factor(int axis) const;
  INLINE dReal get_param_bounce(int axis) const;
  INLINE dReal get_param_CFM(int axis) const;
  INLINE dReal get_param_stop_ERP(int axis) const;
  INLINE dReal get_param_stop_CFM(int axis) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeUniversalJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeUniversalJoint.I"

#endif
