#ifndef ODESLIDERJOINT_H
#define ODESLIDERJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeSliderJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeSliderJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeSliderJoint(dJointID id);

PUBLISHED:
  OdeSliderJoint(OdeWorld &world);
  OdeSliderJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeSliderJoint();

  INLINE void set_axis(dReal x, dReal y, dReal z);
  INLINE void set_axis(const LVecBase3f &axis);
  INLINE void set_axis_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az);
  INLINE void set_axis_delta(const LVecBase3f &axis, const LVecBase3f &vec);
  INLINE void add_force(dReal force);

  INLINE dReal get_position() const;
  INLINE dReal get_position_rate() const;
  INLINE LVecBase3f get_axis() const;

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
    register_type(_type_handle, "OdeSliderJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeSliderJoint.I"

#endif
