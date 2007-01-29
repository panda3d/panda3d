// Filename: odeJoint.h
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ODEJOINT_H
#define ODEJOINT_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode/ode.h"
#include "odeWorld.h" // Needed for derived classes
#include "odeJointGroup.h"


class OdeBody;


////////////////////////////////////////////////////////////////////
//       Class : OdeJoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeJoint : public TypedObject {
  friend class OdeBody;
  friend class OdeUtil;

protected:
  OdeJoint(dJointID id);

PUBLISHED:
  virtual ~OdeJoint();
  void destroy();

  INLINE void set_data(void *data);
  INLINE void *get_data();
  INLINE int get_joint_type() const;
  void get_body(int index, OdeBody &body) const;
  INLINE void set_feedback(dJointFeedback *);
  INLINE dJointFeedback *get_feedback();
   
  void attach(const OdeBody &body1, const OdeBody &body2);
  void attach(const OdeBody &body, int index);
  void detach();

  virtual void write(ostream &out = cout, unsigned int indent=0) const;

public: 
  INLINE dJointID get_id() const;

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
