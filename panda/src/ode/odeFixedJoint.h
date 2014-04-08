#ifndef ODEFIXEDJOINT_H
#define ODEFIXEDJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeFixedJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeFixedJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeFixedJoint(dJointID id);

PUBLISHED:
  OdeFixedJoint(OdeWorld &world);
  OdeFixedJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeFixedJoint();

  INLINE void set();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeFixedJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeFixedJoint.I"

#endif
