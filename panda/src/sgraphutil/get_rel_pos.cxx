// Filename: get_rel_pos.cxx
// Created by:  drose (18Feb99)
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
  x = rel_mat.get_row3(0);
  y = rel_mat.get_row3(1);
  z = rel_mat.get_row3(2);

  // Normalize these axes to eliminate scale.
  x = normalize(x);
  y = normalize(y);
  z = normalize(z);

  // Now build a new matrix which just represents these axes.
  mat.set(x[0], x[1], x[2], 0.0,
    	  y[0], y[1], y[2], 0.0,
    	  z[0], z[1], z[2], 0.0,
    	  0.0, 0.0, 0.0, 1.0);
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
    return LVecBase3f(1.0, 1.0, 1.0);
  }

  // Extract the axes from the matrix.
  const LMatrix4f &rel_mat = tt->get_matrix();
  LVector3f x, y, z;
  x = rel_mat.get_row3(0);
  y = rel_mat.get_row3(1);
  z = rel_mat.get_row3(2);

  // Now return the lengths of these axes as the scale.

  return LVecBase3f(length(x), length(y), length(z));
}


