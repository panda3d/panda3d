#ifndef ODENULLJOINT_H
#define ODENULLJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeNullJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeNullJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeNullJoint(dJointID id);

PUBLISHED:
  OdeNullJoint(OdeWorld &world);
  OdeNullJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeNullJoint();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeNullJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeNullJoint.I"

#endif
