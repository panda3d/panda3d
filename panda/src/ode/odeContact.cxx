// Filename: odeContact.cxx
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

#include "config_ode.h"
#include "odeContact.h"

TypeHandle OdeContact::_type_handle;

OdeContact::
OdeContact() : 
  _contact() {
}

OdeContact::
OdeContact(const dContact &contact) : 
  _contact(contact) {
}

OdeContact::
~OdeContact() {
}

const dContact* OdeContact::
get_contact_ptr() const {
  return &_contact;
}
