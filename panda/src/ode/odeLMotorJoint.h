#ifndef ODELMOTORJOINT_H
#define ODELMOTORJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode/ode.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeLMotorJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeLMotorJoint : public OdeJoint {
PUBLISHED:
  OdeLMotorJoint(OdeWorld &world);
  OdeLMotorJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeLMotorJoint();

  INLINE void set_l_motor_num_axes(int num);
  INLINE void set_l_motor_axis(int anum, int rel, dReal x, dReal y, dReal z);
  INLINE void set_l_motor_param(int parameter, dReal value);

  INLINE int get_l_motor_num_axes() const;
  INLINE void get_l_motor_axis(int anum, dVector3 result) const;
  INLINE dReal get_l_motor_param(int parameter) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeLMotorJoint",
		  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeLMotorJoint.I"

#endif
