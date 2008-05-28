// Filename: baseIntegrator.cxx
// Created by:  charles (11Aug00)
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

#include "baseIntegrator.h"
#include "physicalNode.h"
#include "forceNode.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseIntegrator
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseIntegrator::
BaseIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseIntegrator
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
BaseIntegrator::
~BaseIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : precompute_linear_matrices
//      Access : protected
// Description : effectively caches the xform matrices between
//               the physical's node and every force acting on it
//               so that each PhysicsObject in the set held by the
//               Physical doesn't have to wrt.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
precompute_linear_matrices(Physical *physical,
                           const LinearForceVector &forces) {
  nassertv(physical);
  // make sure the physical's in the scene graph, somewhere.
  PhysicalNode *physical_node = physical->get_physical_node();
  nassertv(physical_node);

  // by global forces, we mean forces not contained in the physical
  int global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  int local_force_vec_size = physical->get_linear_forces().size();

  ForceNode *force_node;

  // prepare the vector
  _precomputed_linear_matrices.clear();
  _precomputed_linear_matrices.reserve(
      global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical->get_physical_node_path());
  NodePath parent_physical_np = physical_np.get_parent();

  // tally the global xforms
  LinearForceVector::const_iterator fi;
  for (fi = forces.begin(); fi != forces.end(); ++fi) {
    //LinearForce *cur_force = *fi;
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_linear_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }

  // tally the local xforms
  const LinearForceVector &force_vector = physical->get_linear_forces();
  for (fi = force_vector.begin(); fi != force_vector.end(); ++fi) {
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_linear_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }
}

////////////////////////////////////////////////////////////////////
//    Function : precompute_angular_matrices
//      Access : protected
// Description : effectively caches the xform matrices between
//               the physical's node and every force acting on it
//               so that each PhysicsObject in the set held by the
//               Physical doesn't have to wrt.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
precompute_angular_matrices(Physical *physical,
                            const AngularForceVector &forces) {
  nassertv(physical);
  // make sure the physical's in the scene graph, somewhere.
  PhysicalNode *physical_node = physical->get_physical_node();
  nassertv(physical_node != NULL);

  // by global forces, we mean forces not contained in the physical
  int global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  int local_force_vec_size = physical->get_angular_forces().size();

  ForceNode *force_node;

  // prepare the vector
  _precomputed_angular_matrices.clear();
  _precomputed_angular_matrices.reserve(
      global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical->get_physical_node_path());
  NodePath parent_physical_np = physical_np.get_parent();

  // tally the global xforms
  AngularForceVector::const_iterator fi;
  for (fi = forces.begin(); fi != forces.end(); ++fi) {
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_angular_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }

  // tally the local xforms
  const AngularForceVector &force_vector = physical->get_angular_forces();
  for (fi = force_vector.begin(); fi != force_vector.end(); ++fi) {
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np = (*fi)->get_force_node_path();
    _precomputed_angular_matrices.push_back(
        force_np.get_transform(parent_physical_np)->get_mat());
  }
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseIntegrator (id "<<this<<")";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write_precomputed_linear_matrices
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
write_precomputed_linear_matrices(ostream &out, unsigned int indent) const {
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

////////////////////////////////////////////////////////////////////
//     Function : write_precomputed_angular_matrices
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
write_precomputed_angular_matrices(ostream &out, unsigned int indent) const {
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

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseIntegrator::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseIntegrator:\n";
  write_precomputed_linear_matrices(out, indent+2);
  write_precomputed_angular_matrices(out, indent+2);
  #endif //] NDEBUG
}
