#ifndef ODEPLANE2DJOINT_H
#define ODEPLANE2DJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdePlane2dJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdePlane2dJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdePlane2dJoint(dJointID id);

PUBLISHED:
  OdePlane2dJoint(OdeWorld &world);
  OdePlane2dJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdePlane2dJoint();

  INLINE void set_x_param(int parameter, dReal value);
  INLINE void set_y_param(int parameter, dReal value);
  INLINE void set_angle_param(int parameter, dReal value);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdePlane2dJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odePlane2dJoint.I"

#endif
