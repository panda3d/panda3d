// Filename: odeBody.cxx
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

#include "config_ode.h"
#include "odeBody.h"

TypeHandle OdeBody::_type_handle;

OdeBody::
OdeBody(dBodyID id) :
  _id(id) {
}

OdeBody::
OdeBody(OdeWorld &world) :
  _id(dBodyCreate(world.get_id())) {
      world.add_body_dampening(*this, 0);
}

OdeBody::
~OdeBody() {
}

void OdeBody::
destroy() {
  nassertv(_id);
  dBodyDestroy(_id);
}

OdeJoint OdeBody::
get_joint(int index) const {
  nassertr(_id != 0, OdeJoint(0));
  nassertr(index < get_num_joints(), OdeJoint(0)); 
  return OdeJoint(dBodyGetJoint(_id, index));
}

void OdeBody::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out << "" << get_type() \
			 << "(id = " << _id \
			 << ")";
  #endif //] NDEBUG
}
