/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularEulerIntegrator.cxx
 * @author charles
 * @date 2000-08-09
 */

#include "angularEulerIntegrator.h"
#include "forceNode.h"
#include "physicalNode.h"
#include "config_physics.h"

/**
 * constructor
 */
AngularEulerIntegrator::
AngularEulerIntegrator() {
}

/**
 * destructor
 */
AngularEulerIntegrator::
~AngularEulerIntegrator() {
}

/**
 * Integrate a step of motion (based on dt) by applying every force in
 * force_vec to every object in obj_vec.
 */
void AngularEulerIntegrator::
child_integrate(Physical *physical,
                AngularForceVector& forces,
                PN_stdfloat dt) {
  // Loop through each object in the set.  This processing occurs in O(pf)
  // time, where p is the number of physical objects and f is the number of
  // forces.  Unfortunately, no precomputation of forces can occur, as each
  // force is possibly contingent on such things as the position and velocity
  // of each physicsobject in the set.  Accordingly, we have to grunt our way
  // through each one.  wrt caching of the xform matrix should help.
  PhysicsObject::Vector::const_iterator current_object_iter;
  current_object_iter = physical->get_object_vector().begin();
  for (; current_object_iter != physical->get_object_vector().end();
       ++current_object_iter) {
    PhysicsObject *current_object = *current_object_iter;

    // bail out if this object doesn't exist or doesn't want to be processed.
    if (current_object == nullptr) {
      continue;
    }

    if (current_object->get_active() == false) {
      continue;
    }

    LRotation accum_quat(0, 0, 0, 0);

    // set up the traversal stuff.
    AngularForceVector::const_iterator f_cur;

    LRotation f;

    // global forces
    f_cur = forces.begin();
    // unsigned int index = 0;
    for (; f_cur != forces.end(); ++f_cur) {
      AngularForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      // tally it into the accumulation quaternion
      f = cur_force->get_quat(current_object);
      accum_quat += f;
    }

    LOrientation orientation = current_object->get_orientation();
    // local forces
    f_cur = physical->get_angular_forces().begin();
    for (; f_cur != physical->get_angular_forces().end(); ++f_cur) {
      AngularForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      f = cur_force->get_quat(current_object);

      // tally it into the accumulation quaternion i.e.  orientation * f *
      // orientation.conjugate()
      accum_quat += orientation.xform(f);
    }

    // apply the accumulated torque vector to the object's inertial tensor.
    // this matrix represents how much force the object 'wants' applied to it
    // in any direction, among other things.
    accum_quat = current_object->get_inertial_tensor() * accum_quat;

    // derive this into the angular velocity vector.
    LRotation rot_quat = current_object->get_rotation();
    #if 0
    rot_quat += accum_quat * dt;

    if (rot_quat.normalize()) {
      LOrientation old_orientation = current_object->get_orientation();
      LOrientation new_orientation = old_orientation * rot_quat;
      new_orientation.normalize();

      // and write the results back.
      current_object->set_orientation(new_orientation);
      current_object->set_rotation(rot_quat);
    }
    #else
    // accum_quat*=viscosityDamper; LOrientation orientation =
    // current_object->get_orientation();

    // accum_quat.normalize(); x = x + v * t + 0.5 * a * t * t
    orientation = orientation * ((rot_quat * dt) * (accum_quat * (0.5 * dt * dt)));
    // v = v + a * t
    rot_quat = rot_quat + (accum_quat * dt);

    // if (rot_quat.normalize()) {
    if (orientation.normalize() && rot_quat.normalize()) {
      // and write the results back.
      current_object->set_orientation(orientation);
      current_object->set_rotation(rot_quat);
    }
    #endif
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularEulerIntegrator::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularEulerIntegrator (id "<<this<<")";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularEulerIntegrator::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularEulerIntegrator:\n";
  AngularIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
