#ifndef ODEBALLJOINT_H
#define ODEBALLJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeBallJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeBallJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeBallJoint(dJointID id);

PUBLISHED:
  OdeBallJoint(OdeWorld &world);
  OdeBallJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeBallJoint();

  INLINE void set_anchor(dReal x, dReal y, dReal z);
  INLINE void set_anchor(const LVecBase3f &anchor);
  INLINE void set_anchor2(dReal x, dReal y, dReal z);
  INLINE void set_anchor2(const LVecBase3f &anchor);

  INLINE LVecBase3f get_anchor() const;
  INLINE LVecBase3f get_anchor2() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeBallJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeBallJoint.I"

#endif
