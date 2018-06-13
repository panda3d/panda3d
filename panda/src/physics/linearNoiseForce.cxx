/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearNoiseForce.cxx
 * @author charles
 * @date 2000-06-13
 */

#include <stdlib.h>
#include <math.h>

#include "linearNoiseForce.h"

// declare the statics

ConfigVariableInt LinearNoiseForce::_random_seed
("default_noise_force_seed", 665);

bool LinearNoiseForce::_initialized = false;
unsigned char LinearNoiseForce::_prn_table[256];
LVector3 LinearNoiseForce::_gradient_table[256];

TypeHandle LinearNoiseForce::_type_handle;

/**
 * One-time config function, sets up the PRN lattice.
 */
void LinearNoiseForce::
init_noise_tables() {
  // since this is a repeatable noise function, we always want to init with
  // the same seed.
  srand(_random_seed);

  LVector3 *gtable = _gradient_table;
  for (int i = 0; i < 256; ++i) {
    // fill the 1d array
    _prn_table[i] = rand() & 255;

    // now fill the gradient array
    *gtable++ = random_unit_vector();
  }
}

/**
 * constructor
 */
LinearNoiseForce::
LinearNoiseForce(PN_stdfloat a, bool mass) :
  LinearRandomForce(a, mass) {
  if (_initialized == false) {
    init_noise_tables();
    _initialized = true;
  }
}

/**
 * copy constructor
 */
LinearNoiseForce::
LinearNoiseForce(const LinearNoiseForce &copy) :
  LinearRandomForce(copy) {
}

/**
 * destructor
 */
LinearNoiseForce::
~LinearNoiseForce() {
}

/**
 * copier
 */
LinearForce *LinearNoiseForce::
make_copy() {
  return new LinearNoiseForce(*this);
}

/**
 * Returns the noise value based on the object's position.
 */
LVector3 LinearNoiseForce::
get_child_vector(const PhysicsObject *po) {
  LPoint3 p = po->get_position();

  // get all of the components
  int int_x, int_y, int_z;
  PN_stdfloat frac_x, frac_y, frac_z;

  int_x = (int) p[0];
  frac_x = p[0] - int_x;

  int_y = (int) p[1];
  frac_y = p[1] - int_y;

  int_z = (int) p[2];
  frac_z = p[2] - int_z;

  // apply the cubic smoother to the fractional values
  PN_stdfloat cubic_x, cubic_y, cubic_z;

  cubic_x = cubic_step(frac_x);
  cubic_y = cubic_step(frac_y);
  cubic_z = cubic_step(frac_z);

  // trilinear interpolation into the cube (over, in, down)
  LVector3 temp0, temp1, temp2, temp3;

  // x direction
  temp0 = vlerp(cubic_x, get_lattice_entry(p),
                get_lattice_entry(p[0] + 1.0f, p[1], p[2]));

  temp1 = vlerp(cubic_x, get_lattice_entry(p[0], p[1], p[2] + 1.0f),
                get_lattice_entry(p[0] + 1.0f, p[1], p[2] + 1.0f));

  temp2 = vlerp(cubic_x, get_lattice_entry(p[0], p[1] + 1.0f, p[2]),
                get_lattice_entry(p[0] + 1.0f, p[1] + 1.0f, p[2]));

  temp3 = vlerp(cubic_x, get_lattice_entry(p[0], p[1] + 1.0f, p[2] + 1.0f),
                get_lattice_entry(p[0] + 1.0f, p[1] + 1.0f, p[2] + 1.0f));

  // y direction
  temp0 = vlerp(cubic_z, temp0, temp1);
  temp1 = vlerp(cubic_z, temp2, temp3);

  // z direction
  return vlerp(cubic_y, temp0, temp1);
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearNoiseForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"LinearNoiseForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearNoiseForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"LinearNoiseForce:";
  out.width(indent+2); out<<""; out<<"_random_seed"<<_random_seed<<" (class)\n";
  out.width(indent+2); out<<""; out<<"_initialized"<<_initialized<<" (class)\n";
  out.width(indent+2); out<<""; out<<"_gradient_table"<<_gradient_table<<" (class)\n";
  out.width(indent+2); out<<""; out<<"_prn_table"<<_prn_table<<" (class)\n";
  LinearRandomForce::write(out, indent+2);
  #endif //] NDEBUG
}
