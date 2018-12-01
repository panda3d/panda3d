/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeMass.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeMass.h"

TypeHandle OdeMass::_type_handle;

OdeMass::
OdeMass() :
  _mass() {
  dMassSetZero(&_mass);
}

OdeMass::
OdeMass(const OdeMass &copy) :
  _mass() {
  _mass.setParameters(copy._mass.mass,
                      copy._mass.c[0], copy._mass.c[1], copy._mass.c[2],
                      copy._mass.I[0], copy._mass.I[5], copy._mass.I[10],
                      copy._mass.I[1], copy._mass.I[2], copy._mass.I[4]);
}

OdeMass::
~OdeMass() {
}

dMass* OdeMass::
get_mass_ptr() {
  return &_mass;
}

void OdeMass::
operator = (const OdeMass &copy) {
  _mass.setParameters(copy._mass.mass,
                      copy._mass.c[0], copy._mass.c[1], copy._mass.c[2],
                      copy._mass.I[0], copy._mass.I[5], copy._mass.I[10],
                      copy._mass.I[1], copy._mass.I[2], copy._mass.I[4]);

}


void OdeMass::
write(std::ostream &out, unsigned int indent) const {
  out.width(indent);
  out << get_type() \
      << "(mag = " << get_magnitude() \
      << ", center = " << get_center() \
      << ", inertia = " << get_inertial_tensor() \
      << ")";
}
