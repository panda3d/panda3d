// Filename: angularEulerIntegrator.cxx
// Created by:  charles (09Aug00)
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

#include "angularEulerIntegrator.h"
#include "forceNode.h"
#include "physicalNode.h"
#include "config_physics.h"

////////////////////////////////////////////////////////////////////
//     Function : AngularEulerIntegrator
//       Access : Public
//  Description : constructor
////////////////////////////////////////////////////////////////////
AngularEulerIntegrator::
AngularEulerIntegrator() {
}

////////////////////////////////////////////////////////////////////
//     Function : AngularEulerIntegrator
//       Access : Public
//  Description : destructor
////////////////////////////////////////////////////////////////////
AngularEulerIntegrator::
~AngularEulerIntegrator() {
}

////////////////////////////////////////////////////////////////////
//     Function : Integrate
//       Access : Public
//  Description : Integrate a step of motion (based on dt) by
//                applying every force in force_vec to every object
//                in obj_vec.
////////////////////////////////////////////////////////////////////
void AngularEulerIntegrator::
child_integrate(Physical *physical,
                AngularForceVector& forces,
                float dt) {
  // perform the precomputation.  Note that the vector returned by
  // get_precomputed_matrices() has the matrices loaded in order of force
  // type: first global, then local.  If you're using this as a guide to write
  // another integrator, be sure to process your forces global, then local.
  // otherwise your transforms will be VERY bad.  No good.
  precompute_angular_matrices(physical, forces);
  const pvector< LMatrix4f > &matrices = get_precomputed_angular_matrices();
  assert(matrices.size() == (forces.size() + physical->get_angular_forces().size()));

  // Loop through each object in the set.  This processing occurs in O(pf) time,
  // where p is the number of physical objects and f is the number of
  // forces.  Unfortunately, no precomputation of forces can occur, as
  // each force is possibly contingent on such things as the position and
  // velocity of each physicsobject in the set.  Accordingly, we have
  // to grunt our way through each one.  wrt caching of the xform matrix
  // should help.
  PhysicsObject::Vector::const_iterator current_object_iter;
  current_object_iter = physical->get_object_vector().begin();
  for (; current_object_iter != physical->get_object_vector().end();
       ++current_object_iter) {
    PhysicsObject *current_object = *current_object_iter;

    // bail out if this object doesn't exist or doesn't want to be
    // processed.
    if (current_object == (PhysicsObject *) NULL) {
      continue;
    }

    if (current_object->get_active() == false) {
      continue;
    }

    LVector3f accum_vec(0, 0, 0);

    // set up the traversal stuff.
    ForceNode *force_node;
    AngularForceVector::const_iterator f_cur;

    LVector3f f;

    // global forces
    f_cur = forces.begin();
    unsigned int index = 0;
    for (; f_cur != forces.end(); ++f_cur) {
      AngularForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      force_node = cur_force->get_force_node();

      // now we go from force space to our object's space.
      assert(index >= 0 && index < matrices.size());
      f = cur_force->get_vector(current_object) * matrices[index++];

      // tally it into the accum vector, applying the inertial tensor.
      accum_vec += f;
      cerr<<"global accum_vec"<<accum_vec<<"\n";
    }

    // local forces
    f_cur = physical->get_angular_forces().begin();
    for (; f_cur != physical->get_angular_forces().end(); ++f_cur) {
      AngularForce *cur_force = *f_cur;

      // make sure the force is turned on.
      if (cur_force->get_active() == false) {
        continue;
      }

      force_node = cur_force->get_force_node();

      // go from force space to object space
      assert(index >= 0 && index < matrices.size());
      f = cur_force->get_vector(current_object) * matrices[index++];

      // tally it into the accum vectors
      accum_vec += f;
      cerr<<"local accum_vec"<<accum_vec<<"\n";
    }
    assert(index == matrices.size());

    // apply the accumulated torque vector to the object's inertial tensor.
    // this matrix represents how much force the object 'wants' applied to it
    // in any direction, among other things.
    accum_vec =  accum_vec * current_object->get_inertial_tensor();
    cerr<<"tensor applied accum_vec"<<accum_vec<<"\n";

    // derive this into the angular velocity vector.
    LVector3f rot_vec = current_object->get_rotation();
    rot_vec += accum_vec * dt;
    cerr<<"accum_vec * dt"<<(accum_vec * dt)<<"\n";

    // here's the trick.  we've been accumulating these forces as vectors
    // and treating them as vectors, but now we're going to treat them as pure
    // imaginary quaternions where r = 0.  This vector NOW represents the
    // imaginary vector formed by (i, j, k).

    float len = rot_vec.length();

    if (len) {
      LVector3f normalized_rot_vec = rot_vec;
      normalized_rot_vec *= 1.0f / len;
      LRotationf rot_quat = LRotationf(normalized_rot_vec, len);

      LOrientationf old_orientation = current_object->get_orientation();
      LOrientationf new_orientation = old_orientation * rot_quat;
      new_orientation.normalize();

      // and write the results back.
      current_object->set_orientation(new_orientation);
      current_object->set_rotation(rot_vec);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularEulerIntegrator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularEulerIntegrator (id "<<this<<")";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularEulerIntegrator::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularEulerIntegrator:\n";
  AngularIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
