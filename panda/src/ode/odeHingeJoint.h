#ifndef ODEHINGEJOINT_H
#define ODEHINGEJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeHingeJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeHingeJoint : public OdeJoint {
  friend class OdeJoint;

private:
  OdeHingeJoint(dJointID id);

PUBLISHED:
  OdeHingeJoint(OdeWorld &world);
  OdeHingeJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeHingeJoint();

  INLINE void set_hinge_anchor(dReal x, dReal y, dReal z);
  INLINE void set_hinge_anchor_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az);
  INLINE void set_hinge_axis(dReal x, dReal y, dReal z);
  INLINE void set_hinge_param(int parameter, dReal value);
  INLINE void add_hinge_torque(dReal torque);

  INLINE void get_hinge_anchor(dVector3 result) const;
  INLINE void get_hinge_anchor2(dVector3 result) const;
  INLINE void get_hinge_axis(dVector3 result) const;
  INLINE dReal get_hinge_param(int parameter) const;
  INLINE dReal get_hinge_angle() const;
  INLINE dReal get_hinge_angle_rate() const;

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
