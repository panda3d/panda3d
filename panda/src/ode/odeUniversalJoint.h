#ifndef ODEUNIVERSALJOINT_H
#define ODEUNIVERSALJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeUniversalJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeUniversalJoint : public OdeJoint {
PUBLISHED:
  OdeUniversalJoint(OdeWorld &world);
  OdeUniversalJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeUniversalJoint();

  INLINE void set_universal_anchor(dReal x, dReal y, dReal z);
  INLINE void set_universal_axis1(dReal x, dReal y, dReal z);
  INLINE void set_universal_axis2(dReal x, dReal y, dReal z);
  INLINE void set_universal_param(int parameter, dReal value);
  INLINE void add_universal_torques(dReal torque1, dReal torque2);

  INLINE void get_universal_anchor(dVector3 result) const;
  INLINE void get_universal_anchor2(dVector3 result) const;
  INLINE void get_universal_axis1(dVector3 result) const;
  INLINE void get_universal_axis2(dVector3 result) const;
  INLINE dReal get_universal_param(int parameter) const;
  INLINE dReal get_universal_angle1() const;
  INLINE dReal get_universal_angle2() const;
  INLINE dReal get_universal_angle1_rate() const;
  INLINE dReal get_universal_angle2_rate() const;

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
