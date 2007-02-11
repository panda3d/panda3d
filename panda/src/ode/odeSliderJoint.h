#ifndef ODESLIDERJOINT_H
#define ODESLIDERJOINT_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeSliderJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeSliderJoint : public OdeJoint {
  friend class OdeJoint;

private:
  OdeSliderJoint(dJointID id);

PUBLISHED:
  OdeSliderJoint(OdeWorld &world);
  OdeSliderJoint(OdeWorld &world, OdeJointGroup &joint_group);
  virtual ~OdeSliderJoint();

  INLINE void set_slider_axis(dReal x, dReal y, dReal z);
  INLINE void set_slider_axis_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az);
  INLINE void set_slider_param(int parameter, dReal value);
  INLINE void add_slider_force(dReal force);

  INLINE dReal get_slider_position() const;
  INLINE dReal get_slider_position_rate() const;
  INLINE void get_slider_axis(dVector3 result) const;
  INLINE dReal get_slider_param(int parameter) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeSliderJoint",
		  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeSliderJoint.I"

#endif
