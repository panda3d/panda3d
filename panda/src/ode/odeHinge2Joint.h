#ifndef ODEHINGE2JOINT_H
#define ODEHINGE2JOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeHinge2Joint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeHinge2Joint : public OdeJoint {
PUBLISHED:
  OdeHinge2Joint(OdeWorld &world, OdeJointGroup &joint_group);
  OdeHinge2Joint(OdeWorld &world);
  virtual ~OdeHinge2Joint();

  INLINE void set_hinge2_anchor(dReal x, dReal y, dReal z);
  INLINE void set_hinge2_axis1(dReal x, dReal y, dReal z);
  INLINE void set_hinge2_axis2(dReal x, dReal y, dReal z);
  INLINE void set_hinge2_param(int parameter, dReal value);
  INLINE void add_hinge2_torques(dReal torque1, dReal torque2);

  INLINE void get_hinge2_anchor(dVector3 result) const;
  INLINE void get_hinge2_anchor2(dVector3 result) const;
  INLINE void get_hinge2_axis1(dVector3 result) const;
  INLINE void get_hinge2_axis2(dVector3 result) const;
  INLINE dReal get_hinge2_param(int parameter) const;
  INLINE dReal get_hinge2_angle1() const;
  INLINE dReal get_hinge2_angle1_rate() const;
  INLINE dReal get_hinge2_angle2_rate() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeHinge2Joint",
		  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeHinge2Joint.I"

#endif
