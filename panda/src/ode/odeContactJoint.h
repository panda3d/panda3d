/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeContactJoint.h
 * @author joswilso
 * @date 2006-12-27
 */


#ifndef ODECONTACTJOINT_H
#define ODECONTACTJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

#include "odeJoint.h"
#include "odeContact.h"

/**
 *
 */
class EXPCL_PANDAODE OdeContactJoint : public OdeJoint {
  friend class OdeJoint;

public:
  OdeContactJoint(dJointID id);

PUBLISHED:
  OdeContactJoint(OdeWorld &world, const OdeContact &contact);
  OdeContactJoint(OdeWorld &world, OdeJointGroup &joint_group, const OdeContact &contact);
  virtual ~OdeContactJoint();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeJoint::init_type();
    register_type(_type_handle, "OdeContactJoint",
                  OdeJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeContactJoint.I"

#endif
