// Filename: baseIntegrator.cxx
// Created by:  charles (11Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

  int i;
  // by global forces, we mean forces not contained in the physical
  int global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  int local_force_vec_size = physical->get_linear_forces().size();

  ForceNode *force_node;

  // prepare the vector
  _precomputed_linear_matrices.clear();
  _precomputed_linear_matrices.reserve(global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical_node);
  NodePath global_physical_np = physical_np.get_parent();

  #if 0

  // tally the global xforms
  for (i = 0; i < global_force_vec_size; ++i) {
    force_node = forces[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    //_precomputed_linear_matrices.push_back(global_physical_np.get_mat(force_node));
    _precomputed_linear_matrices.push_back(force_np.get_mat(global_physical_np));
  }
  #else
  // tally the global xforms
  for (LinearForceVector::const_iterator fi = forces.begin(); 
      fi != forces.end(); 
      ++fi) {
    //LinearForce *cur_force = *fi;
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    //_precomputed_linear_matrices.push_back(global_physical_np.get_mat(force_node));
    _precomputed_linear_matrices.push_back(force_np.get_mat(global_physical_np));
  }
  #endif




  const LinearForceVector &force_vector = physical->get_linear_forces();

  #if 0
  // tally the local xforms
  for (i = 0; i < local_force_vec_size; ++i) {
    force_node = force_vector[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_linear_matrices.push_back(physical_np.get_mat(force_node));
  }
  #else
  // tally the local xforms
  for (LinearForceVector::const_iterator fi = force_vector.begin(); 
      fi != force_vector.end(); 
      ++fi) {
    force_node = (*fi)->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_linear_matrices.push_back(physical_np.get_mat(force_node));
  }
  #endif

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
  int i;

  ForceNode *force_node;

  // prepare the vector
  _precomputed_angular_matrices.clear();
  _precomputed_angular_matrices.reserve(global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical_node);

  // tally the global xforms
  for (i = 0; i < global_force_vec_size; ++i) {
    force_node = forces[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_angular_matrices.push_back(physical_np.get_mat(force_node));
  }

  const AngularForceVector &force_vector =
    physical->get_angular_forces();

  // tally the local xforms
  for (i = 0; i < local_force_vec_size; ++i) {
    force_node = force_vector[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_angular_matrices.push_back(physical_np.get_mat(force_node));
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
  out<<"BaseIntegrator";
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
