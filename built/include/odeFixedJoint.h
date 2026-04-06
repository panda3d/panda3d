/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeFixedJoint.h
 * @author joswilso
 * @date 2006-12-27
 */


#ifndef ODEFIXEDJOINT_H
#define ODEFIXEDJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"

/**
 *
 */
class EXPCL_PANDAODE OdeFixedJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeFixedJoint(dJointID id);

PUBLISHED:
  explicit OdeFixedJoint(OdeWorld &world);
  explicit OdeFixedJoint(OdeWorld &world, OdeJointGroup &joint_group);
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
