// Filename: linearNoiseForce.cxx
// Created by:  charles (13Jun00)
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

#include <stdlib.h>
#include <math.h>

#include "linearNoiseForce.h"

// declare the statics

bool LinearNoiseForce::_initialized = false;
unsigned char LinearNoiseForce::_prn_table[256];
LVector3f LinearNoiseForce::_gradient_table[256];

TypeHandle LinearNoiseForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : InitNoiseTables
//       Access : Public
//  Description : One-time config function, sets up the PRN
//                lattice.
////////////////////////////////////////////////////////////////////
void LinearNoiseForce::
init_noise_tables(void) {
  // since this is a repeatable noise function, we always want
  // to init with the same seed.
  srand(_random_seed);

  LVector3f *gtable = _gradient_table;
  for (int i = 0; i < 256; ++i) {
    // fill the 1d array
    _prn_table[i] = rand() & 255;

    // now fill the gradient array
    *gtable++ = random_unit_vector();
  }
}

////////////////////////////////////////////////////////////////////
//     Function : LinearNoiseForce
//       Access : Public
//  Description : constructor
////////////////////////////////////////////////////////////////////
LinearNoiseForce::
LinearNoiseForce(float a, bool mass) :
  LinearRandomForce(a, mass) {
  if (_initialized == false) {
    init_noise_tables();
    _initialized = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : LinearNoiseForce
//       Access : Public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearNoiseForce::
LinearNoiseForce(const LinearNoiseForce &copy) :
  LinearRandomForce(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function : ~LinearNoiseForce
//       Access : Public
//  Description : destructor
////////////////////////////////////////////////////////////////////
LinearNoiseForce::
~LinearNoiseForce() {
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : Public
//  Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearNoiseForce::
make_copy(void) {
  return new LinearNoiseForce(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : get_child_vector
//       Access : Public
//  Description : Returns the noise value based on the object's
//                position.
////////////////////////////////////////////////////////////////////
LVector3f LinearNoiseForce::
get_child_vector(const PhysicsObject *po) {
  LPoint3f p = po->get_position();

  // get all of the components
  int int_x, int_y, int_z;
  float frac_x, frac_y, frac_z;

  int_x = (int) p[0];
  frac_x = p[0] - int_x;

  int_y = (int) p[1];
  frac_y = p[1] - int_y;

  int_z = (int) p[2];
  frac_z = p[2] - int_z;

  // apply the cubic smoother to the fractional values
  float cubic_x, cubic_y, cubic_z;

  cubic_x = cubic_step(frac_x);
  cubic_y = cubic_step(frac_y);
  cubic_z = cubic_step(frac_z);

  // trilinear interpolation into the cube (over, in, down)
  LVector3f temp0, temp1, temp2, temp3;

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

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearNoiseForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"LinearNoiseForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearNoiseForce::
write(ostream &out, unsigned int indent) const {
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
