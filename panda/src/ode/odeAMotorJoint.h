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

private:
  OdeAMotorJoint(dJointID id);

PUBLISHED:
  OdeAMotorJoint(OdeWorld &world);
  OdeAMotorJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeAMotorJoint();

  INLINE void set_num_axes(int num);
  INLINE void set_axis(int anum, int rel, dReal x, dReal y, dReal z);
  INLINE void set_angle(int anum, dReal angle);
  INLINE void set_param(int parameter, dReal value);
  INLINE void set_mode(int mode);
  INLINE void add_torques(dReal torque1, dReal torque2, dReal torque3);

  INLINE int get_num_axes() const;
  INLINE void get_axis(int anum, dVector3 result) const;
  INLINE int get_axis_rel(int anum) const;
  INLINE dReal get_angle(int anum) const;
  INLINE dReal get_angle_rate(int anum) const;
  INLINE dReal get_param(int parameter) const;
  INLINE int get_mode() const;

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
