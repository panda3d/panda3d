/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeHingeJoint.h
 * @author joswilso
 * @date 2006-12-27
 */


#ifndef ODEHINGEJOINT_H
#define ODEHINGEJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

/**
 *
 */
class EXPCL_PANDAODE OdeHingeJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeHingeJoint(dJointID id);

PUBLISHED:
  OdeHingeJoint(OdeWorld &world);
  OdeHingeJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeHingeJoint();

  INLINE void set_anchor(dReal x, dReal y, dReal z);
  INLINE void set_anchor(const LVecBase3f &anchor);
  INLINE void set_anchor_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az);
  INLINE void set_anchor_delta(const LVecBase3f &anchor, const LVecBase3f &vec);
  INLINE void set_axis(dReal x, dReal y, dReal z);
  INLINE void set_axis(const LVecBase3f &axis);
  INLINE void add_torque(dReal torque);

  INLINE LVecBase3f get_anchor() const;
  INLINE LVecBase3f get_anchor2() const;
  INLINE LVecBase3f get_axis() const;
  INLINE dReal get_angle() const;
  INLINE dReal get_angle_rate() const;

  INLINE void set_param_lo_stop(dReal val);
  INLINE void set_param_hi_stop(dReal val);
  INLINE void set_param_vel(dReal val);
  INLINE void set_param_f_max(dReal val);
  INLINE void set_param_fudge_factor(dReal val);
  INLINE void set_param_bounce(dReal val);
  INLINE void set_param_CFM(dReal val);
  INLINE void set_param_stop_ERP(dReal val);
  INLINE void set_param_stop_CFM(dReal val);

  INLINE dReal get_param_lo_stop() const;
  INLINE dReal get_param_hi_stop() const;
  INLINE dReal get_param_vel() const;
  INLINE dReal get_param_f_max() const;
  INLINE dReal get_param_fudge_factor() const;
  INLINE dReal get_param_bounce() const;
  INLINE dReal get_param_CFM() const;
  INLINE dReal get_param_stop_ERP() const;
  INLINE dReal get_param_stop_CFM() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeHingeJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeHingeJoint.I"

#endif
