#ifndef ODELMOTORJOINT_H
#define ODELMOTORJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeLMotorJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeLMotorJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeLMotorJoint(dJointID id);

PUBLISHED:
  OdeLMotorJoint(OdeWorld &world);
  OdeLMotorJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeLMotorJoint();

  INLINE void set_num_axes(int num);
  INLINE void set_axis(int anum, int rel, dReal x, dReal y, dReal z);
  INLINE void set_axis(int anum, int rel, const LVecBase3f &axis);
  INLINE void set_param(int parameter, dReal value);

  INLINE int get_num_axes() const;
  INLINE LVecBase3f get_axis(int anum) const;
  MAKE_SEQ(get_axes, get_num_axes, get_axis);
  INLINE dReal get_param(int parameter) const;

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
