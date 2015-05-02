// Filename: odeJointGroup.h
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ODEJOINTGROUP_H
#define ODEJOINTGROUP_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeJointGroup
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeJointGroup : public TypedObject {
PUBLISHED:
  OdeJointGroup();
  virtual ~OdeJointGroup();
  void destroy();
  INLINE dJointGroupID get_id() const;

  INLINE void empty() const;

  INLINE int compare_to(const OdeJointGroup &other) const;


private:
  dJointGroupID _id;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeJointGroup",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeJointGroup.I"

#endif

