/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseIntegrator.cxx
 * @author charles
 * @date 2000-08-11
 */

#include "baseIntegrator.h"
#include "physicalNode.h"
#include "forceNode.h"
#include "nodePath.h"

using std::ostream;

/**
 * constructor
 */
BaseIntegrator::
BaseIntegrator() {
}

/**
 * destructor
 */
BaseIntegrator::
~BaseIntegrator() {
}

/**
 * effectively caches the xform matrices between the physical's node and every
 * force acting on it so that each PhysicsObject in the set held by the
 * Physical doesn't have to wrt.
 */
void BaseIntegrator::
precompute_linear_matrices(Physical *physical,
                           const LinearForceVector &forces) {
  nassertv(physical);
  // make sure the physical's in the scene graph, somewhere.
  nassertv(physical->get_physical_node() != nullptr);

  // by global forces, we mean forces not contained in the physical
  size_t global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  size_t local_force_vec_size = physical->get_linear_forces().size();

  // prepare the vector
  _precomputed_linear_matrices.clear();
  _precomputed_linear_matrices.reserve(
      global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical->get_physical_node_path());
  NodePath parent_physical_np = physical_np.get_parent();

  // tally the global xforms
  LinearForceVector::const_iterator fi;
  for (fi = forces.begin(); fi != forces.end(); ++fi) {
    // LinearForce *cur_force = *fi;
    nassertv((*fi)->get_force_node() != nullptr);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_linear_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }

  // tally the local xforms
  const LinearForceVector &force_vector = physical->get_linear_forces();
  for (fi = force_vector.begin(); fi != force_vector.end(); ++fi) {
    nassertv((*fi)->get_force_node() != nullptr);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_linear_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }
}

/**
 * effectively caches the xform matrices between the physical's node and every
 * force acting on it so that each PhysicsObject in the set held by the
 * Physical doesn't have to wrt.
 */
void BaseIntegrator::
precompute_angular_matrices(Physical *physical,
                            const AngularForceVector &forces) {
  nassertv(physical);
  // make sure the physical's in the scene graph, somewhere.
  nassertv(physical->get_physical_node() != nullptr);

  // by global forces, we mean forces not contained in the physical
  size_t global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  size_t local_force_vec_size = physical->get_angular_forces().size();

  // prepare the vector
  _precomputed_angular_matrices.clear();
  _precomputed_angular_matrices.reserve(
      global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical->get_physical_node_path());
  NodePath parent_physical_np = physical_np.get_parent();

  // tally the global xforms
  AngularForceVector::const_iterator fi;
  for (fi = forces.begin(); fi != forces.end(); ++fi) {
    nassertv((*fi)->get_force_node() != nullptr);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_angular_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }

  // tally the local xforms
  const AngularForceVector &force_vector = physical->get_angular_forces();
  for (fi = force_vector.begin(); fi != force_vector.end(); ++fi) {
    nassertv((*fi)->get_force_node() != nullptr);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_angular_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseIntegrator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseIntegrator (id "<<this<<")";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseIntegrator::
write_precomputed_linear_matrices(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_precomputed_linear_matrices\n";
  for (MatrixVector::const_iterator i=_precomputed_linear_matrices.begin();
       i != _precomputed_linear_matrices.end();
       ++i) {
    out.width(indent+2); out<<""; (*i).output(out); out<<"\n";
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseIntegrator::
write_precomputed_angular_matrices(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_precomputed_angular_matrices\n";
  for (MatrixVector::const_iterator i=_precomputed_angular_matrices.begin();
       i != _precomputed_angular_matrices.end();
       ++i) {
    out.width(indent+2); out<<""; (*i).output(out); out<<"\n";
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseIntegrator::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseIntegrator:\n";
  write_precomputed_linear_matrices(out, indent+2);
  write_precomputed_angular_matrices(out, indent+2);
  #endif //] NDEBUG
}
