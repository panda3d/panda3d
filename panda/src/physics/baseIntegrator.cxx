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
                           const pvector< PT(LinearForce) > &forces) {
  // make sure the physical's in the scene graph, somewhere.
  PhysicalNode *physical_node = physical->get_physical_node();
  nassertv(physical_node != NULL);

  // by global forces, we mean forces not contained in the physical
  int global_force_vec_size = forces.size();

  // by local forces, we mean members of the physical's force set.
  int local_force_vec_size = physical->get_linear_forces().size();
  int i;

  ForceNode *force_node;

  // prepare the vector
  _precomputed_linear_matrices.clear();
  _precomputed_linear_matrices.reserve(global_force_vec_size + local_force_vec_size);

  NodePath physical_np(physical_node);

  // tally the global xforms
  for (i = 0; i < global_force_vec_size; i++) {
    force_node = forces[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_linear_matrices.push_back(physical_np.get_mat(force_node));
  }

  const pvector< PT(LinearForce) > &force_vector =
    physical->get_linear_forces();

  // tally the local xforms
  for (i = 0; i < local_force_vec_size; i++) {
    force_node = force_vector[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_linear_matrices.push_back(physical_np.get_mat(force_node));
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
                            const pvector< PT(AngularForce) > &forces) {
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
  for (i = 0; i < global_force_vec_size; i++) {
    force_node = forces[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_angular_matrices.push_back(physical_np.get_mat(force_node));
  }

  const pvector< PT(AngularForce) > &force_vector =
    physical->get_angular_forces();

  // tally the local xforms
  for (i = 0; i < local_force_vec_size; i++) {
    force_node = force_vector[i]->get_force_node();
    nassertv(force_node != (ForceNode *) NULL);

    NodePath force_np(force_node);
    _precomputed_angular_matrices.push_back(physical_np.get_mat(force_node));
  }
}
