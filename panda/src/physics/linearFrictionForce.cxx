/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearFrictionForce.cxx
 * @author charles
 * @date 2000-06-23
 */

#include "linearFrictionForce.h"
#include "config_physics.h"

TypeHandle LinearFrictionForce::_type_handle;

/**
 * Constructor
 */
LinearFrictionForce::
LinearFrictionForce(PN_stdfloat coef, PN_stdfloat a, bool m) :
  LinearForce(a, m) {
  set_coef(coef);
}

/**
 * copy constructor
 */
LinearFrictionForce::
LinearFrictionForce(const LinearFrictionForce &copy) :
  LinearForce(copy) {
  _coef = copy._coef;
}

/**
 * destructor
 */
LinearFrictionForce::
~LinearFrictionForce() {
}

/**
 * copier
 */
LinearForce *LinearFrictionForce::
make_copy() {
  return new LinearFrictionForce(*this);
}

/**
 * Constructor
 */
LVector3 LinearFrictionForce::
get_child_vector(const PhysicsObject* po) {
  LVector3 v = po->get_velocity();
  assert(_coef>=0.0f && _coef<=1.0f);
  // Create a force vector in the opposite direction of v:
  LVector3 friction = v * -_coef;
  physics_debug(" v "<<v<<" len "<<v.length()
      <<" friction "<<friction<<" len "<<friction.length()
      <<" dot "<<(normalize(v).dot(normalize(friction))));
  assert(friction.almost_equal(LVector3::zero())
      || IS_NEARLY_EQUAL(normalize(v).dot(normalize(friction)), -1.0f));
  // cary said to cap this at zero so that friction can't reverse your
  // direction, but it seems to me that if you're computing: v + (-v * _coef),
  // _coef in [0, 1] that this will always be greater than or equal to zero.
  return friction;
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearFrictionForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearFrictionForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearFrictionForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearFrictionForce:\n";
  out.width(indent+2); out<<""; out<<"_coef "<<_coef<<":\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
