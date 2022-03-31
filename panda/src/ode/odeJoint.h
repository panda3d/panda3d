/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJoint.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEJOINT_H
#define ODEJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeWorld.h"      // Needed for derived classes
#include "odeJointGroup.h"

class EXPCL_PANDAODE OdeJointFeedback : public dJointFeedback {
PUBLISHED:
  INLINE const LVector3f get_force1() const { return LVector3f(f1[0], f1[1], f1[2]); };
  INLINE const LVector3f get_force2() const { return LVector3f(f2[0], f2[1], f2[2]); };
  INLINE const LVector3f get_torque1() const { return LVector3f(t1[0], t1[1], t1[2]); };
  INLINE const LVector3f get_torque2() const { return LVector3f(t2[0], t2[1], t2[2]); };
};

// Strange, we should be forced to include this by get_body()
class OdeBody;

class OdeBallJoint;
class OdeHingeJoint;
class OdeSliderJoint;
class OdeContactJoint;
class OdeUniversalJoint;
class OdeHinge2Joint;
class OdeFixedJoint;
class OdeNullJoint;
class OdeAMotorJoint;
class OdeLMotorJoint;
class OdePlane2dJoint;

/**
 *
 */
class EXPCL_PANDAODE OdeJoint : public TypedObject {
  friend class OdeBody;
  friend class OdeUtil;

public:
  OdeJoint();
  OdeJoint(dJointID id);

PUBLISHED:
  enum JointType { JT_none = 0, /* or "unknown" */
                   JT_ball,
                   JT_hinge,
                   JT_slider,
                   JT_contact,
                   JT_universal,
                   JT_hinge2,
                   JT_fixed,
                   JT_null,
                   JT_a_motor,
                   JT_l_motor,
                   JT_plane2d };

  virtual ~OdeJoint();
  void destroy();
  INLINE bool is_empty() const;
  INLINE dJointID get_id() const;

  /* INLINE void set_data(void *data); */
  /* INLINE void *get_data(); */
  INLINE int get_joint_type() const;
  OdeBody get_body(int index) const;
  INLINE void set_feedback(OdeJointFeedback *);
  INLINE void set_feedback(bool flag = true);
  INLINE OdeJointFeedback *get_feedback();

  EXTENSION(void attach(PyObject *body1, PyObject *body2));
  void attach_bodies(const OdeBody &body1, const OdeBody &body2);
  void attach_body(const OdeBody &body, int index);
  void detach();

  virtual void write(std::ostream &out = std::cout, unsigned int indent=0) const;
  INLINE int compare_to(const OdeJoint &other) const;
  INLINE bool operator == (const OdeJoint &other) const;
  operator bool () const;

  EXTENSION(PyObject *convert() const);
  OdeBallJoint convert_to_ball() const;
  OdeHingeJoint convert_to_hinge() const;
  OdeSliderJoint convert_to_slider() const;
  OdeContactJoint convert_to_contact() const;
  OdeUniversalJoint convert_to_universal() const;
  OdeHinge2Joint convert_to_hinge2() const;
  OdeFixedJoint convert_to_fixed() const;
  OdeNullJoint convert_to_null() const;
  OdeAMotorJoint convert_to_a_motor() const;
  OdeLMotorJoint convert_to_l_motor() const;
  OdePlane2dJoint convert_to_plane2d() const;

protected:
  dJointID _id;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeJoint",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeJoint.I"

#endif
