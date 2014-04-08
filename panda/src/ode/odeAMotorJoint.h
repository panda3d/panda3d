#ifndef ODEAMOTORJOINT_H
#define ODEAMOTORJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeAMotorJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeAMotorJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeAMotorJoint(dJointID id);

PUBLISHED:
  OdeAMotorJoint(OdeWorld &world);
  OdeAMotorJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeAMotorJoint();

  INLINE void set_num_axes(int num);
  INLINE void set_axis(int anum, int rel, dReal x, dReal y, dReal z);
  INLINE void set_axis(int anum, int rel, const LVecBase3f &axis);
  INLINE void set_angle(int anum, dReal angle);
  INLINE void set_mode(int mode);
  INLINE void add_torques(dReal torque1, dReal torque2, dReal torque3);

  INLINE int get_num_axes() const;
  INLINE LVecBase3f get_axis(int anum) const;
  MAKE_SEQ(get_axes, get_num_axes, get_axis);
  INLINE int get_axis_rel(int anum) const;
  INLINE dReal get_angle(int anum) const;
  INLINE dReal get_angle_rate(int anum) const;
  INLINE int get_mode() const;

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
    register_type(_type_handle, "OdeAMotorJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeAMotorJoint.I"

#endif
