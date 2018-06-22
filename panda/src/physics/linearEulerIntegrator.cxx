/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearEulerIntegrator.cxx
 * @author charles
 * @date 2000-06-13
 */

#include "linearEulerIntegrator.h"
#include "forceNode.h"
#include "physicalNode.h"
#include "config_physics.h"

/**
 * constructor
 */
LinearEulerIntegrator::
LinearEulerIntegrator() {
}

/**
 * destructor
 */
LinearEulerIntegrator::
~LinearEulerIntegrator() {
}

/**
 * Integrate a step of motion (based on dt) by applying every force in
 * force_vec to every object in obj_vec.
 *
 * physical, The objects being acted upon and the set of local forces that are
 * applied after the global forces.  forces, Global forces to be applied
 * first.  dt, The delta time of this integration step.
 */
void LinearEulerIntegrator::
child_integrate(Physical *physical,
                LinearForceVector& forces,
                PN_stdfloat dt) {
  // perform the precomputation.  Note that the vector returned by
  // get_precomputed_matrices() has the matrices loaded in order of force
  // type: first global, then local.  If you're using this as a guide to write
  // another integrator, be sure to process your forces global, then local.
  // otherwise your transforms will be VERY bad.
  precompute_linear_matrices(physical, forces);
  const MatrixVector &matrices = get_precomputed_linear_matrices();
#ifndef NDEBUG
  MatrixVector::const_iterator mi;
  for (mi = matrices.begin(); mi != matrices.end(); ++mi) {
    nassertv(!(*mi).is_nan());
  }
#endif  // NDEBUG

  // Get the greater of the local or global viscosity:
  PN_stdfloat viscosityDamper=1.0f-physical->get_viscosity();

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

    LVector3 md_accum_vec; // mass dependent accumulation vector.
    LVector3 non_md_accum_vec;
    LVector3 accel_vec;
    LVector3 vel_vec;

    // reset the accumulation vectors for this object
    md_accum_vec.set(0.0f, 0.0f, 0.0f);
    non_md_accum_vec.set(0.0f, 0.0f, 0.0f);

    // run through each acting force and sum it
    LVector3 f;
    // LMatrix4 force_to_object_xform;

    LinearForceVector::const_iterator f_cur;

    // global forces
    f_cur = forces.begin();
    int index = 0;
    for (; f_cur != forces.end(); ++f_cur) {
      LinearForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      // now we go from force space to our object's space.
      f = cur_force->get_vector(current_object) * matrices[index++];

      physics_spam("child_integrate "<<f);
      // tally it into the accum vectors.
      if (cur_force->get_mass_dependent() == true) {
        md_accum_vec += f;
      } else {
        non_md_accum_vec += f;
      }
    }

    // local forces
    f_cur = physical->get_linear_forces().begin();
    for (; f_cur != physical->get_linear_forces().end(); ++f_cur) {
      LinearForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      // go from force space to object space
      f = cur_force->get_vector(current_object) * matrices[index++];

      physics_spam("child_integrate "<<f);
      // tally it into the accum vectors
      if (cur_force->get_mass_dependent() == true) {
        md_accum_vec += f;
      } else {
        non_md_accum_vec += f;
      }
    }

    // get this object's physical info
    LPoint3 pos = current_object->get_position();
    vel_vec = current_object->get_velocity();
    PN_stdfloat mass = current_object->get_mass();

    // we want 'a' in F = ma get it by computing F  m
    nassertv(mass != 0.0f);
    accel_vec = md_accum_vec / mass;
    accel_vec += non_md_accum_vec;

    #if 0 //[
    // step the position and velocity
    vel_vec += accel_vec * dt;

    // cap terminal velocity
    PN_stdfloat len = vel_vec.length();

    if (len > current_object->get_terminal_velocity()) {
      // cout << "Capping terminal velocity at: " <<
      // current_object->get_terminal_velocity() << endl;
      vel_vec *= current_object->get_terminal_velocity() / len;
    }

    pos += vel_vec * dt;
    #else //][
    assert(current_object->get_position()==current_object->get_last_position());

    accel_vec*=viscosityDamper;

    // x = x + v * t + 0.5 * a * t * t
    pos += vel_vec * dt + 0.5 * accel_vec * dt * dt;
    // v = v + a * t
    vel_vec += accel_vec * dt;
    #endif //]

    // and store them back.
    if (!pos.is_nan()) {
      current_object->set_position(pos);
    }
    if (!vel_vec.is_nan()) {
      current_object->set_velocity(vel_vec);
    }
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearEulerIntegrator::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearEulerIntegrator";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearEulerIntegrator::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"LinearEulerIntegrator:\n";
  LinearIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
