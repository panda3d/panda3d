// Filename: get_rel_pos.cxx
// Created by:  drose (18Feb99)
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

#include "get_rel_pos.h"

#include <transformTransition.h>
#include <nodeTransitionWrapper.h>
#include <wrt.h>

////////////////////////////////////////////////////////////////////
//     Function: get_rot_mat
//  Description: Returns a rotation-only matrix that corresponds to
//               the position in space of the node's origin, relative
//               to another node (such as render).
////////////////////////////////////////////////////////////////////
void
get_rel_rot_mat(const Node *node, const Node *relative_to,
                LMatrix4f &mat, TypeHandle graph_type) {
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(node, relative_to, ntw, graph_type);
  const TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    mat = LMatrix4f::ident_mat();
    return;
  }

  // Extract the axes from the matrix.
  const LMatrix4f &rel_mat = tt->get_matrix();
  LVector3f x, y, z;
  rel_mat.get_row3(x,0);
  rel_mat.get_row3(y,1);
  rel_mat.get_row3(z,2);

  // Normalize these axes to eliminate scale.
  x.normalize();
  y.normalize();
  z.normalize();

  // Now build a new matrix which just represents these axes.
  mat.set(x[0], x[1], x[2], 0.0f,
          y[0], y[1], y[2], 0.0f,
          z[0], z[1], z[2], 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f);
}


////////////////////////////////////////////////////////////////////
//     Function: get_scale
//  Description: Returns the relative scale between the indicated node
//               and the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f
get_rel_scale(const Node *node, const Node *relative_to,
              TypeHandle graph_type) {
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(node, relative_to, ntw, graph_type);
  const TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    return LVecBase3f(1.0f, 1.0f, 1.0f);
  }

  // Extract the axes from the matrix.
  const LMatrix4f &rel_mat = tt->get_matrix();
  LVector3f x, y, z;
  rel_mat.get_row3(x,0);
  rel_mat.get_row3(y,1);
  rel_mat.get_row3(z,2);

  // Now return the lengths of these axes as the scale.

  return LVecBase3f(length(x), length(y), length(z));
}


